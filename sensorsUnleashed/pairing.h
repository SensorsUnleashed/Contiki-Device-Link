/*
 * pairing.h
 *
 *  Created on: 12/11/2016
 *      Author: omn
 */

#ifndef SENSORSUNLEASHED_PAIRING_H_
#define SENSORSUNLEASHED_PAIRING_H_

#include "contiki.h"
#include "mmem.h"
#include "net/ip/uiplib.h"
#include "uartsensors.h"
#include "lib/list.h"

//We need to have a way to keep track of which sensor a notification belongs to
enum datatype_e{
	uartsensor,
	sensor
};
struct joinpair_s{
	struct joinpair_s *next;	/* for LIST, points to next resource defined */

	struct mmem dsturl;
	struct mmem srcurl;	//Used only to determine at boot, which resource we are a pair of.
	enum datatype_e devicetype;
	void* deviceptr;
	uip_ip6addr_t destip;
};

typedef struct joinpair_s joinpair_t;

list_t pairing_get_pairs(void);
joinpair_t* getUartSensorPair(uartsensors_device_t* p);
void activateUartSensorPairing(uartsensors_device_t* p);

uint8_t pairing_assembleMessage(const uint8_t* data, uint32_t len, uint32_t num);
uint8_t pairing_handle(void* resource, enum datatype_e restype);
void store_SensorPair(uint8_t* data, uint32_t len);
void restore_SensorPairs(void);



#endif /* SENSORSUNLEASHED_PAIRING_H_ */