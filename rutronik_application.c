/*
 * rutronik_application.c
 *
 *  Created on: 29 Mar 2023
 *      Author: jorda
 */

#include "rutronik_application.h"

#include "hal_i2c.h"
#include "hal_sleep.h"

#include "sht4x/sht4x.h"
#include "bmp581/bmp581.h"
#include "sgp40/sgp40.h"
#include "ams_tmf8828/tmf8828_app.h"
#include "battery_monitor/battery_monitor.h"
#include "dio59020/dio59020.h"

#include "host_main.h"


static void init_sensors_hal(rutronik_application_t* app)
{
	sht4x_init(hal_i2c_read, hal_i2c_write, hal_sleep);
	bmp581_init_i2c_interface(hal_i2c_read, hal_i2c_write);
	sgp40_init(hal_i2c_read, hal_i2c_write, hal_sleep);
	scd41_app_init(&(app->scd41_app), hal_i2c_read, hal_i2c_write, hal_sleep);
	tmf8828_app_init(hal_i2c_read, hal_i2c_write);
	dio59020_init(hal_i2c_read_register, hal_i2c_write_register);
	pasco2_app_init(hal_i2c_read, hal_i2c_write, hal_sleep);
}

static int is_sensor_fusion_board_available()
{
	uint32_t id = 0;
	int result = sht4x_get_serial_id(&id);
	if (result != 0)
	{
		// Not available
		return 0;
	}
	// Available
	return 1;
}

static int is_co2_board_available()
{
	// Only reading the serial number is not enough (needs to wake up, etc...)
	// That is done during the init phase
	// is something goes wrong during the init phase, then set the Co2 board as not present
	return 1; // By default always here
}

static int init_bmp581()
{
	/*Initialize the BMP581 Sensor*/
	uint8_t id = 0;
	int res = bmp581_get_chip_id(&id);
	if (res != 0) return -1;

	res = bmp581_set_oversampling_mode(7, 3, 1);
	if (res != 0) return -2;

	res = bmp581_set_power_mode(BMP51_NON_STOP_MODE, 0x1B);
	if (res != 0) return -3;

	return 0;
}

static int init_sgp40(rutronik_application_t* app)
{
	// Initialize algorithm for VOC type
	GasIndexAlgorithm_init(&app->gas_index_params, GasIndexAlgorithm_ALGORITHM_TYPE_VOC);

	return 0;
}

static void init_sensor_fusion(rutronik_application_t* app)
{
	if (init_bmp581() != 0)
	{
		app->sensor_fusion_available = 0;
		return;
	}

	if (init_sgp40(app) != 0)
	{
		app->sensor_fusion_available = 0;
		return;
	}
}

static void init_co2_board(rutronik_application_t* app)
{
	int retval = scd41_app_initialise_and_start_measurement(&app->scd41_app);
	if (retval != 0)
	{
		app->co2_available = 0;
		return;
	}

	retval = pasco2_app_start_measurement(&app->pasco2_app);
	if (retval != 0)
	{
		app->co2_available = 0;
		return;
	}
}

static void init_ams_osram_board(rutronik_application_t* app)
{
	if(tmf8828_app_init_measurement() != 0)
	{
		app->ams_tof_available = 0;
		return;
	}
}


void rutronik_application_init(rutronik_application_t* app)
{
	app->sensor_fusion_available = 0;
	app->co2_available = 0;
	app->ams_tof_available = 0;

	app->prescaler = 0;

	lowpassfilter_init(&app->filtered_voltage, 0.01);

	init_sensors_hal(app);

	battery_monitor_init();

	if (is_sensor_fusion_board_available() != 0)
	{
		app->sensor_fusion_available = 1;
		init_sensor_fusion(app);
	}

	if (is_co2_board_available() != 0)
	{
		app->co2_available = 1;
		init_co2_board(app);
	}

	if (tmf8828_app_is_board_available() != 0)
	{
		app->ams_tof_available = 1;
		init_ams_osram_board(app);
	}
}

uint32_t rutronik_application_get_available_sensors_mask(rutronik_application_t* app)
{
	return
			(uint32_t) app->sensor_fusion_available
			| ((uint32_t) app->co2_available) << 1
			| ((uint32_t) app->ams_tof_available) << 2;
}

