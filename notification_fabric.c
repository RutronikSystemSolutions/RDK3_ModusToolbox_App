/*
 * notification_fabric.c
 *
 *  Created on: 28 Mar 2023
 *      Author: jorda
 */

#include "notification_fabric.h"
#include "notification_defs.h"

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
	const uint8_t data_size = SHT4X_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = SHT4X_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = temperature;
	*((float*) &data[7]) = humidity;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bmp581(float pressure, float temperature)
{
	const uint8_t data_size = BMP581_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = BMP581_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = pressure;
	*((float*) &data[7]) = temperature;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_sgp40(uint16_t value_raw, uint16_t value_compensated, uint16_t gas_index)
{
	const uint8_t data_size = SGP40_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = SGP40_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = value_raw;
	*((uint16_t*) &data[5]) = value_compensated;
	*((uint16_t*) &data[7]) = gas_index;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_scd41(uint16_t co2_ppm, float temperature, float humidity)
{
	const uint8_t data_size = SCD41_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = SCD41_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = co2_ppm;

	*((float*) &data[5]) = temperature;
	*((float*) &data[9]) = humidity;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_tmf8828(tmf8828_results_t* results)
{
	const uint8_t data_size = TMF8828_DATA_SIZE; // 9*uint32_t (distance) - uint32_t (ambient light)
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = TMF8828_NOTIFICATION_ID;
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

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_battery_monitor(uint16_t voltage, uint8_t charge_status, uint8_t charge_fault, uint8_t dio_status)
{
	const uint8_t data_size = BATT_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = BATT_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = voltage;

	data[5] = charge_status;
	data[6] = charge_fault;
	data[7] = dio_status;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_tmf8828_8x8_mode(uint16_t* distances)
{
	const uint8_t data_size = TMF8828_8X8_DATA_SIZE; // 64*uint16_t (distance)
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = TMF8828_8X8_NOTIFICATION_ID;
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

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_pasco2(uint16_t co2_ppm)
{
	const uint8_t data_size = PASCO2_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = PASCO2_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = co2_ppm;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_dps310(float pressure, float temperature)
{
	const uint8_t data_size = DPS310_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = DPS310_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = pressure;
	*((float*) &data[7]) = temperature;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bmi270(int16_t accx, int16_t accy, int16_t accz, int16_t girx, int16_t giry, int16_t girz)
{
	const uint8_t data_size = BMI270_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = BMI270_NOTIFICATION_ID;

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
	const uint8_t data_size = BME688_DATA_SIZE; // 10 steps. Per step: temperature, pressure, humidity, gas resistance (all floats) => 4 * 4 * 10 => 160 bytes
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = BME688_NOTIFICATION_ID;

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
	const uint8_t data_size =  UM980_DATA_SIZE; // 8*uint8_t + 1*uint16_t + 7*double
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = UM980_NOTIFICATION_ID;

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
	*((uint16*) &data[index]) = packet->correction_age;
	index += sizeof(uint16_t);

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}
#endif

notification_t* notification_fabric_create_for_vcnl4030x01(uint16_t proximity_value, uint16_t als_value, uint16_t white_value)
{
	const uint8_t data_size = VCNL4030X01_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = VCNL4030X01_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = proximity_value;
	*((uint16_t*) &data[5]) = als_value;
	*((uint16_t*) &data[7]) = white_value;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bmm350(float temp, float mag_x, float mag_y, float mag_z)
{
	const uint8_t data_size = BMM350_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = BMM350_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = temp;
	*((float*) &data[7]) = mag_x;
	*((float*) &data[11]) = mag_y;
	*((float*) &data[15]) = mag_z;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_sgp41(uint16_t voc_raw, uint16_t nox_raw, int32_t voc_index, int32_t nox_index)
{
	const uint8_t data_size = SGP41_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = SGP41_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((uint16_t*) &data[3]) = voc_raw;
	*((uint16_t*) &data[5]) = nox_raw;
	*((int32_t*) &data[7]) = voc_index;
	*((int32_t*) &data[11]) = nox_index;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bme690(bme69x_data_t* bme_data)
{
	const uint8_t data_size = BME690_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = BME690_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	data[3] = bme_data->status;
	data[4] = bme_data->gas_index;
	data[5] = bme_data->meas_index;
	data[6] = bme_data->res_heat;
	data[7] = bme_data->idac;
	data[8] = bme_data->gas_wait;

	*((float*) &data[9]) = bme_data->temperature;
	*((float*) &data[13]) = bme_data->pressure;
	*((float*) &data[17]) = bme_data->humidity;
	*((float*) &data[21]) = bme_data->gas_resistance;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bmp585(float press, float temp)
{
	const uint8_t data_size = BMP585_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = BMP585_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = press;
	*((float*) &data[7]) = temp;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_dps368(float press, float temp)
{
	const uint8_t data_size = DPS368_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = DPS368_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((float*) &data[3]) = press;
	*((float*) &data[7]) = temp;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}

notification_t* notification_fabric_create_for_bmi323(int16_t acc_x, int16_t acc_y, int16_t acc_z, int16_t gyr_x, int16_t gyr_y, int16_t gyr_z)
{
	const uint8_t data_size = BMI323_DATA_SIZE;
	const uint8_t notification_size = data_size + notification_overhead;
	const uint16_t sensor_id = BMI323_NOTIFICATION_ID;

	uint8_t* data = (uint8_t*) malloc(notification_size);

	data[0] = (uint8_t) (sensor_id & 0xFF);
	data[1] = (uint8_t) (sensor_id >> 8);

	data[2] = data_size;

	*((int16_t*) &data[3]) = acc_x;
	*((int16_t*) &data[5]) = acc_y;
	*((int16_t*) &data[7]) = acc_z;
	*((int16_t*) &data[9]) = gyr_x;
	*((int16_t*) &data[11]) = gyr_y;
	*((int16_t*) &data[13]) = gyr_z;

	data[notification_size - 1] = compute_crc(data, notification_size - 1);

	notification_t* retval = (notification_t*) malloc(sizeof(notification_t));
	retval->length = notification_size;
	retval->data = data;

	return retval;
}
