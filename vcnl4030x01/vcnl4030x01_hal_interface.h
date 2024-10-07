/*
 * vcnl4030x01_hal_interface.h
 *
 *  Created on: 4 Oct 2024
 *      Author: jorda
 */

#ifndef VCNL4030X01_VCNL4030X01_HAL_INTERFACE_H_
#define VCNL4030X01_VCNL4030X01_HAL_INTERFACE_H_

#include <stdint.h>

typedef int8_t (*vcnl4030x01_read_register_func_t)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
typedef int8_t (*vcnl4030x01_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

#endif /* VCNL4030X01_VCNL4030X01_HAL_INTERFACE_H_ */
