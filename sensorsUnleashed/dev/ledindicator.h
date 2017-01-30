/*
 * ledindicator.h
 *
 *  Created on: 04/01/2017
 *      Author: omn
 */

#ifndef SENSORSUNLEASHED_DEV_LEDINDICATOR_H_
#define SENSORSUNLEASHED_DEV_LEDINDICATOR_H_

#include "lib/susensors.h"

#define LED_INDICATOR "su/ledindicator"

extern struct resourceconf ledindicatorconfig;
susensors_sensor_t* addASULedIndicator(const char* name, struct resourceconf* config);

#endif /* SENSORSUNLEASHED_DEV_LEDINDICATOR_H_ */
