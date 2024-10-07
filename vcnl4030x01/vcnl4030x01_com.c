/*
 * vcnl4030x01_com.c
 *
 *  Created on: 4 Oct 2024
 *      Author: jorda
 */

#include "vcnl4030x01_com.h"
#include "vcnl4030x01_hal_interface.h"
#include "vcnl4030x01_def.h"

static vcnl4030x01_read_register_func_t i2c_read_register;
static vcnl4030x01_write_func_t i2c_write_bytes;

void vcnl4030x01_com_init_hal(vcnl4030x01_read_register_func_t read, vcnl4030x01_write_func_t write)
{
	i2c_read_register = read;
	i2c_write_bytes = write;
}

/**
 * @brief Read register value over I2C
 * Access to the VCNL is limited to 2 bytes (1 word)
 */
int vcnl4030x01_com_register_read_word(uint8_t reg, uint16_t* buffer)
{
	return i2c_read_register(VCNL4030X01_ADDR, reg, (uint8_t*)buffer, sizeof(uint16_t));
}

/**
 * @brief Write a word value over I2C
 */
static int register_write_word(uint8_t reg, uint16_t data)
{
	// Construct buffer to be sent
	uint8_t buffer[3] = {reg, data & 0xFF, (data >> 8) & 0xFF};

	// Send it
	int8_t result = i2c_write_bytes( VCNL4030X01_ADDR, buffer, sizeof(buffer));
	if (result != 0)
	{
		return -1;
	}

	return 0;
}

/**
 * @param [in] status 0: off, 1: on
 */
int vcnl4030x01_com_set_proximity_sensor_on_off(uint8_t status)
{
	const uint16_t ps_sd_mask = 0x1; // first bit

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_CONF_1, &reg_content);
	if (retval != 0)
	{
		return -1;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~ps_sd_mask;

	// In case status is OFF, write 1
	if (status == 0)
		reg_content = reg_content | ps_sd_mask;

	retval = register_write_word(VCNL4030X01_PS_CONF_1, reg_content);
	if (retval != 0)
	{
		return -2;
	}

	return 0;
}

/**
 * @param [in] status 0: off, 1: on
 */
int vcnl4030x01_com_set_active_force_on_off(uint8_t status)
{
	const uint16_t ps_af_mask = 0x1 << 3; // third bit

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_CONF_3, &reg_content);
	if (retval != 0)
	{
		return -1;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~ps_af_mask;

	// In case status is ON, write 1
	if (status == 1)
		reg_content = reg_content | ps_af_mask;

	retval = register_write_word(VCNL4030X01_PS_CONF_3, reg_content);
	if (retval != 0)
	{
		return -2;
	}

	return 0;
}

/**
 * @brief Set the integration time
 * @param [in] integration_time
 * 				0: 1T
 * 				1: 1.5T
 * 				2: 2T
 * 				3: 2.5T
 * 				4: 3T
 * 				5: 3.5T
 * 				6: 4T
 * 				7: 8T
 */
int vcnl4030x01_com_set_integration_time(uint8_t integration_time)
{
	const uint16_t ps_it_mask = 0x7 << 1; // bit 3, 2 and 1

	if (integration_time > 7)
	{
		return -1;
	}

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_CONF_1, &reg_content);
	if (retval != 0)
	{
		return -2;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~ps_it_mask;

	// Set new integration time
	reg_content = reg_content | (integration_time << 1);

	retval = register_write_word(VCNL4030X01_PS_CONF_1, reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}

/**
 * @param [in] size in bits
 * 				Possible values: 12 or 16
 */
int vcnl4030x01_com_set_proximity_sensor_output_size(uint8_t size)
{
	const uint16_t ps_output_size_mask = 0x1 << 11; // bit 11

	if ((size != 12) && (size != 16))
	{
		return -1;
	}

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_CONF_1, &reg_content);
	if (retval != 0)
	{
		return -2;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~ps_output_size_mask;

	// Set output size bit
	if (size == 16)
		reg_content = reg_content | ps_output_size_mask;


	retval = register_write_word(VCNL4030X01_PS_CONF_1, reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}

int vcnl4030x01_com_set_sunlight_cancellation_on_off(uint8_t status)
{
	const uint16_t ps_sc_en_mask = 0x1; // first bit

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_CONF_3, &reg_content);
	if (retval != 0)
	{
		return -1;
	}


	// Mask the bit we want to set
	reg_content = reg_content & ~ps_sc_en_mask;

	// In case status is ON, write 1
	if (status == 1)
		reg_content = reg_content | ps_sc_en_mask;


	retval = register_write_word(VCNL4030X01_PS_CONF_3, reg_content);
	if (retval != 0)
	{
		return -2;
	}

	return 0;
}

/**
 * @param [in] current Current intensity to be set
 * 				Possible values are:
 * 				50 -> 50mA
 * 				75 -> 75mA
 * 				100 -> 100mA
 * 				120 -> 120mA
 * 				140 -> 140mA
 * 				160 -> 160mA
 * 				180 -> 180mA
 * 				200 -> 200mA
 */
int vcnl4030x01_com_set_led_current(uint8_t current)
{
	const uint16_t led_i_mask = 0x7 << 8; // bit 10, 9, 8

	if ((current != 50)
			&& (current != 75)
			&& (current != 100)
			&& (current != 120)
			&& (current != 140)
			&& (current != 160)
			&& (current != 180)
			&& (current != 200))
	{
		return -1;
	}

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_CONF_3, &reg_content);
	if (retval != 0)
	{
		return -2;
	}

	// Mask the bit we want to set
	reg_content = reg_content & ~led_i_mask;

	uint16_t current_bit = 0;
	switch(current)
	{
		case 50:	current_bit = 0; break;
		case 75:	current_bit = 1; break;
		case 100:	current_bit = 2; break;
		case 120:	current_bit = 3; break;
		case 140:	current_bit = 4; break;
		case 160:	current_bit = 5; break;
		case 180:	current_bit = 6; break;
		case 200:	current_bit = 7; break;
	}

	// Set new current
	reg_content = reg_content | (current_bit << 8);

	retval = register_write_word(VCNL4030X01_PS_CONF_3, reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}



