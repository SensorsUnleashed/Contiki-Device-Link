/*
 * uart_handler.c
 *
 *  Created on: 08/09/2016
 *      Author: omn
 */
#include "uarthandler.h"
#ifdef NATIVE
#include "coap_proxy_test.h"
#else
#include "dev/uart.h"
#endif

#define END     0xC0
#define ESC     0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

#define ACK     0x06
#define NAK     0x15

//The rx_buf is the raw uart buffer - implemented as a circular buffer.
#define RX_BUFFERS	5
buffer_t rxbuf[RX_BUFFERS];	//Allocate 5 buffers for receiving.
int activebuffer = 0;

void (* msgrdy_callback)(buffer_t* msg);

void switchbuffer(){
	activebuffer++;
	activebuffer = activebuffer >= RX_BUFFERS ? 0 : activebuffer;

	//Reset the write pointer
	rxbuf[activebuffer].wrt_ptr = &rxbuf[activebuffer].buffer[0];
}
int proxy_rx_callback(unsigned char c) {
	static char last_c = 0;
	if(c == END){	//Is it the last byte in a frame
		msgrdy_callback(&rxbuf[activebuffer]);
		switchbuffer();
	}
	else if(c == ESC_ESC && last_c == ESC){
		*rxbuf[activebuffer].wrt_ptr++ = last_c;
	}
	else if(c == ESC_END && last_c == ESC){
		*rxbuf[activebuffer].wrt_ptr++ = END;
	}
	else{
		*rxbuf[activebuffer].wrt_ptr++ = c;
	}
	last_c = c;
	return 1;
}

void cp_uarthandler_init(void (* callback)(buffer_t* msg)){
	uart_set_input(0, proxy_rx_callback);
	switchbuffer();	//Prepare buffer to be written
	msgrdy_callback =  callback;
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

