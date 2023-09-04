/*
 * dps310_app.h
 *
 *  Created on: 1 Aug 2023
 *      Author: jorda
 */

#ifndef DPS310_DPS310_APP_H_
#define DPS310_DPS310_APP_H_

#include "dps310.h"

void dps310_app_init_i2c_interface(dps310_read_func_t read, dps310_write_func_t write);

void dps310_app_init();

int dps310_app_do();

void dps310_app_get_last_values(float* temperature, float* pressure);

#endif /* DPS310_DPS310_APP_H_ */
