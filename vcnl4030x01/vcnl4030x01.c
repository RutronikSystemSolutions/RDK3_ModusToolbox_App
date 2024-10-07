/*
 * vcnl4030x01.c
 *
 *  Created on: 1 Oct 2024
 *      Author: jorda
 */

#include "vcnl4030x01.h"
#include "vcnl4030x01_com.h"
#include "vcnl4030x01_def.h"

void vcnl4030x01_init_hal(vcnl4030x01_read_register_func_t read, vcnl4030x01_write_func_t write)
{
	vcnl4030x01_com_init_hal(read, write);
}

int vcnl4030x01_read_id(uint16_t* id)
{
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_ID, id);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}

int vcnl4030x01_init(void)
{
	uint16_t device_id = 0;

	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_ID, &device_id);
	if (retval != 0)
	{
		return -1;
	}

	// Disable the proximity sensor
	retval = vcnl4030x01_com_set_proximity_sensor_on_off(0);
	if (retval != 0)
	{
		return -2;
	}

	// Set automatic mode
	retval = vcnl4030x01_com_set_active_force_on_off(0);
	if (retval != 0)
	{
		return -3;
	}

	// Set integration time
	retval = vcnl4030x01_com_set_integration_time(VCNL4030X01_PS_IT_8T);
	if (retval != 0)
	{
		return -4;
	}

	// Set output size
	retval = vcnl4030x01_com_set_proximity_sensor_output_size(16);
	if (retval != 0)
	{
		return -5;
	}

	// Enable sunlight cancellation
	retval = vcnl4030x01_com_set_sunlight_cancellation_on_off(1);
	if (retval != 0)
	{
		return -6;
	}

	// Set LED current
	retval = vcnl4030x01_com_set_led_current(200);
	if (retval != 0)
	{
		return -7;
	}

	// Enable the proximity sensor
	retval = vcnl4030x01_com_set_proximity_sensor_on_off(1);
	if (retval != 0)
	{
		return -8;
	}

	return 0;
}

int vcnl4030x01_get_proximity_sensor_mode(uint8_t* mode)
{
	// First, read PS_SD bit
	uint16_t reg = 0;
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_CONF_1, &reg);
	if (retval != 0)
	{
		return -1;
	}

	if ((reg & 0x1) == 0x1)
	{
		// PS_SD = 1 -> shut-down
		*mode = VCNL4030X01_PS_MODE_SHUTDOWN;
		return 0;
	}

	// Get PS_AF status
	retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_CONF_3, &reg);
	if (retval != 0)
	{
		return -2;
	}

	if ((reg & 0x8) == 0x0)
	{
		// PS_AF = 0 -> active force mode disabled -> auto/self-timed mode
		*mode = VCNL4030X01_PS_MODE_AUTO;
		return 0;
	}
	else
	{
		// PS_AF = 1 -> active force mode enabled
		*mode = VCNL4030X01_PS_MODE_FORCE;
		return 0;
	}


	return 0;
}

int vcnl4030x01_get_proximity_data(uint16_t* data)
{
	int retval = vcnl4030x01_com_register_read_word(VCNL4030X01_PS_DATA, data);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}
