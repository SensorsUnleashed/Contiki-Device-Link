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
#include "res-uartsensor.h"
#include "storage.h"

MEMB(coap_resources, resource_t, MAX_RESOURCES);
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

joinpair_t pairs[5];
int pairscount = 0;

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
 *
 * A put request has always its query as a string, and its payload msgpack encoded.
 * Content type APPLICATION_OCTET_STREAM "
 *
 *  eventstatus:	Events enabled/disabled
 *  		Payload: 1 nipple
 * 				enabled  = 1
 * 			 	disabled = 0
 * 	eventsetup:
 * 			Payload: enum up_parameter, val
 *
 * 	pair:	//Can be split of more than 1 message (block1 transfer)
 * 			Payload: uip_ip6addr_t destip, String uri
 * 			destip stored as an 8 words array of 16bit values
 * */

static uint8_t large_update_store[200] = { 0 };
static int32_t large_update_size = 0;

	static void res_proxy_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_YELLOW);

	const char *url = NULL;
	const char *query = NULL;
	const uint8_t *payload = NULL;
	int url_len;

	unsigned int ct = -1;
	REST.get_header_content_type(request, &ct);

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
		const char *error_msg = "Unknown resource";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		return;
	}

	coap_packet_t *const coap_req = (coap_packet_t *)request;
	int len = REST.get_query(request, &query);
	if(len > 0){
		if(strncmp(query, "eventsetup", len) == 0){	//Emit the raw value, which can be cheaper for the receiver to use
			len = REST.get_request_payload(request, &payload);
			if(len <= 0){
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				const char *error_msg = "no payload in query";
				REST.set_response_payload(response, error_msg, strlen(error_msg));
				return;
			}

			uint32_t bufindex;
			do{
				enum up_parameter param;
				cmp_object_t newval;
				if(cp_decodeU8((uint8_t*)payload, (uint8_t*)&param, &bufindex) == 0){
					if(cp_decodeObject((uint8_t*)payload+len, &newval, &bufindex) == 0){
						if(uartsensors_setEventVal(resource, param, newval) != 0){
							//Something wrong with the encoding
							REST.set_response_status(response, REST.status.CHANGED);
						}
					}
					else{
						//Something wrong with the encoding
						REST.set_response_status(response, REST.status.BAD_REQUEST);
					}
				}
				else{
					//Something wrong with the encoding
					REST.set_response_status(response, REST.status.BAD_REQUEST);
				}
			}while(bufindex < len);
		}
		if(strncmp(query, "join", len) == 0){
			if((len = REST.get_request_payload(request, (const uint8_t **)&payload))) {
				if(coap_req->block1_num * coap_req->block1_size + len <= sizeof(large_update_store)) {
					memcpy(large_update_store + coap_req->block1_num * coap_req->block1_size, payload, len);
					large_update_size = coap_req->block1_num * coap_req->block1_size + len;

					REST.set_response_status(response, REST.status.CHANGED);
					coap_set_header_block1(response, coap_req->block1_num, 0, coap_req->block1_size);

					if(coap_req->block1_more == 0){
						//We're finished receiving the payload, now parse it.

						uip_ip6addr_t server_ipaddr;
						uint32_t bufindex = 0;
						uint32_t stringlen = 100;
						char stringbuf[100];
						payload = &large_update_store[0];

						if(cp_decodeU16Array((uint8_t*) payload + bufindex, (uint16_t*)&server_ipaddr, &bufindex) != 0){
							REST.set_response_status(response, REST.status.BAD_REQUEST);
							const char *error_msg = "IPAdress wrong";
							REST.set_response_payload(response, error_msg, strlen(error_msg));
							return;
						}

						if(cp_decode_string((uint8_t*) payload + bufindex, &stringbuf[0], &stringlen, &bufindex) != 0){
							REST.set_response_status(response, REST.status.BAD_REQUEST);
							const char *error_msg = "URI wrong";
							REST.set_response_payload(response, error_msg, strlen(error_msg));
							return;
						}

						stringlen++;	//We need the /0 also

						//Store the pairing pointer
						if(mmem_alloc(&pairs[pairscount].dsturl, stringlen) == 0){
							REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
						}
						else{
							memcpy(&pairs[pairscount].destip, &server_ipaddr, sizeof(uip_ip6addr_t));
							memcpy(MMEM_PTR(&pairs[pairscount].dsturl), stringbuf, stringlen);
							pairs[pairscount].devicetype = uartsensor;
							pairs[pairscount].deviceptr = resource;

							//Append the srcurl to the message, this way, when we later
							//retrieve the message from flash, we will know
							cp_encodeString((uint8_t*) payload + bufindex, resource->conf.attr, strlen(resource->conf.attr), &bufindex);
							store_SensorPair((uint8_t*) payload, bufindex);

							REST.set_response_status(response, REST.status.CREATED);
							pairscount++;
						}
					}
				}
				else {
					REST.set_response_status(response, REST.status.REQUEST_ENTITY_TOO_LARGE);
					//REST.set_response_payload(response, buffer, snprintf((char *)buffer, 200, "%uB max.", sizeof(large_update_store)));
					return;
				}
			}
			else{
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				const char *error_msg = "no payload in query";
				REST.set_response_payload(response, error_msg, strlen(error_msg));
				return;
			}
		}
	}
}

static void res_proxy_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_RED);

	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "Du har kaldt res_proxy_delete_handler");
	REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}

/* For now we can only pair with 1 at a time*/
joinpair_t* getUartSensorPair(uartsensors_device_t* p){
	joinpair_t* pair = 0;
	for(int i=0; i<pairscount; i++){
		if(pairs[i].deviceptr == (void*)p){
			pair = &pairs[i];
			break;
		}
	}
	return pair;
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

		//To be deleted
		//		joinpair_t* pair = &pairs[pairscount];
		//		pair->destip.u16[0] = 33277;
		//		pair->destip.u16[1] = 43581;
		//		pair->destip.u16[2] = 19195;
		//		pair->destip.u16[3] = 44791;
		//		pair->destip.u16[4] = 4610;
		//		pair->destip.u16[5] = 75;
		//		pair->destip.u16[6] = 44805;
		//		pair->destip.u16[7] = 15235;
		//		pair->deviceptr = p;
		//
		//		const char* url = "SU/ledtoggle";
		//		mmem_alloc(pair->url, strlen(url)+1);
		//		memcpy(pair->url, &url, strlen(url)+1);
		//		pairscount++;
	}
}

