/* 
 * vcnl403x.h
 *
 *  Created on: 20 Feb 2026
 *      Author: dc1
 */

#ifndef VCNL403X_H_
#define VCNL403X_H_

#include <stdint.h>

#include "vcnl403x_hal_interface.h"

#define VCNL403X_PS_IT_1T     0x00
#define VCNL403X_PS_IT_1_5T   0x01
#define VCNL403X_PS_IT_2T     0x02
#define VCNL403X_PS_IT_2_5T   0x03
#define VCNL403X_PS_IT_3T     0x04
#define VCNL403X_PS_IT_3_5T   0x05
#define VCNL403X_PS_IT_4T     0x06
#define VCNL403X_PS_IT_8T     0x07

#define VCNL403X_PS_MODE_SHUTDOWN   0
#define VCNL403X_PS_MODE_AUTO       1
#define VCNL403X_PS_MODE_FORCE      2

#define VCNL403X_INTFLAG_GESTURE_DATA_READY  (1u << 7)

typedef enum
{
    VCNL403X_VARIANT_UNKNOWN = 0,
    VCNL403X_VARIANT_SINGLE_PS = 1,
    VCNL403X_VARIANT_GESTURE_PS = 2
} vcnl403x_variant_t;

void vcnl403x_init_hal(vcnl403x_read_register_func_t read, vcnl403x_write_func_t write);

int vcnl403x_read_id(uint16_t *id);

int vcnl403x_init(void);

int vcnl403x_get_variant(vcnl403x_variant_t *variant);

int vcnl403x_get_proximity_channel_count(uint8_t *channel_count);

int vcnl403x_trigger_measurement(void);

int vcnl403x_get_proximity_sensor_mode(uint8_t *mode);

int vcnl403x_get_proximity_data(uint16_t *ps1, uint16_t *ps2, uint16_t *ps3);

#endif /* VCNL403X_H_ */