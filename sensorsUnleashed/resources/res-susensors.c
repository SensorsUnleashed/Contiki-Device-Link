/*
 * res-systeminfo.c
 *
 *  Created on: 07/10/2016
 *      Author: omn
 */
#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"

#ifndef NATIVE
#include "board.h"
#endif
#include "lib/sensors.h"
#include "../../apps/uartsensors/uart_protocolhandler.h"

#define MAX_RESOURCES	20
MEMB(coap_resources, resource_t, MAX_RESOURCES);

//#define DEBUG 1
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

static void res_susensor_gethandler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_susensor_puthandler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple actuator example. Toggles the red led */
RESOURCE(res_sysinfo,
		"title=\"Radio info\";rt=\"Info\"",
		res_susensor_gethandler,
		NULL,
		NULL,
		NULL);

#define MSG_PACKED	0
#define PLAIN_TEXT	1

static void
res_susensor_gethandler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	const char *url = NULL;
	const char *str = NULL;
	unsigned int ct = -1;
	REST.get_header_content_type(request, &ct);

	if(ct != REST.type.APPLICATION_OCTET_STREAM) {
		REST.set_response_status(response, REST.status.BAD_REQUEST);
		const char *error_msg = "msgPacked, octet-stream only";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		return;
	}

	int len = REST.get_url(request, &url);
	const struct sensors_sensor *sensor = sensors_find(url, len);
	if(sensor != NULL){
		cmp_object_t obj;
		len = REST.get_query(request, &str);
		if(len > 0){
			if(strncmp(str, "AboveEventAt", len) == 0){
				len = sensor->value(AboveEventValue, &obj);
				REST.set_header_max_age(response, 3600);
			}
			else if(strncmp(str, "BelowEventAt", len) == 0){
				len = sensor->value(BelowEventValue, &obj);
				REST.set_header_max_age(response, 3600);
			}
			else if(strncmp(str, "ChangeEventAt", len) == 0){
				len = sensor->value(ChangeEventValue, &obj);
				REST.set_header_max_age(response, 3600);
			}
			else if(strncmp(str, "RangeMin", len) == 0){
				len = sensor->value(RangeMinValue, &obj);
			}
			else if(strncmp(str, "RangeMax", len) == 0){
				len = sensor->value(RangeMaxValue, &obj);
			}
			else if(strncmp(str, "getEventState", len) == 0){
				len = sensor->value(EventState, &obj);
			}
			else if(strncmp(str, "getEventSetup", len) == 0){
				uint8_t* bufptr = buffer;
				sensor->value(AboveEventValue, &obj);
				bufptr += cp_encodeObject(bufptr, &obj);
				sensor->value(BelowEventValue, &obj);
				bufptr += cp_encodeObject(bufptr, &obj);
				sensor->value(ChangeEventValue, &obj);
				bufptr += cp_encodeObject(bufptr, &obj);
				sensor->value(EventState, &obj);
				bufptr += cp_encodeObject(bufptr, &obj);
				REST.set_response_payload(response, buffer, bufptr - buffer);
				return;
			}
			else{
				len = 0;
			}
		}
		else{	//Send the actual value
			len = sensor->value(ActualValue, &obj);
			REST.set_header_max_age(response, sensor->configure(SENSORS_MAX_AGE, 0));
		}

		if(len){
			len = cp_encodeObject(buffer, &obj);
			REST.set_response_status(response, REST.status.OK);
			REST.set_response_payload(response, buffer, len);
		}
	}
}

