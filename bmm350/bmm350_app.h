/*
 * bmm350_app.h
 *
 *  Created on: 2025-04-03
 *      Author: GDR
 */

#ifndef BMM350_BMM350_APP_H_
#define BMM350_BMM350_APP_H_

#include "bmm350_defs.h"
#include "bmm_driver.h"
#include "bmm350.h"

/**
 * @brief Check if the BMM350 is available or not
 *
 * @retval 0: not available
 * @retval 1: available
 */
int8_t bmm350_app_is_available();

int8_t bmm350_app_init(void);
int8_t bmm350_app_read_data(float *temp, float *mag_x, float *mag_y, float *mag_z);

#endif /* BMM350_BMM350_APP_H_ */
