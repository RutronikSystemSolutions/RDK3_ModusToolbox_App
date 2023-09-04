/*
 * bmi270_app.h
 *
 *  Created on: 2021-11-29
 *      Author: GDR
 */

#ifndef BMI270_BMI270_APP_H_
#define BMI270_BMI270_APP_H_

#include "bmi270.h"
#include "common_bmi.h"

/* Macros to select the sensors */
#define ACCEL          UINT8_C(0x00)
#define GYRO           UINT8_C(0x01)

int8_t bmi270_app_init(bmi270_read_func_t read, bmi270_write_func_t write, bmi270_sleep_func_t sleep_us);

int8_t bmi270_app_get_int_status(uint16_t* status);

int8_t bmi270_app_get_sensor_data(struct bmi2_sensor_data *sensor_data, uint8_t n_sens);

#endif /* BMI270_BMI270_APP_H_ */
