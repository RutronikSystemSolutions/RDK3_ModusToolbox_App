/*
 * vcnl3682xx_hal_interface.h
 *
 *  Created on: 20 feb 2026
 *      Author: dc1
 */

#ifndef VCNL3682XX_VCNL3682XX_HAL_INTERFACE_H_
#define VCNL3682XX_VCNL3682XX_HAL_INTERFACE_H_

#include <stdint.h>

typedef int8_t (*vcnl3682xx_read_register_func_t)(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
typedef int8_t (*vcnl3682xx_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

#endif /* VCNL3682XX_VCNL3682XX_HAL_INTERFACE_H_ */