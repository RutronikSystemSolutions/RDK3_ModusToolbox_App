/*
 * vcnl3682xx_com.h
 *
 *  Created on: 20 feb 2026
 *      Author: dc1
 */

#ifndef VCNL3682XX_COM_H_
#define VCNL3682XX_COM_H_

#include <stdint.h>

#include "../vcnl3682xx/vcnl3682xx_hal_interface.h"

void vcnl3682xx_com_init_hal(vcnl3682xx_read_register_func_t read, vcnl3682xx_write_func_t write);

int vcnl3682xx_com_register_read_word(uint8_t reg, uint16_t* buffer);

int vcnl3682xx_com_set_proximity_sensor_on_off(uint8_t status);

int vcnl3682xx_com_set_active_force_on_off(uint8_t status);

int vcnl3682xx_com_set_integration_time(uint8_t integration_time);

int vcnl3682xx_com_set_proximity_sensor_output_size(uint8_t size);

int vcnl3682xx_com_set_sunlight_cancellation_on_off(uint8_t status);

int vcnl3682xx_com_set_vcsel_current(uint8_t current);


#endif /* VCNL3682XX_COM_H_ */