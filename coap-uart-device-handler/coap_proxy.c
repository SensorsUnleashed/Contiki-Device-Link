/*
 * proto1.c
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

//#include "board.h"
#include "coap_proxy.h"
#include "coap_proxy_protocolhandler.h"

#ifdef NATIVE
#include "coap_proxy_test.h"
#else
#include "dev/uart.h"
#endif

#include "lib/memb.h"
#include "process.h"
#include "lib/list.h"
#include "contiki.h"
#include <string.h>
#include "rest-engine.h"
#include "uarthandler.h"

#define DEBUG 0
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


#define LASTVALSIZE	15
//This is the type we use, so that we can find the resource on the sensor
struct proxy_resource{
	struct proxy_resource *next;	/* for LIST, points to next resource defined */
	struct resourceconf conf;
	resource_t* resourceptr;		/* Pointer to the resource COAP knows about */
	uint8_t* lastval;				/* We use LASTVALSIZE bytes to store the last non-decoded reading */
	uint8_t hasEvent;				/* Non handled active event */
};

typedef struct proxy_resource proxy_resource_t;

/* Messages are always initiated from the us (the host).
 */
PROCESS(coap_proxy_server, "Coap uart proxy");

//event_data_ready event is posted when ever a message has been fully decoded.
process_event_t tick_event;	//ticks the message handling statemachine

//When a message has been decoded, this is where it goes
static char payloadbuffer[1024];	//This is also the device's strings database
static rx_msg rx_reply;				//Holds the current parsed reply. Payload is still msgpacked here.

struct pt pt_worker;
int msgid = 1;			//Used to identify that a request/reply pair
uint8_t txbuf[100];		//Used when transmitting commands to the device

static char j;
static char resources_availble = 0;

#define MAX_RESOURCES	20
LIST(proxy_resource_list);
MEMB(coap_resources, resource_t, MAX_RESOURCES);
MEMB(proxy_resources, proxy_resource_t, MAX_RESOURCES);

#include "dev/leds.h"
#include "sys/pt.h"

void res_proxy_get_handler_tester(){
	proxy_resource_t *resource = NULL;
	for(resource = (proxy_resource_t *)list_head(proxy_resource_list);
			resource; resource = resource->next) {

		uint8_t buf[20];
		uint32_t len;
		if(cp_decodeReadings(resource->lastval, &buf[0], &len) == 0){
			PRINTF("ID %d: Lastval = %s\n", resource->conf.id, (char*)buf);
		}
	}
}

static void res_proxy_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_GREEN);
	proxy_resource_t *resource = NULL;
	const char *url = NULL;
	const char *str = NULL;
	int url_len, res_url_len;

	url_len = REST.get_url(request, &url);
	for(resource = (proxy_resource_t *)list_head(proxy_resource_list);
			resource; resource = resource->next) {

		/* if the web service handles that kind of requests and urls matches */
		res_url_len = strlen(resource->resourceptr->url);
		if((url_len == res_url_len
				|| (url_len > res_url_len
						&& (resource->resourceptr->flags & HAS_SUB_RESOURCES)
						&& url[res_url_len] == '/'))
				&& strncmp(resource->resourceptr->url, url, res_url_len) == 0) {

			//Send the lastmessage to who ever is asking
			uint32_t len;
			unsigned int accept = -1;
			REST.get_header_accept(request, &accept);
			REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
			REST.set_header_max_age(response, 180);	//3 minutes

				//If the payload is empty, respond with the measurement
				len = REST.get_query(request, &str);
				if(len > 0){
					//There is a query, find out if its a valid one.
					if(strcmp(str, "spec") == 0){
						REST.set_response_payload(response, resource->conf.spec, strlen(resource->conf.spec));
					}
					else if(strcmp(str, "eventstatus") == 0){
						len = sprintf((char*)buffer, "%u", resource->conf.eventsActive);
						REST.set_response_payload(response, buffer, len);
					}
//					else if(strcmp(str, "higheventvalue") == 0){
//					}
					else{
						//Data error
						const char* dataerr = "Command not recognized....";
						REST.set_response_payload(response, dataerr, strlen((const char *)dataerr));
					}
				}
				else
				if(cp_decodeReadings(resource->lastval, buffer, &len) == 0){

					REST.set_response_payload(response, buffer, len /*strlen((char *)buffer)*/);
				}
				else{
					//Data error
					const char* dataerr = "No data available from the sensor....";
					REST.set_response_payload(response, dataerr, strlen((const char *)dataerr));
				}
			break;
		}
	}
}

static void res_proxy_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	//	leds_toggle(LEDS_ORANGE);
}
static void res_proxy_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_YELLOW);

	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "Du har kaldt res_proxy_put_handler");
	REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}
static void res_proxy_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_RED);

	snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "Du har kaldt res_proxy_delete_handler");
	REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}
