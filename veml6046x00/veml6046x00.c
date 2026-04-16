/* veml6046x00.c
 *
 *  Created on: 24 Feb 2026
 *      Author: dc1
 */

#include "veml6046x00.h"
#include "veml6046x00_com.h"
#include "veml6046x00_def.h"

void veml6046x00_init_hal(veml6046x00_read_register_func_t read, veml6046x00_write_func_t write)
{
	veml6046x00_com_init_hal(read, write);
}


int veml6046x00_read_id(uint16_t* id)
{
    int retval = 0;

    if (id == 0)
    {
        return -1;
    }

    retval = veml6046x00_com_register_read_word(VEML6046X00_ID_L, id);
    if (retval != 0)
    {
        return -2;
    }

    /* expected: ID_L = 0x80, ID_M usually 0x00 at address 0x60 */
    if ((*id & 0x00FF) != 0x01)
    {
        return -3;
    }

    return 0;
}

int veml6046x00_init(void)
{
	uint16_t device_id = 0;

	int retval = veml6046x00_com_register_read_word(VEML6046X00_ID_L, &device_id);
	if (retval != 0)
	{
		return -1;
	}

	// Disable interrupt 
	retval = veml6046x00_com_set_rgb_interrupt_on_off(0);
	if (retval != 0)
	{
		return -2;
	}

	// Set gain 
	retval = veml6046x00_com_set_rgb_gain(VEML6046X00_RGB_GAIN_X1);
	if (retval != 0)
	{
		return -3;
	}

	// Set integration time 
	retval = veml6046x00_com_set_rgb_integration_time(VEML6046X00_RGB_IT_100MS);
	if (retval != 0)
	{
		return -4;
	}

	// Power on RGB 
	retval = veml6046x00_com_set_rgb_shutdown_on_off(0);
	if (retval != 0)
	{
		return -5;
	}

	return 0;
}

int veml6046x00_get_r_data(uint16_t* data)
{
	int retval = veml6046x00_com_register_read_word(VEML6046X00_R_DATA_L, data);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}

int veml6046x00_get_g_data(uint16_t* data)
{
	int retval = veml6046x00_com_register_read_word(VEML6046X00_G_DATA_L, data);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}

int veml6046x00_get_b_data(uint16_t* data)
{
	int retval = veml6046x00_com_register_read_word(VEML6046X00_B_DATA_L, data);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}

int veml6046x00_get_ir_data(uint16_t* data)
{
	int retval = veml6046x00_com_register_read_word(VEML6046X00_IR_DATA_L, data);
	if (retval != 0)
	{
		return -1;
	}

	return 0;
}