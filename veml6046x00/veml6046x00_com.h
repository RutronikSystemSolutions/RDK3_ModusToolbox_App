/* veml6046x00_com.h
 *
 *  Created on: 24 Feb 2026
 *      Author: dc1
 */

#ifndef VEML6046X00_COM_H_
#define VEML6046X00_COM_H_

#include <stdint.h>

#include "veml6046x00_hal_interface.h"

void veml6046x00_com_init_hal(veml6046x00_read_register_func_t read, veml6046x00_write_func_t write);

int veml6046x00_com_register_read_word(uint8_t reg, uint16_t* buffer);

int veml6046x00_com_set_rgb_shutdown_on_off(uint8_t status);

int veml6046x00_com_set_rgb_interrupt_on_off(uint8_t status);

int veml6046x00_com_set_rgb_gain(uint8_t gain);

int veml6046x00_com_set_rgb_integration_time(uint8_t it);

#endif /* VEML6046X00_COM_H_ */