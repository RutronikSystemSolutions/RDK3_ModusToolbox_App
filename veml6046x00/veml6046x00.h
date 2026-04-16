/* veml6046x00.h
 *
 *  Created on: 24 Feb 2026
 *      Author: dc1
 */

#ifndef VEML6046X00_H_
#define VEML6046X00_H_

#include <stdint.h>

#include "veml6046x00_hal_interface.h"

#define VEML6046X00_RGB_GAIN_X1      0x00
#define VEML6046X00_RGB_GAIN_X2      0x01
#define VEML6046X00_RGB_GAIN_X0_66   0x02
#define VEML6046X00_RGB_GAIN_X0_5    0x03

#define VEML6046X00_RGB_IT_3_125MS   0x00
#define VEML6046X00_RGB_IT_6_25MS    0x01
#define VEML6046X00_RGB_IT_12_5MS    0x02
#define VEML6046X00_RGB_IT_25MS      0x03
#define VEML6046X00_RGB_IT_50MS      0x04
#define VEML6046X00_RGB_IT_100MS     0x05
#define VEML6046X00_RGB_IT_200MS     0x06
#define VEML6046X00_RGB_IT_400MS     0x07

void veml6046x00_init_hal(veml6046x00_read_register_func_t read, veml6046x00_write_func_t write);

int veml6046x00_read_id(uint16_t* id);

int veml6046x00_init(void);

int veml6046x00_get_r_data(uint16_t* data);
int veml6046x00_get_g_data(uint16_t* data);
int veml6046x00_get_b_data(uint16_t* data);
int veml6046x00_get_ir_data(uint16_t* data);

#endif /* VEML6046X00_H_ */