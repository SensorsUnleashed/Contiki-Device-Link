/*
 * res-uartsensors.c
 *
 *  Created on: 16/10/2016
 *      Author: omn
 */

#include "uartsensors.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include <string.h>

MEMB(coap_resources, resource_t, MAX_RESOURCES);

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
				len = sprintf((char*)buffer + len, "ID=%d\n", resource->conf.id);
				len = sprintf((char*)buffer + len, "%s", resource->conf.spec);

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
				len = sprintf((char*)buffer + len, "AE=");
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
			if(cp_decodeReadings(resource->lastval, buffer, &len) == 0){
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
 *
 *  eventstatus:	Events enabled/disabled (0/1)
 * 	eventsetup:		AE=AboveEvent, BE=BelowEvent, CE=ChangeEvent *
 *
 * */
static void res_proxy_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_YELLOW);

	const char *url = NULL;
	const char *str = NULL;
	int url_len;

	url_len = REST.get_url(request, &url);
	uartsensors_device_t *resource = uartsensors_find((char*)url, url_len);

	if(resource != NULL){
		//setEventState	=> 0 = disable, 1 = enable
		//setAElevel = upper level in same format as readings
		//setBElevel = lower level in same format as readings
		//setCElevel = change event span
		int len = REST.get_query(request, &str);
		REST.set_response_payload(response, (uint8_t *)str, len);
		//REST.set_response_status(response, REST.status.CHANGED);
		//		const char* msg = "So far so good....";
		//		REST.set_response_payload(response, msg, strlen((const char *)msg));
		//		REST.set_response_payload(response, resource->conf.spec, strlen(resource->conf.spec));
	}
	else{
		//REST.set_response_status(response, REST.status.BAD_REQUEST);
	}
}

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

