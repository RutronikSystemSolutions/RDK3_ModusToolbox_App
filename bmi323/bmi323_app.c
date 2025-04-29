/*
 * bmi323_app.c
 *
 *  Created on: 2025-04-03
 *      Author: GDR
 */



#include "bmi323_app.h"

struct bmi3_dev bmi = { 0 };
struct bmi3_sens_config config[2];
/*Interrupt output pins configuration structure*/
const struct bmi3_int_pin_config bmi323_int_config =
{
		.pin_type = BMI3_INT1,
		.int_latch = BMI3_INT_LATCH_DISABLE,
		.pin_cfg[0].lvl = BMI3_INT_ACTIVE_HIGH,
		.pin_cfg[0].od = BMI3_INT_PUSH_PULL,
		.pin_cfg[0].output_en = BMI3_INT_OUTPUT_ENABLE,
		.pin_cfg[1].lvl = BMI3_INT_ACTIVE_LOW,
		.pin_cfg[1].od = BMI3_INT_PUSH_PULL,
		.pin_cfg[1].output_en = BMI3_INT_OUTPUT_DISABLE,
};
struct bmi3_sensor_data bmi_sensor_data[2] = { 0 };

int8_t bmi323_app_init(void)
{
	int8_t rslt;
    struct bmi3_map_int map_int = { 0 };

    rslt = bmi3_interface_init(&bmi, BMI3_I2C_INTF);
    if(rslt != BMI3_OK)
    {
    	bmi3_error_codes_print_result("bmi3_interface_init", rslt);
    	return rslt;
    }

    rslt = bmi323_init(&bmi);
    if(rslt != BMI3_OK)
    {
    	bmi3_error_codes_print_result("bmi323_init", rslt);
    	return rslt;
    }

    rslt = bmi323_soft_reset(&bmi);
    if(rslt != BMI3_OK)
    {
    	bmi3_error_codes_print_result("bmi323_reset", rslt);
    	return rslt;
    }


    /* Select accel and gyro sensor */
    bmi_sensor_data[0].type = BMI323_ACCEL;
    bmi_sensor_data[1].type = BMI323_GYRO;

    /* Configure the type of feature. */
    config[0].type = BMI323_ACCEL;
    config[1].type = BMI323_GYRO;

    /* NOTE: The user can change the following configuration parameters according to their requirement. */
    /* Accel configuration settings. */
    /* Output Data Rate. The ODR is set for accelerometer. */
    config[0].cfg.acc.odr = BMI3_ACC_ODR_12_5HZ;

    /* The Accel bandwidth coefficient defines the 3 dB cutoff frequency in relation to the ODR. */
    config[0].cfg.acc.bwp = BMI3_ACC_BW_ODR_QUARTER;

    /* Set number of average samples for accel. */
    config[0].cfg.acc.avg_num = BMI3_ACC_AVG64;

    /* Gravity range of the sensor (+/- 2G, 4G, 8G, 16G). */
    config[0].cfg.acc.range = BMI3_ACC_RANGE_2G;

    /* To enable the accelerometer set the power mode to normal mode */
    config[0].cfg.acc.acc_mode = BMI3_ACC_MODE_NORMAL;

    /* Gyro configuration settings. */
    /* Output data Rate. */
    config[1].cfg.gyr.odr = BMI3_GYR_ODR_12_5HZ;

    /*  The Gyroscope bandwidth coefficient defines the 3 dB cutoff frequency in relation to the ODR
     *  Value   Name      Description
     *    0   odr_half   BW = gyr_odr/2
     *    1  odr_quarter BW = gyr_odr/4
     */
    config[1].cfg.gyr.bwp = BMI3_GYR_BW_ODR_HALF;

    /* Value    Name    Description
     *  000     avg_1   No averaging; pass sample without filtering
     *  001     avg_2   Averaging of 2 samples
     *  010     avg_4   Averaging of 4 samples
     *  011     avg_8   Averaging of 8 samples
     *  100     avg_16  Averaging of 16 samples
     *  101     avg_32  Averaging of 32 samples
     *  110     avg_64  Averaging of 64 samples
     */
    config[1].cfg.gyr.avg_num = BMI3_GYR_AVG4;

    /* Gyroscope Angular Rate Measurement Range.By default the range is 2000dps. */
    config[1].cfg.gyr.range = BMI3_GYR_RANGE_125DPS;

    /* To enable the gyroscope set the power mode to normal mode */
    config[1].cfg.gyr.gyr_mode = BMI3_GYR_MODE_NORMAL;

    /* Set new configurations */
    rslt = bmi323_set_sensor_config(config, 2, &bmi);
    bmi3_error_codes_print_result("bmi323_set_sensor_config", rslt);
    if(rslt != BMI3_OK){return rslt;}

    /* Map the FIFO full interrupt to INT1 */
    /* Note: User can map the interrupt to INT1 or INT2 */
    map_int.acc_drdy_int = BMI3_INT1;
    map_int.gyr_drdy_int = BMI3_INT1;

    /* Map the interrupt configuration */
    rslt = bmi323_map_interrupt(map_int, &bmi);
    bmi3_error_codes_print_result("bmi323_map_interrupt", rslt);
    if(rslt != BMI3_OK){return rslt;}

    /*Configure the interrupt pin outputs*/
    rslt = bmi3_set_int_pin_config(&bmi323_int_config, &bmi);
    bmi3_error_codes_print_result("bmi3_set_int_pin_config", rslt);
    if(rslt != BMI3_OK){return rslt;}

	return rslt;
}

int8_t bmi323_int_status(_Bool *acc_rdy, _Bool *gyr_rdr)
{
	int8_t rslt;
	uint16_t int_status = 0;

    rslt = bmi323_get_int1_status(&int_status, &bmi);
    bmi3_error_codes_print_result("bmi323_get_int1_status", rslt);
    if(rslt != BMI3_OK){return rslt;}

    if (int_status & BMI3_INT_STATUS_ACC_DRDY)
    {
    	*acc_rdy = 1;
    }
    else
    {
    	*acc_rdy = 0;
    }

    if (int_status & BMI3_INT_STATUS_GYR_DRDY)
    {
    	*gyr_rdr = 1;
    }
    else
    {
    	*gyr_rdr = 0;
    }

	return rslt;
}

int8_t bmi323_read_acc_data(int16_t *bmi_acc_x, int16_t *bmi_acc_y, int16_t *bmi_acc_z)
{
	int8_t rslt;

	rslt = bmi323_get_sensor_data(&bmi_sensor_data[0], 1, &bmi);
	bmi3_error_codes_print_result("Get accelerometer data", rslt);
	if(rslt != BMI3_OK){return rslt;}

	*bmi_acc_x = bmi_sensor_data[0].sens_data.acc.x;
	*bmi_acc_y = bmi_sensor_data[0].sens_data.acc.y;
	*bmi_acc_z = bmi_sensor_data[0].sens_data.acc.z;

	return rslt;
}

int8_t bmi323_read_gyr_data(int16_t *bmi_gyr_x, int16_t *bmi_gyr_y, int16_t *bmi_gyr_z)
{
	int8_t rslt;

	rslt = bmi323_get_sensor_data(&bmi_sensor_data[1], 1, &bmi);
	bmi3_error_codes_print_result("Get gyroscope data", rslt);
	if(rslt != BMI3_OK){return rslt;}

	*bmi_gyr_x = bmi_sensor_data[1].sens_data.gyr.x;
	*bmi_gyr_y = bmi_sensor_data[1].sens_data.gyr.y;
	*bmi_gyr_z = bmi_sensor_data[1].sens_data.gyr.z;

	return rslt;
}
