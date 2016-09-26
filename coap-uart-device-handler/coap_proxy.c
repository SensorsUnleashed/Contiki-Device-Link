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
#include <stdio.h>
#include <string.h>
#include "rest-engine.h"
#include "uarthandler.h"
#include "cmp.h"

typedef unsigned long size_t;

//This is the type we use, so that we can find the resource on the sensor
struct proxy_resource{
	struct proxy_resource *next;	/* for LIST, points to next resource defined */
	struct resourceconf conf;
	resource_t* resourceptr;		/* Pointer to the resource COAP knows about */
};

typedef struct proxy_resource proxy_resource_t;

/* Messages are always initiated from the us (the host).
 */
PROCESS(coap_proxy_server, "Coap uart proxy");

//event_data_ready event is posted when ever a message has been fully decoded.
process_event_t tick_event;

//When a message has been decoded, this is where it goes
static char payloadbuffer[1024];
static rx_msg rx_reply;

static char j;
static char resources_availble = 0;

#define MAX_RESOURCES	20
LIST(proxy_resource_list);
MEMB(coap_resources, resource_t, MAX_RESOURCES);
MEMB(proxy_resources, proxy_resource_t, MAX_RESOURCES);

#include "dev/leds.h"
#include "sys/pt.h"

static void res_proxy_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_GREEN);
	//int len = 0;

	proxy_resource_t *resource = NULL;
	const char *url = NULL;
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

			//Request a uartmessage, and wait for it here

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

//This is only for intermidiate storage - to avoid warnings about non initialized variables
static resource_t temp_resource;

struct pt pt_worker;
int msgid = 1;
uint8_t txbuf[256];

char rx_msg_payload[100];
buffer_t* rx_buf = 0;


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
			//Its msgpack encoded
			//Place the strings in the beginning of the data.
			//It will override what we received with the relevant strings, but that dont matter.
			cp_decoderesource_conf(&p->conf, rx_reply.payload, (char*)&txbuf[0]);

			//We need to make sure that we dont override the strings.
			rx_reply.payload = p->conf.attr + strlen(p->conf.attr) + 1;

		}
		//Create the resource for the coap engine
		resource_t* r = (resource_t*)memb_alloc(&coap_resources);
		if(r != 0){
			memcpy(r, &temp_resource, sizeof(resource_t));

			if(r->flags & METHOD_GET){
				r->get_handler = res_proxy_get_handler;
			}
			if(r->flags & METHOD_POST){
				r->post_handler = res_proxy_post_handler;
			}
			if(r->flags & METHOD_PUT){
				r->put_handler = res_proxy_put_handler;
			}
			if(r->flags & METHOD_DELETE){
				r->delete_handler = res_proxy_delete_handler;
			}

			//This is a way to easier to find the id of the sensor/actuator later on
			p->resourceptr = r;
			r->url = p->conf.type;
			r->attributes = p->conf.attr;
			r->flags = p->conf.flags;
			list_add(proxy_resource_list, p);

			//Finally activate the resource with the rest coap
			rest_activate_resource(r, (char*)r->url);
			printf("ID %d: Activated resource: %s Attributes: %s - Spec: %s, Unit: %s\n", j, r->url, r->attributes, p->conf.spec, p->conf.unit );
		}
	}
	PT_END(pt);
}

void handleSensorMessages(){

	if(rx_reply.cmd == resource_value_update){

		char str[20];
		char id[2];
		int len = 0;
		if(cp_decodeReadings(rx_reply.payload, &id[0], &len) == 0){	//Its always the id first
			printf("ID: %s = ", (char*)&id);
			rx_reply.payload += len;
		}

		if(cp_decodeReadings(rx_reply.payload, &str[0], &len) == 0){
			printf("%s\n", (char*)&str);
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

	temp_resource.get_handler = NULL;
	temp_resource.put_handler = NULL;
	temp_resource.post_handler = NULL;
	temp_resource.delete_handler = NULL;

//	temp_proxy_resource.id = 0;

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
