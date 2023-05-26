/*
 * tmf8828_app.c
 *
 *  Created on: 14 Apr 2023
 *      Author: jorda
 */

#include "tmf8828_app.h"

#include "cyhal_gpio.h"
#include "cycfg_pins.h"
#include "cyhal_system.h"

#include "tmf882x_interface.h"
#include "tof_bin_image.h"
#include "tof_factory_cal.h"

#include <stdio.h>

/**
 * Following defines are needed to display the values of the sensor in case
 * the 8x8 mode is selected
 */
#define NUM_MEAS_MSG_IN_8X8		  4
#define NUM_ROWS_IN_8X8           8
#define NUM_COLS_IN_8X8           8

static const uint8_t TMF8828_I2C_ADDR = 0x41;

static tmf8828_read_func_t i2c_read_bytes;
static tmf8828_write_func_t i2c_write_bytes;

static struct tmf882x_tof tof;
static struct platform_ctx tof_ctx =
	{
		.curr_num_measurements = 0,
		.debug = 0,
		.gpio_ce = 0,
		.gpio_irq = 0,
		.i2c_addr = TMF8828_I2C_ADDR,
		.i2cdev = "AMS_TOF_RUTRONIK",
		.mode_8x8 = 1,
		.tof = &tof,
	};
static struct tmf882x_mode_app_config tofcfg;

static tmf8828_results_t last_results;
static uint32_t last_sent_result;

static struct tmf882x_msg_meas_results msgs[NUM_MEAS_MSG_IN_8X8] = {0};
static uint16_t distances_mm[64] = { 0 };
static uint32_t measurement_counter = 0;

static uint8_t requested_new_mode = TMF8828_MODE_INVALID;

void tmf8828_app_init(tmf8828_read_func_t read, tmf8828_write_func_t write)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;

	last_results.result_num = 0;
	last_results.num_results = 0;
	last_sent_result = 0;
}

static int tmf8828_get_serial_number(uint8_t* id, uint8_t* revid)
{
	int8_t result = 0;
	const uint8_t get_id_cmd = 0xE3;
	uint8_t cmd[1] = {0};
	uint8_t data[2] = {0};

	// Construct the command
	cmd[0] = get_id_cmd;

	// Send it
	result = i2c_write_bytes(TMF8828_I2C_ADDR, cmd, sizeof(cmd));
	if (result != 0) return -1;

	// Read answer
	result = i2c_read_bytes(TMF8828_I2C_ADDR, data, sizeof(data));
	if (result != 0) return -2;

	// Convert
	*id = data[0];
	*revid = data[1];

	// Check plausibility
	if (*id == 0 && *revid == 0) return -3;

	return 0;
}

int tmf8828_app_is_board_available()
{
	uint8_t id = 0;
	uint8_t revid = 0;

	if (tmf8828_get_serial_number(&id, &revid) != 0) return 0;

	return 1;
}

uint16_t tmf8828_app_is_mode_8x8()
{
	if (tof_ctx.mode_8x8 != 0) return 1;
	return 0;
}

static void power_on_tmf882x(void)
{
    cyhal_gpio_write(ARDU_IO4, false);
    cyhal_system_delay_ms(1000); //Cold Start --> wait 1s
    cyhal_gpio_write(ARDU_IO4, true);
}

/**
 * @brief Check if the mode (in parameter) is valid or not
 *
 * @retval 0 Not valid
 * @retval 1 Valid
 */
static uint8_t is_mode_valid(uint8_t mode)
{
	if (mode == TMF8828_MODE_3X3) return 1;
	if (mode == TMF8828_MODE_8X8) return 1;
	return 0;
}

