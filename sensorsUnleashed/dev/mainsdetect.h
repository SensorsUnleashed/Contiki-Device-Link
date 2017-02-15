/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-relay Generic relay driver
 *
 * Driver for a generic relay driver
 * @{
 *
 * \file
 * Header file for the generic relay driver
 */
/*---------------------------------------------------------------------------*/
#ifndef MAINSDETECT_H_
#define MAINSDETECT_H_
/* -------------------------------------------------------------------------- */

#include "lib/susensors.h"

extern struct resourceconf mainsdetectconfig;

susensors_sensor_t* addASUMainsDetector(const char* name, struct resourceconf* config);
/**
 * \name Mainsdetect default pin and port
 * @{
 */
#ifdef MAINSDETECT_CONF_PIN
#define MAINSDETECT_PIN        MAINSDETECT_CONF_PIN
#else
#define MAINSDETECT_PIN        2
#endif
#ifdef RELAY_CONF_PORT
#define MAINSDETECT_PORT       MAINSDETECT_CONF_PORT
#else
#define MAINSDETECT_PORT       GPIO_A_NUM
#endif

/** @} */

/** @} */
/* -------------------------------------------------------------------------- */
#define MAINSDETECT_ACTUATOR "su/mainsdetect"
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
#endif /* MAINSDETECT_H_ */
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */
