/*
 * bme688_app.c
 *
 *  Created on: 14 Mar 2023
 *      Author: jorda
 */

#include "bme688_app.h"

static void clear_step_data(bme688_step_data_t* data)
{
	data->gas_valid = 0;
	data->heat_stab = 0;
	data->temperature = 0;
	data->pressure = 0;
	data->humidity = 0;
	data->gas_resistance = 0;
}

static void clear_scan_data(bme688_scan_data_t* data)
{
	uint8_t i = 0;
	data->valid = 0;
	data->step_nb = 0;
	data->scan_counter = 0;
	for(i = 0; i < BME688_MAX_STEPS_NB; ++i) clear_step_data(&(data->steps[i]));
}

static void copy_configuration(bme688_measurement_configuration_t* src, bme688_measurement_configuration_t* dst)
{
	dst->heater_step_nb = src->heater_step_nb;
	for(uint16_t i = 0; i < BME688_MAX_STEPS_NB; ++i)
	{
		dst->temperatures[i] = src->temperatures[i];
		dst->steps_duration[i] = src->steps_duration[i];
	}
	dst->step_ms = src->step_ms;
	dst->pressure_os = src->pressure_os;
	dst->temperature_os = src->temperature_os;
	dst->humidity_os = src->humidity_os;
}

int bme688_app_init_parallel_mode(bme688_app_t* app, bme688_measurement_configuration_t* configuration)
{
	const uint16_t ambient_temperature = 25;
	uint8_t id = 0;

	// Check that the component works
	if (bme688_get_chip_id(&id) != 0) return -1;

	// Copy the configuration
	copy_configuration(configuration, &(app->measurement_configuration));

	// Set temperature, humidity and pressure oversampling
	if (bme688_set_oversampling(configuration->temperature_os,
			configuration->pressure_os,
			configuration->humidity_os) != 0) return -2;

	// Enable the gas conversion
	if (bme688_enable_disable_gas_conversion(1) != 0) return -3;

	// Get the factory calibration parameters
	// The factory calibration parameters are needed to convert the raw signals into real values
	if (bme688_get_calibration_parameters(&(app->calibration)) != 0) return -4;


//	// Set the heater profile
//	app->measurement_configuration.heater_step_nb = 10;
//	uint16_t temperatures [10] = {320, 100, 100, 100, 200, 200, 200, 320, 320, 320};
//	if (bme688_set_heater_profile_simple(temperatures, ambient_temperature, app->measurement_configuration.heater_step_nb, &(app->calibration)) != 0) return -5;

	// Set the heater profile
	if (bme688_set_heater_profile(&(app->measurement_configuration),
			ambient_temperature,
			&(app->calibration)) != 0) return -5;

	// Start parallel mode
	if(bme688_set_measurement_mode(BME688_MODE_PARALLEL) != 0) return -6;

	// Clear cache
	app->counter = 0;
	clear_scan_data(&(app->last_data));
	clear_scan_data(&(app->current_data));

	return 0;
}

/**
 * @brief Check if the scan data is complete (i.e. if every heater steps are valid)
 */
static uint8_t is_current_scan_data_complete(bme688_app_t* app)
{
	uint8_t i = 0;

	// Check for overflow
	if (app->measurement_configuration.heater_step_nb > BME688_MAX_STEPS_NB) return 0;

	for (i = 0; i < app->measurement_configuration.heater_step_nb; ++i)
	{
		if (app->current_data.steps[i].gas_valid != 1) return 0;
	}

	// Complete!
	return 1;
}

void bme688_copy_scan_data(bme688_scan_data_t* src, bme688_scan_data_t* dst)
{
	uint8_t i = 0;
	dst->valid = src->valid;
	dst->step_nb = src->step_nb;
	dst->scan_counter = src->scan_counter;
	for (i = 0; i < BME688_MAX_STEPS_NB; ++i)
	{
		dst->steps[i].gas_valid = src->steps[i].gas_valid;
		dst->steps[i].heat_stab = src->steps[i].heat_stab;
		dst->steps[i].temperature = src->steps[i].temperature;
		dst->steps[i].pressure = src->steps[i].pressure;
		dst->steps[i].humidity = src->steps[i].humidity;
		dst->steps[i].gas_resistance = src->steps[i].gas_resistance;
	}
}

/**
 * @brief Copy the content of current_data into last_data
 */
static void copy_current_to_last(bme688_app_t* app)
{
	bme688_copy_scan_data(&(app->current_data), &(app->last_data));
}

int bme688_app_do(bme688_app_t* app)
{
	const uint8_t BME688_DATA_FIELD_NUMBER = 3;
	uint8_t i = 0;

	for(i = 0; i < BME688_DATA_FIELD_NUMBER; ++i)
	{
		bme688_data_field_t data;
		if (bme688_read_data_field(i, &data) != 0) return -1;

		if (data.new_data == 0) continue;

		if (data.gas_meas_index > BME688_MAX_STEPS_NB) return -2;
		if (data.gas_meas_index > app->measurement_configuration.heater_step_nb) return -3;

		// Get pointer to the step
		bme688_step_data_t* step_data = &(app->current_data.steps[data.gas_meas_index]);

		// Get the flags
		step_data->gas_valid = data.gas_valid_r;
		step_data->heat_stab = data.heat_stab_r;

		// Compute the real value
		step_data->temperature = bme688_get_temperature(&data, &(app->calibration));
		step_data->pressure = bme688_get_pressure(&data, &(app->calibration));
		step_data->humidity = bme688_get_humidity(&data, &(app->calibration));
		step_data->gas_resistance = bme688_get_gas_resistance(&data, &(app->calibration));

		// Scan data completed? -> Copy
		if (is_current_scan_data_complete(app) != 0)
		{
			app->current_data.valid = 1;
			app->current_data.step_nb = app->measurement_configuration.heater_step_nb;
			app->current_data.scan_counter = app->counter;

			// Copy
			copy_current_to_last(app);

			// Clear
			clear_scan_data(&(app->current_data));
			app->counter = app->counter + 1;

			// New data is available
			return 0;
		}
	}

	return 1;
}

