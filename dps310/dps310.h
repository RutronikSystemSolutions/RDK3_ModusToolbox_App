/*
 * dps310.h
 *
 *  Created on: 1 Aug 2023
 *      Author: jorda
 */

#ifndef DPS310_DPS310_H_
#define DPS310_DPS310_H_

#include <stdint.h>

#define DPS310_RAW_VALUES_LENGTH 6

/** Struct to hold calibration coefficients read from device*/
typedef struct
{
    /* calibration registers */
    int16_t C0;       /**< 12bit temperature calibration coefficient read from sensor */
    int16_t C1;       /**< 12bit temperature calibration coefficient read from sensor */
    int32_t C00;      /**< 12bit pressure calibration coefficient read from sensor */
    int32_t C10;      /**< 20bit pressure calibration coefficient read from sensor */
    int32_t C01;      /**< 20bit pressure calibration coefficient read from sensor */
    int32_t C11;      /**< 20bit pressure calibration coefficient read from sensor */
    int32_t C20;      /**< 20bit pressure calibration coefficient read from sensor */
    int32_t C21;      /**< 20bit pressure calibration coefficient read from sensor */
    int32_t C30;      /**< 20bit pressure calibration coefficient read from sensor */
} dps3xx_cal_coeff_regs_t;

typedef struct
{
	uint8_t interrupt_triggers;
	uint8_t fifo_enable;
	uint8_t temperature_shift;
	uint8_t pressure_shift;
} dps3xx_configuration_t;

typedef int8_t (*dps310_read_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef int8_t (*dps310_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

void dps310_init_i2c_interface(dps310_read_func_t read, dps310_write_func_t write);

int dps310_get_serial_id(uint8_t* revid, uint8_t* prodid);

int dps310_get_sensor_rdy(uint8_t* sensor_rdy, uint8_t* coef_rdy);

int dps310_read_calibration_regs(dps3xx_cal_coeff_regs_t* coeffs);

int dps310_read_configuration(dps3xx_configuration_t* configuration);

/**
 * @brief Read which temperature sensor has been used for the calibration coefficients (ASIC or MEMS)
 *
 * Use the same temperature sensor during the measurements!
 *
 * src = 0 ==> Internal temperature sensor (ASIC)
 * src = 1 ==> External temperature sensor (of pressure sensor MEMS element)
 */
int dps310_read_temperature_coefficient_src(uint8_t* src);

/**
 * @brief Manufacturer's suggested workaround to deal with early revisions of chip that don't indicate correct temperature.
 * Readings can be in the ~60C range when they should be in the ~20's.
 */
int dps310_correct_temperature();

/**
 * @brief Set the temperature configuration
 *
 * @param [in] tmp_ext Sensor used for calibration (read it using dps310_read_temperature_coefficient_src)
 * @param [in] oversampling Precision to be used x1: 0, x2: 1, x4: 2, x8: 3, x16: 4, x32: 5, x64: 6, x128: 7
 * @param [in] rate Measurement rate 0: 1/sec, 1: 2/sec, 2: 4/sec, ... 7: 128/sec
 */
int dps310_set_temperature_configuration(uint8_t tmp_ext, uint8_t oversampling, uint8_t rate);

/**
 * @brief Set the pressure configuration
 *
 * @param [in] oversampling Precision to be used x1: 0, x2: 1, x4: 2, x8: 3, x16: 4, x32: 5, x64: 6, x128: 7
 * @param [in] rate Measurement rate 0: 1/sec, 1: 2/sec, 2: 4/sec, ... 7: 128/sec
 */
int dps310_set_pressure_configuration(uint8_t oversampling, uint8_t rate);

/**
 * @brief Set the measurement mode
 *
 * @param [in] measurement_mode
 * 	0: Iddle
 * 	5: continuous pressure measurement
 * 	6: continuous temperature measurement
 * 	7: continuous pressure and temperature measurement
 */
int dps310_set_measurement_mode(uint8_t measurement_mode);

int dps310_is_measurement_ready(uint8_t* pressure_ready, uint8_t* temperature_ready);

/**
 * @brief Read the raw values
 *
 * @param [out] raw_values Pointer to an array of length 6
 */
int dps310_read_raw_values(uint8_t* raw_values);

/**
 * @brief Get the pressure value in Pascal
 */
float dps310_convert_pressure(uint8_t* raw_values, dps3xx_cal_coeff_regs_t* coeffs, int32_t scaling_coeff, float temp_scaled);

/**
 * @brief Convert the temperature value to °C
 */
float dps310_convert_temperature(uint8_t* raw_values, dps3xx_cal_coeff_regs_t* coeffs, int32_t scaling_coeff, float* temp_scaled);

int32_t dps310_get_scaling_coef(uint8_t oversampling);

#endif /* DPS310_DPS310_H_ */
