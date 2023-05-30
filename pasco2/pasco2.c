/*
 * pasco2.c
 *
 *  Created on: 22 Mar 2023
 *      Author: jorda
 */

#include "pasco2.h"

static const uint8_t PASCO2_I2C_ADDR = 0x28;

static const uint8_t PROD_ID_ADDR = 0x0;
static const uint8_t SENS_STS_ADDR = 0x1;
static const uint8_t MEAS_RATE_H_ADDR = 0x2;
static const uint8_t MEAS_CFG_ADDR = 0x4;
static const uint8_t CO2PPM_H_ADDR = 0x5;
static const uint8_t MEAS_STS_ADDR = 0x7;
static const uint8_t SCRATCH_PAD_ADDR = 0xF;
static const uint8_t SENS_RST_ADDR = 0x10;

static pasco2_read_func_t i2c_read_bytes;
static pasco2_write_func_t i2c_write_bytes;
static pasco2_sleep_func_t sys_sleep;

void pasco2_init_hal(pasco2_read_func_t read, pasco2_write_func_t write, pasco2_sleep_func_t sleep)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;
	sys_sleep = sleep;
}

/**
 * @brief Write a value into the scratch pad register (usefull to check the integrity of the communication)
 */
static int write_to_scratch_pad(uint8_t value)
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	// Construct the command
	cmd[0] = SCRATCH_PAD_ADDR;
	cmd[1] = value;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	return 0;
}

static int read_scratch_pad(uint8_t* value)
{
	int8_t result = 0;
	uint8_t cmd[1] = {0};

	// Construct the command
	cmd[0] = SCRATCH_PAD_ADDR;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read register
	result = i2c_read_bytes(PASCO2_I2C_ADDR, value, sizeof(uint8_t));
	if (result != 0) return -2;

	return 0;
}

int pasco2_check_i2c_interface()
{
	static const uint8_t TEST_VALUE = 0xA5;
	uint8_t read_back = 0;

	if (write_to_scratch_pad(TEST_VALUE) != 0)
	{
		return -1;
	}

	if (read_scratch_pad(&read_back) != 0)
	{
		return -2;
	}

	if (read_back != TEST_VALUE)
	{
		return -3;
	}

	return 0;
}

int pasco2_get_product_and_revision(uint8_t* product, uint8_t* rev)
{
	int8_t result = 0;
	uint8_t cmd[1] = {0};
	uint8_t data[1] = {0};

	// Construct the command
	cmd[0] = PROD_ID_ADDR;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read register
	result = i2c_read_bytes(PASCO2_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	*product = data[0] >> 5;
	*rev = data[0] & 0x1F;

	return 0;
}

int pasco2_get_sensor_status(uint8_t* status)
{
	int8_t result = 0;
	uint8_t cmd[1] = {0};
	uint8_t data[1] = {0};

	// Construct the command
	cmd[0] = SENS_STS_ADDR;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read register
	result = i2c_read_bytes(PASCO2_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	*status = data[0];

	return 0;
}

int pasco2_get_measurement_status(uint8_t* status)
{
	int8_t result = 0;
	uint8_t cmd[1] = {0};
	uint8_t data[1] = {0};

	// Construct the command
	cmd[0] = MEAS_STS_ADDR;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read register
	result = i2c_read_bytes(PASCO2_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	*status = data[0];

	return 0;
}

static uint16_t convert_to_co2_ppm(uint8_t* data)
{
	uint8_t buffer[2] = {0};
	buffer[0] = data[1];
	buffer[1] = data[0];
	return *((uint16_t*)buffer);
}

int pasco2_read_co2_ppm(uint16_t* co2_ppm)
{
	int8_t result = 0;
	uint8_t cmd[1] = {0};
	uint8_t data[2] = {0};

	// Construct the command
	cmd[0] = CO2PPM_H_ADDR;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read register
	result = i2c_read_bytes(PASCO2_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	*co2_ppm = convert_to_co2_ppm(data);

	return 0;
}

int pasco2_soft_reset(uint8_t wait_for_startup)
{
	static const uint32_t start_delay = 2000;
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	// Construct the command
	cmd[0] = SENS_RST_ADDR;
	cmd[1] = 0xA3;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	if (wait_for_startup != 0)
	{
		sys_sleep(start_delay);
	}

	return 0;
}

int pasco2_set_iddle_mode()
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	// Construct the command
	cmd[0] = MEAS_CFG_ADDR;
	cmd[1] = 0;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	return 0;
}

int pasco2_set_period(uint16_t seconds)
{
	int8_t result = 0;
	uint8_t cmd[3] = {0};

	if (seconds < PASCO2_MEAS_RATE_MIN) return -1;
	if (seconds > PASCO2_MEAS_RATE_MAX) return -2;

	uint8_t msb = (uint8_t)(seconds >> 8);
	uint8_t lsb = (uint8_t)(seconds & 0xFF);

	// Send msb
	cmd[0] = MEAS_RATE_H_ADDR;
	cmd[1] = msb;
	cmd[2] = lsb;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -3;

	return 0;
}

int pasco2_start_continuous_measurement()
{
	static const uint8_t measurement_mode_continuous = 6; //  BOC_CFG = 1, OP_MODE = 2
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	// Construct the command
	cmd[0] = MEAS_CFG_ADDR;
	cmd[1] = measurement_mode_continuous;

	// Send it
	result = i2c_write_bytes(PASCO2_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	return 0;
}
