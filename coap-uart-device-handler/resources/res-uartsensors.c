/*
 * res-uartsensors.c
 *
 *  Created on: 16/10/2016
 *      Author: omn
 */

#include "uartsensors.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ip/uiplib.h"
#include <string.h>
#include "er-coap-constants.h"
#include "er-coap-observe-client.h"
#include "net/rime/rime.h"
#include <stdlib.h>

MEMB(coap_resources, resource_t, MAX_RESOURCES);
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

//We need to have a way to keep track of which sensor a notification belongs to
enum datatype_e{
	uartsensor,
	sensor
};
struct joinpair_s{
	char* url;
	enum datatype_e devicetype;
	void* deviceptr;
	uip_ip6addr_t destip;

};
typedef struct joinpair_s joinpair_t;

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

static void toggleled(int id){
	switch(id){
	case 1:
		leds_set(LEDS_GREEN);
		break;
	case 2:
		leds_set(LEDS_RED);
		break;
	case 3:
		leds_set(LEDS_YELLOW);
		break;
	case 4:
#ifndef NATIVE
		leds_set(LEDS_ORANGE);
#endif
		break;
	}
}

static void
notification_callback(coap_observee_t *obs, void *notification,
		coap_notification_flag_t flag){
	int len = 0;
	const uint8_t *payload = NULL;
	if(notification) {
		len = coap_get_payload(notification, &payload);
		if(len > 0){
			int id = strtol((char*)payload, 0, 0);
			toggleled(id);
		}
	}
}

/*
 * Called when someone asks for the last value available
 * It is possible to make a query for:
 * 	info: 			Send back the spec string
 * 	eventstatus:	Events enabled/disabled (0/1)
 * 	eventsetup:		AE=AboveEvent, BE=BelowEvent, CE=ChangeEvent *
 * */
static void res_proxy_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	const char *url = NULL;
	const char *str = NULL;
	int url_len;

	url_len = REST.get_url(request, &url);
	uartsensors_device_t *resource = uartsensors_find((char*)url, url_len);
	if(resource != NULL){
		uint32_t len = 0;
		len = REST.get_query(request, &str);
		if(len > 0){
			if(strncmp(str, "raw", len) == 0){	//Emit the raw value, which can be cheaper for the receiver to use
				memcpy(buffer, resource->lastval, resource->vallen);
				REST.set_response_payload(response, buffer, resource->vallen);
				REST.set_header_max_age(response, 0);
			}
			else if(strncmp(str, "info", len) == 0){
				len = 0;
				len += sprintf((char*)buffer + len, "ID=%d\n", resource->conf.id);
				len += sprintf((char*)buffer + len, "%s", resource->conf.spec);

				REST.set_response_payload(response, buffer, len);
				REST.set_header_max_age(response, 3600);
			}
			else if(strncmp(str, "eventstatus", len) == 0){
				len = sprintf((char*)buffer, "%u", resource->conf.eventsActive);
				REST.set_response_payload(response, buffer, len);
				REST.set_header_max_age(response, 600);
			}
			else if(strncmp(str, "eventsetup", len) == 0){
				len = 0;
				len += sprintf((char*)buffer + len, "AE=");
				cp_cmp_to_string(&resource->conf.AboveEventAt, buffer+len, &len);
				len += sprintf((char*)buffer + len, "\nBE=");
				cp_cmp_to_string(&resource->conf.BelowEventAt, buffer+len, &len);
				len += sprintf((char*)buffer + len, "\nCE=");
				cp_cmp_to_string(&resource->conf.ChangeEvent, buffer+len, &len);
				REST.set_response_payload(response, buffer, len);
				REST.set_header_max_age(response, 600);
			}
			else{
				//Data error
				const char* dataerr = "Query not recognized....";
				REST.set_response_payload(response, dataerr, strlen(dataerr));
			}
		}
		else{ //If the payload is empty, respond with the measurement
			len = 0;
			if(cp_convMsgPackToString(resource->lastval, buffer, &len) == 0){
				REST.set_response_payload(response, buffer, len);
				REST.set_header_max_age(response, 0);	//Value can change anytime
			}
			else{
				//Data error
				const char* dataerr = "No data available from the sensor....";
				REST.set_response_payload(response, dataerr, strlen((const char *)dataerr));
			}
		}
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
	}
}

