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
/**
 * \file
 *      Example resource
 * \author
 *      Ole Nissen <ole@mnissen.dk>
 */

#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include "er-coap-engine.h"
#include "uart_protocolhandler.h"

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
		leds_toggle(LEDS_GREEN);
		break;
	case 2:
		leds_toggle(LEDS_RED);
		break;
	case 3:
		leds_toggle(LEDS_YELLOW);
		break;
#ifndef NATIVE
	case 4:
		leds_toggle(LEDS_ORANGE);
		break;
#endif
	}
}

static void
res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	const uint8_t* payload;

	unsigned int ct = -1;
	REST.get_header_content_type(request, &ct);
	if(ct != REST.type.APPLICATION_OCTET_STREAM) {
		REST.set_response_status(response, REST.status.BAD_REQUEST);
		const char *error_msg = "msgPacked, octet-stream only";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		return;
	}

	uint32_t len = REST.get_request_payload(request, &payload);
	if(len > 0){
		uint8_t id;
		if(cp_decodeU8(payload, &id, &len) == 0){
			if(id > 0 && id <= 4){
				toggleled(id);
				REST.set_response_status(response, REST.status.CHANGED);
				const char *error_msg = "toggled led";
				REST.set_response_payload(response, error_msg, strlen(error_msg));
				return;

			}
			else{
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				const char *error_msg = "Accepting values from 1 - 4";
				REST.set_response_payload(response, error_msg, strlen(error_msg));
			}
		}
	}
	else{
		REST.set_response_status(response, REST.status.BAD_REQUEST);
		const char *error_msg = "Missing payload";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
	}
}

