/*
 * bme688.c
 *
 *  Created on: 13 Mar 2023
 *      Author: jorda
 */

#include "bme688.h"

static const uint8_t BME688_I2C_ADDR = 0x76;

static const uint8_t BME688_PAR_G2_REG = 0xEB;
static const uint8_t BME688_RES_HEAT_VAL_REG = 0x00;
static const uint8_t BME688_RES_HEAT_RANGE_REG = 0x02;
static const uint8_t BME688_VARIANT_ID_REG = 0xF0;
static const uint8_t BME688_CHIP_ID_REG = 0xD0;
static const uint8_t BME688_CTRL_GAS_0_REG = 0x71;
static const uint8_t BME688_CTRL_MEAS_REG = 0x74;
static const uint8_t BME688_PAR_T1_REG = 0xE9;
static const uint8_t BME688_PAR_T2_REG = 0x8A;
static const uint8_t BME688_PAR_T3_REG = 0x8C;
static const uint8_t BME688_PAR_P1_REG = 0x8E;
static const uint8_t BME688_PAR_P4_REG = 0x94;
static const uint8_t BME688_PAR_P8_REG = 0x9C;
static const uint8_t BME688_PAR_H2_REG = 0xE1;
static const uint8_t BME688_CTRL_HUM_REG = 0x72;
static const uint8_t BME688_GAS_WAIT_SHARED_REG = 0x6E;
static const uint8_t BME688_GAS_WAIT_X_REG = 0x64;
static const uint8_t BME688_RES_HEAT_X_REG = 0x5A;
static const uint8_t BME688_MEAS_STATUS_REG = 0x1D;

static bme68x_read_func_t i2c_read_bytes;
static bme68x_write_func_t i2c_write_bytes;


void bme688_init_i2c_interface(bme68x_read_func_t read, bme68x_write_func_t write)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;
}

int bme688_get_chip_id(uint8_t* chip_id)
{
	int8_t result = 0;
	uint8_t cmd = BME688_CHIP_ID_REG;

	// Send it
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, chip_id, sizeof(uint8_t));
	if (result != 0) return -2;

	// Plausibility (cannot be 0)
	// According to the datasheet, should be 0x61
	if (*chip_id == 0) return -3;

	return 0;
}

int bme688_get_variant_id(uint8_t* variant_id)
{
	int8_t result = 0;
	uint8_t cmd = BME688_VARIANT_ID_REG;

	// Send it
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, variant_id, sizeof(uint8_t));
	if (result != 0) return -2;

	return 0;
}

static int bme688_get_heat_res_calibration_parameters(bme688_calibration_parameters_t* parameters)
{
	int8_t result = 0;
	uint8_t cmd = BME688_PAR_G2_REG;
	uint8_t buffer[4] = {0};

	// Send it
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, sizeof(buffer));
	if (result != 0) return -2;

	// Extract
	parameters->par_g1 = (int8_t) buffer[2];
	parameters->par_g2 = (int16_t) (((uint16_t)buffer[1] << 8) | (uint16_t) buffer[0]);
	parameters->par_g3 = (int8_t) buffer[3];

	// Get res_heat_val
	cmd = BME688_RES_HEAT_VAL_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -3;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, sizeof(uint8_t));
	if (result != 0) return -4;

	// Extract
	parameters->res_heat_val = (int8_t) buffer[0];

	// Get res_heat_range
	cmd = BME688_RES_HEAT_RANGE_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -5;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, sizeof(uint8_t));
	if (result != 0) return -6;

	// Extract
	parameters->res_heat_range = (buffer[0] >> 4) & 3;

	return 0;
}

static int bme688_get_temperature_calibration_parameters(bme688_calibration_parameters_t* parameters)
{
	int8_t result = 0;
	uint8_t cmd = 0;
	uint8_t buffer[2] = {0};

	// Get par_t1
	cmd = BME688_PAR_T1_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, sizeof(uint16_t));
	if (result != 0) return -2;

	// Extract
	parameters->par_t1 = (uint16_t) (((uint16_t)buffer[1] << 8) | (uint16_t) buffer[0]);

	// Get par_t2
	cmd = BME688_PAR_T2_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -3;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, sizeof(uint16_t));
	if (result != 0) return -4;

	// Extract
	parameters->par_t2 = (int16_t) (((uint16_t)buffer[1] << 8) | (uint16_t) buffer[0]);

	// Get par_t3
	cmd = BME688_PAR_T3_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -5;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, sizeof(int8_t));
	if (result != 0) return -6;

	// Extract
	parameters->par_t3 = (int8_t) buffer[0];

	return 0;
}

