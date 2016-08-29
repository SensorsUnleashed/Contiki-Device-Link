/*
 * proto1.c
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#include "dev/uart.h"
#include "coap_proxy.h"
#include "lib/mmem.h"
#include "lib/memb.h"
#include "process.h"
#include "lib/crc16.h"
#include "lib/list.h"
#include "contiki.h"
#include <stdio.h>
#include <string.h>

/* Messages are always initiated from the us (the host).
*/
PROCESS(coap_proxy_server, "Coap uart proxy");

//event_data_ready event is posted when ever a message has been fully decoded.
process_event_t event_data_ready;

#define END     0xC0
#define ESC     0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

#define ACK     0x06
#define NAK     0x15

static unsigned char msgcount = 0;

//The rx_buf is the raw uart buffer - implemented as a circular buffer.
#define RX_BUFLEN	50
static char rx_buf[RX_BUFLEN];
static char* wrt_ptr = &rx_buf[0];
static char* rd_ptr = &rx_buf[0];
static char* rx_end = &rx_buf[RX_BUFLEN-1];
static char* rx_start = &rx_buf[0];

typedef struct  {
	unsigned char seqno;
	enum req_cmd cmd;
	struct mmem payloadbuf;
}rx_msg;
rx_msg rx_reply;

#define MAX_RESOURCES	20
LIST(proxy_resource_list);
MEMB(proxy_resource_mem, rx_msg, MAX_RESOURCES);

unsigned int frameandsend(const unsigned char *s, unsigned int len);

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
	len = msglen - 2;

	//Store the payload in dynamic memory
	if(mmem_alloc(&rx_reply.payloadbuf, len) == 0){
		return -1;	//unable to allocate memory
	}
	char* rs_wrtptr = (char*)MMEM_PTR(&rx_reply.payloadbuf);
	//rx_reply.payload = (int)rs_wrtptr;	//Store the location in memory where we're going to put the payload
	for(int i=0; i<len; i++){
		*rs_wrtptr++ = *tempptr++;
	}

	return 0;
}

int proxy_rx_callback(unsigned char c) {
	static char last_c = 0;
	if(c == END){	//Is it the last byte in a frame
		if(decodemsg() == 0){
			//ACK the message
			//frameandsend(ACK, 1);
			process_post(&coap_proxy_server, event_data_ready, 0);
		}
		else{
			//NAK the message
			// frameandsend(NAK, 1);
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

static char j;
static char resources_availble = 0;

PROCESS_THREAD(coap_proxy_server, ev, data)
{
	PROCESS_BEGIN();

	mmem_init();	//Initialized by contiki
	memb_init(&proxy_resource_mem);

	char* payload;

	/* List container all the resources received through the uart */
	list_init(proxy_resource_list);

	/* allocate the required event */
	event_data_ready = process_alloc_event();

	/* Request list of resources from the device attached to the uart */
	req_resource(resource_count, 0, 0);
	PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);

	payload = (char*) MMEM_PTR(&rx_reply.payloadbuf);

	resources_availble = *((char*)payload);

	char msg[50];
	int n = sprintf(msg, "resource_count=%d", resources_availble);
	req_resource(debugstring, &msg, n+1);
	PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);

	for(j=0; j< resources_availble; j++){

		req_resource(resource_url, &j, 1);
		PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);

		rx_msg* newmsg = memb_alloc(&proxy_resource_mem);
		if(newmsg != NULL){
			memcpy(newmsg, &rx_reply, sizeof(rx_msg));
			list_add(proxy_resource_list, newmsg);
		}
	}

	//Send back debug about what has been stored
	rx_msg* s;
	for(s = list_head(proxy_resource_list);
			s != NULL;
			s = list_item_next(s)) {
		payload = (char*) MMEM_PTR(&s->payloadbuf);
		req_resource(debugstring, payload, s->payloadbuf.size);
	}

//	req_resource(debugstring, &resources[1].url, sizeof(resources[1].url));

	while(1) {
		PROCESS_WAIT_EVENT();
		if(ev == event_data_ready){
			//We have received a new package, ready to be processed.
		}
	}

	PROCESS_END();
}


