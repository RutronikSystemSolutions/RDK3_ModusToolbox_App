/* veml6030_hal_interface.h
 *
 *  Created on: 25 Mar 2026
 *      Author: DC1
 */

#ifndef VEML6030_HAL_INTERFACE_H_
#define VEML6030_HAL_INTERFACE_H_

#include <stdint.h>

typedef int8_t (*veml6030_read_register_func_t)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
typedef int8_t (*veml6030_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

#endif /* VEML6030_HAL_INTERFACE_H_ */
