/*
 * sht4x.h
 *
 *  Created on: 10 Mar 2023
 *      Author: jorda
 */

#ifndef SHT4X_SHT4X_H_
#define SHT4X_SHT4X_H_

#include <stdint.h>

typedef int8_t (*sht4x_read_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef int8_t (*sht4x_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef void (*sht4x_sleep_func_t)(uint32_t ms);

void sht4x_init(sht4x_read_func_t read, sht4x_write_func_t write, sht4x_sleep_func_t sleep);

int sht4x_get_serial_id(uint32_t * id);

int sht4x_get_raw_measurement(uint16_t* temperature, uint16_t* humidity);

int sht4x_get_temperature_and_humidity(float* temperature, float* humidity);

#endif /* SHT4X_SHT4X_H_ */
