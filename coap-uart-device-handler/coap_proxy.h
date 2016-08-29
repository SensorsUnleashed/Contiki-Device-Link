/*
 * protocol.h
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_
#define COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_

enum req_cmd {
	resource_count = 1,
	resource_url,

	debugstring,
};

#ifndef NULL
#define NULL 0
#endif

void proxy_init(void);
void req_resource(enum req_cmd cmd, void* payload, int len);
int req_received(enum req_cmd cmd, void* ret);

#endif /* COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_ */
