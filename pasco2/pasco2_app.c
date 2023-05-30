/*
 * pasco2_app.c
 *
 *  Created on: 30 May 2023
 *      Author: jorda
 */

#include "pasco2_app.h"


void pasco2_app_init(pasco2_read_func_t read, pasco2_write_func_t write, pasco2_sleep_func_t sleep)
{
	pasco2_init_hal(read, write, sleep);
}

int pasco2_app_start_measurement(pasco2_app_t* app)
{
	static const uint16_t refresh_rate_seconds = 5;
	int retval = pasco2_check_i2c_interface();
	if (retval != 0) return -1;

	// Perform a soft reset and wait for startup (2 seconds)
	retval = pasco2_soft_reset(1);
	if (retval != 0) return -2;

	// Get the sensor status and check for error
	uint8_t sens_sts = 0;
	retval = pasco2_get_sensor_status(&sens_sts);
	if (retval != 0) return -3;

	// Check the status
	if (sens_sts & PASCO2_ICCER) return -4;

	if (sens_sts & PASCO2_ORVS) return -5;

	if (sens_sts & PASCO2_ORTMP) return -6;

	if ((sens_sts & PASCO2_RDY) == 0) return -7;


	retval = pasco2_set_iddle_mode();
	if (retval != 0) return -8;

	retval = pasco2_set_period(refresh_rate_seconds);
	if (retval != 0) return -9;

	retval = pasco2_start_continuous_measurement();
	if (retval != 0) return -10;

	app->counter = 0;

	return 0;
}

/**
 * @brief Cyclic call to the sensor
 *
 * @retval 0 New value is available (stored inside the app structure)
 * @retval 1 No new value was available - Need to wait
 * @retval -1 Error occurred
 */
int pasco2_app_do(pasco2_app_t* app)
{
	uint8_t meas_sts = 0;
	int retval = pasco2_get_measurement_status(&meas_sts);
	if (retval != 0) return -1;

	if (meas_sts & PASCO2_DATA_RDY)
	{
		retval = pasco2_read_co2_ppm(&(app->co2_ppm));
		if (retval != 0) return -2;

		app->counter = app->counter + 1;
		return 0;
	}
	return 1;
}


