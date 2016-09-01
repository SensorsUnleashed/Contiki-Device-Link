/*
 * proto1.c
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#include "dev/uart.h"
#include "board.h"
#include "coap_proxy.h"

#include "lib/memb.h"
#include "process.h"
#include "lib/crc16.h"
#include "lib/list.h"
#include "contiki.h"
#include <stdio.h>
#include <string.h>
#include "rest-engine.h"

/* Messages are always initiated from the us (the host).
*/
PROCESS(coap_proxy_server, "Coap uart proxy");

//event_data_ready event is posted when ever a message has been fully decoded.
process_event_t event_data_ready;

//Upcounter, for all messages sent from here
static unsigned char msgcount = 0;

//The rx_buf is the raw uart buffer - implemented as a circular buffer.
#define RX_BUFLEN	50
static char rx_buf[RX_BUFLEN];
static char* wrt_ptr = &rx_buf[0];
static char* rd_ptr = &rx_buf[0];
static char* rx_end = &rx_buf[RX_BUFLEN-1];
static char* rx_start = &rx_buf[0];

//This is an array that is used for the resource info from the uart
static char payloadbuffer[1024];
static char* plbuf_wrt = &payloadbuffer[0];

//When a message has been decoded, this is where it goes
static rx_msg rx_reply;

static char j;
static char resources_availble = 0;

#define MAX_RESOURCES	20
LIST(proxy_resource_list);
MEMB(proxy_resource_mem, rx_msg, MAX_RESOURCES);
MEMB(proxy_resources, resource_t, MAX_RESOURCES);

#include "dev/leds.h"
static void res_proxy_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_GREEN);
}
static void res_proxy_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_ORANGE);
}
static void res_proxy_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_YELLOW);
}
static void res_proxy_delete_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	leds_toggle(LEDS_RED);
}

int decodemsg(){

	char temp[50];
	char* tempptr = &temp[0];
	char* tempend = &temp[49];
	char len = 0;

	//Lets start by copying the bytes into a straight temp buffer (This simplifies decoding)
	while(rd_ptr != wrt_ptr && tempptr <= tempend){
		*(tempptr + len++) = *rd_ptr++;
		if(rd_ptr > rx_end){
			rd_ptr = rx_start;
		}
	}

	//Verify the integrity of the data
	char msglen = *(tempptr + len - 3);
	unsigned short crc = *((unsigned short*)(tempptr + len - 2));
	unsigned short crct = crc16_data((unsigned char*)tempptr, len - 2, 0);
	if(crct != crc) return -1;	//UPS - message not ok

	//Finally store the message in a container, for later use
	//Resource strings will be stored in a seperate
	rx_reply.seqno = *tempptr++;
	rx_reply.cmd = *tempptr++;
	rx_reply.len = msglen - 2;
	rx_reply.payload = (char*)plbuf_wrt;
	for(int i=0; i<rx_reply.len; i++){
		*plbuf_wrt++ = *tempptr++;
	}

	return 0;
}

int proxy_rx_callback(unsigned char c) {
	static char last_c = 0;
	if(c == END){	//Is it the last byte in a frame
		if(decodemsg() == 0){
			process_post(&coap_proxy_server, event_data_ready, 0);
		}
	}
	else if(c == ESC_ESC && last_c == ESC){
		*wrt_ptr++ = last_c;
	}
	else if(c == ESC_END && last_c == ESC){
		*wrt_ptr++ = END;
	}
	else{
		*wrt_ptr++ = c;
	}
	last_c = c;
	wrt_ptr = wrt_ptr > rx_end ? rx_start : wrt_ptr;
	return 1;
}

unsigned int frameandsend(const unsigned char *s, unsigned int len){
	unsigned int i = 0;
	char c;

	while(len--){
		c = *s++;
		if(c == END){
			uart_write_byte(0,ESC);
			uart_write_byte(0,ESC_END);
		}
		else if(c == ESC){
			uart_write_byte(0,ESC);
			uart_write_byte(0,ESC_ESC);
		}
		else{
			uart_write_byte(0,c);
		}
	}
	uart_write_byte(0, END);
	return i;
}

/*
 * Paramters:
 * 	cmd: The command, used by the client to identify the message type
 * 	payload: What to send
 * 	len: The length of the payload.
 * 		0 = No payload
 * */
void req_resource(enum req_cmd cmd, void* payload, int len){

	char* tmp = payload;
	unsigned char msg_id;
	unsigned char msg[50];
	unsigned char index = 0;
	unsigned short crc;
	msg_id = (char)cmd;
	msg[index++] = msgcount++;	//count
	msg[index++] = msg_id;		//cmd
	for(int i=0; i<len; i++){
		msg[index++] = *tmp++;
	}
	msg[index] = index;			//len
	index++;
	crc = crc16_data((unsigned char*)&msg, index, 0);
	msg[index++] = crc & 0xff;
	msg[index++] = crc >> 8;
	frameandsend((unsigned char*)&msg,index);
}

void proxy_init(void){

	uart_set_input(0, proxy_rx_callback);
	process_start(&coap_proxy_server, 0);
}

//This is only for intermidiate storage - to avoid warnings about non initialized variables
static resource_t temp_resource;

PROCESS_THREAD(coap_proxy_server, ev, data)
{
	PROCESS_BEGIN();
	memb_init(&proxy_resource_mem);
	memb_init(&proxy_resources);

	/* List container all the resources received through the uart */
	list_init(proxy_resource_list);

	/* allocate the required event */
	event_data_ready = process_alloc_event();

	/* Request list of resources from the device attached to the uart */
	req_resource(resource_count, 0, 0);
	PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
	resources_availble = *((char*) rx_reply.payload);

	char msg[50];
	sprintf(msg, "resource_count=%d", resources_availble);
	req_resource(debugstring, &msg, strlen((char*)&msg[0]));
	PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);

	for(j=0; j< resources_availble; j++){

		req_resource(resource_url, &j, 1);
		PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
		temp_resource.url = (char*)rx_reply.payload;

		req_resource(resource_attributes, &j, 1);
		PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
		temp_resource.attributes = (char*)rx_reply.payload;

		req_resource(resource_flags, &j, 1);
		PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
		temp_resource.flags = (rest_resource_flags_t)(char*)rx_reply.payload;

		//Create the resource
		resource_t* r = (resource_t*)memb_alloc(&proxy_resources);
		if(r != 0){
			memcpy(r, &temp_resource, sizeof(resource_t));

			r->get_handler = res_proxy_get_handler;
			r->post_handler = res_proxy_post_handler;
			r->put_handler = res_proxy_put_handler;
			r->delete_handler = res_proxy_delete_handler;
			rest_activate_resource(r, (char*)r->url);

			req_resource(debugstring, (char*)temp_resource.url, strlen((char*)temp_resource.url) + 1);	//Remember the \0
			PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
			req_resource(debugstring, (char*)temp_resource.attributes, strlen((char*)temp_resource.attributes) + 1);	//Remember the \0
			PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
		}
	}

	//Send back debug about what has been stored
	rx_msg* s;
	for(s = list_head(proxy_resource_list);
			s != NULL;
			s = list_item_next(s)) {
		req_resource(debugstring, s->payload, s->len);
	}

	while(1) {
		PROCESS_WAIT_EVENT();
		if(ev == event_data_ready){
			//We have received a new package, ready to be processed.
		}
	}

	PROCESS_END();
}

