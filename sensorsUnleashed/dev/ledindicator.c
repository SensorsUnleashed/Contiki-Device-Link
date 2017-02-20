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
#include "ledindicator.h"
#include "contiki.h"
#include "dev/leds.h"

#include "../../apps/uartsensors/uart_protocolhandler.h"
#include "rest-engine.h"
#include "susensorcommon.h"
#include "board.h"

struct resourceconf ledindicatorconfig = {
		.resolution = 1,
		.version = 1,
		.flags = METHOD_GET | METHOD_PUT,
		.max_pollinterval = 2,
		.eventsActive = 0,
		.AboveEventAt = {
				.type = CMP_TYPE_UINT8,
				.as.u8 = 1
		},
		.BelowEventAt = {
				.type = CMP_TYPE_UINT8,
				.as.u8 = 0
		},
		.ChangeEvent = {
				.type = CMP_TYPE_UINT8,
				.as.u8 = 1
		},
		.RangeMin = {
				.type = CMP_TYPE_UINT16,
				.as.u8 = 0
		},
		.RangeMax = {
				.type = CMP_TYPE_UINT16,
				.as.u8 = 1
		},

		.unit = "",
		.spec = "LED indicator",
		.type = LED_INDICATOR,
		.attr = "title=\"LED indicator\" ;rt=\"Indicator\"",
};

static int set(struct susensors_sensor* this, int type, void* data){
	int ret = 1;

	if(type == toggleLED_RED){
		leds_toggle(LEDS_RED);
		ret = 0;
	}
	else if(type == toggleLED_GREEN){
		leds_toggle(LEDS_GREEN);
		ret = 0;
	}
	else if(type == toggleLED_ORANGE){
		leds_toggle(LEDS_ORANGE);
		ret = 0;
	}
	else if(type == toggleLED_YELLOW){
		leds_toggle(LEDS_YELLOW);
		ret = 0;
	}

	return ret;
}

static int configure(struct susensors_sensor* this, int type, int value){
	return 0;
}

int get(struct susensors_sensor* this, int type, void* data){
	int ret = 1;
	cmp_object_t* obj = (cmp_object_t*)data;

	if((enum up_parameter) type == ActualValue){
		obj->type = CMP_TYPE_UINT8;
		obj->as.u8 = leds_get() & LEDS_CONF_ALL;
		ret = 0;
	}
	return ret;
}

/* An event was received from another device - now act on it */
static int eventHandler(struct susensors_sensor* this, int len, uint8_t* payload){
	uint8_t event;
	uint32_t parselen = len;
	cmp_object_t eventval;
	if(cp_decodeU8(payload, &event, &parselen) != 0) return 1;
	payload += parselen;
	parselen = len - parselen;
	if(cp_decodeObject(payload, &eventval, &parselen) != 0) return 2;

	if(event & AboveEventActive){
		this->value(this, toggleLED_RED, NULL);
	}
	else if(event & BelowEventActive){
		this->value(this, toggleLED_YELLOW, NULL);
	}

	if(event & ChangeEventActive){
		this->value(this, toggleLED_ORANGE, NULL);
	}

	return 0;
}

susensors_sensor_t* addASULedIndicator(const char* name, struct resourceconf* config){
	susensors_sensor_t d;
	d.type = (char*)name;
	d.status = get;
	d.value = set;
	d.configure = configure;
	d.eventhandler = eventHandler;
	d.getActiveEventMsg = getActiveEventMsg;
	d.suconfig = suconfig;
	d.data.config = config;
	d.data.runtime = (void*)0;

	return addSUDevices(&d);
}
