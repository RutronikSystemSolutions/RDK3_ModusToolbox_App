/* veml6030_com.c
 *
 *  Created on: 25 Mar 2026
 *      Author: DC1
 */

#include "veml6030_com.h"
#include "veml6030_hal_interface.h"
#include "veml6030_def.h"

static veml6030_read_register_func_t i2c_read_register;
static veml6030_write_func_t i2c_write_bytes;

void veml6030_com_init_hal(veml6030_read_register_func_t read, veml6030_write_func_t write)
{
	i2c_read_register = read;
	i2c_write_bytes = write;
}

/**
 * @brief Read register value over I2C
 * Access to the VEML6030 is limited to 2 bytes (1 word)
 */
int veml6030_com_register_read_word(uint8_t reg, uint16_t* buffer)
{
	if (buffer == 0)
	{
		return -1;
	}

	return i2c_read_register(VEML6030_ADDR, reg, (uint8_t*)buffer, sizeof(uint16_t));
}

/**
 * @brief Write one 16-bit word starting at @p reg.
 * LSB is sent first, followed by MSB.
 */
static int register_write_word(uint8_t reg, uint16_t value)
{
	uint8_t buffer[3] = {reg, (uint8_t)(value & 0xFFu), (uint8_t)((value >> 8) & 0xFFu)};

	int8_t result = i2c_write_bytes(VEML6030_ADDR, buffer, sizeof(buffer));
	if (result != 0)
	{
		return -1;
	}

	return 0;
}

static int read_als_conf_word(uint16_t* conf)
{
	return veml6030_com_register_read_word(VEML6030_ALS_CONF, conf);
}

static int write_als_conf_word(uint16_t conf)
{
	return register_write_word(VEML6030_ALS_CONF, conf);
}

static int read_power_saving_word(uint16_t* psm)
{
	return veml6030_com_register_read_word(VEML6030_POWER_SAVING, psm);
}

static int write_power_saving_word(uint16_t psm)
{
	return register_write_word(VEML6030_POWER_SAVING, psm);
}

/**
 * @param [in] status 0: ALS power on, 1: ALS shut down
 */
int veml6030_com_set_als_shutdown_on_off(uint8_t status)
{
	const uint16_t als_sd_mask = 0x1u << 0; /* bit 0 */

	if (status > 1)
	{
		return -1;
	}

	uint16_t reg_content = 0;
	int retval = read_als_conf_word(&reg_content);
	if (retval != 0)
	{
		return -2;
	}

	reg_content &= ~als_sd_mask;
	reg_content |= ((uint16_t)status << 0);

	retval = write_als_conf_word(reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}

/**
 * @param [in] status 0: ALS INT disable, 1: ALS INT enable
 */
int veml6030_com_set_als_interrupt_on_off(uint8_t status)
{
	const uint16_t als_int_mask = 0x1u << 1; /* bit 1 */

	if (status > 1)
	{
		return -1;
	}

	uint16_t reg_content = 0;
	int retval = read_als_conf_word(&reg_content);
	if (retval != 0)
	{
		return -2;
	}

	reg_content &= ~als_int_mask;
	reg_content |= ((uint16_t)status << 1);

	retval = write_als_conf_word(reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}

/**
 * @brief Set ALS gain
 * @param [in] gain
 *              0: x1
 *              1: x2
 *              2: x1/8
 *              3: x1/4
 */
int veml6030_com_set_als_gain(uint8_t gain)
{
	const uint16_t als_gain_mask = 0x3u << 11; /* bits 12:11 */

	if (gain > 3)
	{
		return -1;
	}

	uint16_t reg_content = 0;
	int retval = read_als_conf_word(&reg_content);
	if (retval != 0)
	{
		return -2;
	}

	reg_content &= ~als_gain_mask;
	reg_content |= ((uint16_t)gain << 11);

	retval = write_als_conf_word(reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}

/**
 * @brief Set ALS integration time
 * @param [in] it register code for ALS_IT in bits 9:6
 *              0x0C: 25 ms
 *              0x08: 50 ms
 *              0x00: 100 ms
 *              0x01: 200 ms
 *              0x02: 400 ms
 *              0x03: 800 ms
 */
int veml6030_com_set_als_integration_time(uint8_t it)
{
	const uint16_t als_it_mask = 0xFu << 6; /* bits 9:6 */

	if ((it != 0x0Cu) && (it != 0x08u) && (it != 0x00u) &&
	    (it != 0x01u) && (it != 0x02u) && (it != 0x03u))
	{
		return -1;
	}

	uint16_t reg_content = 0;
	int retval = read_als_conf_word(&reg_content);
	if (retval != 0)
	{
		return -2;
	}

	reg_content &= ~als_it_mask;
	reg_content |= ((uint16_t)it << 6);

	retval = write_als_conf_word(reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}

/**
 * @brief Set power saving mode.
 * @param [in] mode
 *              0: mode 1
 *              1: mode 2
 *              2: mode 3
 *              3: mode 4
 */
int veml6030_com_set_power_saving_mode(uint8_t mode)
{
	const uint16_t psm_mode_mask = 0x3u << 1; /* bits 2:1 */

	if (mode > 3)
	{
		return -1;
	}

	uint16_t reg_content = 0;
	int retval = read_power_saving_word(&reg_content);
	if (retval != 0)
	{
		return -2;
	}

	reg_content &= ~psm_mode_mask;
	reg_content |= ((uint16_t)mode << 1);

	retval = write_power_saving_word(reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}

/**
 * @param [in] status 0: power saving disable, 1: power saving enable
 */
int veml6030_com_set_power_saving_enable(uint8_t status)
{
	const uint16_t psm_en_mask = 0x1u << 0; /* bit 0 */

	if (status > 1)
	{
		return -1;
	}

	uint16_t reg_content = 0;
	int retval = read_power_saving_word(&reg_content);
	if (retval != 0)
	{
		return -2;
	}

	reg_content &= ~psm_en_mask;
	reg_content |= ((uint16_t)status << 0);

	retval = write_power_saving_word(reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}
