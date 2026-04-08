/* veml6046x00_hal_interface.h
 *
 *  Created on: 24 Feb 2026
 *      Author: dc1
 */

#ifndef VEML6046X00_HAL_INTERFACE_H_
#define VEML6046X00_HAL_INTERFACE_H_

#include <stdint.h>

typedef int8_t (*veml6046x00_read_register_func_t)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
typedef int8_t (*veml6046x00_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

#endif /* VEML6046X00_HAL_INTERFACE_H_ */