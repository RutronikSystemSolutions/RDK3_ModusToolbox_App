/*
 * bmi323_app.h
 *
 *  Created on: 2025-04-03
 *      Author: GDR
 */

#ifndef BMI323_BMI323_APP_H_
#define BMI323_BMI323_APP_H_

#include "bmi3_defs.h"
#include "bmi323_defs.h"
#include "bmi_driver.h"
#include "bmi323.h"

int8_t bmi323_app_init(void);
int8_t bmi323_int_status(_Bool *acc_rdy, _Bool *gyr_rdr);
int8_t bmi323_read_acc_data(int16_t *bmi_acc_x, int16_t *bmi_acc_y, int16_t *bmi_acc_z);
int8_t bmi323_read_gyr_data(int16_t *bmi_gyr_x, int16_t *bmi_gyr_y, int16_t *bmi_gyr_z);

#endif /* BMI323_BMI323_APP_H_ */