static int bme688_get_pressure_calibration_parameters(bme688_calibration_parameters_t* parameters)
{
	int8_t result = 0;
	uint8_t cmd = 0;
	uint8_t buffer[6] = {0};

	// Get par_p1 -> par_p3
	cmd = BME688_PAR_P1_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, 5);
	if (result != 0) return -2;

	// Extract
	parameters->par_p1 = (uint16_t) (((uint16_t)buffer[1] << 8) | (uint16_t) buffer[0]);
	parameters->par_p2 = (int16_t) (((uint16_t)buffer[3] << 8) | (uint16_t) buffer[2]);
	parameters->par_p3 = (int8_t) buffer[4];

	// Get par_p4 -> par_p7
	cmd = BME688_PAR_P4_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -5;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, 6);
	if (result != 0) return -6;

	parameters->par_p4 = (int16_t) (((uint16_t)buffer[1] << 8) | (uint16_t) buffer[0]);
	parameters->par_p5 = (int16_t) (((uint16_t)buffer[3] << 8) | (uint16_t) buffer[2]);
	parameters->par_p7 = (int8_t) buffer[4];
	parameters->par_p6 = (int8_t) buffer[5];

	// Get par_p8 -> par_p10
	cmd = BME688_PAR_P8_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -5;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, 5);
	if (result != 0) return -6;

	parameters->par_p8 = (int16_t) (((uint16_t)buffer[1] << 8) | (uint16_t) buffer[0]);
	parameters->par_p9 = (int16_t) (((uint16_t)buffer[3] << 8) | (uint16_t) buffer[2]);
	parameters->par_p10 = (int8_t) buffer[4];

	return 0;
}

static int bme688_get_humidity_calibration_parameters(bme688_calibration_parameters_t* parameters)
{
	int8_t result = 0;
	uint8_t cmd = 0;
	uint8_t buffer[8] = {0};

	// Get par_h1 -> par_h7
	cmd = BME688_PAR_H2_REG;
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, sizeof(buffer));
	if (result != 0) return -2;

	// Extract
	parameters->par_h2 = (uint16_t) (((uint16_t)buffer[0] << 4) | (uint16_t)buffer[1] >> 4);
	parameters->par_h1 = (int16_t) (((uint16_t)buffer[2] << 4) | ((uint16_t) buffer[1] & 0xF));
	parameters->par_h3 = (int8_t) buffer[3];
	parameters->par_h4 = (int8_t) buffer[4];
	parameters->par_h5 = (int8_t) buffer[5];
	parameters->par_h6 = buffer[6];
	parameters->par_h7 = (int8_t) buffer[7];

	return 0;
}

int bme688_get_calibration_parameters(bme688_calibration_parameters_t* parameters)
{
	if (bme688_get_heat_res_calibration_parameters(parameters) != 0) return -1;
	if (bme688_get_temperature_calibration_parameters(parameters) != 0) return -2;
	if (bme688_get_pressure_calibration_parameters(parameters) != 0) return -3;
	if (bme688_get_humidity_calibration_parameters(parameters) != 0) return -4;
	return 0;
}

/**
 * Read the actual mode of the device
 * 0b00: Sleep mode
 * 0b01: Forced mode
 * 0b10: Parallel mode
 */
static int bme688_get_mode(uint8_t* mode)
{
	uint8_t buffer = 0;
	int8_t result = 0;
	uint8_t cmd = BME688_CTRL_MEAS_REG;

	// Send it
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, &buffer, sizeof(uint8_t));
	if (result != 0) return -2;

	// Convert
	*mode = buffer & 3;

	return 0;
}

/**
 * Get the value of pressure and temperature oversampling
 */
static int bme688_get_temperature_pressure_oversampling(uint8_t* temperature, uint8_t* pressure)
{
	uint8_t buffer = 0;
	int8_t result = 0;
	uint8_t cmd = BME688_CTRL_MEAS_REG;

	// Send it
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, &buffer, sizeof(uint8_t));
	if (result != 0) return -2;

	// Convert
	*temperature = (buffer >> 5) & 7;
	*pressure = (buffer >> 2) & 7;

	return 0;
}

