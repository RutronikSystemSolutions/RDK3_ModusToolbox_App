/*
 * bme688.h
 *
 *  Created on: 13 Mar 2023
 *      Author: jorda
 */

#ifndef BME688_BME688_H_
#define BME688_BME688_H_


#include <stdint.h>


typedef int8_t (*bme68x_read_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef int8_t (*bme68x_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

#define BME688_MAX_STEPS_NB	10

#define BME688_OVERSAMPLING_X1		1
#define BME688_OVERSAMPLING_X2		2
#define BME688_OVERSAMPLING_X4		3
#define BME688_OVERSAMPLING_X8		4
#define BME688_OVERSAMPLING_X16		5

#define BME688_MODE_SLEEP			0
#define BME688_MODE_FORCED			1
#define BME688_MODE_PARALLEL		2

typedef struct
{
	uint8_t heater_step_nb;	/**< Store the number of heating steps used for one scan */
	uint16_t temperatures[BME688_MAX_STEPS_NB]; /**< The temperatures to be used in °C */
	uint16_t steps_duration[BME688_MAX_STEPS_NB]; /**< The duration of each step (unit is step_ms) */
	uint16_t step_ms;	/**< The unit step (used for steps_duration) in ms. Minimum value is 100! */
	uint8_t pressure_os; /**< The oversampling to be used. From BME688_OVERSAMPLING_X1 to BME688_OVERSAMPLING_X16 */
	uint8_t temperature_os; /**< The oversampling to be used. From BME688_OVERSAMPLING_X1 to BME688_OVERSAMPLING_X16 */
	uint8_t humidity_os; /**< The oversampling to be used. From BME688_OVERSAMPLING_X1 to BME688_OVERSAMPLING_X16 */
} bme688_measurement_configuration_t;

typedef struct
{
	int8_t par_g1;
	int16_t par_g2;
	int8_t par_g3;

	uint8_t res_heat_range;
	int8_t res_heat_val;

	uint16_t par_t1;
	int16_t par_t2;
	int8_t par_t3;

	uint16_t par_p1;
	int16_t par_p2;
	int8_t par_p3;
	int16_t par_p4;
	int16_t par_p5;
	int8_t par_p6;
	int8_t par_p7;
	int16_t par_p8;
	int16_t par_p9;
	uint8_t par_p10;

	uint16_t par_h1;
	uint16_t par_h2;
	int8_t par_h3;
	int8_t par_h4;
	int8_t par_h5;
	uint8_t par_h6;
	int8_t par_h7;

	float t_fine;

} bme688_calibration_parameters_t;

typedef struct
{
	uint8_t new_data;
	uint8_t gas_measuring;
	uint8_t measuring;
	uint8_t gas_meas_index;
	uint8_t sub_meas_index;

	uint32_t pressure;
	uint32_t temperature;
	uint16_t humidity;
	uint16_t gas_resistance;

	uint8_t gas_valid_r;
	uint8_t heat_stab_r;
	uint8_t gas_range_r;

} bme688_data_field_t;

void bme688_init_i2c_interface(bme68x_read_func_t read, bme68x_write_func_t write);

int bme688_get_chip_id(uint8_t* chip_id);

int bme688_get_variant_id(uint8_t* variant_id);

int bme688_get_calibration_parameters(bme688_calibration_parameters_t* parameters);

int bme688_set_oversampling(uint8_t temperature, uint8_t pressure, uint8_t humidity);

int bme688_set_measurement_mode(uint8_t mode);

int bme688_enable_disable_gas_conversion(uint8_t flag);

int bme688_set_heater_profile_simple(uint16_t* temperatures, uint16_t ambient_temperature, uint8_t length, bme688_calibration_parameters_t* calib);

int bme688_set_heater_profile(bme688_measurement_configuration_t* configuration, uint16_t ambient_temperature, bme688_calibration_parameters_t* calib);

int bme688_read_data_field(uint8_t index, bme688_data_field_t* data);

float bme688_get_temperature(bme688_data_field_t* data, bme688_calibration_parameters_t* calib);

float bme688_get_pressure(bme688_data_field_t* data, bme688_calibration_parameters_t* calib);

float bme688_get_humidity(bme688_data_field_t* data, bme688_calibration_parameters_t* calib);

float bme688_get_gas_resistance(bme688_data_field_t* data, bme688_calibration_parameters_t* calib);

#endif /* BME688_BME688_H_ */
