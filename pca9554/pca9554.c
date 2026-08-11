/*
 * pca9554.c
 *
 *  Created on: Jan 12, 2023
 *      Author: Gintaras
 */

#include "pca9554.h"
#include "hal_i2c.h"


PCA9554_RET_TYPE pca9554_pins_write(uint8_t address, uint8_t pins)
{
	int8_t result;

	result = hal_i2c_write_register(
			address,
			PCA9554_OUTPUT_CMD,
			&pins,
			1);

	if (result != 0)
	{
		return PCA9554_COM_ERR;
	}

	return PCA9554_OK;
}


PCA9554_RET_TYPE pca9554_pins_read(uint8_t address, uint8_t* pins)
{
	int8_t result;

	result = hal_i2c_read_register(
			address,
			PCA9554_INPUT_CMD,
			pins,
			1);

	if (result != 0)
	{
		return PCA9554_COM_ERR;
	}

	result = hal_i2c_write_register(
			address,
			PCA9554_OUTPUT_CMD,
			pins,
			1);

	if (result != 0)
	{
		return PCA9554_COM_ERR;
	}

	return PCA9554_OK;
}


PCA9554_RET_TYPE pca9554_read_config_dir(uint8_t address, uint8_t* pins)
{
	int8_t result;

	result = hal_i2c_read_register(
			address,
			PCA9554_CONFIG_CMD,
			pins,
			1);

	if (result != 0)
	{
		return PCA9554_COM_ERR;
	}

	return PCA9554_OK;
}


PCA9554_RET_TYPE pca9554_read_config_pol(uint8_t address, uint8_t* pins)
{
	int8_t result;

	result = hal_i2c_read_register(
			address,
			PCA9554_POLARITY_CMD,
			pins,
			1);

	if (result != 0)
	{
		return PCA9554_COM_ERR;
	}

	return PCA9554_OK;
}


PCA9554_RET_TYPE pca9554_write_config_dir(uint8_t address, uint8_t pins)
{
	int8_t result;

	result = hal_i2c_write_register(
			address,
			PCA9554_CONFIG_CMD,
			&pins,
			1);

	if (result != 0)
	{
		return PCA9554_COM_ERR;
	}

	return PCA9554_OK;
}


PCA9554_RET_TYPE pca9554_write_config_pol(uint8_t address, uint8_t pins)
{
	int8_t result;

	result = hal_i2c_write_register(
			address,
			PCA9554_POLARITY_CMD,
			&pins,
			1);

	if (result != 0)
	{
		return PCA9554_COM_ERR;
	}

	return PCA9554_OK;
}