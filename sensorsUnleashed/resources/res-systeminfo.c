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
 * res-systeminfo.c
 *
 *  Created on: 07/10/2016
 *      Author: omn
 */
#ifndef NATIVE
#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include "board.h"


static void res_sysinfo_gethandler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple actuator example. Toggles the red led */
RESOURCE(res_sysinfo,
         "title=\"Radio info\";rt=\"Info\"",
		 res_sysinfo_gethandler,
         NULL,
         NULL,
         NULL);

/*
 * Get the info about the radio:
 * 	IP address
 * 	rssi
 * 	MAC Address
 */
static void
res_sysinfo_gethandler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
	REST.set_header_max_age(response, 0);	//3 minutes

	int len = 0;
	//Pay attention to the max payload length
//	len = sprintf((char*)buffer, "Contiki version: %s\n", CONTIKI_VERSION_STRING);
	len += sprintf((char*)(buffer+len), "Board: %s\n", BOARD_STRING);
	len += sprintf((char*)(buffer+len), "Version: 0.0.1\n");
	REST.set_response_payload(response, buffer, len);
}
#endif
