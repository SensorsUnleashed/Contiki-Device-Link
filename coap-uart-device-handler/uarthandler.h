/*
 * uart_handler.h
 *
 *  Created on: 08/09/2016
 *      Author: omn
 */
#ifndef COAP_UART_DEVICE_HANDLER_UARTHANDLER_H_
#define COAP_UART_DEVICE_HANDLER_UARTHANDLER_H_
#include "contiki.h"

#define RX_BUFLEN	150
typedef struct {
	char buffer[RX_BUFLEN];
	char* wrt_ptr;
}buffer_t;

void cp_uarthandler_init(void (* callback)(buffer_t* msg));
unsigned int frameandsend(const unsigned char *s, unsigned int len);

#endif /* COAP_UART_DEVICE_HANDLER_UARTHANDLER_H_ */
