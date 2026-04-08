/*
 * vcnl3682xx_com.c
 *
 *  Created on: 20 feb 2026
 *      Author: dc1
 */

#include "vcnl3682xx_com.h"
#include "vcnl3682xx_hal_interface.h"
#include "vcnl3682xx_def.h"

static vcnl3682xx_read_register_func_t i2c_read_register;
static vcnl3682xx_write_func_t i2c_write_bytes;

void vcnl3682xx_com_init_hal(vcnl3682xx_read_register_func_t read, vcnl3682xx_write_func_t write)
{
	i2c_read_register = read;
	i2c_write_bytes = write;
}

/**
 * @brief Read register value over I2C
 * Access to the VCNL is limited to 2 bytes (1 word)
 */
int vcnl3682xx_com_register_read_word(uint8_t reg, uint16_t* buffer)
{
	return i2c_read_register(VCNL3682XX_ADDR, reg, (uint8_t*)buffer, sizeof(uint16_t));
}

/**
 * @brief Write a word value over I2C
 */
static int register_write_word(uint8_t reg, uint16_t data)
{
	// Construct buffer to be sent
	uint8_t buffer[3] = {reg, data & 0xFF, (data >> 8) & 0xFF};

	// Send it
	int8_t result = i2c_write_bytes(VCNL3682XX_ADDR, buffer, sizeof(buffer));
	if (result != 0)
	{
		return -1;
	}

	return 0;
}

/**
 * @param [in] status 0: off, 1: on
 */
int vcnl3682xx_com_set_proximity_sensor_on_off(uint8_t status)
{
	uint16_t reg_content = 0;
	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_1, &reg_content);
	if (retval != 0)
	{
		return -1;
	}

	const uint16_t ps_conf1_reserved0_mask = 0x0001;  // bit0 must always stay 1 
	const uint16_t ps_on_mask              = 0x0002;  // bit1 PS_ON 
	const uint16_t ps_cal_mask             = 0x0080;  // bit7 PS_CAL 
	const uint16_t conf1h_reserved9_mask   = 0x0200;  // bit9 must be set to 1 when power on ready 

	// enforce reserved bits
	reg_content |= ps_conf1_reserved0_mask;
	reg_content |= conf1h_reserved9_mask;

	if (status == 1)
	{
		// step 1: PS_ON=1, step 2: PS_CAL=1 
		reg_content |= ps_on_mask;
		reg_content |= ps_cal_mask;
	}
	else
	{
		reg_content &= ~ps_on_mask;
	}

	retval = register_write_word(VCNL3682XX_PS_CONF_1, reg_content);
	if (retval != 0)
	{
		return -2;
	}

	uint16_t reg_conf2 = 0;
	retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_2, &reg_conf2);
	if (retval != 0)
	{
		return -3;
	}

	const uint16_t ps_st_mask = 0x0001; // bit0 PS_ST 

	// Mask the bit we want to set 
	reg_conf2 = reg_conf2 & ~ps_st_mask;

	if (status == 0)
	{
		reg_conf2 = reg_conf2 | ps_st_mask;
	}
	else
	{

	}

	retval = register_write_word(VCNL3682XX_PS_CONF_2, reg_conf2);
	if (retval != 0)
	{
		return -4;
	}

	return 0;
}

/**
 * @param [in] status 0: off, 1: on
 */
int vcnl3682xx_com_set_active_force_on_off(uint8_t status)
{
	const uint16_t ps_af_mask = 0x1 << 6; // bit6 PS_AF 
	const uint16_t ps_use_mask = 0x1 << 3; // bit3 must be 1 when use PS 

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_3, &reg_content);
	if (retval != 0)
	{
		return -1;
	}

	// always enforce bit3 = 1 when using PS 
	reg_content |= ps_use_mask;

	// Mask the bit we want to set
	reg_content = reg_content & ~ps_af_mask;

	// In case status is ON, write 1
	if (status == 1)
		reg_content = reg_content | ps_af_mask;

	retval = register_write_word(VCNL3682XX_PS_CONF_3, reg_content);
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
 * 				1: 2T
 * 				2: 4T
 * 				3: 8T
 */
