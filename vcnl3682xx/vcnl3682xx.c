/*
 * vcnl3682xx.c
 *
 *  Created on: 20 feb 2026
 *      Author: dc1
 */

#include "vcnl3682xx.h"
#include "vcnl3682xx_com.h"
#include "vcnl3682xx_def.h"

void vcnl3682xx_init_hal(vcnl3682xx_read_register_func_t read, vcnl3682xx_write_func_t write)
{
	vcnl3682xx_com_init_hal(read, write);
}

int vcnl3682xx_read_id(uint16_t* id)
{
    int retval = 0;

    if (id == 0)
    {
        return -1;
    }

    retval = vcnl3682xx_com_register_read_word(VCNL3682XX_ID, id);
    if (retval != 0)
    {
        return -2;
    }

    /* expected: ID_L = 0x26, ID_M usually 0x00 at address 0x60 */
    if ((*id & 0x00FF) != 0x26)
    {
        return -3;
    }

    return 0;
}

int vcnl3682xx_init(void)
{
	uint16_t device_id = 0;

	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_ID, &device_id);
	if (retval != 0)
	{
		return -1;
	}

	// Disable the proximity sensor
	retval = vcnl3682xx_com_set_proximity_sensor_on_off(0);
	if (retval != 0)
	{
		return -2;
	}

	// Set automatic mode
	retval = vcnl3682xx_com_set_active_force_on_off(0);
	if (retval != 0)
	{
		return -3;
	}

	// Set integration time
	retval = vcnl3682xx_com_set_integration_time(VCNL3682XX_PS_IT_8T);
	if (retval != 0)
	{
		return -4;
	}

	// Set output size
	retval = vcnl3682xx_com_set_proximity_sensor_output_size(12);
	if (retval != 0)
	{
		return -5;
	}

	// Enable sunlight cancellation
	retval = vcnl3682xx_com_set_sunlight_cancellation_on_off(1);
	if (retval != 0)
	{
		return -6;
	}

	// Set VCSEL current
	retval = vcnl3682xx_com_set_vcsel_current(20);
	if (retval != 0)
	{
		return -7;
	}

	// Enable the proximity sensor
	retval = vcnl3682xx_com_set_proximity_sensor_on_off(1);
	if (retval != 0)
	{
		return -8;
	}

	return 0;
}

int vcnl3682xx_get_proximity_sensor_mode(uint8_t* mode)
{
	// First, read PS_ON bit
	uint16_t reg = 0;
	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_1, &reg);
	if (retval != 0)
	{
		return -1;
	}

	uint8_t ps_on = ((reg & (0x1 << 1)) != 0) ? 1 : 0;

	// Read PS_ST bit
	retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_2, &reg);
	if (retval != 0)
	{
		return -2;
	}

	uint8_t ps_st_stop = ((reg & 0x1) != 0) ? 1 : 0;

	if ((ps_on == 0) || (ps_st_stop == 1))
	{
		// Sensor off or single trigger stopped -> shut-down
		*mode = VCNL3682XX_PS_MODE_SHUTDOWN;
		return 0;
	}

	// Get PS_AF status
	retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_CONF_3, &reg);
	if (retval != 0)
	{
		return -3;
	}

	if ((reg & (0x1 << 6)) == 0x0)
	{
		// PS_AF = 0 -> active force mode disabled -> auto/self-timed mode
		*mode = VCNL3682XX_PS_MODE_AUTO;
		return 0;
	}
	else
	{
		// PS_AF = 1 -> active force mode enabled
		*mode = VCNL3682XX_PS_MODE_FORCE;
		return 0;
	}

	return 0;
}

int vcnl3682xx_get_proximity_data(uint16_t* data)
{
	int retval = vcnl3682xx_com_register_read_word(VCNL3682XX_PS_DATA, data);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}