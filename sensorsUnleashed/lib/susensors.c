#include <string.h>

#include "contiki.h"

#include "lib/susensors.h"
#include "lib/memb.h"
#include "../pairing.h"

#define FLAG_CHANGED    0x80

process_event_t susensors_event;

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
susensors_changed(susensors_sensor_t* s)
{
	s->event_flag |= FLAG_CHANGED;
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
		if(strncmp(prefix, i->type, len) == 0) {
			return i;
		}
	}
	return NULL;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(susensors_process, ev, data)
{
	static int events;
	static susensors_sensor_t* d;
	static joinpair_t *pair = 0;
	PROCESS_BEGIN();

	susensors_event = process_alloc_event();

	for(d = susensors_first(); d; d = susensors_next(d)) {
		d->event_flag = 0;
		d->configure(d, SUSENSORS_HW_INIT, 0);
		d->configure(d, SUSENSORS_ACTIVE, 1);

		//Restore sensor pairs stored in flash
		restore_SensorPairs(d);
	}

	while(1) {

		PROCESS_WAIT_EVENT();

		do {
			events = 0;
			for(d = susensors_first(); d; d = susensors_next(d)) {
				if(d->event_flag & FLAG_CHANGED){
					for(pair = list_head(d->pairs); pair; pair = pair->next) {
						if(process_post(PROCESS_BROADCAST, susensors_event, (void *)pair) == PROCESS_ERR_OK) {
							PROCESS_WAIT_EVENT_UNTIL(ev == susensors_event);
						}
					}
					d->event_flag &= ~FLAG_CHANGED;
					events++;
				}
			}
		} while(events);
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
