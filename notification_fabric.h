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

#endif /* NOTIFICATION_FABRIC_H_ */
