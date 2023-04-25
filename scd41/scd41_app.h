/*
 * scd41_app.h
 *
 *  Created on: 23 Mar 2023
 *      Author: jorda
 */

#ifndef SCD41_SCD41_APP_H_
#define SCD41_SCD41_APP_H_


#include "scd41.h"

typedef struct
{
	uint16_t co2_ppm;
	float temperature;
	float humidity;
} scd41_measurement_value_t;

typedef struct
{
	uint8_t i2c_initialised;
	scd41_measurement_value_t value;
	uint32_t counter;
} scd41_app_t;

void scd41_app_init(scd41_app_t* app, scd41_read_func_t read, scd41_write_func_t write, scd41_sleep_func_t sleep);

int scd41_app_initialise_and_start_measurement(scd41_app_t* app);

/**
 * @brief Cyclic call to the sensor
 *
 * @retval 0 New value is available (stored inside the app structure)
 * @retval 1 No new value was available - Need to wait
 * @retval -1 Error occurred
 */
int scd41_app_do(scd41_app_t* app);

void scd41_app_copy_measurement_value(scd41_measurement_value_t* src, scd41_measurement_value_t* dst);

#endif /* SCD41_SCD41_APP_H_ */