int tmf8828_app_init_measurement()
{
	// Temporary variable for storing version info
	uint8_t ver[16] = { 0 };
	bool is_measuring = false;

	// Assert the CE pin on the TMF882X to turn on the device
	power_on_tmf882x();

	/*************************************************************************
	*
	* Initialize the TMF882X core driver.
	*  - 'struct tmf882x_tof' is the core driver context structure
	*  - performs no I/O, only data initialization
	*
	*************************************************************************/
	tmf882x_init(&tof, &tof_ctx);

	/**************************************************************************
	*
	* DISABLE debug logging in the TMF882X core driver
	*
	*************************************************************************/
	tmf882x_set_debug(&tof, false);

	/**************************************************************************
	*
	* Open the TMF882X core driver
	*  - perform chip initialization
	*  - perform mode-specific initialization
	*
	*************************************************************************/
	if (tmf882x_open(&tof) != 0) return -1;

	/**************************************************************************
	*
	* Switch from the current mode to the Bootloader mode
	*     - Must be in the Bootloader mode to perform Firmware Download (FWDL)
	*     - All modes support switching to the Bootloader mode
	*
	*************************************************************************/
	if(tmf882x_mode_switch(&tof, TMF882X_MODE_BOOTLOADER) != 0) return -2;

	/**************************************************************************
	*
	* Perform FWDL
	*     - FWDL supports "bin" download or "intel hex format" download
	*
	*************************************************************************/
	if(tmf882x_fwdl(&tof, FWDL_TYPE_BIN, tof_bin_image, tof_bin_image_length) != 0) return -3;

	/**************************************************************************
	*
	* Retrieve the current mode Firmware version information
	*
	*************************************************************************/
	(void) tmf882x_get_firmware_ver(&tof, ver, sizeof(ver));

	/**************************************************************************
	*
	* Enable or disable the 8x8 mode
	*
	*************************************************************************/
	bool mode_8x8 = true;
	if(tmf882x_ioctl(&tof, IOCAPP_SET_8X8MODE, &mode_8x8, NULL) != 0) return -4;

	/**************************************************************************
	*
	* Retrieve the current APP mode configuration data
	*  - 'struct tmf882x_mode_app_config' is the APP mode config structure
	*  - IOCAPP_GET_CFG is the ioctl command code used to get the APP config
	*
	*************************************************************************/
	if(tmf882x_ioctl(&tof, IOCAPP_GET_CFG, NULL, &tofcfg) != 0) return -5;

	/**************************************************************************
	*
	* Change the APP configuration
	*  - set the reporting period to 100 milliseconds
	*  - set the spad map configuration to 3x3 (33x32 degree FoV) (in case not 8x8 mode)
	*
	*************************************************************************/
	tofcfg.report_period_ms = 25;
//#else
//	tofcfg.report_period_ms = 100;
//	tofcfg.spad_map_id = 1;
//#endif

	/**************************************************************************
	*
	* Commit the changed APP mode configuration data
	*  - IOCAPP_SET_CFG is the ioctl command code used to set the APP config
	*
	*************************************************************************/
	if(tmf882x_ioctl(&tof, IOCAPP_SET_CFG, &tofcfg, NULL) != 0) return -6;

	/**************************************************************************
	*
	* Write the APP factory calibration data
	*  - IOCAPP_SET_CALIB is the ioctl command code used to set the APP calibration
	*
	*************************************************************************/
	if(tmf882x_ioctl(&tof, IOCAPP_SET_CALIB, &calibration_data, NULL) != 0) return -7;

	/**************************************************************************
	*
	* The demo runs forever polling the TMF882X device for measurement results
	*  - the poll period is POLL_PERIOD_MS
	*  - 'tmf882x_process_irq()' checks and clears the device interrupt flags
	*      and reads out any available data
	*  - check the 'ISP' user button to start/stop measurements
	*  - log measurement results to the debug serial port
	*
	*************************************************************************/

	(void) tmf882x_start(&tof);

	(void) tmf882x_ioctl(&tof, IOCAPP_IS_MEAS, NULL, &is_measuring);

	return 0;
}

static void change_mode(uint8_t mode)
{
	bool is_measuring = false;

	tmf882x_stop(&tof);
	tmf882x_ioctl(&tof, IOCAPP_IS_MEAS, NULL, &is_measuring);

	bool mode_8x8 = true;
	if (mode == TMF8828_MODE_3X3) mode_8x8 = false;
	if(tmf882x_ioctl(&tof, IOCAPP_SET_8X8MODE, &mode_8x8, NULL) != 0) return;


	if(tmf882x_ioctl(&tof, IOCAPP_GET_CFG, NULL, &tofcfg) != 0) return;

	if (mode == TMF8828_MODE_8X8)
	{
		tof_ctx.mode_8x8 = 1;
		tofcfg.report_period_ms = 25;
	}
	else
	{
		tof_ctx.mode_8x8 = 0;
		tofcfg.report_period_ms = 100;
		tofcfg.spad_map_id = 1;
	}

	if(tmf882x_ioctl(&tof, IOCAPP_SET_CFG, &tofcfg, NULL) != 0) return;

	if(tmf882x_ioctl(&tof, IOCAPP_SET_CALIB, &calibration_data, NULL) != 0) return;

	(void) tmf882x_start(&tof);

	(void) tmf882x_ioctl(&tof, IOCAPP_IS_MEAS, NULL, &is_measuring);
}

void tmf8828_app_request_new_mode(uint8_t mode)
{
	if (is_mode_valid(mode))
	{
		requested_new_mode = mode;
	}
	else
	{
		requested_new_mode = TMF8828_MODE_INVALID;
	}
}

