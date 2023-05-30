/*
 * pasco2.h
 *
 *  Created on: 22 Mar 2023
 *      Author: jorda
 */

#ifndef PASCO2_PASCO2_H_
#define PASCO2_PASCO2_H_

#include <stdint.h>

#define PASCO2_ICCER	(1 << 3)
#define PASCO2_ORVS		(1 << 4)
#define PASCO2_ORTMP	(1 << 5)
#define PASCO2_RDY		(1 << 7)

#define PASCO2_DATA_RDY	(1 << 4)

#define PASCO2_MEAS_RATE_MIN	5
#define PASCO2_MEAS_RATE_MAX	4095

typedef int8_t (*pasco2_read_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef int8_t (*pasco2_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef void (*pasco2_sleep_func_t)(uint32_t ms);


/**
 * @brief Initialize the hardware abstraction layer
 *
 * @param read Pointer to the read function (I2C)
 * @param write Pointer to the write function (I2C)
 * @param sleep Pointer to the sleep function
 */
void pasco2_init_hal(pasco2_read_func_t read, pasco2_write_func_t write, pasco2_sleep_func_t sleep);

/**
 * @brief Check if the I2C interface works or not
 *
 * @retval 0 I2C interface is working
 * @retval != 0 Something wrong occurred
 */
int pasco2_check_i2c_interface();


/**
 * @brief Get the product and revision
 *
 * @retval 0 Success else error
 */
int pasco2_get_product_and_revision(uint8_t* product, uint8_t* rev);

/**
 * @brief Get the status
 */
int pasco2_get_sensor_status(uint8_t* status);

int pasco2_get_measurement_status(uint8_t* status);

int pasco2_read_co2_ppm(uint16_t* co2_ppm);

int pasco2_soft_reset(uint8_t wait_for_startup);

int pasco2_set_iddle_mode();

int pasco2_set_period(uint16_t seconds);

int pasco2_start_continuous_measurement();

#endif /* PASCO2_PASCO2_H_ */
