/*
 * bmp581.c
 *
 *  Created on: 13 Mar 2023
 *      Author: jorda
 */

#include "bmp581.h"

static const uint8_t BMP581_I2C_ADDR = 0x47;

static const uint8_t BMP581_CHIP_ID_REG = 0x01;
static const uint8_t BMP581_OSR_REG = 0x36;
static const uint8_t BMP581_ODR_REG = 0x37;
static const uint8_t BMP581_TEMPERATURE_XLSB_REG = 0x1D;

static bmp581_read_func_t i2c_read_bytes;
static bmp581_write_func_t i2c_write_bytes;

void bmp581_init_i2c_interface(bmp581_read_func_t read, bmp581_write_func_t write)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;
}

int bmp581_get_chip_id(uint8_t* chip_id)
{
	int8_t result = 0;
	uint8_t cmd = BMP581_CHIP_ID_REG;

	// Send it
	result = i2c_write_bytes(BMP581_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BMP581_I2C_ADDR, chip_id, sizeof(uint8_t));
	if (result != 0) return -2;

	// Plausibility (cannot be 0)
	if (*chip_id == 0) return -3;

	return 0;
}


int bmp581_set_oversampling_mode(uint8_t pressure, uint8_t temperature, uint8_t press_en)
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	if (pressure > 7) return -1;
	if (temperature > 7) return -1;
	if (press_en > 1) return -1;

	// Generate command
	cmd[0] = BMP581_OSR_REG;
	cmd[1] = (press_en << 6) | (pressure << 3) | temperature;

	// Send it
	result = i2c_write_bytes(BMP581_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -2;

	return 0;
}

int bmp581_set_power_mode(uint8_t power_mode, uint8_t odr)
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	if (power_mode > 3) return -1;
	if (odr > 31) return -1;

	// Generate command
	cmd[0] = BMP581_ODR_REG;
	cmd[1] = (odr << 2) | power_mode;

	// Send it
	result = i2c_write_bytes(BMP581_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -2;

	return 0;
}

int bmp581_read_pressure_and_temperature(float* pressure, float* temperature)
{
	int8_t result = 0;
	uint8_t cmd = BMP581_TEMPERATURE_XLSB_REG;
	uint8_t res[6] = {0};

	// Send it
	result = i2c_write_bytes(BMP581_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(BMP581_I2C_ADDR, res, sizeof(res));
	if (result != 0) return -2;

	// Extract
	int32_t raw_temperature	= (int32_t) ((uint32_t) res[0] | ((uint32_t) res[1]) << 8 | ((uint32_t) res[2]) << 16);
	uint32_t raw_pressure	= (uint32_t) ((uint32_t) res[3] | ((uint32_t) res[4]) << 8 | ((uint32_t) res[5]) << 16);

	*temperature	= (float) raw_temperature / 65536.0f;
	*pressure 		= (float) raw_pressure / 64.0f;

	return 0;
}