int vcnl3682xx_com_set_integration_time(uint8_t integration_time)
{
	const uint16_t ps_it_mask = 0x3 << 14; // bits 15:14 

	if (integration_time > 3)
	{
		return -1;
	}

	// Read content of the register 
	uint16_t reg_content = 0;
	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_2, &reg_content);
	if (retval != 0)
	{
		return -2;
	}

	// Mask the bits we want to set
	reg_content = reg_content & ~ps_it_mask;

	// Set new integration time
	reg_content = reg_content | ((uint16_t)integration_time << 14);

	retval = register_write_word(VCNL3682XX_PS_CONF_2, reg_content);
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
int vcnl3682xx_com_set_proximity_sensor_output_size(uint8_t size)
{
	const uint16_t ps_output_size_mask = 0x1 << 12; // bit12 PS_HD 
	const uint16_t ps_use_mask = 0x1 << 3; // bit3 must be 1 when use PS 

	if ((size != 12) && (size != 16))
	{
		return -1;
	}

	// Read content of the register 
	uint16_t reg_content = 0;
	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_3, &reg_content);
	if (retval != 0)
	{
		return -2;
	}

	// enforce PS use bit in low byte 
	reg_content |= ps_use_mask;

	// Mask the bit we want to set
	reg_content = reg_content & ~ps_output_size_mask;

	// Set output size bit
	if (size == 16)
		reg_content = reg_content | ps_output_size_mask;

	retval = register_write_word(VCNL3682XX_PS_CONF_3, reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}


int vcnl3682xx_com_set_sunlight_cancellation_on_off(uint8_t status)
{
	const uint16_t ps_sc_mask = 0x7 << 13; 
	const uint16_t ps_use_mask = 0x1 << 3; 

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_3, &reg_content);
	if (retval != 0)
	{
		return -1;
	}

	// enforce PS use bit in low byte 
	reg_content |= ps_use_mask;

	// Mask the bits we want to set
	reg_content = reg_content & ~ps_sc_mask;

	// In case status is ON, write 111b
	if (status == 1)
		reg_content = reg_content | (0x7 << 13);

	retval = register_write_word(VCNL3682XX_PS_CONF_3, reg_content);
	if (retval != 0)
	{
		return -2;
	}

	return 0;
}

/**
 * @param [in] current Current intensity to be set
 * 				Possible values are:
 * 				10 -> 10mA
 * 				12 -> 12mA
 * 				14 -> 14mA
 * 				16 -> 16mA
 * 				18 -> 18mA
 * 				20 -> 20mA
 */
int vcnl3682xx_com_set_vcsel_current(uint8_t current)
{
	const uint16_t vscel_i_mask = 0xF << 8; 
	const uint16_t ps_use_mask = 0x1 << 3; 

	if ((current != 10)
			&& (current != 12)
			&& (current != 14)
			&& (current != 16)
			&& (current != 18)
			&& (current != 20))
	{
		return -1;
	}

	// Read content of the register
	uint16_t reg_content = 0;
	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_3, &reg_content);
	if (retval != 0)
	{
		return -2;
	}

	// enforce PS use bit in low byte 
	reg_content |= ps_use_mask;

	// Mask the bit we want to set
	reg_content = reg_content & ~vscel_i_mask;

	uint16_t current_code = 0;
	switch(current)
	{
		case 10: current_code = 2; break;
		case 12: current_code = 3; break;
		case 14: current_code = 4; break;
		case 16: current_code = 5; break;
		case 18: current_code = 6; break;
		case 20: current_code = 7; break;
	}

	// Set new current
	reg_content = reg_content | (current_code << 8);

	retval = register_write_word(VCNL3682XX_PS_CONF_3, reg_content);
	if (retval != 0)
	{
		return -3;
	}

	return 0;
}