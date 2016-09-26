/*
 * coap_proxy_protocolhandler.h
 *
 *  Created on: 18/09/2016
 *      Author: omn
 */

#ifndef COAP_UART_DEVICE_HANDLER_COAP_PROXY_PROTOCOLHANDLER_H_
#define COAP_UART_DEVICE_HANDLER_COAP_PROXY_PROTOCOLHANDLER_H_

#include "contiki.h"
#include "cmp.h"

//TODO: When implementation is done, simplify these commands
enum req_cmd {
	resource_count = 1,		//Get the total no of resources
	resource_group,			//Part of url
	resource_type,			//Part of url
    resource_attributes,    //Get a specific resource attribute
	resource_spec,			//Get the specification of the device
	resource_unit,			//Get the unit, human readable
	resource_flags,			//Get the flags for a specific resource
	resource_updateinterval,//Get how often values can be requested from the device
	resource_config,
	resource_get,			//Get the data for a specific resource
	resource_value_update,			//A resource autonomously postes its value
	debugstring,
};

//This struct contains the raw messages.
typedef struct  {
	unsigned char seqno;
	enum req_cmd cmd;
	char len;
	void* payload;
}rx_msg;

struct pl_res_post{
	char resource_id;
	char data[10];	//msgpacked buffer
};

//void* lastval;			//MsgPacked value. Actual SI value is calculated as: Value * 1/resolution = X [Unit].
struct resourceconf{
	uint8_t id;
	uint32_t resolution;	//1/resoltion of a unit. eg. if resolution is 1000 and unit is C, then each LSB equals 0.001C
	uint32_t hysteresis;	//Same as resolution
	uint32_t version;		//Q22.10
	uint8_t flags;			//Flags - which handle do we serve: Get/Post/PUT/Delete
	int32_t max_pollinterval;	//How often can you ask for a new reading
	char* unit;				//SI unit - can be preceeded with mC for 1/1000 of a C. In that case the resolution should be 1;
	char* spec;				//Human readable spec of the sensor
	char* type;				//Can be button, sensor, motor, relay etc.
	char* group;			//Can be actuator, sensor, kitchen etc
	char* attr;				//Attributes served to the coap clients

	//uint8_t notation;		//Qm.f => MMMM.FFFF	eg. Q1.29 = int32_t with 1 integer bit and 29 fraction bits, Q32 = uint32_t = 32 positive integer bits. Q31 is a int32_t
};

/*
 * resource{
 * 	int lastval	- get: Last value that the radio has in its database.
 * 	char* spec	- get: Explain what it is, and its resolution (Read from sensor)
 * 	char* type	- get: Is it a button, temperature sensor etc (Read from sensor)
 * 	char* group - get/set(radio only): default is sensor or actuator (Read from sensor)
 * 	int hysteresis	- get/set. How much of a difference before an event.
 * 					  eg. 10 for tempsensor will give an event when temperaure changes more than 0.1C since last update
 * 					  eg. 1 for an on/off actuator, or >1 if we never want an event (Will disable an on/off actuator)
 * 	int updateinterval	get/set: Default 0, how often to update subscribers with the current resource value
 *	char* unit - C for Celcius, g for gram etc.
 *	int resolution	:get 1/1000 * resolution. 1 = 0.001, 1000 = 1, 10000 = 10 (32bit = 31 bit without sign eg. a max value of 536870.912 - -536870.912
 *	int version		: x.xxx, like with the resolution.
 * }
 * Capabilities:
 * Sensor:
 * 		Button:
 * 			Auto resond when state changes:
 * 				1 = ON
 * 				0 = OFF
 * 			Get state
 * 		Temperature:
 * 			Auto respond when temp changed more than a setpoint
 * 			Temp is send as int, with 10000 meaning 100.00C
 *
 * 	Actuator
 * 		Relay:
 * 			Get state
 * 				1 = ON  (Coil is engaged)
 * 				0 = OFF (Coil is not engaged)
 *
 * */


int cp_decodemessage(char* source, int len, rx_msg* destination);
int cp_encodemessage(uint8_t msgid, enum req_cmd cmd, void* payload, char len, uint8_t* buffer);

//Msgpack encode the configuration
int cp_encoderesource_conf(struct resourceconf* data, uint8_t* buffer);
int cp_encodereading(uint8_t* buffer, cmp_object_t *obj);
int cp_decoderesource_conf(struct resourceconf* data, uint8_t* buffer, char* strings);
int cp_decodeReadings(uint8_t* buffer, char* conv, int* len);

#endif /* COAP_UART_DEVICE_HANDLER_COAP_PROXY_PROTOCOLHANDLER_H_ */
