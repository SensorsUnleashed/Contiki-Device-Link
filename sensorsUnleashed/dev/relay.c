/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-relay
 * @{
 *
 * \file
 *  Driver for a relay actuator
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "relay.h"
#include "lib/susensors.h"
#include "dev/gpio.h"
#include "dev/ioc.h"

/*---------------------------------------------------------------------------*/
#define RELAY_PORT_BASE          GPIO_PORT_TO_BASE(RELAY_PORT)
#define RELAY_PIN_MASK           GPIO_PIN_MASK(RELAY_PIN)

#include "../../apps/uartsensors/uart_protocolhandler.h"
#include "rest-engine.h"
#include "susensorcommon.h"

const struct susensors_sensor relay;
/*---------------------------------------------------------------------------*/
struct relayRuntime {
	uint8_t enabled;
	uint8_t hasEvent;
	enum susensors_event_cmd lastEvent;
	cmp_object_t LastEventValue;
	const char* eventtype;
};
static struct relayRuntime rdata = {
		.enabled = 0,
		.hasEvent = 0,
		.LastEventValue = {
				.type = CMP_TYPE_UINT8,
				.as.u8 = 0
		},
};

static struct resourceconf config = {
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
		.spec = "Relay output control OFF=0, ON=1, TOGGLE=2",
		.type = RELAY_ACTUATOR,
		.attr = "title=\"Relay output\" ;rt=\"Control\"",
};

/*---------------------------------------------------------------------------*/

static int
relay_on(struct susensors_sensor* this)
{
	GPIO_SET_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK);
	struct relayRuntime* r = (struct relayRuntime*)this->data->runtime;
	if(r->LastEventValue.as.u8 != 1){
		r->LastEventValue.as.u8 = 1;
		r->hasEvent = 1;
		r->lastEvent = SUSENSORS_ABOVE_EVENT_SET;
		r->eventtype = suEventAboveEventString;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
static int
relay_off(struct susensors_sensor* this)
{
	GPIO_CLR_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK);
	struct relayRuntime* r = (struct relayRuntime*)this->data->runtime;
	if(r->LastEventValue.as.u8 != 0){
		r->LastEventValue.as.u8 = 0;
		r->hasEvent = 1;
		r->lastEvent = SUSENSORS_BELOW_EVENT_SET;
		r->eventtype = suEventBelowEventString;
	}

	return 0;
}
/*---------------------------------------------------------------------------*/

static int
get(struct susensors_sensor* this, int type, void* data)
{
	int ret = 1;
	cmp_object_t* obj = (cmp_object_t*)data;
	if((enum up_parameter) type == ActualValue){
		obj->type = CMP_TYPE_UINT8;
		obj->as.u8 = (uint8_t) GPIO_READ_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK) > 0;
		ret = 0;
	}
	return ret;
}
/*---------------------------------------------------------------------------*/
static int
set(struct susensors_sensor* this, int type, void* data)
{
	int enabled = ((struct relayRuntime*)this->data->runtime)->enabled;
	int ret = 1;
	if((enum suactions)type == setRelay_off && enabled){
		ret = relay_off(this);
	}
	else if((enum suactions)type == setRelay_on && enabled){
		ret = relay_on(this);
	}
	else if((enum suactions)type == setRelay_toggle && enabled){
		if(GPIO_READ_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK) > 0){
			ret = relay_off(this);
		}
		else{
			ret = relay_on(this);
		}
	}

	//Just signal that we have an event. Let the event logic handle if it should be fired or not
	if(ret == 0){
		susensors_changed(this);
	}

	return ret;
}

/*---------------------------------------------------------------------------*/
static int
configure(struct susensors_sensor* this, int type, int value)
{
	if(type != SUSENSORS_ACTIVE) {
		return RELAY_ERROR;
	}

	if(value) {
		GPIO_SOFTWARE_CONTROL(RELAY_PORT_BASE, RELAY_PIN_MASK);
		GPIO_SET_OUTPUT(RELAY_PORT_BASE, RELAY_PIN_MASK);
		ioc_set_over(RELAY_PORT, RELAY_PIN, IOC_OVERRIDE_OE);
		GPIO_CLR_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK);
		((struct relayRuntime*)this->data->runtime)->enabled = 1;
		return RELAY_SUCCESS;
	}
	GPIO_SET_INPUT(RELAY_PORT_BASE, RELAY_PIN_MASK);
	((struct relayRuntime*)this->data->runtime)->enabled = 0;
	return RELAY_SUCCESS;
}

/* An event was received from another device - now act on it */
static int eventHandler(struct susensors_sensor* this, int type, int len, uint8_t* payload){
	enum susensors_event_cmd cmd = (enum susensors_event_cmd)type;
	int ret = 1;
	switch(cmd){
	case SUSENSORS_ABOVE_EVENT_SET:
		this->value(this, setRelay_on, NULL);
		ret = 0;
		break;
	case SUSENSORS_BELOW_EVENT_SET:
		this->value(this, setRelay_off, NULL);
		ret = 0;
		break;
	case SUSENSORS_CHANGE_EVENT_SET:
//		this->value(this, setRelay_toggle, NULL);
//		ret = 0;
		break;
	}
	return ret;
}

static int getActiveEventMsg(struct susensors_sensor* this, const char** eventstr, uint8_t* payload){
	struct relayRuntime* d = (struct relayRuntime*)this->data->runtime;
	int len = cp_encodeObject(payload, &d->LastEventValue);
	*eventstr = d->eventtype;

	return len;
}

/*---------------------------------------------------------------------------*/
static struct extras extra = { .type = 1, .config = (void*)&config, .runtime = (void*) &rdata};
SUSENSORS_SENSOR(relay, RELAY_ACTUATOR, set, configure, get, eventHandler, getActiveEventMsg, (void*)&extra);
/*---------------------------------------------------------------------------*/
/** @} */
