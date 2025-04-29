/*
 * bmm350_app.c
 *
 *  Created on: 2025-04-03
 *      Author: GDR
 */


#include "bmm350_app.h"

/*BMM350*/
struct bmm350_dev bmm = { 0 };
struct bmm350_mag_temp_data bmm_data;
struct bmm350_pmu_cmd_status_0 pmu_cmd_stat_0;
uint8_t bmm_set_int_ctrl, bmm_int_status, bmm_int_ctrl, bmm_err_reg_data = 0;

static uint8_t interface_initialized = 0;

int8_t bmm350_app_is_available()
{
	if (interface_initialized == 0)
	{
		bmm350_interface_init(&bmm);
		interface_initialized = 1;
	}

	uint8_t chip_id = BMM350_DISABLE;
	int8_t rslt = bmm350_get_regs(BMM350_REG_CHIP_ID, &chip_id, 1, &bmm);

	if ((rslt == BMM350_OK) && (chip_id == BMM350_CHIP_ID))
	{
		return 1;
	}

	return 0;
}


int8_t bmm350_app_init(void)
{
	int8_t rslt;

	if (interface_initialized == 0)
	{
		bmm350_interface_init(&bmm);
		interface_initialized = 1;
	}

    rslt = bmm350_init(&bmm);
    bmm350_error_codes_print_result("bmm350_init", rslt);
    if(rslt != BMM350_OK){return rslt;}

    rslt = bmm350_get_pmu_cmd_status_0(&pmu_cmd_stat_0, &bmm);
    bmm350_error_codes_print_result("bmm350_get_pmu_cmd_status_0", rslt);
    if(rslt != BMM350_OK){return rslt;}

    rslt = bmm350_get_regs(BMM350_REG_ERR_REG, &bmm_err_reg_data, 1, &bmm);
    bmm350_error_codes_print_result("bmm350_get_error_reg_data", rslt);
    if(rslt != BMM350_OK){return rslt;}

    rslt = bmm350_configure_interrupt(BMM350_PULSED, BMM350_ACTIVE_HIGH, BMM350_INTR_PUSH_PULL, BMM350_MAP_TO_PIN, &bmm);
    bmm350_error_codes_print_result("bmm350_configure_interrupt", rslt);
    if(rslt != BMM350_OK){return rslt;}

    rslt = bmm350_enable_interrupt(BMM350_ENABLE_INTERRUPT, &bmm);
    bmm350_error_codes_print_result("bmm350_enable_interrupt", rslt);
    if(rslt != BMM350_OK){return rslt;}

    rslt = bmm350_get_regs(BMM350_REG_INT_CTRL, &bmm_int_ctrl, 1, &bmm);
    bmm350_error_codes_print_result("bmm350_get_regs", rslt);
    if(rslt != BMM350_OK){return rslt;}

    bmm_set_int_ctrl = ((BMM350_INT_POL_ACTIVE_HIGH << 1) | (BMM350_INT_OD_PUSHPULL << 2) | (BMM350_ENABLE << 3) | BMM350_ENABLE << 7);

    rslt = bmm350_set_odr_performance(BMM350_DATA_RATE_6_25HZ, BMM350_AVERAGING_4, &bmm);
    bmm350_error_codes_print_result("bmm350_set_odr_performance", rslt);
    if(rslt != BMM350_OK){return rslt;}

    rslt = bmm350_delay_us(10000, &bmm);
    bmm350_error_codes_print_result("bmm350_delay_us", rslt);
    if(rslt != BMM350_OK){return rslt;}

    rslt = bmm350_enable_axes(BMM350_X_EN, BMM350_Y_EN, BMM350_Z_EN, &bmm);
    bmm350_error_codes_print_result("bmm350_enable_axes", rslt);
    if(rslt != BMM350_OK){return rslt;}

    if (rslt == BMM350_OK)
    {
        rslt = bmm350_set_pad_drive(BMM350_PAD_DRIVE_STRONGEST, &bmm);
        bmm350_error_codes_print_result("bmm350_set_pad_drive", rslt);
        if(rslt != BMM350_OK){return rslt;}

        rslt = bmm350_set_powermode(BMM350_NORMAL_MODE, &bmm);
        bmm350_error_codes_print_result("bmm350_set_powermode", rslt);
        if(rslt != BMM350_OK){return rslt;}
    }

	return rslt;
}

int8_t bmm350_app_read_data(float *temp, float *mag_x, float *mag_y, float *mag_z)
{
	int8_t rslt;
	bmm_int_status = 0;

    rslt = bmm350_get_regs(BMM350_REG_INT_STATUS, &bmm_int_status, 1, &bmm);
    bmm350_error_codes_print_result("bmm350_get_regs", rslt);
    if(rslt != BMM350_OK){return rslt;}

    /* Check if data ready interrupt occurred */
    if (bmm_int_status & BMM350_DRDY_DATA_REG_MSK)
    {
        rslt = bmm350_get_compensated_mag_xyz_temp_data(&bmm_data, &bmm);
        bmm350_error_codes_print_result("bmm350_get_compensated_mag_xyz_temp_data", rslt);
        if(rslt != BMM350_OK){return rslt;}

        *temp = bmm_data.temperature;
        *mag_x = bmm_data.x;
        *mag_y = bmm_data.y;
        *mag_z = bmm_data.z;
    }
    else
    {
    	// Data not ready
    	return 1;
    }

	return rslt;
}

