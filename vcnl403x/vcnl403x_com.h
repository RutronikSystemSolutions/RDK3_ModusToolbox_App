/* 
 * vcnl403x_com.h
 *
 *  Created on: 20 Feb 2026
 *      Author: DC1
 */

#ifndef VCNL403X_COM_H_
#define VCNL403X_COM_H_

#include <stdint.h>

#include "vcnl403x_hal_interface.h"

void vcnl403x_com_init_hal(vcnl403x_read_register_func_t read, vcnl403x_write_func_t write);

int vcnl403x_com_register_read_word(uint8_t reg, uint16_t *buffer);

int vcnl403x_com_set_proximity_sensor_on_off(uint8_t status);

int vcnl403x_com_set_active_force_on_off(uint8_t status);

int vcnl403x_com_set_integration_time(uint8_t integration_time);

int vcnl403x_com_set_proximity_sensor_output_size(uint8_t size);

int vcnl403x_com_set_sunlight_cancellation_on_off(uint8_t status);

int vcnl403x_com_set_led_current(uint8_t current);

int vcnl403x_com_set_gesture_mode_on_off(uint8_t status);
int vcnl403x_com_set_gesture_interrupt_on_off(uint8_t status);
int vcnl403x_com_trigger_gesture_sequence(void);

int vcnl403x_com_read_int_flag(uint8_t *int_flag);

int vcnl403x_com_read_ps1(uint16_t *data);
int vcnl403x_com_read_ps2(uint16_t *data);
int vcnl403x_com_read_ps3(uint16_t *data);

#endif /* VCNL403X_COM_H_ */