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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
//#include "contiki-net.h"
#include "rest-engine.h"

#include "dev/button-sensor.h"
#include "coap_proxy.h"

#include "pt.h"

struct timer timer;

struct pt testpt, testpt2;
struct pt childpt;

static struct timer input_timer;

PT_THREAD(wait5sec(struct pt *pt)){
	PT_BEGIN(pt);
	  printf("Waiting 1 second\n");
	  timer_set(&input_timer, 1000);
	  PT_WAIT_UNTIL(pt, timer_expired(&input_timer));

	  printf("Waiting 5 second\n");
	  timer_set(&input_timer, 5000);
	  PT_WAIT_UNTIL(pt, timer_expired(&input_timer));

	  printf("Waiting 5 second\n");
	  timer_set(&input_timer, 5000);
	  PT_WAIT_UNTIL(pt, timer_expired(&input_timer));

	  printf("Waiting 5 second\n");
	  timer_set(&input_timer, 5000);
	  PT_WAIT_UNTIL(pt, timer_expired(&input_timer));

	PT_END(pt);

}

PT_THREAD(waitagainsec(struct pt *pt)){
	PT_BEGIN(pt);

	while(1){

	  printf("Waiting 1 second\n");
	  timer_set(&input_timer, 1000);
	  PT_WAIT_UNTIL(pt, timer_expired(&input_timer));

	  printf("Waiting 2 second\n");
	  timer_set(&input_timer, 2000);
	  PT_WAIT_UNTIL(pt, timer_expired(&input_timer));

	  printf("Waiting 2 second\n");
	  timer_set(&input_timer, 2000);
	  PT_WAIT_UNTIL(pt, timer_expired(&input_timer));

	  printf("Waiting 2 second\n");
	  timer_set(&input_timer, 2000);
	  PT_WAIT_UNTIL(pt, timer_expired(&input_timer));
	}
	PT_END(pt);

}




/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t res_toggle, res_mirror;

PROCESS(er_uart_server, "Erbium Uart Server");
AUTOSTART_PROCESSES(&er_uart_server);

PROCESS_THREAD(er_uart_server, ev, data)
{
	PROCESS_BEGIN();

	//PT_INIT(&testpt);
	//PT_INIT(&testpt2);



	/* Initialize the REST engine. */
//	rest_init_engine();
//	rest_activate_resource(&res_mirror, "debug/mirror");



	proxy_init();

//	while(wait5sec(&testpt) == 0);
//	while(waitagainsec(&testpt2) == 0);
//	PROCESS_PT_SPAWN(&testpt2, waitagainsec(&testpt2));

	/* Define application-specific events here. */

	while(1) {
	     PROCESS_WAIT_EVENT();

	     printf("Got event number %d\n", ev);

		//PROCESS_WAIT_EVENT_UNTIL(wait5sec(&testpt) == 0);
		//while (wait5sec(&testpt) == 0);
		//PROCESS_WAIT_EVENT();
	}

	PROCESS_END();
}

#if 0
PROCESS(example_process1, "Example process");
AUTOSTART_PROCESSES(&example_process1);
struct etimer et;
PROCESS_THREAD(example_process1, ev, data)
{
  PROCESS_BEGIN();

  /* Delay 1 second */
  etimer_set(&et, CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();
    /* Reset the etimer to trig again in 1 second */
    etimer_reset(&et);
    printf("Got event number %d\n", ev);
  }

  PROCESS_END();
}
#endif
