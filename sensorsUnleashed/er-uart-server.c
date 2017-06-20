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
#include "contiki.h"
#include "rest-engine.h"
#include "er-coap-engine.h"
#include "dev/leds.h"
#include "dev/pulsesensor.h"
#include <stdlib.h>

//#include "uartsensors.h"
#include "button-sensor.h"
#include "dev/relay.h"
#include "dev/ledindicator.h"
#include "dev/mainsdetect.h"
#include "../sensorsUnleashed/pairing.h"
#include "resources/res-uartsensor.h"
#include "resources/res-susensors.h"

//#include "lib/sensors.h"

extern process_event_t sensors_event;

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
//extern  resource_t  res_large_update, res_sysinfo;

PROCESS(er_uart_server, "Erbium Uart Server");
AUTOSTART_PROCESSES(&er_uart_server);

#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

const char* AboveEventString  = "aboveEvent";
const char* BelowEventString  = "belowEvent";
const char* ChangeEventString = "changeEvent";

static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */

static uint8_t payload[50];
static uint8_t coapTx[60];

/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
static void
client_chunk_handler(void *response)
{
  const uint8_t *chunk;

  int len = coap_get_payload(response, &chunk);
  (void)(len);

//  PRINTF("|%.*s", len, (char *)chunk);
}

joinpair_t *pair;
PROCESS_THREAD(er_uart_server, ev, data)
{

	PROCESS_BEGIN();

	//Init dynamic memory	(Default 4096Kb)
	mmem_init();

	initSUSensors();

	/* Initialize the REST engine. */
	rest_init_engine();
	coap_init_engine();

	susensors_sensor_t* d;
	d = addASURelay(RELAY_ACTUATOR, &relayconfigs);
	if(d != NULL) {
		res_susensor_activate(d);
	}
	d = addASULedIndicator(LED_INDICATOR, &ledindicatorconfig);
	if(d != NULL){
		res_susensor_activate(d);
	}
	d = addASUPulseInputRelay(PULSE_SENSOR, &pulseconfig);
	if(d != NULL){
		res_susensor_activate(d);
	}
	d = addASUMainsDetector(MAINSDETECT_ACTUATOR, &mainsdetectconfig);
	if(d != NULL){
		res_susensor_activate(d);
	}

	process_start(&susensors_process, NULL);

#ifndef NATIVE
	process_start(&sensors_process, NULL);
	rest_activate_resource(&res_sysinfo, "SU/SystemInfo");
#endif
	//rest_activate_resource(&res_large_update, "large-update");

	//Activate all attached sensors
	//	SUSENSORS_ACTIVATE(pulse_sensor);
	//	res_susensor_activate(&pulse_sensor);
	//	activateSUSensorPairing(&pulse_sensor);

	//	uartsensors_init();
	//	while(1) {	//Wait until uartsensors has been initialized
	//		PROCESS_WAIT_EVENT_UNTIL(ev == uartsensors_event);
	//		if(data != NULL){
	//			res_uartsensors_activate((uartsensors_device_t*)data);
	//			activateUartSensorPairing((uartsensors_device_t*)data);
	//		}
	//		else
	//			break;
	//	}
	leds_on(LEDS_RED);

	while(1) {
		PROCESS_YIELD();

		if(ev == susensors_event) {
			/*joinpair_t* */ pair = (joinpair_t*)data;
			susensors_sensor_t* p = (susensors_sensor_t*)pair->deviceptr;
			int len = p->getActiveEventMsg(p, payload);

			coap_message_type_t type = COAP_TYPE_NON;

			coap_init_message(request, type, COAP_PUT, coap_get_mid());

			coap_set_header_uri_path(request, (char*)MMEM_PTR(&pair->dsturl));
			coap_set_header_uri_query(request, "postEvent"); //eventstr);
			REST.set_header_content_type(request, APPLICATION_OCTET_STREAM);
			coap_set_payload(request, payload, len);	//Its already msgpack encoded.

			len = coap_serialize_message(request, &coapTx[0]);
			//if(type == COAP_TYPE_NON)
			//coap_send_message(&pair->destip, REMOTE_PORT, &coapTx[0], len);
			//else
			leds_toggle(LEDS_GREEN);
			COAP_BLOCKING_REQUEST(&pair->destip, REMOTE_PORT, request, client_chunk_handler);
		}
		else if(ev == sensors_event){	//Button was pressed
			PRINTF("Button press\n");
		}
	}
	PROCESS_END();
}
