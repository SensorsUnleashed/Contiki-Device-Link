/*******************************************************************************
 * Copyright (c) 2017, Ole Nissen.
 *  All rights reserved. 
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met: 
 *  1. Redistributions of source code must retain the above copyright 
 *  notice, this list of conditions and the following disclaimer. 
 *  2. Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer in the documentation and/or other materials provided
 *  with the distribution. 
 *  3. The name of the author may not be used to endorse or promote
 *  products derived from this software without specific prior
 *  written permission.  
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Sensors Unleashed project
 *******************************************************************************/
#include <string.h>

#include "contiki.h"

#include "lib/susensors.h"
#include "lib/memb.h"
#include "../pairing.h"
#include "rest-engine.h"
#include "er-coap-observe.h"

const char* strAbove = "/above";
const char* strBelow = "/below";
const char* strChange = "/change";

process_event_t susensors_event;
process_event_t susensors_pair;

PROCESS(susensors_process, "Sensors");

LIST(sudevices);
MEMB(sudevices_memb, susensors_sensor_t, DEVICES_MAX);

void initSUSensors(){
	list_init(sudevices);
	memb_init(&sudevices_memb);
}

susensors_sensor_t* addSUDevices(susensors_sensor_t* device){
	susensors_sensor_t* d;
	d = memb_alloc(&sudevices_memb);
	if(d == 0) return NULL;

	memcpy(d, device, sizeof(susensors_sensor_t));
	LIST_STRUCT_INIT(d, pairs);
	list_add(sudevices, d);

	return d;
}

/*---------------------------------------------------------------------------*/
susensors_sensor_t *
susensors_first(void)
{
	return list_head(sudevices);
}
/*---------------------------------------------------------------------------*/
susensors_sensor_t *
susensors_next(susensors_sensor_t* s)
{
	return list_item_next(s);
}
/*---------------------------------------------------------------------------*/
void
susensors_changed(susensors_sensor_t* s, uint8_t event)
{
	s->event_flag |= event;
	process_poll(&susensors_process);
}
/*---------------------------------------------------------------------------*/
susensors_sensor_t*
susensors_find(const char *prefix, unsigned short len)
{
	susensors_sensor_t* i;

	/* Search through all processes and search for the specified process
     name. */
	if(!len)
		len = strlen(prefix);

	for(i = susensors_first(); i; i = susensors_next(i)) {
		uint8_t su_url_len = strlen(i->type);

	    if((su_url_len == len
	        || (len > su_url_len
	            && (((struct resourceconf*)(i->data.config))->flags & HAS_SUB_RESOURCES)
	            && prefix[su_url_len] == '/'))
	       && strncmp(prefix, i->type, su_url_len-1) == 0) {

	    	return i;

	    }
	}
	return NULL;
}

static void notification_callback(coap_observee_t *obs, void *notification,
                      coap_notification_flag_t flag){

	  int len = 0;
	  const uint8_t *payload = NULL;

	  printf("Notification handler\n");
	  printf("Observee URI: %s\n", obs->url);
	  if(notification) {
	    len = coap_get_payload(notification, &payload);
	  }
	  switch(flag) {
	  case NOTIFICATION_OK:
	    printf("NOTIFICATION OK: %*s\n", len, (char *)payload);
	    break;
	  case OBSERVE_OK: /* server accepeted observation request */
	    printf("OBSERVE_OK: %*s\n", len, (char *)payload);
	    break;
	  case OBSERVE_NOT_SUPPORTED:
	    printf("OBSERVE_NOT_SUPPORTED: %*s\n", len, (char *)payload);
	    obs = NULL;
	    break;
	  case ERROR_RESPONSE_CODE:
	    printf("ERROR_RESPONSE_CODE: %*s\n", len, (char *)payload);
	    obs = NULL;
	    break;
	  case NO_REPLY_FROM_SERVER:
	    printf("NO_REPLY_FROM_SERVER: "
	           "removing observe registration with token %x%x\n",
	           obs->token[0], obs->token[1]);
	    obs = NULL;
	    break;
	  }
}

