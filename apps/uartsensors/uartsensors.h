/*
 * protocol.h
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_
#define COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_

#include "contiki.h"
#include "uart_protocolhandler.h"
#ifndef NULL
#define NULL 0
#endif

#define MAX_RESOURCES	20

//This is the type we use, so that we can find the resource on the sensor
struct proxy_resource{
	struct proxy_resource *next;	/* for LIST, points to next resource defined */
	struct resourceconf conf;
	uint8_t* lastval;				/* We use LASTVALSIZE bytes to store the last non-decoded reading */
	uint8_t vallen;
};
typedef struct proxy_resource uartsensors_device_t;

uartsensors_device_t *uartsensors_find(const char *name, int len);
void uartsensors_getEventVal(uartsensors_device_t* this, enum up_parameter parameter);
int uartsensors_setEventVal(uartsensors_device_t* this, enum up_parameter parameter, cmp_object_t val);
void uartsensors_reset(uartsensors_device_t* this);
void uartsensors_init(void);

extern process_event_t uartsensors_event;

PROCESS_NAME(uartsensors_server);

#endif /* COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_ */
