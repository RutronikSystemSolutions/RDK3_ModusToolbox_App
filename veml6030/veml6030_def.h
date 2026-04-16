/* veml6030_def.h
 *
 *  Created on: 25 Mar 2026
 *      Author: DC1
 */

#ifndef VEML6030_DEF_H_
#define VEML6030_DEF_H_

/*
 * 7-bit I2C address.
 * Datasheet options:
 *   ADDR = VDD  -> 0x48
 *   ADDR = GND  -> 0x10
 */
#define VEML6030_ADDR               0x10

#define VEML6030_ALS_CONF           0x00
#define VEML6030_ALS_WH             0x01
#define VEML6030_ALS_WL             0x02
#define VEML6030_POWER_SAVING       0x03
#define VEML6030_ALS                0x04
#define VEML6030_WHITE              0x05
#define VEML6030_ALS_INT            0x06
#define VEML6030_ID                 0x07

#endif /* VEML6030_DEF_H_ */
