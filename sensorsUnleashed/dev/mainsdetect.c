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
#include "sys/ctimer.h"
#include "mainsdetect.h"

#include "dev/leds.h"
#include "dev/gpio.h"
#include "dev/ioc.h"

#define MAINSDETECT_PORT_BASE          GPIO_PORT_TO_BASE(MAINSDETECT_PORT)
#define MAINSDETECT_PIN_MASK           GPIO_PIN_MASK(MAINSDETECT_PIN)
#define MAINSDETECT_VECTOR			   GPIO_A_IRQn //NVIC_INT_GPIO_PORT_A

#include "rest-engine.h"
#include "susensorcommon.h"

static struct ctimer mains_gone_timeout;

static struct relayRuntime mainsdetectruntime[1];

struct resourceconf mainsdetectconfig = {
		.resolution = 1,
		.version = 1,
		.flags = METHOD_GET | METHOD_PUT,
		.max_pollinterval = 2,
		.eventsActive = AboveEventActive | BelowEventActive,
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
				.type = CMP_TYPE_UINT8,
				.as.u8 = 0
		},
		.RangeMax = {
				.type = CMP_TYPE_UINT8,
				.as.u8 = 1
		},

		.unit = "",
		.spec = "Detect if mains is present or not",
		.type = MAINSDETECT_ACTUATOR,
		.attr = "title=\"Mainsdetect\" ;rt=\"Monitor\"",
};


static int get(struct susensors_sensor* this, int type, void* data)
{
	int ret = 1;
	cmp_object_t* obj = (cmp_object_t*)data;
	if((enum up_parameter) type == ActualValue){
		struct susensors_sensor* this = (struct susensors_sensor*)mains_gone_timeout.ptr;
		struct relayRuntime* r = (struct relayRuntime*)this->data.runtime;
		*obj = r->LastValue;
		ret = 0;
	}
	return ret;
}

static int set(struct susensors_sensor* this, int type, void* data)
{
	int ret = 1;
	return ret;
}

//#include "dev/leds.h"

/**
 * \brief Callback registered with the GPIO module. Gets fired when mains is detected
 * \param port The port number that generated the interrupt
 * \param pin The pin number that generated the interrupt. This is the pin
 * absolute number (i.e. 0, 1, ..., 7), not a mask
 */
static void
mainsdetect_isr_callback(uint8_t port, uint8_t pin)
{
	/* The mains has been gone for more than 1 second, but is back again.
	 * Restart the mains gone timeout again
	*/
	ctimer_restart(&mains_gone_timeout);

	struct susensors_sensor* this = (struct susensors_sensor*)mains_gone_timeout.ptr;
	struct relayRuntime* r = (struct relayRuntime*)this->data.runtime;

	if(r->LastValue.as.u8 == 0){
		r->LastValue.as.u8 = 1;
		setEventU8(this, 1, 1);
		leds_on(LEDS_YELLOW);
	}
}

static void
mainsgonecallback(void *ptr)
{
	struct susensors_sensor* this = (struct susensors_sensor*)ptr;
	struct relayRuntime* r = (struct relayRuntime*)this->data.runtime;

	if(r->LastValue.as.u8 == 1){
		r->LastValue.as.u8 = 0;
		setEventU8(this, -1, 1);
		leds_off(LEDS_YELLOW);
	}
}

static int configure(struct susensors_sensor* this, int type, int value)
{
	switch(type) {
	case SUSENSORS_HW_INIT:
		/* Software controlled */
		GPIO_SOFTWARE_CONTROL(MAINSDETECT_PORT_BASE, MAINSDETECT_PIN_MASK);

		/* Set pin to input */
		GPIO_SET_INPUT(MAINSDETECT_PORT_BASE, MAINSDETECT_PIN_MASK);

		/* Enable edge detection */
		GPIO_DETECT_EDGE(MAINSDETECT_PORT_BASE, MAINSDETECT_PIN_MASK);

		/* Rising Edges */
		GPIO_DETECT_RISING(MAINSDETECT_PORT_BASE, MAINSDETECT_PIN_MASK);

		ioc_set_over(MAINSDETECT_PORT, MAINSDETECT_PIN, IOC_OVERRIDE_PUE);

		gpio_register_callback(mainsdetect_isr_callback, MAINSDETECT_PORT, MAINSDETECT_PIN);
		break;
	case SUSENSORS_ACTIVE:
		if(value) {
			ctimer_set(&mains_gone_timeout, CLOCK_SECOND/5, mainsgonecallback, this);
			GPIO_ENABLE_INTERRUPT(MAINSDETECT_PORT_BASE, MAINSDETECT_PIN_MASK);
			NVIC_EnableIRQ(MAINSDETECT_VECTOR);

		} else {
			ctimer_stop(&mains_gone_timeout);
			GPIO_DISABLE_INTERRUPT(MAINSDETECT_PORT_BASE, MAINSDETECT_PIN_MASK);
			NVIC_DisableIRQ(MAINSDETECT_VECTOR);
		}
		return value;
	default:
		break;
	}
	return 0;
}

/* An event was received from another device - now act on it */
static int eventHandler(struct susensors_sensor* this, int len, uint8_t* payload){
	return 0;
}

susensors_sensor_t* addASUMainsDetector(const char* name, struct resourceconf* config){
	susensors_sensor_t d;
	d.type = (char*)name;
	d.status = get;
	d.value = set;
	d.configure = configure;
	d.eventhandler = eventHandler;
	d.getActiveEventMsg = getActiveEventMsg;
	d.suconfig = suconfig;
	d.data.config = config;

	mainsdetectruntime[0].enabled = 0;
	mainsdetectruntime[0].hasEvent = 0,
	mainsdetectruntime[0].LastEventValue.type = CMP_TYPE_UINT8;
	mainsdetectruntime[0].LastEventValue.as.u8 = 0;
	mainsdetectruntime[0].ChangeEventAcc.as.u8 = 0;
	d.data.runtime = (void*) &mainsdetectruntime[0];

	return addSUDevices(&d);
}
