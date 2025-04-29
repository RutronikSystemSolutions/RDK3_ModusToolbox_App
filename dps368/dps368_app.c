/*
 * dps368_app.c
 *
 *  Created on: 2025-04-03
 *      Author: GDR
 */


#include "dps368_app.h"

xensiv_dps3xx_t dps368_sensor = {0};
xensiv_dps3xx_config_t dps368_config =
{
    .dev_mode               = XENSIV_DPS3XX_MODE_BACKGROUND_ALL, 	// Set device to read temp & pressure
    .pressure_rate          = XENSIV_DPS3XX_RATE_4,        			// 1x sample rate for pressure
    .temperature_rate       = XENSIV_DPS3XX_RATE_4,        			// 1x sample rate for data
    .pressure_oversample    = XENSIV_DPS3XX_OVERSAMPLE_1,  			// 1x oversample for pressure
    .temperature_oversample = XENSIV_DPS3XX_OVERSAMPLE_1,  			// 1x oversample for temp
    .data_timeout           = 1,                      				// Wait up to 500ms for measurement data
    .i2c_timeout            = 5,                       				// Wait up to 10ms for i2c operations
};

int8_t dps368_app_init(void)
{
	int8_t rslt = 0;
	cy_rslt_t result = CY_RSLT_SUCCESS;

    result = xensiv_dps3xx_mtb_init_i2c(&dps368_sensor, XENSIV_DPS3XX_I2C_ADDR_DEFAULT);
    if(result != CY_RSLT_SUCCESS)
    {
    	return -1;
    }
    result = xensiv_dps3xx_set_config(&dps368_sensor, &dps368_config);
    if(result != CY_RSLT_SUCCESS)
    {
    	return -1;
    }

	return rslt;
}

int8_t dps368_read_data(float *temp, float *press)
{
	int8_t rslt = 0;
	cy_rslt_t result = CY_RSLT_SUCCESS;

	result = xensiv_dps3xx_read(&dps368_sensor, press, temp);
    if(result != CY_RSLT_SUCCESS)
    {
    	return -1;
    }

	return rslt;
}

