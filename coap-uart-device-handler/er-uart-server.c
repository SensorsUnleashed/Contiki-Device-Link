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
 *      Erbium (Er) REST Engine example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include "contiki.h"
#include "rest-engine.h"
#include "er-coap-engine.h"
#include "res-uartsensor.h"
#include "dev/leds.h"
#include <stdlib.h>

#include "storage.h"
#include "uartsensors.h"
/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
#ifndef NATIVE
extern resource_t res_sysinfo;
#endif

extern  resource_t  res_large_update, res_ledtoggle;

PROCESS(er_uart_server, "Erbium Uart Server");
AUTOSTART_PROCESSES(&er_uart_server);

#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)
#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfd81, 0x3daa, 0xfb4a, 0xf7ae, 0x0212, 0x4b00, 0x5af, 0x8323)

#include "dev/button-sensor.h"
uint8_t rx[200];
uint8_t tx[] = {
		0xdc,
		0x00,
		0x10,
		0xcd,
		0x80,
		0xfe,
		0xcd,
		0x00,
		0x00,
		0xcd,
		0x00,
		0x00,
		0xcd,
		0x00,
		0x00,
		0xcd,
		0x7c,
		0x79,
		0xcd,
		0x23,
		0x12,
		0xcd,
		0x1f,
		0x5f,
		0xcd,
		0xfb,
		0xe7,
		0xaa,
		0x74,
		0x65,
		0x73,
		0x74,
		0x2f,
		0x74,
		0x65,
		0x73,
		0x74,
		0x32
};
PROCESS_THREAD(er_uart_server, ev, data)
{
	PROCESS_BEGIN();
	static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */
	static joinpair_t* coappair = 0;

	//Init dynamic memory	(Default 4096Kb)
	mmem_init();

	//const char* teststr = "Dette er en streng der skal gemmes i flash";
	uint32_t txlen = sizeof(tx);
	store_SensorPair((uint8_t*)tx, txlen);
	store_SensorPair((uint8_t*)tx, txlen);
//	memset(tx, 0, txlen);
	read_SensorPairs(rx, &txlen);

	/* Initialize the REST engine. */
	rest_init_engine();
	coap_init_engine();
#ifndef NATIVE
	rest_activate_resource(&res_sysinfo, "SU/SystemInfo");
#endif
	  rest_activate_resource(&res_large_update, "large-update");
	  rest_activate_resource(&res_ledtoggle, "SU/ledtoggle");
	//	rest_activate_resource(&res_mirror, "debug/mirror");

	uartsensors_init();
	while(1) {	//Wait until uartsensors has been initialized
		PROCESS_WAIT_EVENT_UNTIL(ev == uartsensors_event);
		if(data != NULL){
			res_uartsensors_activate((uartsensors_device_t*)data);
		}
		else
		break;
	}
	leds_on(LEDS_RED);
	while(1) {
		//PROCESS_WAIT_EVENT_UNTIL(ev == uartsensors_event/* || ev == sensors_event*/);
		PROCESS_YIELD();

		if(ev == uartsensors_event){
			uartsensors_device_t* p = (uartsensors_device_t*)data;
			coappair = getUartSensorPair(p);

			if(coappair != 0){
				coap_message_type_t type = COAP_TYPE_NON;
				//Found a pair, now send the reading to the subscriber
				coap_init_message(request, type, COAP_PUT, coap_get_mid());
				coap_set_header_uri_path(request, MMEM_PTR(&coappair->dsturl));
				REST.set_header_content_type(request, APPLICATION_OCTET_STREAM);
				coap_set_payload(request, p->lastval, p->vallen);	//Its already msgpack encoded.
				uint16_t len = coap_serialize_message(request, &tx[0]);

				if(type == COAP_TYPE_NON)
					coap_send_message(&coappair->destip, REMOTE_PORT, &tx[0], len);
				else
					COAP_BLOCKING_REQUEST(&coappair->destip, REMOTE_PORT, request, 0/*client_chunk_handler*/);

				leds_toggle(LEDS_YELLOW);
			}
		}

	}

	PROCESS_END();
}
