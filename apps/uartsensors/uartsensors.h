/*
 * protocol.h
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_
#define COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_

#include "contiki.h"
#ifndef NULL
#define NULL 0
#endif



void res_proxy_get_handler_tester();
void uartsensors_init(void);

extern process_event_t uartsensors_event;
PROCESS_NAME(uartsensors_server);

#endif /* COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_ */
