/*
 * scd41_app.c
 *
 *  Created on: 23 Mar 2023
 *      Author: jorda
 */

#include "scd41_app.h"

static const uint8_t I2C_INITIALISED = 1;

void scd41_app_init(scd41_app_t* app, scd41_read_func_t read, scd41_write_func_t write, scd41_sleep_func_t sleep)
{
	scd41_init(read, write, sleep);
	app->i2c_initialised = I2C_INITIALISED;
}

int scd41_app_initialise_and_start_measurement(scd41_app_t* app)
{
	if (app->i2c_initialised != I2C_INITIALISED) return -1;

	// Wake-up might fail in case of software restart
	scd41_wake_up();

	int retval = scd41_stop_periodic_measurement();
	if (retval != 0) return -3;

	retval = scd41_reinit();
	if (retval != 0) return -4;

	scd41_serial_id_t scd41_id;
	retval = scd41_get_serial_number(&scd41_id);
	if (retval != 0) return -5;

	retval = scd41_start_periodic_measurement();
	if (retval != 0) return -6;

	// Initializes internal values
	app->counter = 0;
	app->value.co2_ppm = 0;
	app->value.humidity = 0;
	app->value.temperature = 0;

	return 0;
}

int scd41_app_do(scd41_app_t* app)
{
	uint8_t ready_flag = 0;
	int retval = scd41_get_data_ready_status(&ready_flag);
	if (retval != 0) return -1;

	if (ready_flag)
	{
		float humidity = 0;
		float temperature = 0;
		uint16_t co2_ppm = 0;
		retval = scd41_read_measurement(&co2_ppm, &temperature, &humidity);
		if (retval != 0) return -2;

		// Copy and increment
		app->value.co2_ppm = co2_ppm;
		app->value.temperature = temperature;
		app->value.humidity = humidity;
		app->counter = app->counter + 1;

		return 0;
	}

	// Was not ready...
	return 1;
}

void scd41_app_copy_measurement_value(scd41_measurement_value_t* src, scd41_measurement_value_t* dst)
{
	dst->co2_ppm = src->co2_ppm;
	dst->humidity = src->humidity;
	dst->temperature = src->temperature;
}
