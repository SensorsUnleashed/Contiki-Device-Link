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
#ifndef RELAY_H_
#define RELAY_H_
/* -------------------------------------------------------------------------- */

/**
 * \name Relay default pin and port
 * @{
 */
#ifdef RELAY_CONF_PIN
#define RELAY_PIN        RELAY_CONF_PIN
#else
#define RELAY_PIN        4
#endif
#ifdef RELAY_CONF_PORT
#define RELAY_PORT       RELAY_CONF_PORT
#else
#define RELAY_PORT       GPIO_A_NUM
#endif
/** @} */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/**
 * \name Relay return types
 * @{
 */
#define RELAY_ERROR             (1)
#define RELAY_SUCCESS           0x00
/** @} */
/* -------------------------------------------------------------------------- */
#define RELAY_ACTUATOR "su/powerrelay"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor relay;
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
#endif /* RELAY_H_ */
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */
