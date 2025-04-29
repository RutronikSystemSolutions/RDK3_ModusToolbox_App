/*
 * notification_fabric.h
 *
 *  Created on: 28 Mar 2023
 *      Author: jorda
 */

#ifndef NOTIFICATION_FABRIC_H_
#define NOTIFICATION_FABRIC_H_

#include <stdint.h>

#include "ams_tmf8828/tmf8828_app.h"
#include "bme688/bme688_app.h"

#ifdef UM980_SUPPORT
#include "um980/gga_packet.h"
#endif

#include "bme690/bme690_app.h"

typedef struct
{
	uint8_t length;
	uint8_t* data;
} notification_t;

void notification_fabric_free_notification(notification_t* notification);

notification_t* notification_fabric_create_for_sht4x(float temperature, float humidity);

notification_t* notification_fabric_create_for_bmp581(float pressure, float temperature);

notification_t* notification_fabric_create_for_sgp40(uint16_t value_raw, uint16_t value_compensated, uint16_t gas_index);

notification_t* notification_fabric_create_for_scd41(uint16_t co2_ppm, float temperature, float humidity);

notification_t* notification_fabric_create_for_tmf8828(tmf8828_results_t* results);

notification_t* notification_fabric_create_for_battery_monitor(uint16_t voltage, uint8_t charge_status, uint8_t charge_fault, uint8_t dio_status);

notification_t* notification_fabric_create_for_tmf8828_8x8_mode(uint16_t* distances);

notification_t* notification_fabric_create_for_pasco2(uint16_t co2_ppm);

notification_t* notification_fabric_create_for_dps310(float pressure, float temperature);

notification_t* notification_fabric_create_for_bmi270(int16_t accx, int16_t accy, int16_t accz, int16_t girx, int16_t giry, int16_t girz);

notification_t* notification_fabric_create_for_bme688(bme688_scan_data_t * values);

#ifdef UM980_SUPPORT
notification_t* notification_fabric_create_for_um980(um980_gga_packet_t* packet);
#endif

notification_t* notification_fabric_create_for_vcnl4030x01(uint16_t proximity_value, uint16_t als_value, uint16_t white_value);

notification_t* notification_fabric_create_for_bmm350(float temp, float mag_x, float mag_y, float mag_z);

notification_t* notification_fabric_create_for_sgp41(uint16_t voc_raw, uint16_t nox_raw, int32_t voc_index, int32_t nox_index);

notification_t* notification_fabric_create_for_bme690(bme69x_data_t* bme_data);

notification_t* notification_fabric_create_for_bmp585(float press, float temp);

notification_t* notification_fabric_create_for_dps368(float press, float temp);

notification_t* notification_fabric_create_for_bmi323(int16_t acc_x, int16_t acc_y, int16_t acc_z, int16_t gyr_x, int16_t gyr_y, int16_t gyr_z);

#endif /* NOTIFICATION_FABRIC_H_ */
