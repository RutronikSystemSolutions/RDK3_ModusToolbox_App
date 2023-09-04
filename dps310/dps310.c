/*
 * dps310.c
 *
 *  Created on: 1 Aug 2023
 *      Author: jorda
 */

#include "dps310.h"

static dps310_read_func_t i2c_read_bytes;
static dps310_write_func_t i2c_write_bytes;

static const uint8_t DPS310_I2C_ADDR = 0x77;

static const uint8_t PSR_B2_REG_ADDR = 0x00;
static const uint8_t PRS_CFG_REG_ADDR = 0x06;
static const uint8_t TMP_CFG_REG_ADDR = 0x07;
static const uint8_t MEAS_CFG_REG_ADDR = 0x08;
static const uint8_t CFG_REG_ADDR = 0x09;
static const uint8_t PRODUCT_REG_ADDR = 0x0D;
static const uint8_t COEF_REG_ADDR = 0x10;
static const uint8_t COEF_SRCE_ADDR = 0x28;

#define DPS3XX_COEF_LEN	(18)

#define POW_2_11_MINUS_1                            (0x7FF)
#define POW_2_12                                    (0x1000)
#define POW_2_15_MINUS_1                            (0x7FFF)
#define POW_2_16                                    (0x10000)
#define POW_2_19_MINUS_1                            (0x7FFFF)
#define POW_2_20                                    (0x100000)
#define POW_2_23_MINUS_1                            (0x7FFFFF)
#define POW_2_24                                    (0x1000000)

static int read_register(uint8_t reg_address, uint8_t* data, uint16_t len)
{
	uint8_t cmd[1] = {reg_address};

	// Send register address
	int8_t result = i2c_write_bytes(DPS310_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(DPS310_I2C_ADDR, data, len);
	if (result != 0) return -2;

	return 0;
}

/**
 * @brief Write values to a register
 *
 * /!\ Needs to be done within one I2C transfer else does not work!
 *
 */
static int write_register(uint8_t reg_address, uint8_t* data, uint16_t len)
{
	uint8_t towrite[5] = {0};
	if (len > 4) return -1;

	towrite[0] = reg_address;
	for(int i = 0; i < len; ++i)
	{
		towrite[1 + i] = data[i];
	}

	int8_t result = i2c_write_bytes(DPS310_I2C_ADDR, towrite, 1 + len);
	if (result != 0) return -1;

	return 0;
}

void dps310_init_i2c_interface(dps310_read_func_t read, dps310_write_func_t write)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;
}

int dps310_get_serial_id(uint8_t* revid, uint8_t* prodid)
{
	uint8_t data[1] = {0};
	int retval = read_register(PRODUCT_REG_ADDR, data, sizeof(data));

	if (retval != 0) return -1;

	// Convert
	*revid = data[0] >> 4;
	*prodid = data[0] & 0xF;

	// Should be revid = 1 and prodid = 0 according to datasheet
	if ((*revid != 1) || (*prodid != 0)) return -2;

	return 0;
}

int dps310_get_sensor_rdy(uint8_t* sensor_rdy, uint8_t* coef_rdy)
{
	uint8_t data[1] = {0};
	int retval = read_register(MEAS_CFG_REG_ADDR, data, sizeof(data));

	if (retval != 0) return -1;

	// Convert
	*sensor_rdy = (data[0] >> 6) & 1;
	*coef_rdy = (data[0] >> 7) & 1;

	return 0;
}

