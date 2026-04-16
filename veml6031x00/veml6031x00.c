/* veml6031x00.c
 *
 *  Created on: 24 Feb 2026
 *      Author: dc1
 */

#include "veml6031x00.h"
#include "veml6031x00_com.h"
#include "veml6031x00_def.h"

void veml6031x00_init_hal(veml6031x00_read_register_func_t read, veml6031x00_write_func_t write)
{
	veml6031x00_com_init_hal(read, write);
}


int veml6031x00_read_id(uint16_t* id)
{
    int retval = 0;

    if (id == 0)
        return -1;

    retval = veml6031x00_com_register_read_word(VEML6031X00_ID_L, id);
    if (retval != 0)
        return -2;

    if ((*id & 0x00FF) != 0x01)
        return -3;

    return 0;
}


int veml6031x00_init(void)
{
	uint16_t device_id = 0;

	int retval = veml6031x00_com_register_read_word(VEML6031X00_ID_L, &device_id);
	if (retval != 0)
	{
		return -1;
	}

	// Disable interrupt 
	retval = veml6031x00_com_set_als_interrupt_on_off(0);
	if (retval != 0)
	{
		return -2;
	}

	// Set gain 
	retval = veml6031x00_com_set_als_gain(VEML6031X00_ALS_GAIN_X1);
	if (retval != 0)
	{
		return -3;
	}

	// Set integration time 
	retval = veml6031x00_com_set_als_integration_time(VEML6031X00_ALS_IT_100MS);
	if (retval != 0)
	{
		return -4;
	}

	// Power on ALS 
	retval = veml6031x00_com_set_als_shutdown_on_off(0);
	if (retval != 0)
	{
		return -5;
	}

	return 0;
}

int veml6031x00_get_als_data(uint16_t* data)
{
	int retval = veml6031x00_com_register_read_word(VEML6031X00_ALS_DATA_L, data);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}

int veml6031x00_get_ir_data(uint16_t* data)
{
	int retval = veml6031x00_com_register_read_word(VEML6031X00_IR_DATA_L, data);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}