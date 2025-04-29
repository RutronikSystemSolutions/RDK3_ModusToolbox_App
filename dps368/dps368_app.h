/*
 * dps368_app.h
 *
 *  Created on: 2025-04-03
 *      Author: GDR
 */

#ifndef DPS368_DPS368_APP_H_
#define DPS368_DPS368_APP_H_

#include "hal_i2c.h"
#include "hal_sleep.h"
#include "xensiv_dps3xx.h"
#include "xensiv_dps3xx_mtb.h"

int8_t dps368_app_init(void);
int8_t dps368_read_data(float *temp, float *press);


#endif /* DPS368_DPS368_APP_H_ */
