/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup openmote-cc2538
 * @{
 *
 * \defgroup openmote-button-sensor OpenMote-CC2538 user button driver
 *
 * The user button will generate a sensors_changed event on press as
 * well as on release.
 *
 * @{
 *
 * \file
 * Header for the OpenMote-CC2538 button driver
 */
/*---------------------------------------------------------------------------*/
#ifndef BUTTON_SENSOR_H_
#define BUTTON_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include "lib/susensors.h"

extern struct resourceconf pushbuttonconfig;

susensors_sensor_t* addASUButtonSensor(const char* name, struct resourceconf* config);

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern process_event_t button_press_duration_exceeded;
/*---------------------------------------------------------------------------*/
#define BUTTON_SENSOR_CONFIG_TYPE_INTERVAL      0x0100

#define BUTTON_SENSOR_VALUE_TYPE_LEVEL          0
#define BUTTON_SENSOR_VALUE_TYPE_PRESS_DURATION 1

#define BUTTON_SENSOR_PRESSED_LEVEL             0
#define BUTTON_SENSOR_RELEASED_LEVEL            8

#define BUTTON_SENSOR "su/pushbutton"

/*---------------------------------------------------------------------------*/
#endif /* BUTTON_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
