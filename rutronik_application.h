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

/**
 * @def RUTRONIK_APP_PERIOD_MS
 * @brief Define the period at which the rutronik app function is called
 */
#define RUTRONIK_APP_PERIOD_MS	10

#define SGP41_CONDITIONING_DURATION_MS	10000
#define SGP41_CONDITIONING_PERIOD_MS	500
#define SGP41_MEASUREMENT_PERIOD_MS		1000
#define BMP585_MEASUREMENT_PERIOD_MS	100
#define DPS368_MEASUREMENT_PERIOD_MS	250
#define BMI323_MEASUREMENT_PERIOD_MS	100

typedef enum
{
	SGP41_CONDITIONING,
	SGP41_MEASUREMENT
} sgp41_state_e;

typedef struct
{
	uint8_t sensor_fusion_available;	/**< Store if the sensor fusion board is available (1) or not (0) */
	uint8_t co2_available;				/**< Store if the CO2 board is available (1) or not (0) */
	uint8_t ams_tof_available;			/**< Store if the AMS OSRAM TOF board is available (1) or not (0) */
	uint8_t um980_available;			/**< Store if the UM980 board is available (1) or not (0) */
	uint8_t vcnl4030x01_available;		/**< Store if the VNCL30x01 board is available (1) or not (0) */
	uint8_t rab7_available;

	GasIndexAlgorithmParams gas_index_voc_params;
	GasIndexAlgorithmParams gas_index_nox_params;

	scd41_app_t scd41_app;
	pasco2_app_t pasco2_app;

	uint16_t sht4x_prescaler;
	uint16_t sgp40_prescaler;
	uint16_t bmp581_prescaler;
	uint16_t scd41_prescaler;
	uint16_t battery_prescaler;
	uint16_t pasco2_prescaler;
	uint16_t dps310_prescaler;
	uint16_t bmi270_prescaler;
	uint16_t bme688_prescaler;
	uint16_t vcnl4030x01_prescaler;

	uint16_t bmm350_prescaler;

#ifdef BME688_SUPPORT
	bme688_app_t bme688_app;
#endif

	lowpassfilter_t filtered_voltage;

	sgp41_state_e sgp41_state;
	uint16_t sgp41_prescaler;

	uint16_t bme690_prescaler;
	uint16_t bmp585_prescaler;
	uint16_t dps368_prescaler;
	uint16_t bmi323_prescaler;

	float sht4x_temperature;	/**< Store last temperature (used for SGP41 compensation) */
	float sht4x_humidity;		/**< Store last humidity (used for SGP41 compensation) */

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
