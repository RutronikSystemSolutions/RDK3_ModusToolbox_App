/*
 * rutronik_application.h
 *
 *  Created on: 29 Mar 2023
 *      Author: jorda
 */

#ifndef RUTRONIK_APPLICATION_H_
#define RUTRONIK_APPLICATION_H_

#include <stdint.h>

#include "sgp40/sensirion_gas_index_algorithm.h"
#include "scd41/scd41_app.h"
#include "filter/lowpassfilter.h"
#include "pasco2/pasco2_app.h"

#ifdef BME688_SUPPORT
#include "bme688/bme688_app.h"
#endif

typedef struct
{
	uint8_t sensor_fusion_available;	/**< Store if the sensor fusion board is available (1) or not (0) */
	uint8_t co2_available;				/**< Store if the CO2 board is available (1) or not (0) */
	uint8_t ams_tof_available;			/**< Store if the AMS OSRAM TOF board is available (1) or not (0) */
	uint8_t um980_available;			/**< Store if the UM980 board is available (1) or not (0) */

	GasIndexAlgorithmParams gas_index_params;
	scd41_app_t scd41_app;
	pasco2_app_t pasco2_app;

	uint16_t prescaler;

#ifdef BME688_SUPPORT
	bme688_app_t bme688_app;
#endif

	lowpassfilter_t filtered_voltage;

} rutronik_application_t;

void rutronik_application_init(rutronik_application_t* app);

/**
 * @brief Generate a mask of the available sensor
 *
 * @retval
 * 		Bit 0: Sensor Fusion Board available (1) or not (0)
 * 		Bit 1: CO2 Board available (1) or not (0)
 */
uint32_t rutronik_application_get_available_sensors_mask(rutronik_application_t* app);


/**
 * @brief Change the mode of the TMF8828 sensor
 *
 * @param [in] mode TMF8828_MODE_3X3 (0) or TMF8828_MODE_8X8 (1)
 */
void rutronik_application_set_tmf8828_mode(rutronik_application_t* app, uint8_t mode);

/**
 * @brief Perform cyclic operation
 */
void rutronik_application_do(rutronik_application_t* app);

#endif /* RUTRONIK_APPLICATION_H_ */
