#include "ledindicator.h"
#include "contiki.h"
#include "lib/sensors.h"
#include "dev/leds.h"

#include "../../apps/uartsensors/uart_protocolhandler.h"
#include "rest-engine.h"
#include "susensorcommon.h"

const struct sensors_sensor ledindicator;
static struct resourceconf config = {
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

static int set(int type, void* data){
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
		sensors_changed(&ledindicator);
	}

	return ret;
}

static int configure(int type, int value){
	return 0;
}

int get(int type, void* data){
	int ret = 1;
	cmp_object_t* obj = (cmp_object_t*)data;

	if((enum up_parameter) type == ActualValue){
		obj->type = CMP_TYPE_UINT8;
		obj->as.u8 = leds_get() & LEDS_CONF_ALL;
		ret = 0;
	}
	else if(type >= (int)ChangeEventConfigValue){
		ret = su_sensorvalue(type, obj, &config);
	}
	return ret;
}


static struct extras extra = { .type = 1, .data = (void*)&config };
SENSORS_SENSOR(ledindicator, LED_INDICATOR, set, configure, get, &extra);
