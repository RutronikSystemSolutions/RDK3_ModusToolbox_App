/* veml6031x00_com.c
 *
 *  Created on: 24 Feb 2026
 *      Author: dc1
 */

#include "veml6031x00_com.h"
#include "veml6031x00_hal_interface.h"
#include "veml6031x00_def.h"

static veml6031x00_read_register_func_t i2c_read_register;
static veml6031x00_write_func_t i2c_write_bytes;

void veml6031x00_com_init_hal(veml6031x00_read_register_func_t read, veml6031x00_write_func_t write)
{
	i2c_read_register = read;
	i2c_write_bytes = write;
}

/**
 * @brief Read register value over I2C
 * Access to the VEML is limited to 2 bytes (1 word)
 */
int veml6031x00_com_register_read_word(uint8_t reg, uint16_t* buffer)
{
	return i2c_read_register(VEML6031X00_ADDR, reg, (uint8_t*)buffer, sizeof(uint16_t));
}

/**
 * @brief Write two consecutive bytes starting at @p reg
 * Used for writing ALS_CONF_0 and ALS_CONF_1 together.
 */
static int register_write_2bytes(uint8_t reg, uint8_t low, uint8_t high)
{
	// Construct buffer to be sent 
	uint8_t buffer[3] = {reg, low, high};

	// Send it
	int8_t result = i2c_write_bytes(VEML6031X00_ADDR, buffer, sizeof(buffer));
	if (result != 0)
	{
		return -1;
	}

	return 0;
}

/**
 * @brief Read ALS_CONF_0 and ALS_CONF_1 as one 16-bit value.
 * LSB = ALS_CONF_0 (0x00), MSB = ALS_CONF_1 (0x01)
 */
static int read_als_conf_word(uint16_t* conf)
{
	return veml6031x00_com_register_read_word(VEML6031X00_ALS_CONF_0, conf);
}

/**
 * @brief Write ALS_CONF_0 and ALS_CONF_1 together.
 */
static int write_als_conf_word(uint16_t conf)
{
	uint8_t conf0 = (uint8_t)(conf & 0xFF);
	uint8_t conf1 = (uint8_t)((conf >> 8) & 0xFF);
	return register_write_2bytes(VEML6031X00_ALS_CONF_0, conf0, conf1);
}

/**
 * @param [in] status 0: ALS power on, 1: ALS shut down
 */
int veml6031x00_com_set_als_shutdown_on_off(uint8_t status)
{
	const uint16_t als_on0_mask = 0x1;          // bit 0
	const uint16_t als_on1_mask = 0x1u << 15;   // bit 7

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = read_als_conf_word(&reg_content);
	if (retval != 0)
	{
		return -1;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~als_on0_mask;
	reg_content = reg_content & ~als_on1_mask;

	// In case status is OFF, write 1
	if (status == 1)
	{
		reg_content = reg_content | als_on0_mask;
		reg_content = reg_content | als_on1_mask;
	}

	retval = write_als_conf_word(reg_content);
	if (retval != 0)
	{
		return -2;
	}

	return 0;
}

/**
 * @param [in] status 0: ALS INT disable, 1: ALS INT enable
 */
int veml6031x00_com_set_als_interrupt_on_off(uint8_t status)
{
	const uint16_t als_int_mask = 0x1 << 1; // bit 1

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = read_als_conf_word(&reg_content);
	if (retval != 0)
	{
		return -1;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~als_int_mask;

	// If enable requested -> set bit
	if (status == 1)
	{
		reg_content = reg_content | als_int_mask;
	}

	retval = write_als_conf_word(reg_content);
	if (retval != 0)
	{
		return -2;
	}

	return 0;
}

/**
 * @brief Set ALS gain
 * @param [in] gain
 *              0: x1
 *              1: x2
 *              2: x0.66
 *              3: x0.5
 */
int veml6031x00_com_set_als_gain(uint8_t gain)
{
	const uint16_t als_gain_mask = 0x3u << 11; // bits 12:11

	if (gain > 3)
	{
		return -1;
	}

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = read_als_conf_word(&reg_content);
	if (retval != 0)
	{
		return -2;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~als_gain_mask;

	// Set new gain
	reg_content = reg_content | ((uint16_t)gain << 11);

	retval = write_als_conf_word(reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}

/**
 * @brief Set ALS integration time
 * @param [in] it (3-bit code for ALS_IT in ALS_CONF_0 bits 6:4)
 *              0x0: 3.125 ms
 *              0x1: 6.25 ms
 *              0x2: 12.5 ms
 *              0x3: 25 ms
 *              0x4: 50 ms
 *              0x5: 100 ms
 *              0x6: 200 ms
 *              0x7: 400 ms
 */
int veml6031x00_com_set_als_integration_time(uint8_t it)
{
	const uint16_t als_it_mask = 0x7u << 4; // bits 6:4 

	if (it > 7)
	{
		return -1;
	}

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = read_als_conf_word(&reg_content);
	if (retval != 0)
	{
		return -2;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~als_it_mask;

	// Set new integration time code
	reg_content = reg_content | ((uint16_t)it << 4);

	retval = write_als_conf_word(reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}