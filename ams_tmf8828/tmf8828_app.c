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

static void power_on_tmf882x(void)
{
    cyhal_gpio_write(ARDU_IO4, false);
    cyhal_system_delay_ms(1000); //Cold Start --> wait 1s
    cyhal_gpio_write(ARDU_IO4, true);
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
	* Disable 8x8 mode (if applicable) for tmf8828 devices
	*  - IOCAPP_SET_8X8MODE is the ioctl command code used to set the 8x8 mode
	*
	*************************************************************************/
	bool mode_8x8 = false;
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
	*  - set the spad map configuration to 3x3 (33x32 degree FoV)
	*
	*************************************************************************/
	tofcfg.report_period_ms = 100;
	tofcfg.spad_map_id = 1;

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

int tmf8828_app_do()
{
	tmf882x_process_irq(&tof);

	if (last_sent_result != last_results.result_num)
	{
		last_sent_result = last_results.result_num;
		return 0;
	}

	// Nothing available
	return 1;
}

tmf8828_results_t* tmpf8828_get_last_results()
{
	return &last_results;
}

void tmpf8828_on_new_result(struct tmf882x_msg_meas_results *result_msg)
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

