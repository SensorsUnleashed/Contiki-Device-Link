/*
 * res-uartsensor.h
 *
 *  Created on: 16/10/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_RESOURCES_RES_UARTSENSOR_H_
#define COAP_UART_DEVICE_HANDLER_RESOURCES_RES_UARTSENSOR_H_

#include "uartsensors.h"

void res_uartsensors_event(uartsensors_device_t* p);
void res_uartsensors_activate(uartsensors_device_t* p);

#endif /* COAP_UART_DEVICE_HANDLER_RESOURCES_RES_UARTSENSOR_H_ */
