/*
 * pulsesensor.h
 *
 *  Created on: 08/12/2016
 *      Author: omn
 */

#ifndef SENSORSUNLEASHED_DEV_PULSESENSOR_H_
#define SENSORSUNLEASHED_DEV_PULSESENSOR_H_

#include "susensors.h"

#define PULSE_SENSOR "su/pulsecounter"

extern struct resourceconf pulseconfig;
susensors_sensor_t* addASUPulseInputRelay(const char* name, struct resourceconf* config);


/**
 * NOTE:
 * The pulsecounter will generate interrupts for every changeEvent value. This means, that
 * the resolution will be this value. Below event, will never get fired, because, for now,
 * the pulse counter cant count down (Could be an option later on, to allow that). The above
 * event will be fired when the counting value goes beyond the set value, but still, the
 * resolution will be defined by the changeEvent value.
 */

#endif /* SENSORSUNLEASHED_DEV_PULSESENSOR_H_ */
