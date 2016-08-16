/*
 * proto1.c
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#include "process.h"
#include "protocol.h"
#include "lib/crc16.h"

/* Messages are always initiated from the us (the host).
*/

#define END     0xC0
#define ESC     0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

#define ACK     0x06
#define NAK     0x15

static unsigned char msgcount = 0;
static char msg_ready = -1;			//Msg id received

#define RX_BUFLEN	50
static char rx_buf[RX_BUFLEN];
static char* wrt_ptr = &rx_buf[0];
static char* rd_ptr = &rx_buf[0];
static char* rx_end = &rx_buf[RX_BUFLEN-1];
static char* rx_start = &rx_buf[0];

#define STR_BUFLEN	100
static char resource_strings[STR_BUFLEN];
static char* rs_wrtptr = &resource_strings[0];

typedef struct  {
	unsigned char seqno;
	enum req_cmd cmd;
	void* payload;
	unsigned char len;
}rx_msg;

rx_msg rx_reply;

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
	rx_reply.payload = (int)rs_wrtptr;
	for(int i=0; i<rx_reply.len; i++){
		*rs_wrtptr++ = *tempptr++;
	}

	return 0;
}


int uart_rx_callback(unsigned char c) {
	static char last_c = 0;
	if(c == END){	//Is it the last byte in a frame
		if(decodemsg() == 0){
			//ACK the message
			uart_write_byte(0, ACK);
		}
		else{
			//NAK the message
			uart_write_byte(0, NAK);
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

void protocol_init(void){
	uart_set_input(0, uart_rx_callback);
}


void req_resource(enum req_cmd cmd){

	unsigned char msg_id;
	unsigned char msg[25];
	unsigned char index = 0;
	unsigned short crc;
	msg_id = 0x01;
	msg[index++] = msgcount;	//count
	msg[index++] = msg_id;		//cmd
	msg[index] = index;			//len
	index++;
	crc = crc16_data((unsigned char*)&msg, index, 0);
	msg[index++] = crc & 0xff;
	msg[index++] = crc >> 8;
	frameandsend((unsigned char*)&msg,index);
}

int req_received(enum req_cmd cmd){

	switch(cmd){
	case resource_count:
		if(msg_ready == 1){
			return msg_ready;
		}
		break;
	case resource_url:
		break;
	default:
		break;
	}
	return PROCESS_NONE;
}


