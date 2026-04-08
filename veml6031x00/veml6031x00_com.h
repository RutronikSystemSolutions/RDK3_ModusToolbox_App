/* veml6031x00_com.h
 *
 *  Created on: 24 Feb 2026
 *      Author: dc1
 */

#ifndef VEML6031X00_COM_H_
#define VEML6031X00_COM_H_

#include <stdint.h>

#include "veml6031x00_hal_interface.h"

void veml6031x00_com_init_hal(veml6031x00_read_register_func_t read, veml6031x00_write_func_t write);

int veml6031x00_com_register_read_word(uint8_t reg, uint16_t* buffer);

int veml6031x00_com_set_als_shutdown_on_off(uint8_t status);

int veml6031x00_com_set_als_interrupt_on_off(uint8_t status);

int veml6031x00_com_set_als_gain(uint8_t gain);

int veml6031x00_com_set_als_integration_time(uint8_t it);

#endif /* VEML6031X00_COM_H_ */