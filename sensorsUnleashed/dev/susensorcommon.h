/*
 * susensorcommon.h
 *
 *  Created on: 25/12/2016
 *      Author: omn
 */

#ifndef SENSORSUNLEASHED_DEV_SUSENSORCOMMON_H_
#define SENSORSUNLEASHED_DEV_SUSENSORCOMMON_H_

#include "lib/susensors.h"

int suconfig(struct susensors_sensor* this, int type, void* data);
int getActiveEventMsg(struct susensors_sensor* this, uint8_t* payload);
void setEventU8(struct susensors_sensor* this, int dir, uint8_t step);
void setEventU16(struct susensors_sensor* this, int dir, uint8_t step);
#endif /* SENSORSUNLEASHED_DEV_SUSENSORCOMMON_H_ */
