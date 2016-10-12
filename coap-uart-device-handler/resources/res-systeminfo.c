/*
 * res-systeminfo.c
 *
 *  Created on: 07/10/2016
 *      Author: omn
 */

#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include "board.h"

static void res_sysinfo_gethandler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple actuator example. Toggles the red led */
RESOURCE(res_sysinfo,
         "title=\"Radio info\";rt=\"Info\"",
		 res_sysinfo_gethandler,
         NULL,
         NULL,
         NULL);

/*
 * Get the info about the radio:
 * 	IP address
 * 	rssi
 * 	MAC Address
 */
static void
res_sysinfo_gethandler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
	REST.set_header_max_age(response, 0);	//3 minutes

	int len = 0;
	//Pay attention to the max payload length
//	len = sprintf((char*)buffer, "Contiki version: %s\n", CONTIKI_VERSION_STRING);
	len += sprintf((char*)(buffer+len), "Board: %s\n", BOARD_STRING);
	len += sprintf((char*)(buffer+len), "Version: 0.0.1\n");
	REST.set_response_payload(response, buffer, len);
}

