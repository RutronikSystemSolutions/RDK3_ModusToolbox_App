/*
 * vcnl403x_hal_interface.h
 *
 *  Created on: 20 Feb 2026
 *      Author: dc1
 */

#ifndef VCNL403X_VCNL403X_HAL_INTERFACE_H_
#define VCNL403X_VCNL403X_HAL_INTERFACE_H_

#include <stdint.h>

typedef int8_t (*vcnl403x_read_register_func_t)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
typedef int8_t (*vcnl403x_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

#endif /* VCNL403X_VCNL403X_HAL_INTERFACE_H_ */