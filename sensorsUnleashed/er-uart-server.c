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
#include "dev/leds.h"
#include "dev/pulsesensor.h"
#include <stdlib.h>

#include "uartsensors.h"
#include "dev/button-sensor.h"
#include "dev/relay.h"
#ifndef NATIVE
#include "dev/cc2538-sensors.h"
#endif
#include "dev/button-sensor.h"

#include "../sensorsUnleashed/pairing.h"
#include "resources/res-uartsensor.h"
#include "resources/res-susensors.h"

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

SENSORS(
		&button_sensor,
#ifndef NATIVE
		&cc2538_temp_sensor,
#endif
		&pulse_sensor, &relay
		);


static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */
static joinpair_t *pair = NULL;
static uint8_t tx[200];
PROCESS_THREAD(er_uart_server, ev, data)
{

	PROCESS_BEGIN();

	//Init dynamic memory	(Default 4096Kb)
	mmem_init();

	//Restore sensor pairs stored in flash
	restore_SensorPairs();

	/* Initialize the REST engine. */
	rest_init_engine();
	coap_init_engine();
#ifndef NATIVE
	process_start(&sensors_process, NULL);
	rest_activate_resource(&res_sysinfo, "SU/SystemInfo");
#endif
	//rest_activate_resource(&res_large_update, "large-update");
	rest_activate_resource(&res_ledtoggle, "SU/ledtoggle");
	//	rest_activate_resource(&res_mirror, "debug/mirror");

	//Activate all attached sensors
	SENSORS_ACTIVATE(pulse_sensor);
	res_susensor_activate(&pulse_sensor);
	SENSORS_ACTIVATE(relay);
	res_susensor_activate(&relay);

	uartsensors_init();
	while(1) {	//Wait until uartsensors has been initialized
		PROCESS_WAIT_EVENT_UNTIL(ev == uartsensors_event);
		if(data != NULL){
			res_uartsensors_activate((uartsensors_device_t*)data);
			activateUartSensorPairing((uartsensors_device_t*)data);
		}
		else
			break;
	}
	leds_on(LEDS_RED);
	while(1) {
		PROCESS_YIELD();

		if(ev == uartsensors_event){
			list_t pairinglist = pairing_get_pairs();
			for(pair = (joinpair_t *)list_head(pairinglist); pair; pair = pair->next) {
				if(pair->deviceptr == data){
					coap_message_type_t type = COAP_TYPE_NON;
					//Found a pair, now send the reading to the subscriber
					coap_init_message(request, type, COAP_PUT, coap_get_mid());
					coap_set_header_uri_path(request, (char*)MMEM_PTR(&pair->dsturl));
					REST.set_header_content_type(request, APPLICATION_OCTET_STREAM);


					if(pair->devicetype == uartsensor){
						uartsensors_device_t* p = (uartsensors_device_t*)data;
						coap_set_payload(request, p->lastval, p->vallen);	//Its already msgpack encoded.
					}

					uint16_t len = coap_serialize_message(request, &tx[0]);
					if(type == COAP_TYPE_NON)
						coap_send_message(&pair->destip, REMOTE_PORT, &tx[0], len);
					else
						COAP_BLOCKING_REQUEST(&pair->destip, REMOTE_PORT, request, 0/*client_chunk_handler*/);

					leds_toggle(LEDS_YELLOW);
				}
			}
		}
	}
	PROCESS_END();
}