/*
 * coap_proxy_test.c
 *
 *  Created on: 18/09/2016
 *      Author: omn
 */
#include "contiki.h"
#include "coap_proxy_protocolhandler.h"
#include <stdio.h>
#include <string.h>
#include "rest-engine.h"

#define END     0xC0
#define ESC     0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

#define ACK     0x06
#define NAK     0x15

#define RESOURCE_COUNT	2
char* rs001_group = "button";
char* rs001_type  = "actuator";
char* rs001_att = "title=\"Green LED\" ;rt=\"Control\"";
char* rs001_spec = "ON=1, OFF=0";
char* rs001_unit = "";	//No unit

char* rs002_group = "timer";
char* rs002_type  = "counter";
char* rs002_att = "title=\"Orange LED\";rt=\"Control\"";
char* rs002_spec = "1 sec";
char* rs002_unit = "Sec";

static void tx_value(int id);
static struct resourceconf rs[RESOURCE_COUNT];

//When a message has been decoded, this is where it goes
static rx_msg rx_req;

char inputbuffer[1024];
char* rd_ptr = &inputbuffer[0];
char* wrt_ptr = &inputbuffer[0];

static struct etimer et;

process_event_t event_data_ready;
PROCESS(coap_proxy_test, "Coap uart proxy test");

static int (* input_handler)(unsigned char c);
void uart_set_input(uint8_t uart, int (* input)(unsigned char c))
{
	input_handler = input;

	//init the resources, that we would like to emulate
	rs[0] = (struct resourceconf){
		.id = 0,
				.resolution = 100,
				.hysteresis = 10,
				.flags = METHOD_GET | METHOD_POST,
				.max_pollinterval = 2000,
				.version = 0001,
				.unit = rs001_unit,
				.spec = rs001_spec,
				.group = rs001_group,
				.type = rs001_type,
				.attr = rs001_att,
	};
	rs[1] = (struct resourceconf){
		.id = 1,
				.resolution = 100,
				.hysteresis = 1,
				.flags = METHOD_GET | METHOD_PUT,
				.max_pollinterval = -1,
				.version = 0001,
				.unit = rs002_unit,
				.spec = rs002_spec,
				.group = rs002_group,
				.type = rs002_type,
				.attr = rs002_att,
	};

	rx_req.payload = &inputbuffer[0];

	//Start the process from here
	process_start(&coap_proxy_test, 0);
}



unsigned int frametx(const unsigned char *s, unsigned int len){
	unsigned int i = 0;
	char c;
	while(len--){
		c = *s++;
		if(c == END){
			input_handler(ESC);
			input_handler(ESC_END);
		}
		else if(c == ESC){
			input_handler(ESC);
			input_handler(ESC_ESC);
		}
		else{
			input_handler(c);
		}
	}
	input_handler(END);
	return i;
}