void rutronik_application_set_tmf8828_mode(rutronik_application_t* app, uint8_t mode)
{
	if (app->ams_tof_available == 0) return;

	tmf8828_app_request_new_mode(mode);
}

static int measure_sgp40_values(rutronik_application_t* app, float temperature, float humidity, uint16_t * voc_value_raw, uint16_t * voc_value_compensated, int32_t * gas_index)
{
	if (sgp40_measure_raw_signal_without_compensation(voc_value_raw) != 0) return -1;
	if (sgp40_measure_with_compensation(temperature, humidity, voc_value_compensated) != 0) return -2;

	GasIndexAlgorithm_process(&app->gas_index_params, *voc_value_compensated, gas_index);

	return 0;
}

void rutronik_application_do(rutronik_application_t* app)
{
	if (host_main_is_ready_for_notification() == 0) return;


	// This function is called every 50ms (20Hz)
	// Only read the values every 250ms (4Hz) for CO2 and sensor fusion
	if (app->prescaler == 0)
	{
		if (app->sensor_fusion_available != 0)
		{
			float temperature = 0;
			float humidity = 0;

			if (sht4x_get_temperature_and_humidity(&temperature, &humidity) == 0)
				host_main_add_notification(notification_fabric_create_for_sht4x(temperature, humidity));
		}

		app->prescaler = 6;
	}
	else if (app->prescaler == 1)
	{
		if (app->sensor_fusion_available != 0)
		{
			float temperature = 0;
			float humidity = 0;
			uint16_t voc_value_raw = 0;
			uint16_t voc_value_compensated = 0;
			int32_t gas_index = 0;

			if (measure_sgp40_values(app, temperature, humidity, &voc_value_raw, &voc_value_compensated, &gas_index) == 0)
				host_main_add_notification(notification_fabric_create_for_sgp40(voc_value_raw, voc_value_compensated, (uint16_t) gas_index));
		}
	}
	else if (app->prescaler == 2)
	{
		float temperature = 0;
		float pressure = 0;

		if (bmp581_read_pressure_and_temperature(&pressure, &temperature) == 0)
			host_main_add_notification(notification_fabric_create_for_bmp581(pressure, temperature));
	}
	else if (app->prescaler == 3)
	{
		if(app->co2_available != 0)
		{
			if (scd41_app_do(&app->scd41_app) == 0)
				host_main_add_notification(
						notification_fabric_create_for_scd41(app->scd41_app.value.co2_ppm, app->scd41_app.value.temperature, app->scd41_app.value.humidity));
		}
	}
	else if (app->prescaler == 4)
	{
		// Get the battery voltage
		uint16_t battery_voltage = battery_monitor_get_voltage_mv();

		charge_stat_t charge_stat;
		dio_get_status(&charge_stat);

		chrg_fault_t chrg_fault;
		dio_get_fault(&chrg_fault);

		uint8_t dio_status;
		dio_monitor_read_raw(&dio_status);

		lowpassfilter_feed(&app->filtered_voltage, battery_voltage);
		battery_voltage = lowpassfilter_get_value(&app->filtered_voltage);

		host_main_add_notification(
				notification_fabric_create_for_battery_monitor(battery_voltage, (uint8_t) charge_stat, (uint8_t) chrg_fault, dio_status));
	}
	else if (app->prescaler == 5)
	{
		if(app->co2_available != 0)
		{
			if (pasco2_app_do(&app->pasco2_app) == 0)
				host_main_add_notification(
						notification_fabric_create_for_pasco2(app->pasco2_app.co2_ppm));
		}
	}

	app->prescaler = app->prescaler - 1;

	/**
	 * In order to achieve high number of values per seconds
	 * the values of the TMF8828 are continuously pushed
	 */
	if(app->ams_tof_available != 0)
	{
		if (tmf8828_app_do() == 0)
		{
			if (tmf8828_app_is_mode_8x8())
			{
				host_main_add_notification(
						notification_fabric_create_for_tmf8828_8x8_mode(tmpf8828_get_last_8x8_results()));
			}
			else
			{
				host_main_add_notification(
						notification_fabric_create_for_tmf8828(tmpf8828_get_last_results()));
			}
		}
	}
}