static void above_notificationcb(coap_observee_t *obs, void *notification,
                      coap_notification_flag_t flag){

	int len = 0;
	const uint8_t *payload = NULL;

	if(flag == NOTIFICATION_OK){

		if(notification) {
			len = coap_get_payload(notification, &payload);
		}

		susensors_sensor_t* this = (susensors_sensor_t*)obs->data;
		if(this->aboveEventhandler != 0){
			this->aboveEventhandler(this, len, payload);
		}

	}
	else{
		notification_callback(obs, notification, flag);
	}
}

static void below_notificationcb(coap_observee_t *obs, void *notification,
                      coap_notification_flag_t flag){
	int len = 0;
	const uint8_t *payload = NULL;

	if(flag == NOTIFICATION_OK){

		if(notification) {
			len = coap_get_payload(notification, &payload);
		}

		susensors_sensor_t* this = (susensors_sensor_t*)obs->data;
		if(this->belowEventhandler != 0){
			this->belowEventhandler(this, len, payload);
		}

	}
	else{
		notification_callback(obs, notification, flag);
	}

}

static void change_notificationcb(coap_observee_t *obs, void *notification,
                      coap_notification_flag_t flag){
	int len = 0;
	const uint8_t *payload = NULL;

	if(flag == NOTIFICATION_OK){

		if(notification) {
			len = coap_get_payload(notification, &payload);
		}

		susensors_sensor_t* this = (susensors_sensor_t*)obs->data;
		if(this->changeEventhandler != 0){
			this->changeEventhandler(this, len, payload);
		}

	}
	else{
		notification_callback(obs, notification, flag);
	}
}

void requestRegistration(joinpair_t* pair, susensors_sensor_t* this){

}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(susensors_process, ev, data)
{
	static int events;
	static susensors_sensor_t* d;

	PROCESS_BEGIN();

	susensors_event = process_alloc_event();
	susensors_pair = process_alloc_event();

	for(d = susensors_first(); d; d = susensors_next(d)) {
		d->event_flag = 0;
		d->configure(d, SUSENSORS_HW_INIT, 0);
		d->configure(d, SUSENSORS_ACTIVE, 1);

		//Restore sensor pairs stored in flash
		restore_SensorPairs(d);
	}

	while(1) {

		PROCESS_WAIT_EVENT();

		if(ev == susensors_pair){
			joinpair_t* pair = (joinpair_t*) data;
			susensors_sensor_t* this = (susensors_sensor_t*) pair->deviceptr;
			if(pair->triggers[0] != -1){	//Above
				coap_obs_request_registration(&pair->destip, UIP_HTONS(COAP_DEFAULT_PORT), pair->dsturlAbove,
						above_notificationcb, pair->deviceptr);
			}
			if(pair->triggers[1] != -1){	//Below
				coap_obs_request_registration(&pair->destip, UIP_HTONS(COAP_DEFAULT_PORT), pair->dsturlBelow,
						below_notificationcb, pair->deviceptr);
			}
			if(pair->triggers[2] != -1){	//Change
				coap_obs_request_registration(&pair->destip, UIP_HTONS(COAP_DEFAULT_PORT), pair->dsturlChange,
						change_notificationcb, pair->deviceptr);
			}
			if(this->setEventhandlers != NULL){
				this->setEventhandlers(this, pair->triggers);
			}
		}
		else {
			do {
			events = 0;
			for(d = susensors_first(); d; d = susensors_next(d)) {
				resource_t* resource = d->data.resource;
				if(resource != NULL){
					if(d->event_flag & SUSENSORS_CHANGE_EVENT){
						d->event_flag &= ~SUSENSORS_CHANGE_EVENT;
						coap_notify_observers_sub(resource, strChange);
					}
					if(d->event_flag & SUSENSORS_BELOW_EVENT){
						d->event_flag &= ~SUSENSORS_BELOW_EVENT;
						coap_notify_observers_sub(resource, strBelow);
					}
					if(d->event_flag & SUSENSORS_ABOVE_EVENT){
						d->event_flag &= ~SUSENSORS_ABOVE_EVENT;
						coap_notify_observers_sub(resource, strAbove);
					}
				}
				d->event_flag = SUSENSORS_NO_EVENT;
			}
		} while(events);
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

