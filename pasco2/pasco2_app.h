/*
 * pasco2_app.h
 *
 *  Created on: 30 May 2023
 *      Author: jorda
 */

#ifndef PASCO2_PASCO2_APP_H_
#define PASCO2_PASCO2_APP_H_

#include "pasco2.h"

typedef struct
{
	uint16_t co2_ppm;
	uint32_t counter;
} pasco2_app_t;


/**
 * @brief Initializes the module
 */
void pasco2_app_init(pasco2_read_func_t read, pasco2_write_func_t write, pasco2_sleep_func_t sleep);

/**
 * @brief Start the measurement
 * The CO2 sensor will deliver a value every 5 seconds
 * The call to the method will last ~2 seconds (since the method performs a soft reset of the sensor)
 *
 * @retval 0 Success
 * @retval != 0 An error occurred
 */
int pasco2_app_start_measurement(pasco2_app_t* app);

/**
 * @brief Cyclic call to the sensor
 *
 * @retval 0 New value is available (stored inside the app structure)
 * @retval 1 No new value was available - Need to wait
 * @retval -1 Error occurred
 */
int pasco2_app_do(pasco2_app_t* app);



#endif /* PASCO2_PASCO2_APP_H_ */
