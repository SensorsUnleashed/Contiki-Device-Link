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
#include "lib/sensors.h"
#ifndef NATIVE
#include "dev/gpio.h"
#include "dev/ioc.h"



/*---------------------------------------------------------------------------*/
#define RELAY_PORT_BASE          GPIO_PORT_TO_BASE(RELAY_PORT)
#define RELAY_PIN_MASK           GPIO_PIN_MASK(RELAY_PIN)
#endif

#include "../../apps/uartsensors/uart_protocolhandler.h"
#include "rest-engine.h"

const struct sensors_sensor relay;
/*---------------------------------------------------------------------------*/
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

static uint8_t enabled;
/*---------------------------------------------------------------------------*/
static int
relay_on(void)
{
  if(enabled) {
#ifndef NATIVE
    GPIO_SET_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK);
#endif
    return RELAY_SUCCESS;
  }
  return RELAY_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
relay_off(void)
{
  if(enabled) {
#ifndef NATIVE
	GPIO_CLR_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK);
#endif
    return RELAY_SUCCESS;
  }
  return RELAY_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
    return GPIO_READ_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK);
  case SENSORS_READY:
    return enabled;
  }
  return RELAY_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
value(int type, void* data)
{
  switch(type) {
    case RELAY_OFF:
      return relay_on();
    case RELAY_ON:
      return relay_off();
    case RELAY_TOGGLE:
      if(status(SENSORS_ACTIVE)) {
        return relay_off();
      } else {
        return relay_on();
      }
    default:
      return RELAY_ERROR;
  }
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != SENSORS_ACTIVE) {
    return RELAY_ERROR;
  }

  if(value) {
#ifndef NATIVE
    GPIO_SOFTWARE_CONTROL(RELAY_PORT_BASE, RELAY_PIN_MASK);
    GPIO_SET_OUTPUT(RELAY_PORT_BASE, RELAY_PIN_MASK);
    ioc_set_over(RELAY_PORT, RELAY_PIN, IOC_OVERRIDE_OE);
    GPIO_CLR_PIN(RELAY_PORT_BASE, RELAY_PIN_MASK);
#endif
    enabled = 1;
    return RELAY_SUCCESS;
  }
#ifndef NATIVE
  GPIO_SET_INPUT(RELAY_PORT_BASE, RELAY_PIN_MASK);
#endif
  enabled = 0;
  return RELAY_SUCCESS;
}


/*---------------------------------------------------------------------------*/
static struct extras extra = { .type = 1, .data = (void*)&config };
SENSORS_SENSOR(relay, RELAY_ACTUATOR, value, configure, status, &extra);
/*---------------------------------------------------------------------------*/
/** @} */