static void res_proxy_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	//	leds_toggle(LEDS_ORANGE);
}

/* Called when a devices should be updated
 *  eventstatus:	Events enabled/disabled (0/1)
 * 	eventsetup:		AE=AboveEvent, BE=BelowEvent, CE=ChangeEvent *
 * */
static void res_proxy_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_YELLOW);

	const char *url = NULL;
	const char *query = NULL;
	const uint8_t *payload = NULL;
	int url_len;

	unsigned int ct = -1;
	REST.get_header_content_type(request, &ct);

	//if(!REST.get_header_content_type(request, REST.type.APPLICATION_OCTET_STREAM)) {
	if(ct != REST.type.APPLICATION_OCTET_STREAM) {
		REST.set_response_status(response, REST.status.BAD_REQUEST);
		const char *error_msg = "msgPacked, octet-stream only";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		return;
	}

	url_len = REST.get_url(request, &url);
	uartsensors_device_t *resource = uartsensors_find((char*)url, url_len);
	if(resource == NULL){
		REST.set_response_status(response, REST.status.BAD_REQUEST);
		return;
	}

	int len = REST.get_query(request, &query);
	if(len > 0){
		if(strncmp(query, "eventsetup", len) == 0){	//Emit the raw value, which can be cheaper for the receiver to use
			len = REST.get_request_payload(request, &payload);
			if(len <= 0){
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				return;
			}

			char key[20];
			char* keyptr = &key[0];
			int val;
			int templen = 0, buflen = 0;
			cmp_object_t newval;
			for(int i=0; i<4; i++){
				sscanf((char*)payload + buflen, "%20[^=]=%d %n", &key[0], &val, &templen);
				buflen += templen;

				if(strcmp(keyptr, "AE") == 0){
					newval = resource->conf.AboveEventAt;
					newval.as.s32 = val;
					uartsensors_setEventVal(resource, AboveEventValue, newval);
					REST.set_response_status(response, REST.status.CHANGED);
				}
				else if(strcmp(keyptr, "BE") == 0){
					newval = resource->conf.BelowEventAt;
					newval.as.s32 = val;
					uartsensors_setEventVal(resource, BelowEventValue, newval);
					REST.set_response_status(response, REST.status.CHANGED);
				}
				else if(strcmp(keyptr, "CE") == 0){
					newval = resource->conf.ChangeEvent;
					newval.as.s32 = val;
					uartsensors_setEventVal(resource, ChangeEventValue, newval);
					REST.set_response_status(response, REST.status.CHANGED);
				}

				if(buflen >= len){
					break;
				}
			}
		}
		if(strncmp(query, "join", len) == 0){
			len = REST.get_request_payload(request, &payload);
			if(len <= 0){
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				return;
			}
//			static uip_ipaddr_t server_ipaddr[1];
//			uip_ip6addr(ipaddr, 0xfd81, 0x3daa, 0xfb4a, 0xf7ae, 0x0212, 0x4b00, 0x5af, 0x8323);

			/* The payload will look like this:
			 *
			 * A IP6 address is made up of a prefix and a suffix - the suffix is the only unique part.
			 * IP6 address: fd81:3daa:fb4a:f7ae:0212:4b00:5af:8323
			 * Prefix: fd81:3daa:fb4a:f7ae
			 * Suffix: 0212:4b00:5af:8323
			 *
			 * :: means that all number from start to here, is 0
			 *
			 * ::0212:4b00:5af:8323
			 * button/actuator
			 * Line 1 is the ip of the respource to observe and will be prefixed with what we already know
			 * Note: The IP address will need to be prefixed with :: to immitate all zeroes
			 * Line 2 is the url to observe
			 * */
			const char* ip6 = strtok((char*)payload, "\n");
			if(ip6 == 0){
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				return;
			}
			char* uri = strtok(NULL, "\n");
			if(uri == 0){
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				return;
			}

			uip_ip6addr_t destaddr;
			uip_ip6addr_t hostaddr;
			if( uiplib_ip6addrconv(ip6, &destaddr)){
				//destaddr will miss the prefix part - add it now
				//uip_gethostaddr(&hostaddr);
				destaddr.u16[0] = hostaddr.u16[0];
				destaddr.u16[1] = hostaddr.u16[1];
				destaddr.u16[2] = hostaddr.u16[2];
				destaddr.u16[3] = hostaddr.u16[3];
//TODO: Place the address in the global memory
				coap_obs_request_registration(&destaddr, REMOTE_PORT, uri, notification_callback, NULL);

				//uiplib_ip6addrconv
				//uip_debug_ipaddr_print
			}

//			if(uiplib_ip6addrconv(ip6, &ipaddr)){
//				joinpair_t pair;
//				pair.url = resource->conf.type;
//				pair.deviceptr = resource;
//				pair.devicetype = uartsensor;
//				pair.destip = uiplib_ip6addrconv(ip6, &ipaddr);
//			}
		}
//		REST.set_response_payload(response, (uint8_t *)query, len);
//		return;

	}
}

