/*
 * protocol.h
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_PROTOCOL_H_
#define COAP_UART_DEVICE_HANDLER_PROTOCOL_H_

#include "dev/uart.h"

enum req_cmd {
	resource_count = 1,
	resource_url
};

void protocol_init(void);
int uart_rx_callback(unsigned char c);
void req_resource(enum req_cmd cmd);
int req_received(enum req_cmd cmd);

#endif /* COAP_UART_DEVICE_HANDLER_PROTOCOL_H_ */
