/*
 * sht4x.c
 *
 *  Created on: 10 Mar 2023
 *      Author: jorda
 */


#include "sht4x.h"

static const uint8_t SHT4X_I2C_ADDR = 0x44;
static const uint8_t SHT4X_CMD_READ_SERIAL = 0x89;
static const uint32_t SHT4X_GET_SERIAL_CMD_DURATION_MS = 1;
static const uint8_t SHT4X_CMD_READ_HIGH_PRECISION = 0xFD;
static const uint32_t SHT4X_READ_MEASUREMENT_CMD_DURATION_MS = 9;

static sht4x_read_func_t i2c_read_bytes;
static sht4x_write_func_t i2c_write_bytes;
static sht4x_sleep_func_t sys_sleep;

void sht4x_init(sht4x_read_func_t read, sht4x_write_func_t write, sht4x_sleep_func_t sleep)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;
	sys_sleep = sleep;
}

int sht4x_get_serial_id(uint32_t * id)
{
	int8_t result = 0;
	uint8_t cmd = SHT4X_CMD_READ_SERIAL;
	uint8_t data[6] = {0};

	// Send it
	result = i2c_write_bytes(SHT4X_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SHT4X_GET_SERIAL_CMD_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SHT4X_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	// Format of data is
	// | 0 | 1 | 2   | 3 | 4 | 5   |
	// | Data  | CRC | Data  | CRC |

	// TODO: compute CRC

	uint16_t msb = ((uint16_t*)data)[0];
	uint16_t lsb = *((uint16_t*) &data[3]);

	*id = ((uint32_t)msb) << 16 | (uint32_t) lsb;

	return 0;
}

int sht4x_get_raw_measurement(uint16_t* temperature, uint16_t* humidity)
{
	int8_t result = 0;
	uint8_t cmd = SHT4X_CMD_READ_HIGH_PRECISION;
	uint8_t data[6] = {0};

	// Send it
	result = i2c_write_bytes(SHT4X_I2C_ADDR, &cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SHT4X_READ_MEASUREMENT_CMD_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SHT4X_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	// Format of data is
	// | 0 | 1 | 2   | 3 | 4 | 5   |
	// | Temp  | CRC | Hum.  | CRC |

	// TODO: compute CRC

	// valRHT = (data[0] << 8) + data[1];
	*temperature = ((uint16_t)data[0] << 8) | data[1];
	*humidity = ((uint16_t)data[3] << 8) | data[4];

	return 0;
}

int sht4x_get_temperature_and_humidity(float* temperature, float* humidity)
{
	uint16_t raw_temperature = 0;
	uint16_t raw_humidity = 0;
	if (sht4x_get_raw_measurement(&raw_temperature, &raw_humidity) != 0) return -1;

	// Convert
	*temperature = -45.f + 175.f * ((float)raw_temperature / 65535.0f);
	*humidity = -6.f + 125.f * ((float)raw_humidity / 65535.0f);

	return 0;
}
