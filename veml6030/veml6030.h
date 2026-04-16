/* veml6030.h
 *
 *  Created on: 25 Mar 2026
 *      Author: Dc1
 */

#ifndef VEML6030_H_
#define VEML6030_H_

#include <stdint.h>

#include "veml6030_hal_interface.h"

#define VEML6030_ALS_GAIN_X1        0x00
#define VEML6030_ALS_GAIN_X2        0x01
#define VEML6030_ALS_GAIN_X1_8      0x02
#define VEML6030_ALS_GAIN_X1_4      0x03

#define VEML6030_ALS_IT_25MS        0x0C
#define VEML6030_ALS_IT_50MS        0x08
#define VEML6030_ALS_IT_100MS       0x00
#define VEML6030_ALS_IT_200MS       0x01
#define VEML6030_ALS_IT_400MS       0x02
#define VEML6030_ALS_IT_800MS       0x03

#define VEML6030_PSM_MODE_1         0x00
#define VEML6030_PSM_MODE_2         0x01
#define VEML6030_PSM_MODE_3         0x02
#define VEML6030_PSM_MODE_4         0x03

void veml6030_init_hal(veml6030_read_register_func_t read, veml6030_write_func_t write);

int veml6030_read_id(uint16_t* id);

int veml6030_init(void);

int veml6030_get_als_data(uint16_t* data);

int veml6030_get_white_data(uint16_t* data);

#endif /* VEML6030_H_ */
