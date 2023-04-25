/*
 * sgp40.h
 *
 *  Created on: 9 Mar 2023
 *      Author: jorda
 */

#ifndef SGP40_SGP40_H_
#define SGP40_SGP40_H_

#include <stdint.h>

typedef struct
{
	uint32_t msb;
	uint32_t lsb;
} sgp40_serial_id_t;

typedef int8_t (*sgp40_read_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef int8_t (*sgp40_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef void (*sgp40_sleep_func_t)(uint32_t ms);


void sgp40_init(sgp40_read_func_t read, sgp40_write_func_t write, sgp40_sleep_func_t sleep);

/**
 * @brief Get the serial number of the SGP40
 *
 * @retval 0 Success else error
 */
int sgp40_get_serial_id(sgp40_serial_id_t * id);

/**
 * @brief Get the raw signal measured by the SGP40 without humidity compensation
 *
 * @retval 0 Success else error
 */
int sgp40_measure_raw_signal_without_compensation(uint16_t * voc_value);

/**
 * @brief Get the VOC measurement using internal temperature and humidity compensation
 *
 * @retval 0 Success else error
 */
int sgp40_measure_with_compensation(float temperature, float humidity, uint16_t* voc_value);


#endif /* SGP40_SGP40_H_ */
