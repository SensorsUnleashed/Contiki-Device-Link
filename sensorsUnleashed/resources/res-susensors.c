/*
 * res-systeminfo.c
 *
 *  Created on: 07/10/2016
 *      Author: omn
 */
#include <string.h>
#include <stdlib.h>     /* strtol */
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include "er-coap.h"
#include "board.h"
#include "lib/susensors.h"
#include "../../sensorsUnleashed/pairing.h"
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
	struct susensors_sensor *sensor = (struct susensors_sensor *)susensors_find(url, len);
	if(sensor != NULL){
		cmp_object_t obj;
		len = REST.get_query(request, &str);
		if(len > 0){
			if(strncmp(str, "AboveEventAt", len) == 0){
				len = sensor->suconfig(sensor, SUSENSORS_AEVENT_GET, &obj) == 0;
				REST.set_header_max_age(response, 3600);
			}
			else if(strncmp(str, "BelowEventAt", len) == 0){
				len = sensor->suconfig(sensor, SUSENSORS_BEVENT_GET, &obj) == 0;
				REST.set_header_max_age(response, 3600);
			}
			else if(strncmp(str, "ChangeEventAt", len) == 0){
				len = sensor->suconfig(sensor, SUSENSORS_CEVENT_GET, &obj) == 0;
				REST.set_header_max_age(response, 3600);
			}
			else if(strncmp(str, "RangeMin", len) == 0){
				len = sensor->suconfig(sensor, SUSENSORS_RANGEMIN_GET, &obj) == 0;
			}
			else if(strncmp(str, "RangeMax", len) == 0){
				len = sensor->suconfig(sensor, SUSENSORS_RANGEMAX_GET, &obj) == 0;
			}
			else if(strncmp(str, "getEventState", len) == 0){
				len = sensor->suconfig(sensor, SUSENSORS_EVENTSTATE_GET, &obj) == 0;
			}
			else if(strncmp(str, "getEventSetup", len) == 0){
				len = sensor->suconfig(sensor, SUSENSORS_EVENTSETUP_GET, buffer);
				REST.set_response_payload(response, buffer, len);
				return;
			}
			else{
				len = 0;
			}
		}
		else{	//Send the actual value
			len = sensor->status(sensor, ActualValue, &obj) == 0;
			REST.set_header_max_age(response, 30);
		}

		if(len){
			len = cp_encodeObject(buffer, &obj);
			REST.set_response_status(response, REST.status.OK);
			REST.set_response_payload(response, buffer, len);
		}
	}
}

static uint8_t large_update_store[200] = { 0 };

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
	coap_packet_t *const coap_req = (coap_packet_t *)request;
	struct susensors_sensor *sensor = (struct susensors_sensor *)susensors_find(url, len);
	if(sensor != NULL){
		len = REST.get_query(request, &str);
		if(len > 0){
			/* Issue a command. The command is one of the enum suactions values*/
			const char *commandstr = NULL;
			char *pEnd;
			if(strncmp(str, "aboveEvent", len) == 0){
				len = REST.get_request_payload(request, &payload);
				sensor->eventhandler(sensor, SUSENSORS_ABOVE_EVENT_SET, len, (uint8_t*)payload);
			}
			else if(strncmp(str, "belowEvent", len) == 0){
				len = REST.get_request_payload(request, &payload);
				sensor->eventhandler(sensor, SUSENSORS_BELOW_EVENT_SET, len, (uint8_t*)payload);
			}
			else if(strncmp(str, "changeEvent", len) == 0){
				len = REST.get_request_payload(request, &payload);
				sensor->eventhandler(sensor, SUSENSORS_CHANGE_EVENT_SET, len, (uint8_t*)payload);
			}
			else if(REST.get_query_variable(request, "setCommand", &commandstr) > 0 && commandstr != NULL) {
				if(sensor->value(sensor, strtol(commandstr, &pEnd, 10), 0) == 0){	//For now, no payload - might be necessary in the furture
					cmp_object_t obj;
					len = sensor->status(sensor, ActualValue, &obj) == 0;
					len = cp_encodeObject(buffer, &obj);
					REST.set_response_payload(response, buffer, len);
					REST.set_response_status(response, REST.status.OK);
				}
				else{
					REST.set_response_status(response, REST.status.BAD_REQUEST);
				}
			}
			else if(strncmp(str, "suconfig", len) == 0){
				len = REST.get_request_payload(request, &payload);
				if(len <= 0){
					REST.set_response_status(response, REST.status.BAD_REQUEST);
					const char *error_msg = "no payload in query";
					REST.set_response_payload(response, error_msg, strlen(error_msg));
					return;
				}

				//TODO: Generate human readable error messages, if parsing fails (see pairing)
				if(sensor->suconfig(sensor, SUSENSORS_EVENTSETUP_SET, (void*)payload) == 0){
					REST.set_response_status(response, REST.status.CHANGED);
				}
				else{
					REST.set_response_status(response, REST.status.BAD_REQUEST);
				}
			}
			else if(strncmp(str, "join", len) == 0){
				if((len = REST.get_request_payload(request, (const uint8_t **)&payload))) {
					if(coap_req->block1_num * coap_req->block1_size + len <= sizeof(large_update_store)) {

						if(pairing_assembleMessage(payload, len, coap_req->block1_num) == 0){
							REST.set_response_status(response, REST.status.CHANGED);
							coap_set_header_block1(response, coap_req->block1_num, 0, coap_req->block1_size);
						}

						if(coap_req->block1_more == 0){
							//We're finished receiving the payload, now parse it.
							int res = pairing_handle((void*)sensor, susensor);	//TODO: This is not good

							switch(res){
							case 0:
								//All is good
								REST.set_response_status(response, REST.status.CREATED);
								break;
							case 1:
								REST.set_response_status(response, REST.status.BAD_REQUEST);
								const char *error_msg1 = "IPAdress wrong";
								REST.set_response_payload(response, error_msg1, strlen(error_msg1));
								break;
							case 2:
								REST.set_response_status(response, REST.status.BAD_REQUEST);
								const char *error_msg2 = "Destination URI wrong";
								REST.set_response_payload(response, error_msg2, strlen(error_msg2));
								break;
							case 3:
								REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
								break;
							case 4:
								REST.set_response_status(response, REST.status.BAD_REQUEST);
								const char *error_msg3 = "Source URI wrong";
								REST.set_response_payload(response, error_msg3, strlen(error_msg3));
								break;
							case 5:
								REST.set_response_status(response, REST.status.NOT_MODIFIED);
								break;
							}
						}
					}
					else {
						REST.set_response_status(response, REST.status.REQUEST_ENTITY_TOO_LARGE);
						return;
					}
				}
			}/* join */
		}/* len > 0 */
	}/* sensor != 0 */
}

#define CONFIG_SENSORS	1
//Return 0 if the sensor was added as a coap resource
//Return 1 if the sensor does not contain the necassery coap config
//Return 2 if we can't allocate any more sensors
int res_susensor_activate(const struct susensors_sensor* sensor){

	const struct extras* extra = sensor->data;
	struct resourceconf* config;

	if(extra->type != 1){
		return 1;
	}
	config = (struct resourceconf*)extra->config;
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
