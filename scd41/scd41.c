/*
 * scd41.c
 *
 *  Created on: 22 Mar 2023
 *      Author: jorda
 */

#include "scd41.h"

#include <stddef.h> // Used for size_t

static const uint8_t SCD41_I2C_ADDR = 0x62;

static const uint16_t SCD41_CMD_GET_SERIAL_ID = 0x3682;
static const uint32_t SCD41_GET_SERIAL_ID_DURATION_MS = 1;

static const uint16_t SCD41_CMD_START_PERIODIC_MEASUREMENT = 0x21B1;
static const uint16_t SCD41_START_PERIODIC_MEASUREMENT_DURATION_MS = 1;

static const uint16_t SCD41_CMD_STOP_PERIODIC_MEASUREMENT = 0x3F86;
static const uint32_t SCD41_STOP_PERIOD_MEASUREMENT_DURATION_MS = 500;

static const uint16_t SCD41_CMD_REINIT = 0x3646;
static const uint32_t SCD41_REINIT_DURATION_MS = 20;

static const uint16_t SCD41_CMD_GET_DATA_READY_STATUS = 0xE4B8;
static const uint32_t SCD41_GET_DATA_READY_STATUS_DURATION_MS = 1;

static const uint16_t SCD41_CMD_READ_MEASUREMENT = 0xEC05;
static const uint32_t SCD41_READ_MEASUREMENT_DURATION_MS = 1;

static const uint16_t SCD41_CMD_PERFORM_SELF_TEST = 0x3639;
static const uint32_t SCD41_PERFORM_SELF_TEST_DURATION_MS = 10000;

static const uint16_t SCD41_CMD_WAKEUP = 0x36F6;
static const uint32_t SCD41_WAKEUP_DURATION_MS = 20;

static scd41_read_func_t i2c_read_bytes;
static scd41_write_func_t i2c_write_bytes;
static scd41_sleep_func_t sys_sleep;

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

static uint16_t bytes_to_uint16_t(uint8_t* bytes, int index) {
    return (uint16_t)bytes[index] << 8 | (uint16_t)bytes[index + 1];
}

void scd41_init(scd41_read_func_t read, scd41_write_func_t write, scd41_sleep_func_t sleep)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;
	sys_sleep = sleep;

	// Sensor needs 1sec to boot
	sys_sleep(1000);
}

int scd41_get_serial_number(scd41_serial_id_t* id)
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};
	uint8_t data[9] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SCD41_CMD_GET_SERIAL_ID & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SCD41_CMD_GET_SERIAL_ID & 0x00FF);

	// Send it
	result = i2c_write_bytes(SCD41_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SCD41_GET_SERIAL_ID_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SCD41_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	id->word0 = bytes_to_uint16_t(data, 0);
	id->word1 = bytes_to_uint16_t(data, 3);
	id->word2 = bytes_to_uint16_t(data, 6);

	// Check plausibility
	if (id->word0 == 0 && id->word1 == 0 && id->word2 == 0) return -3;

	return 0;
}

int scd41_start_periodic_measurement()
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SCD41_CMD_START_PERIODIC_MEASUREMENT & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SCD41_CMD_START_PERIODIC_MEASUREMENT & 0x00FF);

	// Send it
	result = i2c_write_bytes(SCD41_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SCD41_START_PERIODIC_MEASUREMENT_DURATION_MS);

	return 0;
}

int scd41_reinit()
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SCD41_CMD_REINIT & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SCD41_CMD_REINIT & 0x00FF);

	// Send it
	result = i2c_write_bytes(SCD41_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SCD41_REINIT_DURATION_MS);

	return 0;
}

int scd41_stop_periodic_measurement()
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SCD41_CMD_STOP_PERIODIC_MEASUREMENT & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SCD41_CMD_STOP_PERIODIC_MEASUREMENT & 0x00FF);

	// Send it
	result = i2c_write_bytes(SCD41_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SCD41_STOP_PERIOD_MEASUREMENT_DURATION_MS);

	return 0;
}

int scd41_get_data_ready_status(uint8_t* flag)
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};
	uint8_t data[3] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SCD41_CMD_GET_DATA_READY_STATUS & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SCD41_CMD_GET_DATA_READY_STATUS & 0x00FF);

	// Send it
	result = i2c_write_bytes(SCD41_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SCD41_GET_DATA_READY_STATUS_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SCD41_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	// Compute CRC and check
	uint8_t crc = crc8(data, 2);
	if (crc != data[2]) return -3;


	uint16_t dataready = bytes_to_uint16_t(data, 0);

	if ((dataready & 0x7FF) == 0)
		*flag = 0;
	else
		*flag = 1;

	return 0;
}

int scd41_read_measurement(uint16_t* co2_ppm, float* temperature, float* humidity)
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};
	uint8_t data[9] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SCD41_CMD_READ_MEASUREMENT & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SCD41_CMD_READ_MEASUREMENT & 0x00FF);

	// Send it
	result = i2c_write_bytes(SCD41_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SCD41_READ_MEASUREMENT_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SCD41_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	*co2_ppm = bytes_to_uint16_t(data, 0);
	uint8_t crc = crc8(data, 2);
	if (crc != data[2]) return -3;

	uint16_t digital_temperature = bytes_to_uint16_t(data, 3);
	*temperature = -45.f + (175.f * (float)digital_temperature) / 65535.f;
	crc = crc8(&data[3], 2);
	if (crc != data[5]) return -4;

	uint16_t digital_humidity = bytes_to_uint16_t(data, 6);
	*humidity = (100.f * (float)digital_humidity) / 65535.f;
	crc = crc8(&data[6], 2);
	if (crc != data[8]) return -5;

	return 0;
}

int scd41_perform_self_test()
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};
	uint8_t data[3] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SCD41_CMD_PERFORM_SELF_TEST & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SCD41_CMD_PERFORM_SELF_TEST & 0x00FF);

	// Send it
	result = i2c_write_bytes(SCD41_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SCD41_PERFORM_SELF_TEST_DURATION_MS);

	// Read answer
	result = i2c_read_bytes(SCD41_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	// Compute CRC and check
	uint8_t crc = crc8(data, 2);
	if (crc != data[2]) return -3;

	uint16_t status = *((uint16_t*) &data[0]);
	if (status != 0) return -4;

	return 0;
}

int scd41_wake_up()
{
	int8_t result = 0;
	uint8_t cmd[2] = {0};

	// Construct the command
	cmd[0] = (uint8_t)((SCD41_CMD_WAKEUP & 0xFF00) >> 8);
	cmd[1] = (uint8_t)(SCD41_CMD_WAKEUP & 0x00FF);

	// Send it
	result = i2c_write_bytes(SCD41_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	sys_sleep(SCD41_WAKEUP_DURATION_MS);

	return 0;
}
