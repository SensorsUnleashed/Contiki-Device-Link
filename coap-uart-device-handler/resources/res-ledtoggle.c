/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Example resource
 * \author
 *      Ole Nissen <ole@mnissen.dk>
 */

#include "contiki.h"



#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include <stdlib.h>
#include "er-coap-engine.h"

static coap_observee_t *obs;

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple actuator example. Toggles the red led */
RESOURCE(res_ledtoggle,
         "title=\"Toggle LEDs\";rt=\"Control\"",
		 NULL,
		 NULL,
		 res_put_handler,
         NULL);

static void toggleled(int id){
	switch(id){
	case 1:
		leds_set(LEDS_GREEN);
		break;
	case 2:
		leds_set(LEDS_RED);
		break;
	case 3:
		leds_set(LEDS_YELLOW);
		break;
	case 4:
#ifndef NATIVE
		leds_set(LEDS_ORANGE);
#endif
		break;
	}
}

static void
notification_callback(coap_observee_t *obs, void *notification,
		coap_notification_flag_t flag){
	int len = 0;
	const uint8_t *payload = NULL;
	if(notification) {
		len = coap_get_payload(notification, &payload);
		if(len > 0){
			int id = strtol((char*)payload, 0, 0);
			toggleled(id);
		}
	}
}

static void
res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	const char* str;
	const uint8_t* payload;
	int len = REST.get_query(request, &str);
	if(len > 0){
		int id = strtol(str, 0, 0);
		if(id >= 0 && id <= 4){
			REST.set_response_status(response, REST.status.CHANGED);
		}
		else{
			toggleled(id);
		}

		/*
		 * Payload:
		 * url\n
		 * ip6 address to join\n
		 * obs or poll=interval
		 * */
		if(strncmp(str, "join", len) == 0){
			REST.get_request_payload(request, &payload);


		}
	}
}

