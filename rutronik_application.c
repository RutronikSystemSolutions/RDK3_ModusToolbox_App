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

#ifdef AMS_TMF_SUPPORT
#include "ams_tmf8828/tmf8828_app.h"
#endif

#ifdef UM980_SUPPORT
#include "hal/hal_uart.h"
#include "hal/hal_timer.h"
#include "um980/um980_app.h"
#include "um980/gga_packet.h"
#include "um980/nmea_packet.h"
#endif

#include "battery_monitor/battery_monitor.h"
#include "dio59020/dio59020.h"
#include "dps310/dps310_app.h"
#include "bmi270/bmi270_app.h"

#include "host_main.h"

static void init_sensors_hal(rutronik_application_t* app)
{
	sht4x_init(hal_i2c_read, hal_i2c_write, hal_sleep);
	bmp581_init_i2c_interface(hal_i2c_read, hal_i2c_write);
	sgp40_init(hal_i2c_read, hal_i2c_write, hal_sleep);
	scd41_app_init(&(app->scd41_app), hal_i2c_read, hal_i2c_write, hal_sleep);

#ifdef AMS_TMF_SUPPORT
	tmf8828_app_init(hal_i2c_read, hal_i2c_write);
#endif

	dio59020_init(hal_i2c_read_register, hal_i2c_write_register);
	pasco2_app_init(hal_i2c_read, hal_i2c_write, hal_sleep);
	dps310_app_init_i2c_interface(hal_i2c_read, hal_i2c_write);
#ifdef BME688_SUPPORT
	bme688_init_i2c_interface(hal_i2c_read, hal_i2c_write);
#endif

#ifdef UM980_SUPPORT
	hal_uart_init();
	hal_timer_init();
	um980_app_init_hal(hal_uart_readable, hal_uart_read, hal_uart_write, hal_timer_get_uticks);
#endif
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

#ifdef UM980_SUPPORT
static int is_um980_board_available()
{
	if (um980_app_init() != 0)
	{
		return 0;
	}
	return 1;
}
#endif

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

#ifdef BME688_SUPPORT
static int init_bme688(rutronik_application_t* app)
{
	uint16_t temperatures [10] = {320, 100, 100, 100, 200, 200, 200, 320, 320, 320};
	uint16_t steps_duration [10] = {5, 2, 10, 30, 5, 5, 5, 5, 5, 5};

	bme688_measurement_configuration_t bme688_configuration;
	bme688_configuration.heater_step_nb = 10;
	for(uint16_t i = 0; i < bme688_configuration.heater_step_nb; ++i)
	{
		bme688_configuration.temperatures[i] = temperatures[i];
		bme688_configuration.steps_duration[i] = steps_duration[i];
	}
	bme688_configuration.step_ms = 140;
	bme688_configuration.pressure_os = BME688_OVERSAMPLING_X16;
	bme688_configuration.temperature_os = BME688_OVERSAMPLING_X16;
	bme688_configuration.humidity_os = BME688_OVERSAMPLING_X16;

	return bme688_app_init_parallel_mode(&(app->bme688_app), &bme688_configuration);
}
#endif

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

	if (bmi270_app_init(hal_i2c_read, hal_i2c_write, hal_sleep_us) != 0)
	{
		app->sensor_fusion_available = 0;
		return;
	}

#ifdef BME688_SUPPORT
	if (init_bme688(app) != 0)
	{
		app->sensor_fusion_available = 0;
		return;
	}
#endif

	dps310_app_init();
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

#ifdef AMS_TMF_SUPPORT
static void init_ams_osram_board(rutronik_application_t* app)
{
	if(tmf8828_app_init_measurement() != 0)
	{
		app->ams_tof_available = 0;
		return;
	}
}
#endif

#ifdef UM980_SUPPORT

static uint8_t um980_packet_available = 0;
static um980_gga_packet_t um980_last_packet;

static void um980_nmea_listener(uint8_t* buffer, uint16_t len)
{
	if (nmea_packet_get_type(buffer, len) == PACKET_TYPE_GGA)
	{
		if (gga_packet_extract_data(buffer, len, &um980_last_packet) == 0)
		{
			um980_packet_available = 1;
		}
	}
}

static void init_um980_board(rutronik_application_t* app)
{
	if (um980_app_set_mode_rover() != 0)
	{
		app->um980_available = 0;
		return;
	}

	// Request position every second
	if (um980_app_start_gga_generation(FREQUENCY_1HZ) != 0)
	{
		app->um980_available = 0;
		return;
	}

	// Install listener
	um980_app_set_nmea_listener(um980_nmea_listener);
}
#endif


void rutronik_application_init(rutronik_application_t* app)
{
	app->sensor_fusion_available = 0;
	app->co2_available = 0;
	app->ams_tof_available = 0;
	app->um980_available = 0;

	// Init prescalers
	// 1 Hz prescalers -> shifted by 10ms (to avoid to many I2C read inside one loop)
	app->sht4x_prescaler = 0;
	app->sgp40_prescaler = 10;
	app->bmp581_prescaler = 20;
	app->scd41_prescaler = 30;
	app->battery_prescaler = 40;
	app->pasco2_prescaler = 50;
	app->dps310_prescaler = 60;

	// 10 Hz
	app->bmi270_prescaler = 0;

	// 5 Hz
	app->bme688_prescaler = 0;

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

#ifdef AMS_TMF_SUPPORT
	if (tmf8828_app_is_board_available() != 0)
	{
		app->ams_tof_available = 1;
		init_ams_osram_board(app);
	}
#endif

#ifdef UM980_SUPPORT
	if (is_um980_board_available() != 0)
	{
		app->um980_available = 1;
		init_um980_board(app);
	}
#endif
}

uint32_t rutronik_application_get_available_sensors_mask(rutronik_application_t* app)
{
	return
			(uint32_t) app->sensor_fusion_available
			| ((uint32_t) app->co2_available) << 1
			| ((uint32_t) app->ams_tof_available) << 2
			// << 3 is for radar board
			| ((uint32_t) app->um980_available) << 4;
}

#ifdef AMS_TMF_SUPPORT
void rutronik_application_set_tmf8828_mode(rutronik_application_t* app, uint8_t mode)
{
	if (app->ams_tof_available == 0) return;

	tmf8828_app_request_new_mode(mode);
}
#endif

static int measure_sgp40_values(rutronik_application_t* app, float temperature, float humidity, uint16_t * voc_value_raw, uint16_t * voc_value_compensated, int32_t * gas_index)
{
	if (sgp40_measure_raw_signal_without_compensation(voc_value_raw) != 0) return -1;
	if (sgp40_measure_with_compensation(temperature, humidity, voc_value_compensated) != 0) return -2;

	GasIndexAlgorithm_process(&app->gas_index_params, *voc_value_compensated, gas_index);

	return 0;
}

/**
 * Remark: this function is called every 10ms (100Hz)
 */
void rutronik_application_do(rutronik_application_t* app)
{
	/**
	 * SHT41
	 */
	if (app->sht4x_prescaler == 0)
	{
		if (app->sensor_fusion_available != 0)
		{
			float temperature = 0;
			float humidity = 0;

			if (sht4x_get_temperature_and_humidity(&temperature, &humidity) == 0)
				host_main_add_notification(notification_fabric_create_for_sht4x(temperature, humidity));
		}
	}
	// 1 Hz prescaler (100 Hz / 100 = 1 Hz)
	app->sht4x_prescaler++;
	if (app->sht4x_prescaler >= 100) app->sht4x_prescaler = 0;


	/**
	 * SGP40
	 */
	if (app->sgp40_prescaler == 0)
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
	// 1 Hz prescaler (100 Hz / 100 = 1 Hz)
	app->sgp40_prescaler++;
	if (app->sgp40_prescaler >= 100) app->sgp40_prescaler = 0;

	/**
	 * BMP581
	 */
	if (app->bmp581_prescaler == 0)
	{
		float temperature = 0;
		float pressure = 0;

		if (bmp581_read_pressure_and_temperature(&pressure, &temperature) == 0)
			host_main_add_notification(notification_fabric_create_for_bmp581(pressure, temperature));
	}
	// 1 Hz prescaler (100 Hz / 100 = 1 Hz)
	app->bmp581_prescaler++;
	if (app->bmp581_prescaler >= 100) app->bmp581_prescaler = 0;

	/**
	 * SCD41
	 */
	if (app->scd41_prescaler == 0)
	{
		if(app->co2_available != 0)
		{
			if (scd41_app_do(&app->scd41_app) == 0)
				host_main_add_notification(
						notification_fabric_create_for_scd41(app->scd41_app.value.co2_ppm, app->scd41_app.value.temperature, app->scd41_app.value.humidity));
		}
	}
	// 1 Hz prescaler (100 Hz / 100 = 1 Hz)
	app->scd41_prescaler++;
	if (app->scd41_prescaler >= 100) app->scd41_prescaler = 0;

	/**
	 * Battery monitor
	 */
	if (app->battery_prescaler == 0)
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
	// 1 Hz prescaler (100 Hz / 100 = 1 Hz)
	app->battery_prescaler++;
	if (app->battery_prescaler >= 100) app->battery_prescaler = 0;

	/**
	 * PAS CO2
	 */
	if (app->pasco2_prescaler == 0)
	{
		if(app->co2_available != 0)
		{
			if (pasco2_app_do(&app->pasco2_app) == 0)
				host_main_add_notification(
						notification_fabric_create_for_pasco2(app->pasco2_app.co2_ppm));
		}
	}
	// 1 Hz prescaler (100 Hz / 100 = 1 Hz)
	app->pasco2_prescaler++;
	if (app->pasco2_prescaler >= 100) app->pasco2_prescaler = 0;


	/**
	 * DPS310
	 */
	if (app->dps310_prescaler == 0)
	{
		if (app->sensor_fusion_available)
		{
			if (dps310_app_do() == 0)
			{
				float pressure = 0;
				float temperature = 0;
				dps310_app_get_last_values(&temperature, &pressure);
				host_main_add_notification(notification_fabric_create_for_dps310(pressure, temperature));
			}
		}
	}
	// 1 Hz prescaler (100 Hz / 100 = 1 Hz)
	app->dps310_prescaler++;
	if (app->dps310_prescaler >= 100) app->dps310_prescaler = 0;


	/**
	 * BMI270
	 */
	if (app->bmi270_prescaler == 0)
	{
		if (app->sensor_fusion_available)
		{
			struct bmi2_sensor_data data[2] = { { 0 } };
			data[ACCEL].type = BMI2_ACCEL;
			data[GYRO].type = BMI2_GYRO;

			if (bmi270_app_get_sensor_data(data, 2) == 0)
			{
				host_main_add_notification(
						notification_fabric_create_for_bmi270(
								data[ACCEL].sens_data.acc.x, data[ACCEL].sens_data.acc.y, data[ACCEL].sens_data.acc.z,
								data[GYRO].sens_data.gyr.x, data[GYRO].sens_data.gyr.y, data[GYRO].sens_data.gyr.z));
			}
		}
	}
	// 10 Hz prescaler (100 Hz / 10 = 10 Hz)
	app->bmi270_prescaler++;
	if (app->bmi270_prescaler >= 10) app->bmi270_prescaler = 0;


	/**
	 * BME688
	 */
	if (app->bme688_prescaler == 0)
	{
#ifdef BME688_SUPPORT
		if (app->sensor_fusion_available)
		{
			if(bme688_app_do(&(app->bme688_app)) == 0)
			{
				bme688_scan_data_t last_data;
				bme688_copy_scan_data(&(app->bme688_app.last_data), &last_data);
				host_main_add_notification(
						notification_fabric_create_for_bme688(&last_data));
			}
		}
#endif
	}
	// 5 Hz prescaler (100 Hz / 20 = 5 Hz)
	app->bme688_prescaler++;
	if (app->bme688_prescaler >= 20) app->bme688_prescaler = 0;

#ifdef AMS_TMF_SUPPORT
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
#endif

#ifdef UM980_SUPPORT
	if (app->um980_available != 0)
	{
		if (um980_app_do() != 0)
		{
			um980_app_reset();
		}

		if (um980_packet_available != 0)
		{
			um980_packet_available = 0;
			host_main_add_notification(
					notification_fabric_create_for_um980(&um980_last_packet));
		}
	}
#endif
}

