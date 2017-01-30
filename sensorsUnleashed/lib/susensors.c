/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 *
 */
/* exeperimental code, will be renamed to susensors.c when done */


#include <string.h>

#include "contiki.h"

#include "lib/susensors.h"
#include "lib/memb.h"

const char* suEventAboveEventString = "aboveEvent";
const char* suEventBelowEventString = "belowEvent";
const char* suEventChangeEventString = "changeEvent";

const extern struct susensors_sensor *susensors[];

#define FLAG_CHANGED    0x80

process_event_t susensors_event;

static unsigned char num_susensors = 0;

PROCESS(susensors_process, "Sensors");

LIST(sudevices);
MEMB(sudevices_memb, susensors_sensor_t, DEVICES_MAX);

void initSUSensors(){
	list_init(sudevices);
	memb_init(&sudevices_memb);
}

susensors_sensor_t* addSUDevices(susensors_sensor_t* device){
	susensors_sensor_t* d;
	d = memb_alloc(&sudevices_memb);
	if(d == 0) return NULL;

	memcpy(d, device, sizeof(susensors_sensor_t));
	LIST_STRUCT_INIT(d, pairs);
	list_add(sudevices, d);

	return d;
}

/*---------------------------------------------------------------------------*/
susensors_sensor_t *
susensors_first(void)
{
	return list_head(sudevices);
}
/*---------------------------------------------------------------------------*/
susensors_sensor_t *
susensors_next(susensors_sensor_t* s)
{
	return list_item_next(s);
}
/*---------------------------------------------------------------------------*/
void
susensors_changed(susensors_sensor_t* s)
{
	s->event_flag |= FLAG_CHANGED;
	process_poll(&susensors_process);
}
/*---------------------------------------------------------------------------*/
susensors_sensor_t*
susensors_find(const char *prefix, unsigned short len)
{
	susensors_sensor_t* i;

	/* Search through all processes and search for the specified process
     name. */
	if(!len)
		len = strlen(prefix);

	for(i = susensors_first(); i; i = susensors_next(i)) {
		if(strncmp(prefix, i->type, len) == 0) {
			return i;
		}
	}
	return NULL;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(susensors_process, ev, data)
{
	static int i;
	static int events;
	susensors_sensor_t* d;
	PROCESS_BEGIN();

	susensors_event = process_alloc_event();

	for(d = susensors_first(); d; d = susensors_next(d)) {
		d->event_flag = 0;
		d->configure(d, SUSENSORS_HW_INIT, 0);
		num_susensors++;
	}

	while(1) {

		PROCESS_WAIT_EVENT();

		do {
			events = 0;
			for(d = susensors_first(); d; d = susensors_next(d)) {
				if(d->event_flag & FLAG_CHANGED){
					if(process_post(PROCESS_BROADCAST, susensors_event, (void *)d) == PROCESS_ERR_OK) {
						PROCESS_WAIT_EVENT_UNTIL(ev == susensors_event);
					}
					d->event_flag &= ~FLAG_CHANGED;
					events++;
				}
			}
		} while(events);
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
