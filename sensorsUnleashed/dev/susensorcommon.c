/*
 * susensorcommon.c
 *
 *  Created on: 25/12/2016
 *      Author: omn
 */
#include "../../apps/uartsensors/uart_protocolhandler.h"

int su_sensorvalue(int type, cmp_object_t* obj, struct resourceconf* config){
	int ret = 1;

	switch((enum up_parameter) type){
	case ActualValue:
		/* Implemented at sensor level */
		ret = 1;
		break;
	case ChangeEventConfigValue:
		*obj = config->ChangeEvent;
		ret = 0;
		break;
	case AboveEventConfigValue:
		*obj = config->AboveEventAt;
		ret = 0;
		break;
	case BelowEventConfigValue:
		*obj = config->BelowEventAt;
		ret = 0;
		break;
	case RangeMinValue:
		*obj = config->RangeMin;
		ret = 0;
		break;
	case RangeMaxValue:
		*obj = config->RangeMax;
		ret = 0;
		break;
	case EventState:
		obj->type = CMP_TYPE_UINT8;
		obj->as.u8 = config->eventsActive;
		ret = 0;
		break;
	}
	return ret;
}