int dps310_read_calibration_regs(dps3xx_cal_coeff_regs_t* coeffs)
{
    uint8_t data[DPS3XX_COEF_LEN] = {0};

    // read coefficients
    int retval = read_register(COEF_REG_ADDR, data, sizeof(data));
    if (retval != 0) return -1;

    coeffs->C0 = ((int32_t)data[0] << 4u) + (((int32_t)data[1] >> 4u) & 0x0F);
    if (coeffs->C0 > POW_2_11_MINUS_1)
    {
        coeffs->C0 = coeffs->C0 - POW_2_12;
    }

    coeffs->C1 = (data[2] + ((data[1] & 0x0F) << 8u));
    if (coeffs->C1 > POW_2_11_MINUS_1)
    {
        coeffs->C1 = coeffs->C1 - POW_2_12;
    }

    coeffs->C00 = (((int32_t)data[4] << 4u)
                             + ((int32_t)data[3] << 12u)) + (((int32_t)data[5] >> 4) & 0x0F);
    if (coeffs->C00 > POW_2_19_MINUS_1)
    {
        coeffs->C00 = coeffs->C00 - POW_2_20;
    }

    coeffs->C10 = (((int32_t)data[5] & (int32_t)0x0F) << 16u)
                            + ((int32_t)data[6] << 8u) + data[7];
    if (coeffs->C10 > POW_2_19_MINUS_1)
    {
        coeffs->C10 = coeffs->C10 - POW_2_20;
    }

    coeffs->C01 = (data[9] + ((int32_t)data[8] << 8u));
    if (coeffs->C01 > POW_2_15_MINUS_1)
    {
        coeffs->C01 = coeffs->C01 - POW_2_16;
    }

    coeffs->C11 = (data[11] + ((int32_t)data[10] << 8u));
    if (coeffs->C11 > POW_2_15_MINUS_1)
    {
        coeffs->C11 = coeffs->C11 - POW_2_16;
    }

    coeffs->C20 = (data[13] + ((int32_t)data[12] << 8u));
    if (coeffs->C20 > POW_2_15_MINUS_1)
    {
        coeffs->C20 = coeffs->C20 - POW_2_16;
    }

    coeffs->C21 = (data[15] + ((int32_t)data[14] << 8u));
    if (coeffs->C21 > POW_2_15_MINUS_1)
    {
        coeffs->C21 = coeffs->C21 - POW_2_16;
    }

    coeffs->C30 = (data[17] + ((int32_t)data[16] << 8u));
    if (coeffs->C30 > POW_2_15_MINUS_1)
    {
        coeffs->C30 = coeffs->C30 - POW_2_16;
    }

    return 0;
}

int dps310_read_configuration(dps3xx_configuration_t* configuration)
{
    uint8_t data[1] = {0};

    // read coefficients
    int retval = read_register(CFG_REG_ADDR, data, sizeof(data));
    if (retval != 0) return -1;

    configuration->fifo_enable = (data[0] >> 1) & 1;
    configuration->pressure_shift = (data[0] >> 2) & 1;
    configuration->temperature_shift = (data[0] >> 3) & 1;
    configuration->interrupt_triggers = (data[0] >> 4) & 0xF;

    return 0;
}

int dps310_read_temperature_coefficient_src(uint8_t* src)
{
    uint8_t data[1] = {0};

    // read coefficients
    int retval = read_register(COEF_SRCE_ADDR, data, sizeof(data));
    if (retval != 0) return -1;

    *src = (data[0] >> 7) & 1;

    return 0;
}

int dps310_correct_temperature()
{
	uint8_t data = 0xA5;
	int retval = write_register(0x0E, &data, sizeof(data));
	if (retval != 0) return -1;

	data = 0x96;
	retval = write_register(0x0F, &data, sizeof(data));
	if (retval != 0) return -1;

	data = 0x02;
	retval = write_register(0x62, &data, sizeof(data));
	if (retval != 0) return -1;

	data = 0x00;
	retval = write_register(0x0E, &data, sizeof(data));
	if (retval != 0) return -1;

	data = 0x00;
	retval = write_register(0x0F, &data, sizeof(data));
	if (retval != 0) return -1;

	return 0;
}

int dps310_set_temperature_configuration(uint8_t tmp_ext, uint8_t oversampling, uint8_t rate)
{
	const uint8_t oversampling_8x = 3;
	uint8_t data = (tmp_ext << 7) | (rate << 4) | oversampling;
	int retval = write_register(TMP_CFG_REG_ADDR, &data, sizeof(data));
	if (retval != 0) return -1;

	// in case oversampling is greater than 8x, set the T_SHIFT bit
	if (oversampling > oversampling_8x)
	{
		retval = read_register(CFG_REG_ADDR, &data, sizeof(data));
		if (retval != 0) return -2;

		data |= (1 << 3);
		retval = write_register(CFG_REG_ADDR, &data, sizeof(data));
		if (retval != 0) return -3;
	}

	return 0;
}

