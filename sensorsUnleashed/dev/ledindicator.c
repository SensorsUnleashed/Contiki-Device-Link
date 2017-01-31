#include "ledindicator.h"
#include "contiki.h"
#include "dev/leds.h"

#include "../../apps/uartsensors/uart_protocolhandler.h"
#include "rest-engine.h"
#include "susensorcommon.h"
#include "board.h"

struct resourceconf ledindicatorconfig = {
		.resolution = 1,
		.version = 1,
		.flags = METHOD_GET | METHOD_PUT,
		.max_pollinterval = 2,
		.eventsActive = 0,
		.AboveEventAt = {
				.type = CMP_TYPE_UINT8,
				.as.u8 = 1
		},
		.BelowEventAt = {
				.type = CMP_TYPE_UINT8,
				.as.u8 = 0
		},
		.ChangeEvent = {
				.type = CMP_TYPE_UINT8,
				.as.u8 = 1
		},
		.RangeMin = {
				.type = CMP_TYPE_UINT16,
				.as.u8 = 0
		},
		.RangeMax = {
				.type = CMP_TYPE_UINT16,
				.as.u8 = 1
		},

		.unit = "",
		.spec = "LED indicator",
		.type = LED_INDICATOR,
		.attr = "title=\"LED indicator\" ;rt=\"Indicator\"",
};

static int set(struct susensors_sensor* this, int type, void* data){
	int ret = 1;

	if(type == toggleLED_RED){
		leds_toggle(LEDS_RED);
		ret = 0;
	}
	else if(type == toggleLED_GREEN){
		leds_toggle(LEDS_GREEN);
		ret = 0;
	}
	else if(type == toggleLED_ORANGE){
		leds_toggle(LEDS_ORANGE);
		ret = 0;
	}
	else if(type == toggleLED_YELLOW){
		leds_toggle(LEDS_YELLOW);
		ret = 0;
	}

	if(ret == 0){
		susensors_changed(this);
	}

	return ret;
}

static int configure(struct susensors_sensor* this, int type, int value){
	return 0;
}

int get(struct susensors_sensor* this, int type, void* data){
	int ret = 1;
	cmp_object_t* obj = (cmp_object_t*)data;

	if((enum up_parameter) type == ActualValue){
		obj->type = CMP_TYPE_UINT8;
		obj->as.u8 = leds_get() & LEDS_CONF_ALL;
		ret = 0;
	}
	return ret;
}

/* An event was received from another device - now act on it */
static int eventHandler(struct susensors_sensor* this, int type, int len, uint8_t* payload){
	enum susensors_event_cmd cmd = (enum susensors_event_cmd)type;
	int ret = 1;
	switch(cmd){
	case SUSENSORS_ABOVE_EVENT_SET:
		this->value(this, toggleLED_RED, NULL);
		ret = 0;
		break;
	case SUSENSORS_BELOW_EVENT_SET:
		this->value(this, toggleLED_YELLOW, NULL);
		ret = 0;
		break;
	case SUSENSORS_CHANGE_EVENT_SET:
		this->value(this, toggleLED_ORANGE, NULL);
		ret = 0;
		break;
	}
	return ret;
}

static int getActiveEventMsg(struct susensors_sensor* this, const char** eventstr, uint8_t* payload){
	return 1;
}

susensors_sensor_t* addASULedIndicator(const char* name, struct resourceconf* config){
	susensors_sensor_t d;
	d.type = (char*)name;
	d.status = get;
	d.value = set;
	d.configure = configure;
	d.eventhandler = eventHandler;
	d.getActiveEventMsg = getActiveEventMsg;
	d.suconfig = suconfig;
	d.data.config = config;
	d.data.runtime = (void*)0;

	return addSUDevices(&d);
}
