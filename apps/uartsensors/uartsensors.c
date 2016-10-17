/*
 * proto1.c
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#include "uartsensors.h"
#include "uart_protocolhandler.h"
#include "uarthandler.h"

#ifdef NATIVE
#include "coap_proxy_test.h"
#else
#include "dev/uart.h"
#endif

#include "lib/memb.h"
#include "lib/list.h"
#include <string.h>

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

/* Messages are always initiated from the us (the host).
 */
PROCESS(uartsensors_server, "Coap uart proxy");

//event_data_ready event is posted when ever a message has been fully decoded.
process_event_t uart_event;	//ticks the message handling statemachine
process_event_t uartsensors_event;	//When a sensor emits an event

//When a message has been decoded, this is where it goes
static char payloadbuffer[1024];	//This is also the device's strings database
static rx_msg rx_reply;				//Holds the current parsed reply. Payload is still msgpacked here.

struct pt pt_worker;
int msgid = 1;			//Used to identify that a request/reply pair
uint8_t txbuf[100];		//Used when transmitting commands to the device

static char j;
static char resources_availble = 0;
LIST(proxy_resource_list);
MEMB(proxy_resources, uartsensors_device_t, MAX_RESOURCES);

uartsensors_device_t *uartsensors_find(const char *name, int len){
	uartsensors_device_t *resource = NULL;
	int res_len;

	for(resource = (uartsensors_device_t *)list_head(proxy_resource_list);
			resource; resource = resource->next) {

		/* if the web service handles that kind of requests and urls matches */
		res_len = strlen(resource->conf.type);
		if(len == res_len && strncmp(resource->conf.type, name, res_len) == 0) {
			return resource;
		}
	}
	return NULL;
}

/*
 * Returns the actual value of the actuator/sensor
 * */
void uartsensors_getVal(uartsensors_device_t* this, uint8_t type, cmp_object_t* val){

}

/*
 * Set the value in the actuator
 * */
void uartsensors_setVal(uartsensors_device_t* this, uint8_t type, cmp_object_t val){

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
		uartsensors_device_t* p = (uartsensors_device_t*)memb_alloc(&proxy_resources);
		if(p != 0){
			//Place the strings in the beginning of the payloadbuffer.
			//It will override what we received with the relevant strings, but as
			//long as we read ahead, it doesn't matter.
			cp_decoderesource_conf(&p->conf, rx_reply.payload, (char*)rx_reply.payload);

			//We need to make sure that we dont allocate the mememory and override the strings.
			rx_reply.payload = p->conf.attr + strlen(p->conf.attr) + 1;
			p->conf.id = j;
			//Allocate LASTVALSIZE in the buffer, for values received.
			p->lastval = rx_reply.payload;
			rx_reply.payload += LASTVALSIZE;

			list_add(proxy_resource_list, p);
			process_post(PROCESS_BROADCAST, uartsensors_event, (void*)p);
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
			uartsensors_device_t* resource;
			for(resource = (uartsensors_device_t *)list_head(proxy_resource_list);
					resource; resource = resource->next) {
				if(resource->conf.id == id){
					if(len <= LASTVALSIZE){
						resource->vallen = rx_reply.len - len;
						memcpy(resource->lastval, rx_reply.payload + len, rx_reply.len - len);	//Copy the value to to the lastvalue loc

						//If its an event, post it further on. Else, its just latently awaiting someone to request it
						if(rx_reply.cmd == resource_event){
							process_post(PROCESS_BROADCAST, uartsensors_event, (void*)resource);
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
void uart_event_callback(buffer_t* msg){
	process_post(&uartsensors_server, uart_event, msg);
}

void uartsensors_init(void){

	uarthandler_init(uart_event_callback);

	PT_INIT(&pt_worker);

	process_start(&uartsensors_server, 0);

	rx_reply.payload = &payloadbuffer[0];
}

/*
 * Main statemachine
 * All new messages recieved will come from here.
 */
PROCESS_THREAD(uartsensors_server, ev, data)
{
	PROCESS_BEGIN();
	char state;

	uartsensors_event = process_alloc_event();

	initResources(&pt_worker);	//start the init process

	//Init
	do{
		PROCESS_WAIT_EVENT_UNTIL(ev == uart_event);
		state = PT_WAITING;
		buffer_t* lastmsg = data;
		if(cp_decodemessage(lastmsg->buffer, lastmsg->wrt_ptr - &lastmsg->buffer[0], &rx_reply) == 0){
			state = initResources(&pt_worker);
		}
	}while(state != PT_ENDED);

	process_post(PROCESS_BROADCAST, uartsensors_event, NULL);
	//Request that the attached devices updates us with the current value(s)
	frameandsend(&txbuf[0], cp_encodemessage(++msgid, resource_req_updateAll, 0, 0, &txbuf[0]));

	//Normal operation
	while(1){
		PROCESS_WAIT_EVENT_UNTIL(ev == uart_event);
		buffer_t* lastmsg = data;
		if(cp_decodemessage(lastmsg->buffer, lastmsg->wrt_ptr - &lastmsg->buffer[0], &rx_reply) == 0){
			handleSensorMessages();
		}
	}

	PROCESS_END();
}
