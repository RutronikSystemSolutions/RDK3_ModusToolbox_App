/*
 * hal_i2c.c
 *
 *  Created on: 24 Mar 2023
 *      Author: jorda
 */

#include "hal_i2c.h"

#include "cyhal_i2c.h"
#include "cycfg_pins.h"

/**
 * @var i2c_master Global variable storing the instance of the I2C master module
 */
static cyhal_i2c_t i2c_master;

static const uint32_t STD_TIMEOUT_MS = 100;


/**
 * @brief Init the I2C communication
 */
int8_t hal_i2c_init()
{
	cyhal_i2c_cfg_t i2c_config;
	i2c_config.is_slave = false;
	i2c_config.address = 0;
	i2c_config.frequencyhal_hz = 400000UL;

	cy_rslt_t result = cyhal_i2c_init(&i2c_master, ARDU_SDA, ARDU_SCL, NULL);
	if (result != CY_RSLT_SUCCESS) return result;

	result = cyhal_i2c_configure(&i2c_master, &i2c_config);
	return result;
}

int8_t hal_i2c_read(uint8_t address, uint8_t* data, uint16_t len)
{
	cy_rslt_t result = cyhal_i2c_master_read(&i2c_master, (uint16_t)address, data, len, STD_TIMEOUT_MS, true);
	if (result != CY_RSLT_SUCCESS) return -1;
	return 0;
}

int8_t hal_i2c_write(uint8_t address, uint8_t* data, uint16_t len)
{
	cy_rslt_t result = cyhal_i2c_master_write(&i2c_master, (uint16_t)address, data, len, STD_TIMEOUT_MS, true);
	if (result != CY_RSLT_SUCCESS) return -1;
	return 0;
}