static void
res_susensor_puthandler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	const char *url = NULL;
	const char *str = NULL;
	const uint8_t *payload = NULL;
	unsigned int ct = -1;
	REST.get_header_content_type(request, &ct);

	if(ct != REST.type.APPLICATION_OCTET_STREAM) {
		REST.set_response_status(response, REST.status.BAD_REQUEST);
		const char *error_msg = "msgPacked, octet-stream only";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		return;
	}

	int len = REST.get_url(request, &url);
	const struct sensors_sensor *sensor = sensors_find(url, len);
	if(sensor != NULL){
		len = REST.get_query(request, &str);
		if(len > 0){
			if(strncmp(str, "eventsetup", len) == 0){
				len = REST.get_request_payload(request, &payload);
				if(len <= 0){
					REST.set_response_status(response, REST.status.BAD_REQUEST);
					const char *error_msg = "no payload in query";
					REST.set_response_payload(response, error_msg, strlen(error_msg));
					return;
				}

				/* The eventsetup is send as a fixed list of cmp_objects:
				 * 1. AboveEventAt
				 * 2. BelowEventAt
				 * 3. ChangeEvent
				 * 4. eventsActive
				 * */
				uint32_t bufindex;
				len = 0;
				struct resourceconf* config;
				/* TODO: Consider moving the logic away from coap - could be we move to something else at some point */
				/* TODO: This cast should be dependendt on the xtra type field */
				config = (struct resourceconf*) sensor->extra->data;

				cmp_object_t newval;
				/* Read the AboveEventAt object */
				if(cp_decodeObject((uint8_t*)payload + len, &newval, &bufindex) == 0){
					len += bufindex;
					if(newval.type == config->AboveEventAt.type){
						config->AboveEventAt = newval;
					}
					else{
						REST.set_response_status(response, REST.status.BAD_REQUEST);
						return;
					}
				}
				else{
					REST.set_response_status(response, REST.status.BAD_REQUEST);
					return;
				}

				/* Read the BelowEventAt object */
				if(cp_decodeObject((uint8_t*)payload + len, &newval, &bufindex) == 0){
					len += bufindex;
					if(newval.type == config->BelowEventAt.type){
						config->BelowEventAt = newval;
					}
					else{
						REST.set_response_status(response, REST.status.BAD_REQUEST);
						return;
					}
				}
				else{
					REST.set_response_status(response, REST.status.BAD_REQUEST);
					return;
				}
				/* Read the ChangeEvent object */
				if(cp_decodeObject((uint8_t*)payload + len, &newval, &bufindex) == 0){
					len += bufindex;
					if(newval.type == config->ChangeEvent.type){
						config->ChangeEvent = newval;
					}
					else{
						REST.set_response_status(response, REST.status.BAD_REQUEST);
						return;
					}
				}
				else{
					REST.set_response_status(response, REST.status.BAD_REQUEST);
					return;
				}
				/* Read the eventsActive object */
				if(cp_decodeObject((uint8_t*)payload + len, &newval, &bufindex) == 0){
					len += bufindex;
					if(newval.type == CMP_TYPE_UINT8){
						config->eventsActive = newval.as.u8;
					}
					else{
						REST.set_response_status(response, REST.status.BAD_REQUEST);
						return;
					}
				}
				else{
					REST.set_response_status(response, REST.status.BAD_REQUEST);
					return;
				}

				REST.set_response_status(response, REST.status.CHANGED);
			}
		}
	}
}

#define CONFIG_SENSORS	1
//Return 0 if the sensor was added as a coap resource
//Return 1 if the sensor does not contain the necassery coap config
//Return 2 if we can't allocate any more sensors
int res_susensor_activate(const struct sensors_sensor* sensor){

	const struct extras* extra = sensor->extra;
	struct resourceconf* config;

	if(extra->type != 1){
		return 1;
	}
	config = (struct resourceconf*)extra->data;
	//Create the resource for the coap engine
	resource_t* r = (resource_t*)memb_alloc(&coap_resources);
	if(r == 0)
		return 2;


	r->url = config->type;
	r->attributes = config->attr;
	r->flags = config->flags;

	if(r->flags & METHOD_GET){
		r->get_handler = res_susensor_gethandler;
	}else r->get_handler = NULL;
	if(r->flags & METHOD_POST){
		//r->post_handler = res_proxy_post_handler;
	}else r->post_handler = NULL;
	if(r->flags & METHOD_PUT){
		r->put_handler = res_susensor_puthandler;
	}else r->put_handler = NULL;
	if(r->flags & METHOD_DELETE){
		//r->delete_handler = res_proxy_delete_handler;
	}else r->delete_handler = NULL;

	//Finally activate the resource with the rest coap
	rest_activate_resource(r, (char*)r->url);
	PRINTF("Activated resource: %s Attributes: %s - Spec: %s, Unit: %s\n", r->url, r->attributes, config->spec, config->unit );

	return 0;
}
