/*
 * coap_proxy_test.h
 *
 *  Created on: 18/09/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_COAP_PROXY_TEST_H_
#define COAP_UART_DEVICE_HANDLER_COAP_PROXY_TEST_H_


void cp_uarthandler_init();
void uart_write_byte(uint8_t uart, uint8_t b);
void uart_set_input(uint8_t uart, int (* input)(unsigned char c));

#endif /* COAP_UART_DEVICE_HANDLER_COAP_PROXY_TEST_H_ */
