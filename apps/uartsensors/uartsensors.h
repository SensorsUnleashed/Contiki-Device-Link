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
 * protocol.h
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_
#define COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_

#include "contiki.h"
#include "uart_protocolhandler.h"
#ifndef NULL
#define NULL 0
#endif

#define MAX_RESOURCES	20

//This is the type we use, so that we can find the resource on the sensor
struct proxy_resource{
	struct proxy_resource *next;	/* for LIST, points to next resource defined */
	struct resourceconf conf;
	uint8_t* lastval;				/* We use LASTVALSIZE bytes to store the last non-decoded reading */
	uint8_t vallen;
};
typedef struct proxy_resource uartsensors_device_t;

uartsensors_device_t *uartsensors_find(const char *name, int len);
void uartsensors_getEventVal(uartsensors_device_t* this, enum up_parameter parameter);
int uartsensors_setEventVal(uartsensors_device_t* this, enum up_parameter parameter, cmp_object_t val);
void uartsensors_reset(uartsensors_device_t* this);
void uartsensors_init(void);

extern process_event_t uartsensors_event;

PROCESS_NAME(uartsensors_server);

#endif /* COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_ */