//This emulates a byte is received from the proxy
void uart_emulate_rx(uint8_t c){
	//Unstuff the message before storing it in the input buffer
	static char last_c = 0;
	if(c == END){	//Is it the last byte in a frame
		process_post(&coap_proxy_test, event_data_ready, 0);
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
}
void uart_write_byte(uint8_t uart, uint8_t c)
{
	uart_emulate_rx(c);
}

PROCESS_THREAD(coap_proxy_test, ev, data)
{
	PROCESS_BEGIN();
	printf("Coap proxy test started\n");

	/* allocate the required event */
	event_data_ready = process_alloc_event();

	/* Send a new message every second */
	etimer_set(&et, CLOCK_SECOND);

	while(1) {
		PROCESS_WAIT_EVENT();
		if(ev == event_data_ready){

			if(cp_decodemessage(rd_ptr, wrt_ptr - rd_ptr, &rx_req) == 0){
				if(rx_req.cmd == resource_count){
					char c = RESOURCE_COUNT;
					frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(rx_req.seqno, resource_count, &c, 1, (uint8_t*)&inputbuffer[0]));
				}
				else if(rx_req.cmd == resource_group){
					int id = *((char*)rx_req.payload);
					if(id < RESOURCE_COUNT){
						frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(rx_req.seqno, resource_group, rs[id].group, strlen(rs[id].group)+1, (uint8_t*)&inputbuffer[0]));
					}
					else
						printf("Wrong Resource ID (resource_group)");
				}
				else if(rx_req.cmd == resource_type){
					int id = *((char*)rx_req.payload);
					if(id < RESOURCE_COUNT){
						frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(rx_req.seqno, resource_type, rs[id].type, strlen(rs[id].type)+1, (uint8_t*)&inputbuffer[0]));
					}
					else
						printf("Wrong Resource ID (resource_type)");
				}
				else if(rx_req.cmd == resource_attributes){
					int id = *((char*)rx_req.payload);
					if(id < RESOURCE_COUNT){
						frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(rx_req.seqno, resource_attributes, rs[id].attr, strlen(rs[id].attr)+1, (uint8_t*)&inputbuffer[0]));
					}
					else
						printf("Wrong Resource ID (resource_attributes)");
				}
				else if(rx_req.cmd == resource_spec){
					int id = *((char*)rx_req.payload);
					if(id < RESOURCE_COUNT){
						frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(rx_req.seqno, resource_spec, rs[id].spec, strlen(rs[id].spec)+1, (uint8_t*)&inputbuffer[0]));
					}
					else
						printf("Wrong Resource ID (resource_spec)");
				}
				else if(rx_req.cmd == resource_flags){
					int id = *((char*)rx_req.payload);
					if(id < RESOURCE_COUNT){
						frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(rx_req.seqno, resource_flags, (char*)&rs[id].flags, 1, (uint8_t*)&inputbuffer[0]));
					}
					else
						printf("Wrong Resource ID (resource_flags)");
				}
				else if(rx_req.cmd == resource_unit){
					int id = *((char*)rx_req.payload);
					if(id < RESOURCE_COUNT){
						frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(rx_req.seqno, resource_unit, rs[id].unit, strlen(rs[id].unit)+1, (uint8_t*)&inputbuffer[0]));
					}
					else
						printf("Wrong Resource ID (resource_unit)");
				}
				else if(rx_req.cmd == resource_config){
					int id = *((char*)rx_req.payload);
					if(id < RESOURCE_COUNT){
						int len = cp_encoderesource_conf(&rs[id], (uint8_t*)&inputbuffer[0]);
						frametx((uint8_t*)&inputbuffer[len], cp_encodemessage(rx_req.seqno, resource_config, &inputbuffer[0], len, (uint8_t*)&inputbuffer[len]));
					}
					else
						printf("Wrong Resource ID (resource_config)");
				}
				else if(rx_req.cmd == resource_req_update){
					for(int i=0; i<RESOURCE_COUNT; i++){
						wrt_ptr = &inputbuffer[0];
						tx_value(i);
					}
				}
				//Reset buffer to be ready to receive yet another message
				wrt_ptr = &inputbuffer[0];
			}
		}
#include "cmp.h"

		else if(ev == PROCESS_EVENT_TIMER){
			etimer_reset(&et);

			char pl[20];
			char* plptr = &pl[0];
			cmp_object_t obj;
			obj.type = CMP_TYPE_POSITIVE_FIXNUM;
			obj.as.u8 = 1;
			plptr += cp_encodereading((uint8_t*)plptr, &obj);
			obj.type = CMP_TYPE_UINT64;
			obj.as.u64 = clock_seconds();
			plptr += cp_encodereading((uint8_t*)plptr, &obj);
			frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(rx_req.seqno, resource_value_update, (char*)&pl[0], (char)((unsigned long)plptr - (unsigned long)&pl[0]), (uint8_t*)&inputbuffer[0]));
		}
	}

	PROCESS_END();
}

void tx_value(int id){
	char pl[20];
	char* plptr = &pl[0];
	cmp_object_t obj;

	//first pack the ID
	obj.type = CMP_TYPE_POSITIVE_FIXNUM;
	obj.as.u8 = id;
	plptr += cp_encodereading((uint8_t*)plptr, &obj);

	if(id == 0){
		//Next the payload
		obj.type = CMP_TYPE_POSITIVE_FIXNUM;
		obj.as.u8 = 1;
		plptr += cp_encodereading((uint8_t*)plptr, &obj);
	}
	else if(id == 1){
		//Next the payload
		obj.type = CMP_TYPE_UINT64;
		obj.as.u64 = clock_seconds();
		plptr += cp_encodereading((uint8_t*)plptr, &obj);
	}

	frametx((uint8_t*)&inputbuffer[0], cp_encodemessage(255, resource_value_update, (char*)&pl[0], (char)((unsigned long)plptr - (unsigned long)&pl[0]), (uint8_t*)&inputbuffer[0]));
}
