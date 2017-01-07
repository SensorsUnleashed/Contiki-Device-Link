/*
 * pulsesensor.c
 *
 *  Created on: 06/12/2016
 *      Author: omn
 */
#include "pulsesensor.h"
#include "contiki.h"

#include "lib/sensors.h"
#include "dev/sys-ctrl.h"
#include "dev/ioc.h"
#include "../../apps/uartsensors/uart_protocolhandler.h"
#include "rest-engine.h"
#include "dev/gptimer.h"
#include "dev/gpio.h"

#include "susensorcommon.h"

const struct sensors_sensor pulse_sensor;

#define PULSE_PORT            GPIO_A_NUM
#define PULSE_PIN             3

static struct resourceconf config = {
		.resolution = 100,	//0.1W
		.version = 1,		//Q22.10
		.flags = METHOD_GET | METHOD_PUT,			//Flags - which handle do we serve: Get/Post/PUT/Delete
		.max_pollinterval = 30, 					//How often can you ask for a new reading

		.eventsActive = 0,
		.AboveEventAt = {
				.type = CMP_TYPE_UINT16,
		     	.as.u16 = 10000
		},
		.BelowEventAt = {
				.type = CMP_TYPE_UINT16,
		     	.as.u16 = 0
		},
		.ChangeEvent = {
				.type = CMP_TYPE_UINT16,
				.as.u16 = 10000
		},
		.RangeMin = {
				.type = CMP_TYPE_UINT16,
				.as.u16 = 0
		},
		.RangeMax = {
				.type = CMP_TYPE_UINT16,
				.as.u16 = 60000
		},
		.unit = "W",
		.spec = "This is a pulse counter, counting 10000 pulses/kwh",				//Human readable spec of the sensor
		.type = PULSE_SENSOR,
		.attr = "title=\"Puls counter\" ;rt=\"Control\"",
		//uint8_t notation;		//Qm.f => MMMM.FFFF	eg. Q1.29 = int32_t with 1 integer bit and 29 fraction bits, Q
};

/**
 * \brief Retrieves the current upcounted value of the pulse input
 * 		  The counter is reset for every read.
 * \param type Returns ....
 */
//Return 0 for success
//Return 1 for invalid request
static int
get(int type, void* data)
{
	int ret = 1;
	cmp_object_t* obj = (cmp_object_t*)data;

	if((enum up_parameter) type == ActualValue){
		obj->type = CMP_TYPE_UINT16;
		obj->as.u16 = (uint16_t) REG(GPT_1_BASE + GPTIMER_TAR);
		ret = 0;
	}
	else if(type >= (int)ChangeEventConfigValue){
		ret = su_sensorvalue(type, obj, &config);
	}
	return ret;
}

//Return 0 for success
//Return 1 for invalid request
static int
set(int type, void* data)
{
	int ret = 1;
	return ret;
}

static int
config_user(int type, int value)
{
	switch(type) {
	case SENSORS_HW_INIT:
		/* We use Timer1A as a 16bit pulse counter */

		/*
		 * Remove the clock gate to enable GPT1 and then initialise it
		 */
		REG(SYS_CTRL_RCGCGPT) |= SYS_CTRL_RCGCGPT_GPT1;

		/* When hardware control is enabled by the GPIO Alternate Function
		 * Select (GPIO_AFSEL) register (see Section 9.1), the pin state
		 * is controlled by its alternate function (i.e., the peripheral). */
		/* Set PA3 to be pulse counter input */
		REG(IOC_GPT1OCP1) = (PULSE_PORT << 3) + PULSE_PIN;

		/* Set pin PA3 (DIO1) to peripheral mode */
		GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(PULSE_PORT),GPIO_PIN_MASK(PULSE_PIN));

		/* Disable any pull ups or the like */
		//ioc_set_over(PULSE_PORT, PULSE_PIN, IOC_OVERRIDE_DIS);

		/* From user manual p. 329 */
		/* 1. Ensure the timer is disabled (the TAEN bit is cleared) before making any changes. */
		REG(GPT_1_BASE + GPTIMER_CTL) = 0;

		/* 2. Write the GPTM Configuration (GPTIMER_CFG) register with a value of 0x0000.0004. */
		/* 16-bit timer configuration */
		REG(GPT_1_BASE + GPTIMER_CFG) = 0x04;

		/* 3. In the GPTM Timer Mode (GPTIMER_TnMR) register, write the TnCMR field to 0x0 and the TnMR field to 0x3.*/
		/* Capture mode, Edge-count mode*/
		REG(GPT_1_BASE + GPTIMER_TAMR) |= GPTIMER_TAMR_TAMR;		//Capture mode
		REG(GPT_1_BASE + GPTIMER_TAMR) &= ~(GPTIMER_TAMR_TACMR);	//Edge count mode
		REG(GPT_1_BASE + GPTIMER_TAMR) |= GPTIMER_TAMR_TACDIR;		//Count up


		/* 4. Configure the type of event(s) that the timer captures by writing the TnEVENT field of the G Control (GPTIMER_CTL) register. */
		/* Positive edges */
		REG(GPT_1_BASE + GPTIMER_CTL) &= ~GPTIMER_CTL_TAEVENT;	//0=Postive Edge, 1=Negative Edge, 3=Both

		/* 5. If a prescaler is to be used, write the prescale value to the GPTM Timer n Prescale Regist (GPTIMER_TnPR). */
		/* No prescaler */
		REG(GPT_1_BASE + GPTIMER_TAPR) = 0;

		/* 6. Load the timer start value into the GPTM Timer n Interval Load (GPTIMER_TnILR) registe */
		/* When the timer is counting up, this register sets the upper bound for the timeout event. */
		REG(GPT_1_BASE + GPTIMER_TAILR) = config.RangeMax.as.u16;;	//When reached, its starts over from 0

		/* 7. Load the event count into the GPTM Timer n Match (GPTIMER_TnMATCHR) register. */
		REG(GPT_1_BASE + GPTIMER_TAMATCHR) = config.RangeMax.as.u16;


		break;
	case SENSORS_ACTIVE:
		if(value){	//Activate
			if(!REG(GPT_1_BASE + GPTIMER_CTL) & GPTIMER_CTL_TAEN){
				REG(GPT_1_BASE + GPTIMER_CTL) |= GPTIMER_CTL_TAEN;	//Enable puls counter
			}
		}
		else{
			if(REG(GPT_1_BASE + GPTIMER_CTL) & GPTIMER_CTL_TAEN){
				REG(GPT_1_BASE + GPTIMER_CTL) &= ~GPTIMER_CTL_TAEN;	//disable puls counter
			}
		}
		break;
	case SENSORS_MAX_AGE:
		return 30;	//Updated value every 30 sec.
		break;
	}
	return 1;
}

static struct extras extra = { .type = 1, .data = (void*)&config };
SENSORS_SENSOR(pulse_sensor, PULSE_SENSOR, set, config_user, get, &extra);
