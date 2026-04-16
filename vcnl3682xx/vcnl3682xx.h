/*
 * vcnl3682xx.h
 *
 *  Created on: 20 feb 2026
 *      Author: dc1
 */

#ifndef VCNL3682XX_H_
#define VCNL3682XX_H_

#include <stdint.h>

#include "../vcnl3682xx/vcnl3682xx_hal_interface.h"

#define VCNL3682XX_PS_IT_1T		0x00
#define VCNL3682XX_PS_IT_2T		0x01
#define VCNL3682XX_PS_IT_4T		0x02
#define VCNL3682XX_PS_IT_8T		0x03

#define VCNL3682XX_PS_MODE_SHUTDOWN		0
#define VCNL3682XX_PS_MODE_AUTO			1
#define VCNL3682XX_PS_MODE_FORCE		2

void vcnl3682xx_init_hal(vcnl3682xx_read_register_func_t read, vcnl3682xx_write_func_t write);

int vcnl3682xx_read_id(uint16_t* id);

int vcnl3682xx_init(void);

int vcnl3682xx_get_proximity_sensor_mode(uint8_t* mode);

int vcnl3682xx_get_proximity_data(uint16_t* data);

#endif /* VCNL3682XX_H_ */