static void res_proxy_trigger_handler(void){

	proxy_resource_t* resource;
	for(resource = (proxy_resource_t *)list_head(proxy_resource_list);
			resource; resource = resource->next) {
		if(resource->hasEvent == 1){
			/* Notify the registered observers which will trigger the res_get_handler to create the response. */
			REST.notify_subscribers(resource->resourceptr);
			resource->hasEvent = 0;
		}
	}
}

PT_THREAD(initResources(struct pt *pt)){
	PT_BEGIN(pt);

	//Request the number of resources available
	frameandsend(&txbuf[0], cp_encodemessage(msgid, resource_count, 0, 0, &txbuf[0]));
	PT_WAIT_UNTIL(pt, rx_reply.seqno == msgid);	//Wait until we have the right message
	resources_availble = *((char*) rx_reply.payload);

	for(j=0; j< resources_availble; j++){
		frameandsend(&txbuf[0], cp_encodemessage(++msgid, resource_config, &j, 1, &txbuf[0]));
		PT_WAIT_UNTIL(pt, rx_reply.seqno == msgid);

		//create the proxy resource
		proxy_resource_t* p = (proxy_resource_t*)memb_alloc(&proxy_resources);
		if(p != 0){
			//Place the strings in the beginning of the payloadbuffer.
			//It will override what we received with the relevant strings, but as
			//long as we read ahead, it doesn't matter.
			cp_decoderesource_conf(&p->conf, rx_reply.payload, (char*)rx_reply.payload);

			//We need to make sure that we dont allocate the mememory and override the strings.
			rx_reply.payload = p->conf.attr + strlen(p->conf.attr) + 1;

			//Allocate LASTVALSIZE in the buffer, for values received.
			p->lastval = rx_reply.payload;
			rx_reply.payload += LASTVALSIZE;
		}
		//Create the resource for the coap engine
		resource_t* r = (resource_t*)memb_alloc(&coap_resources);
		if(r != 0){

			//This is a way to easier to find the id of the sensor/actuator later on
			p->resourceptr = r;
			p->conf.id = j;
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

			list_add(proxy_resource_list, p);

			//Finally activate the resource with the rest coap
			rest_activate_resource(r, (char*)r->url);
			PRINTF("ID %d: Activated resource: %s Attributes: %s - Spec: %s, Unit: %s\n", j, r->url, r->attributes, p->conf.spec, p->conf.unit );
		}
	}
	PT_END(pt);
}

void handleSensorMessages(){

	/* When a new reading comes in from the device,
	 * we chose to store the raw message, non-decoded.
	 * The reason is, that we dont want to waste energy
	 * converting, if noone needs the value anyway.
	 *
	 * */

	if(rx_reply.cmd == resource_value_update || rx_reply.cmd == resource_event){
		uint8_t id;
		uint32_t len = 0;
		if(cp_decodeID(rx_reply.payload, &id, &len) == 0){	//Its always the id first
			proxy_resource_t* resource;
			for(resource = (proxy_resource_t *)list_head(proxy_resource_list);
					resource; resource = resource->next) {
				if(resource->conf.id == id){
					if(len <= LASTVALSIZE){
						memcpy(resource->lastval, rx_reply.payload + len, rx_reply.len - len);	//Copy the value to to the lastvalue loc

						if(rx_reply.cmd == resource_event){
							/* Call the event_handler for this application-specific event. */
							resource->hasEvent = 1;
							resource->resourceptr->trigger();
						}
					}else{
						PRINTF("Couldn't store the lastval\n");
					}
				}
			}
		}
	}
}
//Callback; used when the uart has parsed a message
void proxy_tick_event(buffer_t* msg){
	process_post(&coap_proxy_server, tick_event, msg);
}

void proxy_init(void){

	cp_uarthandler_init(proxy_tick_event);

	PT_INIT(&pt_worker);

	process_start(&coap_proxy_server, 0);

	rx_reply.payload = &payloadbuffer[0];
}

/*
 * Main statemachine
 * All new messages recieved will come from here.
 */
PROCESS_THREAD(coap_proxy_server, ev, data)
{
	PROCESS_BEGIN();
	char state;
	initResources(&pt_worker);	//start the init process

	//Init
	do{
		PROCESS_WAIT_EVENT_UNTIL(ev == tick_event);
		state = PT_WAITING;
		buffer_t* lastmsg = data;
		if(cp_decodemessage(lastmsg->buffer, lastmsg->wrt_ptr - &lastmsg->buffer[0], &rx_reply) == 0){
			state = initResources(&pt_worker);
		}
	}while(state != PT_ENDED);

	//Request that the attached device updates us with its current value(s)
	frameandsend(&txbuf[0], cp_encodemessage(++msgid, resource_req_updateAll, 0, 0, &txbuf[0]));

	//Normal operation
	while(1){
		PROCESS_WAIT_EVENT_UNTIL(ev == tick_event);
		buffer_t* lastmsg = data;
		if(cp_decodemessage(lastmsg->buffer, lastmsg->wrt_ptr - &lastmsg->buffer[0], &rx_reply) == 0){
			handleSensorMessages();	//Just print a message
		}
	}

	PROCESS_END();
}