int tmf8828_app_do()
{
	tmf882x_process_irq(&tof);

	if (is_mode_valid(requested_new_mode))
	{
		printf("Do something with the new mode! %u \r\n", requested_new_mode);
		change_mode(requested_new_mode);
		requested_new_mode = TMF8828_MODE_INVALID;
	}

	if (tof_ctx.mode_8x8 != 0)
	{
		if (last_sent_result != measurement_counter)
		{
			last_sent_result = measurement_counter;
			return 0;
		}
	}
	else
	{
		if (last_sent_result != last_results.result_num)
		{
			last_sent_result = last_results.result_num;
			return 0;
		}
	}

	// Nothing available
	return 1;
}

tmf8828_results_t* tmpf8828_get_last_results()
{
	return &last_results;
}

uint16_t* tmpf8828_get_last_8x8_results()
{
	return distances_mm;
}

void tmpf8828_on_new_result(struct platform_ctx *ctx, struct tmf882x_msg_meas_results *result_msg)
{
	if (!ctx || !result_msg) return;

	if (ctx->mode_8x8 != 0)
	{
		uint32_t msg_idx = 0;
		uint32_t row = 0, col = 0, ch = 0;

		// which measurement in the 4x group is this?
		msg_idx = result_msg->result_num % NUM_MEAS_MSG_IN_8X8;

		// Buffer up 4 messages (for 64 zones)
		memcpy(&msgs[msg_idx], result_msg, sizeof(*msgs));

		if (msg_idx != (NUM_MEAS_MSG_IN_8X8 - 1))
			return;	// wait until we have all measurement messages in an 8x8 group before logging

		/**
		 * Reset the distances
		 * This is needed since some channels might have no reflection and in that case will be 0 because they were not updated
		 */
		for (uint16_t i = 0; i < 64; ++i) distances_mm[i] = 0;

		// Map 4 measurement messages to one combined 64 pixel depth map
		for (uint32_t idx = 0; idx < NUM_MEAS_MSG_IN_8X8; ++idx) {
			for (uint32_t res = 0; res < msgs[idx].num_results; ++res) {

				// Result Number, SubCapture, Channel indicate 1:64 zone mapping
				ch = msgs[idx].results[res].channel;
				row = col = 0;

				row += (1 - (ch - 1) / 4) * 4;
				row += (1 - (((ch - 1) % 4) / 2)) * 2;
				row += (1 - idx/2);

				col += (1 - (ch % 2)) * 4;
				col += (idx % 2) * 2;
				col += msgs[idx].results[res].sub_capture;

				distances_mm[row * NUM_COLS_IN_8X8 + col] = (uint16_t) msgs[idx].results[res].distance_mm;
			}
		}

		measurement_counter = measurement_counter + 1;

//		// Print 8x8 distance results (1st object only)
//		for (uint32_t idx = 0; idx < sizeof(distances_mm)/sizeof(*distances_mm); ++idx) {
//			printf("%4.0u", distances_mm[idx] / 10);
//			if (((idx+1) % 8) == 0) { // print in 8x8 grid
//				printf("\r\n");
//			}
//		}
//
//		printf("\r\n\r\n");
	}
	else
	{
		// Copy
		last_results.result_num = result_msg->result_num;
		last_results.temperature = result_msg->temperature;
		last_results.ambient_light = result_msg->ambient_light;
		last_results.photon_count = result_msg->photon_count;
		last_results.ref_photon_count = result_msg->ref_photon_count;
		last_results.sys_ticks = result_msg->sys_ticks;
		last_results.valid_results = result_msg->valid_results;
		last_results.num_results = result_msg->num_results;
		for (uint32_t i = 0; i < result_msg->num_results; ++i)
		{
			last_results.results[i].confidence = result_msg->results[i].confidence;
			last_results.results[i].distance_mm = result_msg->results[i].distance_mm;
			last_results.results[i].channel = result_msg->results[i].channel;
			last_results.results[i].ch_target_idx = result_msg->results[i].ch_target_idx;
			last_results.results[i].sub_capture = result_msg->results[i].sub_capture;
		}
	}

//	printf("tmpf8828_on_new_result\r\n");
//	printf("measurement_num: %u num_results: %u valid results: %u \r\n",
//	    		(unsigned int)result_msg->result_num,
//				(unsigned int)result_msg->num_results,
//				(unsigned int)result_msg->valid_results);
//
//	    for (uint32_t i = 0; i < result_msg->num_results; ++i)
//	    {
//	        printf("conf: %u distance_mm: %u channel: %u sub_capture: %u\r\n",
//	        		(unsigned int)result_msg->results[i].confidence,
//					(unsigned int)result_msg->results[i].distance_mm,
//					(unsigned int)result_msg->results[i].channel,
//					(unsigned int)result_msg->results[i].sub_capture);
//	    }
//
//	    printf("photon: %u ref_photon: %u ALS: %u\r\n",
//	    		(unsigned int)result_msg->photon_count,
//				(unsigned int)result_msg->ref_photon_count,
//				(unsigned int)result_msg->ambient_light);
}

