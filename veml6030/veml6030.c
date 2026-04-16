/* veml6030.c
 *
 *  Created on: 25 Mar 2026
 *      Author: DC1
 */

#include "veml6030.h"
#include "veml6030_com.h"
#include "veml6030_def.h"

void veml6030_init_hal(veml6030_read_register_func_t read, veml6030_write_func_t write)
{
	veml6030_com_init_hal(read, write);
}

int veml6030_read_id(uint16_t* id)
{
	uint8_t expected_id_high = 0;
	int retval = 0;

	if (id == 0)
	{
		return -1;
	}

	retval = veml6030_com_register_read_word(VEML6030_ID, id);
	if (retval != 0)
	{
		return -2;
	}

#if (VEML6030_ADDR == 0x10)
	expected_id_high = 0xC4;
#else
	expected_id_high = 0xD4;
#endif

	if (((*id & 0x00FFu) != 0x81u) || (((*id >> 8) & 0x00FFu) != expected_id_high))
	{
		return -3;
	}

	return 0;
}

int veml6030_init(void)
{
	uint16_t device_id = 0;

	int retval = veml6030_read_id(&device_id);
	if (retval != 0)
	{
		return -1;
	}

	/* Disable power saving */
	retval = veml6030_com_set_power_saving_enable(0);
	if (retval != 0)
	{
		return -2;
	}

	/* Disable interrupt */
	retval = veml6030_com_set_als_interrupt_on_off(0);
	if (retval != 0)
	{
		return -3;
	}

	/* Set gain */
	retval = veml6030_com_set_als_gain(VEML6030_ALS_GAIN_X1);
	if (retval != 0)
	{
		return -4;
	}

	/* Set integration time */
	retval = veml6030_com_set_als_integration_time(VEML6030_ALS_IT_100MS);
	if (retval != 0)
	{
		return -5;
	}

	/* Power on ALS */
	retval = veml6030_com_set_als_shutdown_on_off(0);
	if (retval != 0)
	{
		return -6;
	}

	return 0;
}

int veml6030_get_als_data(uint16_t* data)
{
	if (data == 0)
	{
		return -1;
	}

	int retval = veml6030_com_register_read_word(VEML6030_ALS, data);
	if (retval != 0)
	{
		return -2;
	}

	return 0;
}

int veml6030_get_white_data(uint16_t* data)
{
	if (data == 0)
	{
		return -1;
	}

	int retval = veml6030_com_register_read_word(VEML6030_WHITE, data);
	if (retval != 0)
	{
		return -2;
	}

	return 0;
}
