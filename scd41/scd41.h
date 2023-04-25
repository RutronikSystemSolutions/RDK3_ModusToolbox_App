/*
 * scd41.h
 *
 *  Created on: 22 Mar 2023
 *      Author: jorda
 */

#ifndef SCD41_SCD41_H_
#define SCD41_SCD41_H_

#include <stdint.h>

typedef struct
{
	uint16_t word0;
	uint16_t word1;
	uint16_t word2;
} scd41_serial_id_t;

typedef int8_t (*scd41_read_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef int8_t (*scd41_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef void (*scd41_sleep_func_t)(uint32_t ms);

void scd41_init(scd41_read_func_t read, scd41_write_func_t write, scd41_sleep_func_t sleep);

/**
 * @brief Get the serial number of the device
 */
int scd41_get_serial_number(scd41_serial_id_t* id);

/**
 * @brief Start a periodic measurement (signal update -> every 5 seconds)
 */
int scd41_start_periodic_measurement();

/**
 * @brief Reinitializes the sensor
 */
int scd41_reinit();

/**
 * @brief Stop the periodic measurement
 *
 * Remark: can be also used to init the sensor at boot.
 */
int scd41_stop_periodic_measurement();

/**
 * @brief Check if a measurement is available or not
 */
int scd41_get_data_ready_status(uint8_t* flag);

/**
 * @brief Read a measurement value
 *
 * Remark: make sure a data is available before calling that function using scd41_get_data_ready_status
 */
int scd41_read_measurement(uint16_t* co2_ppm, float* temperature, float* humidity);

int scd41_perform_self_test();

int scd41_wake_up();

#endif /* SCD41_SCD41_H_ */
