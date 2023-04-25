/*
 * sgp40.c
 *
 *  Created on: 9 Mar 2023
 *      Author: jorda
 */

#include "sgp40.h"

#include <stddef.h>

static const uint8_t SGP40_I2C_ADDR = 0x59;
static const uint32_t SGP40_GET_RAW_MEASUREMENT_DURATION_MS = 30;
static const uint16_t SGP40_CMD_GET_SERIAL_ID = 0x3682;
static const uint32_t SGP40_GET_SERIAL_ID_DURATION_MS = 1;

/**
 * Compute CRC value out of the given buffer
 *
 * @param [in] data Pointer to an array containing the data to be computed
 * @param [in] length Size of the buffer (in bytes)
 *
 * @retval Computed CRC value
 */
static uint8_t crc8(uint8_t* data, size_t length)
{
	uint8_t crc = 0xFF;
	for(int i=0; i<length; i++)
	{
		crc^=data[i];
		for(uint8_t bit=8; bit>0; --bit)
		{
			if(crc & 0x80) {crc = (crc << 1) ^ 0x31;}
			else {crc = (crc << 1);}
		}
	}

	return crc;
}

static sgp40_read_func_t i2c_read_bytes;
static sgp40_write_func_t i2c_write_bytes;
static sgp40_sleep_func_t sys_sleep;

void sgp40_init(sgp40_read_func_t read, sgp40_write_func_t write, sgp40_sleep_func_t sleep)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;
	sys_sleep = sleep;
}

int sgp40_get_serial_id(sgp40_serial_id_t * id)
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};
	uint8_t data[8] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SGP40_CMD_GET_SERIAL_ID & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SGP40_CMD_GET_SERIAL_ID & 0x00FF);

	// Send it
	result = i2c_write_bytes(SGP40_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SGP40_GET_SERIAL_ID_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SGP40_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	id->msb = *((uint32_t*) data);
	id->lsb = *((uint32_t*)&data[4]);

	// Check plausibility
	if (id->msb == 0 && id->lsb == 0) return -3;

	return 0;
}

int sgp40_measure_raw_signal_without_compensation(uint16_t * voc_value)
{
	int8_t result = 0;
	uint8_t cmd[8] = {0x26, 0x0F, 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93};
	uint8_t data[3] = {0};

	// Send the command
	result = i2c_write_bytes(SGP40_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SGP40_GET_RAW_MEASUREMENT_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SGP40_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Verify CRC
	uint8_t crc = crc8(data, sizeof(data) - 1);
	if (crc != data[2]) return -3;

	// Get value
	*voc_value = ((uint16_t)data[0] << 8 | data[1]);

	return 0;
}

/**
 * Convert the temperature and the humidity to format supported by the SGP40
 * The values are needed for internal value compensation
 */
static void convert_to_sensor_format(float temperature, float humidity,
		uint16_t* temperature_sensor_format, uint16_t* humidity_sensor_format)
{
	// Supported temperature range: -45 -> 130
	if (temperature < -45) temperature = -45;
	if (temperature > 130) temperature = 130;

	// Supported humidity range: 0 -> 100
	if (humidity < 0) humidity = 0;
	if (humidity > 100) humidity = 100;

	// Convert
	float tmp = (temperature + 45.f) * 65535.f / 175.f;
	*temperature_sensor_format = (uint16_t) tmp;

	tmp = humidity * 65535.f / 100.f;
	*humidity_sensor_format = (uint16_t) tmp;
}

int sgp40_measure_with_compensation(float temperature, float humidity, uint16_t* voc_value)
{
	int8_t result;
	uint8_t cmd[8] = {0x26, 0x0F, 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93};
	uint8_t data[3] = {0};
	uint16_t temperature_sensor_format = 0;
	uint16_t humidity_sensor_format = 0;

	// Convert humidity and temperature into sensor compatible values
	convert_to_sensor_format(temperature, humidity, &temperature_sensor_format, &humidity_sensor_format);

	cmd[2] = (uint8_t)((humidity_sensor_format & 0xFF00) >> 8);
	cmd[3] = (uint8_t)(humidity_sensor_format & 0x00FF);
	cmd[4] = crc8(&cmd[2], 2);

	cmd[5] = (uint8_t)((temperature_sensor_format & 0xFF00) >> 8);
	cmd[6] = (uint8_t)(temperature_sensor_format & 0x00FF);
	cmd[7] = crc8(&cmd[5], 2);

	// Send the command
	result = i2c_write_bytes(SGP40_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SGP40_GET_RAW_MEASUREMENT_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SGP40_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Verify CRC
	uint8_t crc = crc8(data, sizeof(data) - 1);
	if (crc != data[2]) return -3;

	// Get value
	*voc_value = ((uint16_t)data[0] << 8 | data[1]);

	return 0;
}
