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
#include "vcnl4030x01/vcnl4030x01.h"

#include "bmm350/bmm350_app.h"
#include "sgp41/sgp41.h"
#include "bme690/bme690_app.h"
#include "bmp585/bmp585_app.h"
#include "dps368/dps368_app.h"
#include "bmi323/bmi323_app.h"

#include "host_main.h"

// TODO remove me
#include <stdio.h>

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

	vcnl4030x01_init_hal(hal_i2c_read_register, hal_i2c_write);

	sgp41_init(hal_i2c_read, hal_i2c_write, hal_sleep);
}

static int is_sensor_fusion_board_available()
{
	if (bme688_app_is_available() == 1) return 1;
	return 0;
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

static int is_vcnl4030x01_available()
{
	uint16_t id = 0;
	if (vcnl4030x01_read_id(&id) != 0)
	{
		return 0;
	}
	return 1;
}

static int is_rab7_available()
{
	if (bmm350_app_is_available() != 1) return 0;
	return 1;
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
	GasIndexAlgorithm_init(&(app->gas_index_voc_params), GasIndexAlgorithm_ALGORITHM_TYPE_VOC);

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

static void init_sgp41(rutronik_application_t* app)
{
	// Initialize algorithm for VOC type
	GasIndexAlgorithm_init(&(app->gas_index_voc_params), GasIndexAlgorithm_ALGORITHM_TYPE_VOC);
	GasIndexAlgorithm_init(&(app->gas_index_nox_params), GasIndexAlgorithm_ALGORITHM_TYPE_NOX);
}

static void init_rab7(rutronik_application_t* app)
{
	if (bmm350_app_init() != 0)
	{
		printf("bmm350_app_init error\r\n");
		app->rab7_available = 0;
		return;
	}

	if (bme690_app_init() != 0)
	{
		printf("bme690_app_init error\r\n");
		app->rab7_available = 0;
		return;
	}

	if (bmp585_app_init() != 0)
	{
		printf("bmp585_app_init error\r\n");
		app->rab7_available = 0;
		return;
	}

	if (dps368_app_init() != 0)
	{
		printf("dps368_app_init error\r\n");
		app->rab7_available = 0;
		return;
	}

	if (bmi323_app_init() != 0)
	{
		printf("bmi323_app_init error\r\n");
		app->rab7_available = 0;
		return;
	}

	init_sgp41(app);
}

void rutronik_application_init(rutronik_application_t* app)
{
	app->sensor_fusion_available = 0;
	app->co2_available = 0;
	app->ams_tof_available = 0;
	app->um980_available = 0;
	app->vcnl4030x01_available = 0;
	app->rab7_available = 0;

	app->sgp41_state = SGP41_CONDITIONING;
	app->sgp41_prescaler = 0;
	app->bme690_prescaler = 0;
	app->bmp585_prescaler = 0;
	app->dps368_prescaler = 0;

	// Init prescalers
	// 1 Hz prescalers -> shifted by 10ms (to avoid to many I2C read inside one loop)
	app->sht4x_prescaler = 0;
	app->sgp40_prescaler = 10;
	app->bmp581_prescaler = 20;
	app->scd41_prescaler = 30;
	app->battery_prescaler = 40;
	app->pasco2_prescaler = 50;
	app->dps310_prescaler = 60;
	app->vcnl4030x01_prescaler = 70;
	app->bmm350_prescaler =  0;
	app->bmi323_prescaler = 0;

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
	if (is_vcnl4030x01_available() != 0)
	{
		app->vcnl4030x01_available = 1;
		// Init measurement mode
		vcnl4030x01_init();
	}

	if (is_rab7_available() != 0)
	{
		// TODO
		printf("RAB7 available!\r\n");
		app->rab7_available = 1;
		init_rab7(app);
	}
}

uint32_t rutronik_application_get_available_sensors_mask(rutronik_application_t* app)
{
	return
			(uint32_t) app->sensor_fusion_available
			| ((uint32_t) app->co2_available) << 1
			| ((uint32_t) app->ams_tof_available) << 2
			// << 3 is for radar board
			| ((uint32_t) app->um980_available) << 4
			| ((uint32_t) app->vcnl4030x01_available) << 5
			| ((uint32_t) app->rab7_available) << 6;
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

	GasIndexAlgorithm_process(&app->gas_index_voc_params, *voc_value_compensated, gas_index);

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
		if ((app->sensor_fusion_available != 0)
				|| (app->rab7_available != 0))
		{
			if (sht4x_get_temperature_and_humidity(&(app->sht4x_temperature), &(app->sht4x_humidity)) == 0)
				host_main_add_notification(notification_fabric_create_for_sht4x(app->sht4x_temperature, app->sht4x_humidity));
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

	/**
	 * VCNL30x01
	 */
	if (app->vcnl4030x01_prescaler == 0)
	{
		if (app->vcnl4030x01_available)
		{
			uint16_t proximity_value = 0;
			if (vcnl4030x01_get_proximity_data(&proximity_value) == 0)
			{
				host_main_add_notification(
						notification_fabric_create_for_vcnl4030x01(proximity_value, 0, 0));
			}
		}
	}
	// 2 Hz prescaler (100 Hz / 20 = 5 Hz)
	app->vcnl4030x01_prescaler++;
	if (app->vcnl4030x01_prescaler >= 20) app->vcnl4030x01_prescaler = 0;

	/**
	 * BMM350
	 */
	if (app->rab7_available)
	{
		if (app->bmm350_prescaler == 0)
		{
			float mag_temp = 0;
			float mag_x = 0;
			float mag_y = 0;
			float mag_z = 0;
			int8_t bmm350_ret = 0;

			// Try to read the BMM350 sensor
			bmm350_ret = bmm350_app_read_data(&mag_temp, &mag_x, &mag_y, &mag_z);
			if (bmm350_ret == 0)
			{
				host_main_add_notification(
						notification_fabric_create_for_bmm350(mag_temp, mag_x, mag_y, mag_z));
			}
		}
		// 100 Hz / 10 -> 10Hz
		app->bmm350_prescaler++;
		if (app->bmm350_prescaler >= 10) app->bmm350_prescaler = 0;
	}

	/**
	 * SGP41
	 */
	if (app->rab7_available)
	{
		if (app->sgp41_state == SGP41_CONDITIONING)
		{
			if (app->sgp41_prescaler >= (SGP41_CONDITIONING_DURATION_MS / RUTRONIK_APP_PERIOD_MS))
			{
				app->sgp41_prescaler = 0;
				app->sgp41_state = SGP41_MEASUREMENT;
			}
			else if ((app->sgp41_prescaler % (SGP41_CONDITIONING_PERIOD_MS / RUTRONIK_APP_PERIOD_MS)) == 0)
			{
				// Call to conditioning
				sgp41_conditioning_cmd(SGP41_DEFAULT_RH, SGP41_DEFAULT_T);
				app->sgp41_prescaler++;
			}
			else
			{
				app->sgp41_prescaler++;
			}
		}
		else if (app->sgp41_state == SGP41_MEASUREMENT)
		{
			if (app->sgp41_prescaler == 0)
			{
				uint16_t sgp_sraw_voc = 0;
				uint16_t sgp_sraw_nox = 0;
				int32_t sgp_voc_index = 0;
				int32_t sgp_nox_index = 0;

				/*Read the data and send a new command to start measurement*/
				sgp41_read_raw_cmd(&sgp_sraw_voc, &sgp_sraw_nox);
				uint16 comp_rh = (uint16_t)app->sht4x_humidity * 65535 / 100;
				uint16_t comp_t = (uint16_t)(app->sht4x_temperature + 45) * 65535 / 175;
				sgp41_measure_raw_cmd(comp_rh, comp_t); /*Do the measurements with a temperature and humidity acquired from SHT4x*/
				GasIndexAlgorithm_process(&app->gas_index_voc_params, sgp_sraw_voc, &sgp_voc_index);
				GasIndexAlgorithm_process(&app->gas_index_nox_params, sgp_sraw_nox, &sgp_nox_index);

				host_main_add_notification(
						notification_fabric_create_for_sgp41(sgp_sraw_voc, sgp_sraw_nox, sgp_voc_index, sgp_nox_index));
			}

			app->sgp41_prescaler++;
			if (app->sgp41_prescaler >= (SGP41_MEASUREMENT_PERIOD_MS / RUTRONIK_APP_PERIOD_MS))
				app->sgp41_prescaler = 0;
		}
	}

	/**
	 * BME690
	 */
	if (app->rab7_available)
	{
		if (app->bme690_prescaler == 0)
		{
			int8_t bme690_result = 0;
			uint8_t fields_cnt = 0;
			bme69x_data_t data = {0};

			bme690_result = bme690_data_available(&fields_cnt);
			if(bme690_result == 0)
			{
		        for (uint8_t i = 0; i < fields_cnt; i++)
		        {
		        	if(bme690_data_read(&data, i))
		        	{
		        		host_main_add_notification(notification_fabric_create_for_bme690(&data));
		        	}
		        }
			}

		}
		app->bme690_prescaler++;
		if (app->bme690_prescaler >= (bme690_app_get_measurement_period() / RUTRONIK_APP_PERIOD_MS))
			app->bme690_prescaler = 0;
	}

	/**
	 * BMP585
	 */
	if (app->rab7_available)
	{
		if (app->bmp585_prescaler == 0)
		{
			float pressure = 0;
			float temperature = 0;
			int8_t bmp585_retval = 0;

			bmp585_retval = bmp585_read_data(&temperature, &pressure);
			if(bmp585_retval == 0)
			{
				host_main_add_notification(notification_fabric_create_for_bmp585(pressure, temperature));
			}

		}
		app->bmp585_prescaler++;
		if (app->bmp585_prescaler >= (BMP585_MEASUREMENT_PERIOD_MS / RUTRONIK_APP_PERIOD_MS))
			app->bmp585_prescaler = 0;
	}

	/**
	 * DPS368
	 */
	if (app->rab7_available)
	{
		if (app->dps368_prescaler == 0)
		{
			float pressure = 0;
			float temperature = 0;
			int8_t dps368_retval = 0;

			dps368_retval = dps368_read_data(&temperature, &pressure);
			if(dps368_retval == 0)
			{
				// convert to Pa (currently in hPa)
				pressure = pressure * 100.f;
				host_main_add_notification(notification_fabric_create_for_dps368(pressure, temperature));
			}
		}
		app->dps368_prescaler++;
		if (app->dps368_prescaler >= (DPS368_MEASUREMENT_PERIOD_MS / RUTRONIK_APP_PERIOD_MS))
			app->dps368_prescaler = 0;

	}

	/**
	 * BMI323
	 */
	if (app->rab7_available)
	{
		if (app->bmi323_prescaler == 0)
		{
			int16_t acc_x = 0;
			int16_t acc_y = 0;
			int16_t acc_z = 0;
			int16_t gyr_x = 0;
			int16_t gyr_y = 0;
			int16_t gyr_z = 0;
			_Bool acc_ready = 0;
			_Bool gyr_ready = 0;

			int8_t bmi323_retval = 0;

			// Check if sensors are ready
			bmi323_retval = bmi323_int_status(&acc_ready, &gyr_ready);
			if(bmi323_retval == 0)
			{
				if( (acc_ready) && (gyr_ready))
				{
					if (bmi323_read_acc_data(&acc_x, &acc_y, &acc_z) != 0)
					{
						acc_x = 1;
						acc_y = 2;
						acc_z = 3;
					}
					if (bmi323_read_gyr_data(&gyr_x, &gyr_y, &gyr_z) != 0)
					{
						gyr_x = 4;
						gyr_y = 5;
						gyr_z = 6;
					}

					host_main_add_notification(
							notification_fabric_create_for_bmi323(acc_x, acc_y, acc_z, gyr_x, gyr_y, gyr_z));

				}
			}
		}
		app->bmi323_prescaler++;
		if (app->bmi323_prescaler >= (BMI323_MEASUREMENT_PERIOD_MS / RUTRONIK_APP_PERIOD_MS))
			app->bmi323_prescaler = 0;
	}
}

