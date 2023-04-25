/*
 * bmp581.h
 *
 *  Created on: 13 Mar 2023
 *      Author: jorda
 */

#ifndef BMP581_BMP581_H_
#define BMP581_BMP581_H_

#include <stdint.h>

#define BMP51_NON_STOP_MODE 0x3


typedef int8_t (*bmp581_read_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef int8_t (*bmp581_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);


void bmp581_init_i2c_interface(bmp581_read_func_t read, bmp581_write_func_t write);

int bmp581_get_chip_id(uint8_t* chip_id);

/**
 * Set pressure and temperature over sampling modes
 *
 * @param [in] pressure (0 .. 7) (x1 -> x128)	   [0x7 -> 128x]
 * @param [in] temperature (0 .. 7) (x1 -> x128)   [0x3 -> 8x]
 * @param [in] press_en (0 or 1)
 *
 * Highest resolution: Pressure = 128 / Temperature = 8 => 12 Hz ODR
 */
int bmp581_set_oversampling_mode(uint8_t pressure, uint8_t temperature, uint8_t press_en);

/**
 * Set the power mode and the output data rate
 *
 * @param [in] power_mode
 * 				0: no measurement on-going
 * 				1: normal mode
 * 				2: forced mode
 * 				3: non-stop mode (ignore ODR)
 *
 * @param [in] odr (0x0 .. 0x1F) (240Hz -> 0.125Hz)   [0x1B -> 2Hz]
 */
int bmp581_set_power_mode(uint8_t power_mode, uint8_t odr);

int bmp581_read_pressure_and_temperature(float* pressure, float* temperature);


#endif /* BMP581_BMP581_H_ */
