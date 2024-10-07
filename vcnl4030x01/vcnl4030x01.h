/*
 * vcnl4030x01.h
 *
 *  Created on: 1 Oct 2024
 *      Author: jorda
 */

#ifndef VCNL4030X01_H_
#define VCNL4030X01_H_

#include <stdint.h>

#include "vcnl4030x01_hal_interface.h"

#define VCNL4030X01_PS_IT_1T		0x00
#define VCNL4030X01_PS_IT_1_5T		0x01
#define VCNL4030X01_PS_IT_2T		0x02
#define VCNL4030X01_PS_IT_2_5T		0x03
#define VCNL4030X01_PS_IT_3T		0x04
#define VCNL4030X01_PS_IT_3_5T		0x05
#define VCNL4030X01_PS_IT_4T		0x06
#define VCNL4030X01_PS_IT_8T		0x07


#define VCNL4030X01_PS_MODE_SHUTDOWN	0
#define VCNL4030X01_PS_MODE_AUTO		1
#define VCNL4030X01_PS_MODE_FORCE		2

void vcnl4030x01_init_hal(vcnl4030x01_read_register_func_t read, vcnl4030x01_write_func_t write);

int vcnl4030x01_read_id(uint16_t* id);

int vcnl4030x01_init(void);

int vcnl4030x01_get_proximity_sensor_mode(uint8_t* mode);

int vcnl4030x01_get_proximity_data(uint16_t* data);

#endif /* VCNL4030X01_H_ */
