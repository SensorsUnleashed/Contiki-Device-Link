/*******************************************************************************
 * Copyright (c) 2017, Ole Nissen.
 *  All rights reserved. 
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met: 
 *  1. Redistributions of source code must retain the above copyright 
 *  notice, this list of conditions and the following disclaimer. 
 *  2. Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer in the documentation and/or other materials provided
 *  with the distribution. 
 *  3. The name of the author may not be used to endorse or promote
 *  products derived from this software without specific prior
 *  written permission.  
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Sensors Unleashed project
 *******************************************************************************/
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

void uarthandler_init(void (* callback)(buffer_t* msg)){
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

