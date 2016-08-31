/*
 * protocol.h
 *
 *  Created on: 13/08/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_
#define COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_

/*
 * Toggle resource
 * char *path		: "actuators/toggle"
 *
 * name				:	res_toggle	(variable name)
 * attributes		:	"title=\"Red LED\";rt=\"Control\""
 * get_handler		:	NULL
 * post_handler		:	res_post_handler
 * put_handler		:	NULL
 * delete_handler	:	NULL
 *
 * #define RESOURCE(name, attributes, get_handler, post_handler, put_handler, delete_handler) \
  resource_t name = { NULL, NULL, NO_FLAGS, attributes, get_handler, post_handler, put_handler, delete_handler, { NULL } }
 *
 * RESOURCE(res_toggle,
         "title=\"Red LED\";rt=\"Control\"",
         NULL,
         res_post_handler,
         NULL,
         NULL);

	EVENT_RESOURCE(res_event,
               "title=\"Event demo\";obs",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);
 * */

enum req_cmd {
	resource_count = 1,
	resource_url,

	debugstring,
};

#define END     0xC0
#define ESC     0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

#define ACK     0x06
#define NAK     0x15

//This struct contains the raw messages.
typedef struct  {
	struct resource *next;
	unsigned char seqno;
	enum req_cmd cmd;
	char len;
	void* payload;
}rx_msg;

#ifndef NULL
#define NULL 0
#endif

void proxy_init(void);
void req_resource(enum req_cmd cmd, void* payload, int len);
int req_received(enum req_cmd cmd, void* ret);

#endif /* COAP_UART_DEVICE_HANDLER_COAP_PROXY_H_ */
