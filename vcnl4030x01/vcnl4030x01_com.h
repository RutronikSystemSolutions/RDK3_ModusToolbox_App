/*
 * vcnl4030x01_com.h
 *
 *  Created on: 4 Oct 2024
 *      Author: jorda
 */

#ifndef VCNL4030X01_COM_H_
#define VCNL4030X01_COM_H_

#include <stdint.h>

#include "vcnl4030x01_hal_interface.h"

void vcnl4030x01_com_init_hal(vcnl4030x01_read_register_func_t read, vcnl4030x01_write_func_t write);

int vcnl4030x01_com_register_read_word(uint8_t reg, uint16_t* buffer);

int vcnl4030x01_com_set_proximity_sensor_on_off(uint8_t status);

int vcnl4030x01_com_set_active_force_on_off(uint8_t status);

int vcnl4030x01_com_set_integration_time(uint8_t integration_time);

int vcnl4030x01_com_set_proximity_sensor_output_size(uint8_t size);

int vcnl4030x01_com_set_sunlight_cancellation_on_off(uint8_t status);

int vcnl4030x01_com_set_led_current(uint8_t current);

#endif /* VCNL4030X01_COM_H_ */