int bme688_set_oversampling(uint8_t temperature, uint8_t pressure, uint8_t humidity)
{
	uint8_t measurement_mode = 0;
	uint8_t cmd[2] = {0};
	int8_t result = 0;

	int retval = bme688_get_mode(&measurement_mode);
	if (retval != 0) return -1;

	if (humidity > 7) return -2;
	if (pressure > 7) return -2;
	if (temperature > 7) return -2;

	// Write osrs_h
	cmd[0] = BME688_CTRL_HUM_REG;
	cmd[1] = humidity & 7;

	result = i2c_write_bytes(BME688_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -3;

	// Write osrs_t and osrs_p
	cmd[0] = BME688_CTRL_MEAS_REG;
	cmd[1] = (temperature << 5) | (pressure << 2) | measurement_mode;

	result = i2c_write_bytes(BME688_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -4;

	return 0;
}

int bme688_set_measurement_mode(uint8_t mode)
{
	uint8_t pressure_os = 0;
	uint8_t temperature_os = 0;
	uint8_t cmd[2] = {0};
	int8_t result = 0;

	int retval = bme688_get_temperature_pressure_oversampling(&temperature_os, &pressure_os);
	if (retval != 0) return -1;

	if (mode > 2) return -2;

	// Write register
	cmd[0] = BME688_CTRL_MEAS_REG;
	cmd[1] = (temperature_os << 5) | (pressure_os << 2) | mode;

	result = i2c_write_bytes(BME688_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -4;

	return 0;
}



/**
 * Read the actual number of conversion of the device
 */
static int bme688_get_conversion_number(uint8_t* nb_conv)
{
	uint8_t buffer = 0;
	int8_t result = 0;
	uint8_t cmd = BME688_CTRL_GAS_0_REG;

	// Send it
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, &buffer, sizeof(uint8_t));
	if (result != 0) return -2;

	// Convert
	*nb_conv = buffer & 0xF;

	return 0;
}

/**
 * Read the status of the run_gas variable (is gaz sensing enabled or not)
 */
static int bme688_get_run_gas(uint8_t* run_gas)
{
	uint8_t buffer = 0;
	int8_t result = 0;
	uint8_t cmd = BME688_CTRL_GAS_0_REG;

	// Send it
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, &buffer, sizeof(uint8_t));
	if (result != 0) return -2;

	// Convert
	*run_gas = (buffer >> 5) & 1;

	return 0;
}

int bme688_enable_disable_gas_conversion(uint8_t flag)
{
	uint8_t nb_conv = 0;
	uint8_t cmd[2] = {0};
	int8_t result = 0;

	int retval = bme688_get_conversion_number(&nb_conv);
	if (retval != 0) return -1;

	if (flag > 1) return -2;

	// Write run_gas
	cmd[0] = BME688_CTRL_GAS_0_REG;
	cmd[1] = (flag << 5) | nb_conv;

	result = i2c_write_bytes(BME688_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -3;

	return 0;
}

/**
 * Set gas wait shared parameters
 * Note: not clear how the real timing is computed for measurement
 *
 * @param [in] factor Wait time multiplication factor (0, 1, 2, 3) -> [1, 4, 16, 64]
 * @param [in] timer (0..63) -> step size of 0.477ms
 */
static int bme688_set_gas_wait_shared(uint8_t factor, uint8_t timer)
{
	uint8_t cmd[2] = {0};
	int8_t result = 0;

	if (factor > 3) return -2;
	if (timer > 63) return -2;

	// Write the value
	cmd[0] = BME688_GAS_WAIT_SHARED_REG;
	cmd[1] = (factor << 6) | timer;

	result = i2c_write_bytes(BME688_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -3;

	return 0;
}

/**
 * Set the number of time the TPHG sequence needs to be repeated
 * Total duration: Duration = gas_wait_X * (gas_wait_shared + TTPHG_duration)
 *
 * @param [in] index (0..9)
 * @param [in] repetition (0..255)
 */
static int bme688_set_gas_wait_for_index(uint8_t index, uint8_t repetition)
{
	uint8_t cmd[2] = {0};
	int8_t result = 0;

	if (index > 9) return -2;

	// Write the value
	cmd[0] = BME688_GAS_WAIT_X_REG + index;
	cmd[1] = repetition;

	result = i2c_write_bytes(BME688_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -3;

	return 0;
}

/**
 * Set the value of the resistance for the heater step
 * The value can be computed using the function compute_res_heat_x_value
 *
 * @param [in] index Index of the step (0..9)
 * @param [in] res_heat The value to be applied
 */
static int bme688_set_res_heat_for_index(uint8_t index, uint8_t res_heat)
{
	uint8_t cmd[2] = {0};
	int8_t result = 0;

	if (index > 9) return -2;

	// Write the value
	cmd[0] = BME688_RES_HEAT_X_REG + index;
	cmd[1] = res_heat;

	result = i2c_write_bytes(BME688_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -3;

	return 0;
}

static int bme688_set_heater_step_number(uint8_t nb_conv)
{
	uint8_t run_gas = 0;
	uint8_t cmd[2] = {0};
	int8_t result = 0;

	int retval = bme688_get_run_gas(&run_gas);
	if (retval != 0) return -1;

	if (nb_conv > 10) return -2;

	// Write run_gas
	cmd[0] = BME688_CTRL_GAS_0_REG;
	cmd[1] = (run_gas << 5) | nb_conv;

	result = i2c_write_bytes(BME688_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -3;

	return 0;
}

static uint8_t compute_res_heat_x_value(uint16_t temperature, uint16_t ambient_temperature, bme688_calibration_parameters_t* calib)
{
	float var1 = 0;
	float var2 = 0;
	float var3 = 0;
	float var4 = 0;
	float var5 = 0;
	uint8_t res_heat = 0;

	// Maximum temperature
	if (temperature > 400) temperature = 400;

	var1 = (((float)calib->par_g1 / (16.0f)) + 49.0f);
	var2 = ((((float)calib->par_g2 / (32768.0f)) * (0.0005f)) + 0.00235f);
	var3 = ((float)calib->par_g3 / (1024.0f));
	var4 = (var1 * (1.0f + (var2 * (float)temperature)));
	var5 = (var4 + (var3 * (float)ambient_temperature));
	res_heat = (uint8_t)(3.4f *
				  ((var5 * (4 / (4 + (float)calib->res_heat_range)) *
					(1 / (1 + ((float)calib->res_heat_val * 0.002f)))) -
				   25));

	return res_heat;
}

/**
 * @brief Get the duration needed for the measurement of Temperature, Pressure, Humidity and Gas
 *
 * This duration is needed to compute the duration of one heating step
 *
 * @param [in] temperature_os Oversampling configuration used for temperature measurement (0, 1, 2, 3, 4, 5)
 * @param [in] pressure_os Oversampling configuration used for pressure measurement (0, 1, 2, 3, 4, 5)
 * @param [in] humidity_os Oversampling configuration used for humidity measurement (0, 1, 2, 3, 4, 5)
 *
 * @retval Duration in µs
 *
 * When using maximum oversampling (x16, x16, x16) the duration time is 98 517µs (98.5ms)
 */
static uint32_t bme688_get_tphg_measurement_duration(uint8_t temperature_os, uint8_t pressure_os, uint8_t humidity_os)
{
	uint8_t os_to_meas_cycles[6] = { 0, 1, 2, 4, 8, 16 };
	uint32_t retval = 0;
	uint32_t meas_cycles = 0;

	if (temperature_os > 5) return 0;
	if (pressure_os > 5) return 0;
	if (humidity_os > 5) return 0;

	// How much cycles are needed to measure temperature pressure and humidity
	meas_cycles = os_to_meas_cycles[temperature_os];
	meas_cycles += os_to_meas_cycles[pressure_os];
	meas_cycles += os_to_meas_cycles[humidity_os];

	// Convert measurement cycles to µs
	// Remark: the information is not inside the database, it is extracted from a source code of bosch
	retval = meas_cycles * UINT32_C(1963);

	// Same as previous, those informations are not inside the datasheet
	retval += UINT32_C(477 * 4); /* TPH switching duration */
	retval += UINT32_C(477 * 5); /* Gas measurement duration */

	return retval;
}

/**
 * @brief Given a duration in ms compute the value to be written inside the gas_wait_shared register
 *
 * @param [in] duration Duration in ms
 * @param [out] wait_shared
 * @param [out] multiplication_factor
 *
 * @retval Value to be written inside the register
 */
static void bme688_compute_heater_duration_shared_content(uint16_t duration, uint8_t* wait_shared, uint8_t* multiplication_factor)
{
	*multiplication_factor = 0;
	*wait_shared = 0;

	// Maximum duration is 63 * 0.477 * 64 ms
	if (duration >= 0x783)
	{
		*wait_shared = 0xff; // Maximum duration
	}
	else
	{
		// Step size is 0.477ms
		duration = (uint16_t)(((uint32_t)duration * 1000) / 477);
		while (duration > 0x3F)
		{
			duration = duration >> 2; // Multiplication factors are 1, 4, 16 and 64, therefore shift of 2
			*multiplication_factor += 1;
		}

		*wait_shared = (uint8_t)(duration);
	}
}

int bme688_set_heater_profile(bme688_measurement_configuration_t* configuration, uint16_t ambient_temperature, bme688_calibration_parameters_t* calib)
{
	uint8_t index = 0;

	if (configuration->heater_step_nb > BME688_MAX_STEPS_NB) return -1;

	if (bme688_set_heater_step_number(configuration->heater_step_nb) != 0) return -2;

	// Get the duration needed to sample temperature, humidity, pressure and gas
	uint32_t tphg_duration_us = bme688_get_tphg_measurement_duration(configuration->temperature_os, configuration->pressure_os, configuration->humidity_os);
	uint32_t tphg_duration_ms = tphg_duration_us / 1000;

	// Compute the shared duration in milliseconds
	uint16_t shared_duration_ms = configuration->step_ms - tphg_duration_ms;

	// Set the shared duration
	uint8_t wait_shared = 0;
	uint8_t multiplication_factor = 0;
	bme688_compute_heater_duration_shared_content(shared_duration_ms, &wait_shared, &multiplication_factor);

	if (bme688_set_gas_wait_shared(multiplication_factor, wait_shared) != 0) return -3;

	for (index = 0; index < configuration->heater_step_nb; ++index)
	{
		if (bme688_set_gas_wait_for_index(index, configuration->steps_duration[index]) != 0) return -4;

		// Compute resistance
		uint8_t resistance = compute_res_heat_x_value(configuration->temperatures[index], ambient_temperature, calib);

		// Apply
		if (bme688_set_res_heat_for_index(index, resistance) != 0) return -5;
	}

	return 0;
}

int bme688_set_heater_profile_simple(uint16_t* temperatures, uint16_t ambient_temperature, uint8_t length, bme688_calibration_parameters_t* calib)
{
	uint8_t index = 0;

	if (length > BME688_MAX_STEPS_NB) return -1;

	if (bme688_set_heater_step_number(length) != 0) return -2;

	if (bme688_set_gas_wait_shared(2, 63) != 0) return -3;

	for (index = 0; index < length; ++index)
	{
		if (bme688_set_gas_wait_for_index(index, 1) != 0) return -4;

		// Compute resistance
		uint8_t resistance = compute_res_heat_x_value(temperatures[index], ambient_temperature, calib);

		// Apply
		if (bme688_set_res_heat_for_index(index, resistance) != 0) return -5;
	}

	return 0;
}

#define BME688_FIELD_LENGTH 17

int bme688_read_data_field(uint8_t index, bme688_data_field_t* data)
{
	uint8_t buffer[BME688_FIELD_LENGTH] = {0};
	int8_t result = 0;
	uint8_t cmd = BME688_MEAS_STATUS_REG + (index * BME688_FIELD_LENGTH);

	// Send it
	result = i2c_write_bytes(BME688_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BME688_I2C_ADDR, buffer, sizeof(buffer));
	if (result != 0) return -2;

	// Convert
	data->new_data = buffer[0] >> 7;
	data->gas_measuring = (buffer[0] >> 6) & 1;
	data->measuring = (buffer[0] >> 5) & 1;
	data->gas_meas_index = buffer[0] & 0xF;
	data->sub_meas_index = buffer[1];

	data->pressure = ((uint32_t) buffer[4]) >> 4
			| ((uint32_t) buffer[3]) << 4
			| ((uint32_t) buffer[2]) << 12;

	data->temperature = ((uint32_t) buffer[7]) >> 4
			| ((uint32_t) buffer[6]) << 4
			| ((uint32_t) buffer[5]) << 12;

	data->humidity = ((uint16_t) buffer[9])
			| ((uint16_t) buffer[8] << 8);

	data->gas_resistance = ((uint16_t) buffer[16]) >> 6
			| ((uint16_t) buffer[15] << 2);

	data->gas_valid_r = (buffer[16] >> 5) & 1;
	data->heat_stab_r = (buffer[16] >> 4) & 1;
	data->gas_range_r = buffer[16] & 0xF;

	return 0;
}

float bme688_get_temperature(bme688_data_field_t* data, bme688_calibration_parameters_t* calib)
{
    float var1 = 0;
    float var2 = 0;
    float calc_temp = 0;

    /* calculate var1 data */
    var1 = ((((float)data->temperature / 16384.0f) - ((float)calib->par_t1 / 1024.0f)) * ((float)calib->par_t2));

    /* calculate var2 data */
    var2 =
        (((((float)data->temperature / 131072.0f) - ((float)calib->par_t1 / 8192.0f)) *
          (((float)data->temperature / 131072.0f) - ((float)calib->par_t1 / 8192.0f))) * ((float)calib->par_t3 * 16.0f));

    /* t_fine value*/
    calib->t_fine = (var1 + var2);

    /* compensated temperature data*/
    calc_temp = (calib->t_fine) / 5120.0f;

    return calc_temp;
}

float bme688_get_pressure(bme688_data_field_t* data, bme688_calibration_parameters_t* calib)
{
    float var1 = 0;
    float var2 = 0;
    float var3 = 0;
    float calc_pres = 0;

    var1 = (((float)calib->t_fine / 2.0f) - 64000.0f);
    var2 = var1 * var1 * (((float)calib->par_p6) / (131072.0f));
    var2 = var2 + (var1 * ((float)calib->par_p5) * 2.0f);
    var2 = (var2 / 4.0f) + (((float)calib->par_p4) * 65536.0f);
    var1 = (((((float)calib->par_p3 * var1 * var1) / 16384.0f) + ((float)calib->par_p2 * var1)) / 524288.0f);
    var1 = ((1.0f + (var1 / 32768.0f)) * ((float)calib->par_p1));
    calc_pres = (1048576.0f - ((float)data->pressure));

    /* Avoid exception caused by division by zero */
    if ((int)var1 != 0)
    {
        calc_pres = (((calc_pres - (var2 / 4096.0f)) * 6250.0f) / var1);
        var1 = (((float)calib->par_p9) * calc_pres * calc_pres) / 2147483648.0f;
        var2 = calc_pres * (((float)calib->par_p8) / 32768.0f);
        var3 = ((calc_pres / 256.0f) * (calc_pres / 256.0f) * (calc_pres / 256.0f) * (calib->par_p10 / 131072.0f));
        calc_pres = (calc_pres + (var1 + var2 + var3 + ((float)calib->par_p7 * 128.0f)) / 16.0f);
    }
    else
    {
        calc_pres = 0;
    }

    return calc_pres;
}

float bme688_get_humidity(bme688_data_field_t* data, bme688_calibration_parameters_t* calib)
{
    float calc_hum = 0;
    float var1 = 0;
    float var2 = 0;
    float var3 = 0;
    float var4 = 0;
    float temp_comp = 0;

    /* compensated temperature data*/
    temp_comp = ((calib->t_fine) / 5120.0f);
    var1 = (float)((float)data->humidity) -
           (((float)calib->par_h1 * 16.0f) + (((float)calib->par_h3 / 2.0f) * temp_comp));
    var2 = var1 *
           ((float)(((float)calib->par_h2 / 262144.0f) *
                    (1.0f + (((float)calib->par_h4 / 16384.0f) * temp_comp) +
                     (((float)calib->par_h5 / 1048576.0f) * temp_comp * temp_comp))));
    var3 = (float)calib->par_h6 / 16384.0f;
    var4 = (float)calib->par_h7 / 2097152.0f;
    calc_hum = var2 + ((var3 + (var4 * temp_comp)) * var2 * var2);
    if (calc_hum > 100.0f)
    {
        calc_hum = 100.0f;
    }
    else if (calc_hum < 0.0f)
    {
        calc_hum = 0.0f;
    }

    return calc_hum;
}

float bme688_get_gas_resistance(bme688_data_field_t* data, bme688_calibration_parameters_t* calib)
{
    float calc_gas_res = 0;
    uint32_t var1 = UINT32_C(262144) >> data->gas_range_r;
    int32_t var2 = (int32_t)data->gas_resistance - INT32_C(512);

    var2 *= INT32_C(3);
    var2 = INT32_C(4096) + var2;

    calc_gas_res = 1000000.0f * (float)var1 / (float)var2;

    return calc_gas_res;
}
