/*
 * dps310_app.c
 *
 *  Created on: 1 Aug 2023
 *      Author: jorda
 */

#include "dps310_app.h"

enum dps310_app_status
{
	WAITING_SENSOR_READY,
	WAITING_MEASUREMENT_READY
};

typedef struct
{
	enum dps310_app_status status;
	dps3xx_cal_coeff_regs_t coeffs;
	dps3xx_configuration_t config;
	uint8_t coef_temperature_src;
	int32_t scaling_coef;
	float last_temperature;
	float last_pressure;
} dps310_internal_t;

static dps310_internal_t internal_state;

void dps310_app_init_i2c_interface(dps310_read_func_t read, dps310_write_func_t write)
{
	dps310_init_i2c_interface(read, write);
}

void dps310_app_init()
{
	internal_state.status = WAITING_SENSOR_READY;
}

static int init_sensor()
{
	const uint8_t oversampling = 1; // x2
	const uint8_t rate = 3; // 8 measurements per sec
	const uint8_t measurement_mode = 7; // continous pressure and temperature
	int retval = dps310_read_calibration_regs(&internal_state.coeffs);
	if (retval != 0) return -1;

	retval = dps310_read_configuration(&internal_state.config);
	if (retval != 0) return -2;

	retval = dps310_read_temperature_coefficient_src(&internal_state.coef_temperature_src);
	if (retval != 0) return -3;

	retval = dps310_correct_temperature();
	if (retval != 0) return -4;

	retval = dps310_set_temperature_configuration(internal_state.coef_temperature_src, oversampling, rate);
	if (retval != 0) return -5;

	retval = dps310_set_pressure_configuration(oversampling, rate);
	if (retval != 0) return -6;

	internal_state.scaling_coef = dps310_get_scaling_coef(oversampling);

	retval = dps310_set_measurement_mode(measurement_mode);
	if (retval != 0) return -7;

	return 0;
}

int dps310_app_do()
{
	switch(internal_state.status)
	{
		case WAITING_SENSOR_READY:
		{
			uint8_t sensor_rdy = 0;
			uint8_t coef_rdy = 0;
			int retval = dps310_get_sensor_rdy(&sensor_rdy, &coef_rdy);
			if (retval != 0) return -1;

			if ((sensor_rdy != 0) && (coef_rdy != 0))
			{
				// Sensor is ready, initialize it
				if (init_sensor() != 0) return -1;
				internal_state.status = WAITING_MEASUREMENT_READY;
			}
			return 1;
		}

		case WAITING_MEASUREMENT_READY:
		{
			uint8_t pressure_ready = 0;
			uint8_t temperature_ready = 0;
			int retval = dps310_is_measurement_ready(&pressure_ready, &temperature_ready);
			if (retval != 0) return -1;
			if ((pressure_ready != 0) && (temperature_ready != 0))
			{
				uint8_t raw_values[DPS310_RAW_VALUES_LENGTH];
				retval = dps310_read_raw_values(raw_values);

				float temp_scaled = 0;
				internal_state.last_temperature = dps310_convert_temperature(raw_values, &internal_state.coeffs, internal_state.scaling_coef, &temp_scaled);

				// x100 => Convert to Pa
				internal_state.last_pressure = 100.f * dps310_convert_pressure(raw_values, &internal_state.coeffs, internal_state.scaling_coef, temp_scaled);
				return 0;
			}
			return 1;
		}
	}

	// Should never happen
	return -1;
}


void dps310_app_get_last_values(float* temperature, float* pressure)
{
	*temperature = internal_state.last_temperature;
	*pressure = internal_state.last_pressure;
}
