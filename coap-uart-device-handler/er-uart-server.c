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
#include <stdlib.h>

#include "uartsensors.h"
/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t res_mirror;
extern resource_t res_sysinfo;
extern resource_t res_ledtoggle;

PROCESS(er_uart_server, "Erbium Uart Server");
AUTOSTART_PROCESSES(&er_uart_server);
//static struct etimer et;

#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)
#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfd81, 0x3daa, 0xfb4a, 0xf7ae, 0x0212, 0x4b00, 0x5af, 0x8323)
static uip_ipaddr_t server_ipaddr[1]; /* holds the server ip address */

static void
notification_callback(coap_observee_t *obs, void *notification,
		coap_notification_flag_t flag){
	int len = 0;
	const uint8_t *payload = NULL;
	if(notification) {
		len = coap_get_payload(notification, &payload);

		if(len > 0){
			int id = strtol((char*)payload, 0, 0);
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
				leds_set(LEDS_ORANGE);
				break;
			}
		}
	}
}
#include "dev/button-sensor.h"

PROCESS_THREAD(er_uart_server, ev, data)
{
	PROCESS_BEGIN();

	/* Initialize the REST engine. */
	rest_init_engine();
	rest_activate_resource(&res_sysinfo, "SU/SystemInfo");
	rest_activate_resource(&res_ledtoggle, "SU/ledtoggle");
	//	rest_activate_resource(&res_mirror, "debug/mirror");

	/* store server address in server_ipaddr */
//	SERVER_NODE(server_ipaddr);
//	/* receives all CoAP messages */
//	coap_init_engine();
//
//	coap_obs_request_registration(server_ipaddr, REMOTE_PORT,
//			"actuator/button", notification_callback, NULL);


	uartsensors_init();

	/* Delay 1 second */
	//etimer_set(&et, CLOCK_SECOND*10);

	while(1) {
		//PROCESS_WAIT_EVENT();

		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);
		//Sync the current observed resources

		/* Reset the etimer to trig again in 1 second */
		//		etimer_reset(&et);

		//Check what the last value is for all attached devices
		//		res_proxy_get_handler_tester();
	}

	PROCESS_END();
}
