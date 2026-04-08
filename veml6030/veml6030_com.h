/* veml6030_com.h
 *
 *  Created on: 25 Mar 2026
 *      Author: DC1
 */

#ifndef VEML6030_COM_H_
#define VEML6030_COM_H_

#include <stdint.h>

#include "veml6030_hal_interface.h"

void veml6030_com_init_hal(veml6030_read_register_func_t read, veml6030_write_func_t write);

int veml6030_com_register_read_word(uint8_t reg, uint16_t* buffer);

int veml6030_com_set_als_shutdown_on_off(uint8_t status);

int veml6030_com_set_als_interrupt_on_off(uint8_t status);

int veml6030_com_set_als_gain(uint8_t gain);

int veml6030_com_set_als_integration_time(uint8_t it);

int veml6030_com_set_power_saving_mode(uint8_t mode);

int veml6030_com_set_power_saving_enable(uint8_t status);

#endif /* VEML6030_COM_H_ */
