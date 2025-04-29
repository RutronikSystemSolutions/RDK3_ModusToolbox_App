/*
 * bme688_app.h
 *
 *  Created on: 14 Mar 2023
 *      Author: jorda
 */

#ifndef BME688_BME688_APP_H_
#define BME688_BME688_APP_H_

#include "bme688.h"

#define BME688_STEP_DATA_LEN 18

/**
 * 6 + 10 * 18 => 186 bytes
 */
#define BME688_SCAN_DATA_LEN (6 + BME688_MAX_STEPS_NB * BME688_STEP_DATA_LEN)

/**
 * Represent one heat step of the BME688
 * Total size is 18 bytes
 */
typedef struct
{
	uint8_t gas_valid;		/**< 1 */
	uint8_t heat_stab;		/**< 1 + 1	=> 2 */
	float temperature;		/**< 4 + 2	=> 6 */
	float pressure;			/**< 4 + 6	=> 10 */
	float humidity;			/**< 4 + 10	=> 14 */
	float gas_resistance;	/**< 4 + 14	=> 18 */
} bme688_step_data_t;

typedef struct
{
	uint8_t valid;			/**< Store if data is valid or not - 0 not valid - 1 byte */
	uint8_t step_nb;		/**< Number of steps per scan - 1 byte */
	uint32_t scan_counter;	/**< Index of the scan counter - 4 bytes */
	bme688_step_data_t steps[BME688_MAX_STEPS_NB];	/**< BME688_MAX_STEPS_NB * 18 bytes */
} bme688_scan_data_t;

typedef struct
{
	bme688_calibration_parameters_t calibration; /**< Store calibration parameters needed to convert raw values to real values */
	bme688_measurement_configuration_t measurement_configuration;
	uint32_t counter;
	bme688_scan_data_t last_data;
	bme688_scan_data_t current_data;
} bme688_app_t;

/**
 * @brief Check if the BME688 is available or not
 *
 * @retval 0 Not available
 * @retval 1 Available
 */
int bme688_app_is_available();

/**
 * Initialize the BME688 in parallel mode
 */
int bme688_app_init_parallel_mode(bme688_app_t* app, bme688_measurement_configuration_t* configuration);

/**
 * Cyclic call
 *
 * @retval 0 New data is available
 * @retval 1 Everything OK but no data available
 * @retval < 0 Error happened
 */
int bme688_app_do(bme688_app_t* app);

/**
 * @brief Copy scan data from src to dst
 */
void bme688_copy_scan_data(bme688_scan_data_t* src, bme688_scan_data_t* dst);

#endif /* BME688_BME688_APP_H_ */
