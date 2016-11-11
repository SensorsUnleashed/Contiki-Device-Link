/*
 * res-uartsensor.h
 *
 *  Created on: 16/10/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_RESOURCES_RES_UARTSENSOR_H_
#define COAP_UART_DEVICE_HANDLER_RESOURCES_RES_UARTSENSOR_H_

#include "uartsensors.h"

//We need to have a way to keep track of which sensor a notification belongs to
enum datatype_e{
	uartsensor,
	sensor
};
struct joinpair_s{
	struct mmem* url;
	enum datatype_e devicetype;
	void* deviceptr;
	uip_ip6addr_t destip;
};
typedef struct joinpair_s joinpair_t;

void res_uartsensors_activate(uartsensors_device_t* p);
joinpair_t* getUartSensorPair(uartsensors_device_t* p);

#endif /* COAP_UART_DEVICE_HANDLER_RESOURCES_RES_UARTSENSOR_H_ */
