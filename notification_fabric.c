/*
 * notification_fabric.c
 *
 *  Created on: 28 Mar 2023
 *      Author: jorda
 */

#include "notification_fabric.h"

#include <stdlib.h>

static const uint8_t notification_overhead = 4;

static uint8_t compute_crc(uint8_t* data, uint8_t length)
{
	// TODO
	return 0x3;
}

void notification_fabric_free_notification(notification_t* notification)
{
	free(notification->data);
	free(notification);
}

notification_t* notification_fabric_create_for_sht4x(float temperature, float humidity)
{
	const uint8_t data_size = 8;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0x1;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = temperature;
	*((float*) &data[7]) = humidity;

	data[11] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bmp581(float pressure, float temperature)
{
	const uint8_t data_size = 8;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0x2;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = pressure;
	*((float*) &data[7]) = temperature;

	data[11] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_sgp40(uint16_t value_raw, uint16_t value_compensated, uint16_t gas_index)
{
	const uint8_t data_size = 6;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0x3;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = value_raw;
	*((uint16_t*) &data[5]) = value_compensated;
	*((uint16_t*) &data[7]) = gas_index;

	data[9] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_scd41(uint16_t co2_ppm, float temperature, float humidity)
{
	const uint8_t data_size = 10;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0x4;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = co2_ppm;

	*((float*) &data[5]) = temperature;
	*((float*) &data[9]) = humidity;

	data[13] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_tmf8828(tmf8828_results_t* results)
{
	const uint8_t data_size = 40; // 9*uint32_t (distance) - uint32_t (ambient light)
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0x5;
	const uint16_t values_count = 9;
	uint16_t index = 0;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	index = 3;
	*((uint32_t*) &data[index]) = results->ambient_light;
	index += sizeof(uint32_t);

	for(uint16_t i = 0; i < values_count; ++i)
	{
		*((uint32_t*) &data[index]) = results->results[i].distance_mm;
		index += sizeof(uint32_t);
	}

	data[index] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_battery_monitor(uint16_t voltage, uint8_t charge_status, uint8_t charge_fault, uint8_t dio_status)
{
	const uint8_t data_size = 5;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0x6;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = voltage;

	data[5] = charge_status;
	data[6] = charge_fault;
	data[7] = dio_status;

	data[8] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_tmf8828_8x8_mode(uint16_t* distances)
{
	const uint8_t data_size = 128; // 64*uint16_t (distance)
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0x7;
	const uint16_t values_count = 64;
	uint16_t index = 0;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	index = 3;

	for(uint16_t i = 0; i < values_count; ++i)
	{
		*((uint16_t*) &data[index]) = distances[i];
		index += sizeof(uint16_t);
	}

	data[index] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_pasco2(uint16_t co2_ppm)
{
	const uint8_t data_size = 2;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0x8;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = co2_ppm;

	data[5] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_dps310(float pressure, float temperature)
{
	const uint8_t data_size = 8;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0xA;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = pressure;
	*((float*) &data[7]) = temperature;

	data[11] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bmi270(int16_t accx, int16_t accy, int16_t accz, int16_t girx, int16_t giry, int16_t girz)
{
	const uint8_t data_size = 12;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0xB;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((int16_t*) &data[3]) = accx;
	*((int16_t*) &data[5]) = accy;
	*((int16_t*) &data[7]) = accz;
	*((int16_t*) &data[9]) = girx;
	*((int16_t*) &data[11]) = giry;
	*((int16_t*) &data[13]) = girz;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bme688(bme688_scan_data_t * values)
{
	const uint8_t data_size = sizeof(float) * 4 * 10; // 10 steps. Per step: temperature, pressure, humidity, gas resistance (all floats) => 4 * 4 * 10 => 160 bytes
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0xC;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	uint8_t index = 3;
	for (uint8_t i = 0; i < BME688_MAX_STEPS_NB; ++i)
	{
		*((float*) &data[index]) = values->steps[i].temperature;
		index += sizeof(float);
		*((float*) &data[index]) = values->steps[i].pressure;
		index += sizeof(float);
		*((float*) &data[index]) = values->steps[i].humidity;
		index += sizeof(float);
		*((float*) &data[index]) = values->steps[i].gas_resistance;
		index += sizeof(float);
	}

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

#ifdef UM980_SUPPORT
notification_t* notification_fabric_create_for_um980(um980_gga_packet_t* packet)
{
	const uint8_t data_size =  8 + 8 * 7; // 8*uint8_t + 7*double
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = 0xD;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	uint8_t index = 3;
	data[index] = packet->hours;
	index++;
	data[index] = packet->minutes;
	index++;
	data[index] = packet->seconds;
	index++;
	data[index] = packet->sub_seconds;
	index++;
	*((double*) &data[index]) = packet->lat_degree;
	index += sizeof(double);
	*((double*) &data[index]) = packet->lat_seconds;
	index += sizeof(double);
	data[index] = packet->lat_dir;
	index++;
	*((double*) &data[index]) = packet->lon_degree;
	index += sizeof(double);
	*((double*) &data[index]) = packet->lon_seconds;
	index += sizeof(double);
	data[index] = packet->lon_dir;
	index++;
	data[index] = packet->quality;
	index++;
	data[index] = packet->satellites_in_use;
	index++;
	*((double*) &data[index]) = packet->hdop;
	index += sizeof(double);
	*((double*) &data[index]) = packet->alt;
	index += sizeof(double);
	*((double*) &data[index]) = packet->undulation;
	index += sizeof(double);

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}
#endif