//uip_ipaddr_t parseIP6addr(uint8_t* addr, int len){
//	uip_ipaddr_t ip;
//	sscanf((char*)addr, "%20[^=]=%d %n", &ip.u16[0], &val, &templen);
//}

static void res_proxy_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_RED);

	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "Du har kaldt res_proxy_delete_handler");
	REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}

static void res_proxy_trigger_handler(void){

	//	uartsensors_device_t* resource;
	//	for(resource = (uartsensors_device_t *)list_head(proxy_resource_list);
	//			resource; resource = resource->next) {
	//		if(resource->hasEvent == 1){
	//			/* Notify the registered observers which will trigger the res_get_handler to create the response. */
	//			REST.notify_subscribers(resource->resourceptr);
	//			resource->hasEvent = 0;
	//		}
	//	}
}

void res_uartsensors_event(uartsensors_device_t* p){
	resource_t *resource = NULL;
	for(resource = (resource_t *)list_head(rest_get_resources()); resource;
			resource = resource->next) {
		if(strcmp(resource->url, p->conf.type) == 0){
			/* Notify the registered observers which will trigger the res_get_handler to create the response. */
			REST.notify_subscribers(resource);
		}
	}
}

void res_uartsensors_activate(uartsensors_device_t* p){
	//Create the resource for the coap engine
	resource_t* r = (resource_t*)memb_alloc(&coap_resources);
	if(r != 0){
		r->url = p->conf.type;
		r->attributes = p->conf.attr;
		r->flags = p->conf.flags;

		if(r->flags & METHOD_GET){
			r->get_handler = res_proxy_get_handler;
			r->trigger = res_proxy_trigger_handler;
		}else r->get_handler = NULL;
		if(r->flags & METHOD_POST){
			r->post_handler = res_proxy_post_handler;
		}else r->post_handler = NULL;
		if(r->flags & METHOD_PUT){
			r->put_handler = res_proxy_put_handler;
		}else r->put_handler = NULL;
		if(r->flags & METHOD_DELETE){
			r->delete_handler = res_proxy_delete_handler;
		}else r->delete_handler = NULL;

		//Finally activate the resource with the rest coap
		rest_activate_resource(r, (char*)r->url);
		PRINTF("Activated resource: %s Attributes: %s - Spec: %s, Unit: %s\n", r->url, r->attributes, p->conf.spec, p->conf.unit );
	}
}