int dps310_set_pressure_configuration(uint8_t oversampling, uint8_t rate)
{
	const uint8_t oversampling_8x = 3;
	uint8_t data = (rate << 4) | oversampling;
	int retval = write_register(PRS_CFG_REG_ADDR, &data, sizeof(data));
	if (retval != 0) return -1;

	// in case oversampling is greater than 8x, set the P_SHIFT bit
	if (oversampling > oversampling_8x)
	{
		retval = read_register(CFG_REG_ADDR, &data, sizeof(data));
		if (retval != 0) return -2;

		data |= (1 << 2);
		retval = write_register(CFG_REG_ADDR, &data, sizeof(data));
		if (retval != 0) return -3;
	}

	return 0;
}

int dps310_set_measurement_mode(uint8_t measurement_mode)
{
	int retval = write_register(MEAS_CFG_REG_ADDR, &measurement_mode, sizeof(measurement_mode));
	if (retval != 0) return -1;
	return 0;
}

int dps310_is_measurement_ready(uint8_t* pressure_ready, uint8_t* temperature_ready)
{
	uint8_t data = 0;
	int retval = read_register(MEAS_CFG_REG_ADDR, &data, sizeof(data));
	if (retval != 0) return -1;

	*pressure_ready = (data >> 4) & 1;
	*temperature_ready = (data >> 5) & 1;

	return 0;
}

int dps310_read_raw_values(uint8_t* raw_values)
{
	int retval = read_register(PSR_B2_REG_ADDR, raw_values, DPS310_RAW_VALUES_LENGTH);
	if (retval != 0) return -1;

	return 0;
}

static float calc_pressure(dps3xx_cal_coeff_regs_t* coeffs, int32_t scaling_coeff, int32_t press_raw, float temp_scaled)
{
    if (press_raw > POW_2_23_MINUS_1)
    {
        press_raw -= POW_2_24;
    }

    float press_scaled = (float)press_raw / (float)scaling_coeff;
    int64_t c00 = coeffs->C00;
    int64_t c10 = coeffs->C10;
    int64_t c20 = coeffs->C20;
    int64_t c30 = coeffs->C30;
    int64_t c01 = coeffs->C01;
    int64_t c11 = coeffs->C11;
    int64_t c21 = coeffs->C21;

    float Pcal = c00
                 + press_scaled * (c10 + press_scaled * (c20 + press_scaled * c30))
                 + (temp_scaled * c01)
                 + (temp_scaled * press_scaled * (c11 + press_scaled * c21));
    return Pcal * 0.01f;
}

float dps310_convert_pressure(uint8_t* raw_values, dps3xx_cal_coeff_regs_t* coeffs, int32_t scaling_coeff, float temp_scaled)
{
	int32_t press_raw = (int32_t)(raw_values[2]) + (raw_values[1] << 8) + (raw_values[0] << 16);
	return calc_pressure(coeffs, scaling_coeff, press_raw, temp_scaled);
}

static float calc_temperature(dps3xx_cal_coeff_regs_t* coeffs, int32_t temp_raw, int32_t scaling_coeff, float* temp_scaled)
{
    if (temp_raw > POW_2_23_MINUS_1)
    {
        temp_raw -= POW_2_24;
    }
    *temp_scaled = (float)temp_raw / (float)scaling_coeff;

    int64_t c0 = coeffs->C0;
    int64_t c1 = coeffs->C1;
    return (c0 / 2.0f) + (c1 * *temp_scaled);
}

float dps310_convert_temperature(uint8_t* raw_values, dps3xx_cal_coeff_regs_t* coeffs, int32_t scaling_coeff, float* temp_scaled)
{
	int32_t temp_raw = (int32_t)(raw_values[5]) + (raw_values[4] << 8) + (raw_values[3] << 16);
	return calc_temperature(coeffs, temp_raw, scaling_coeff, temp_scaled);
}

int32_t dps310_get_scaling_coef(uint8_t oversampling)
{
	switch(oversampling)
	{
		case 0:
			return 524288;

		case 1:
			return 1572864;

		case 2:
			return 3670016;

		case 3:
			return 7864320;

		case 4:
			return 253952;

		case 5:
			return 516096;

		case 6:
			return 1040384;

		case 7:
			return 2088960;
	}

	// Should never happen
	return 524288;
}




