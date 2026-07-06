/* vcnl403x.c
 *
 *  Created on: 20 Feb 2026
 *      Author: dc1
 */

#include "vcnl403x.h"
#include "vcnl403x_com.h"
#include "vcnl403x_def.h"

static vcnl403x_variant_t g_vcnl403x_variant = VCNL403X_VARIANT_UNKNOWN;

static vcnl403x_sleep_func_t sleep_func;

void vcnl403x_init_hal(vcnl403x_read_register_func_t read, vcnl403x_write_func_t write, vcnl403x_sleep_func_t sleep_f)
{
    vcnl403x_com_init_hal(read, write);
	sleep_func = sleep_f;
}

int vcnl403x_read_id(uint16_t *id)
{
    int retval = 0;

    if (id == 0) return -1;

    retval = vcnl403x_com_register_read_word(VCNL403X_ID, id);
    if (retval != 0) return -2;

    if ((*id & 0x00FF) != 0x80) return -3;

    return 0;
}

int vcnl403x_init(void)
{
    uint16_t device_id = 0;

    int retval = vcnl403x_com_register_read_word(VCNL403X_ID, &device_id);
    if (retval != 0) return -1;

    retval = vcnl403x_com_set_proximity_sensor_on_off(0);
    if (retval != 0) return -3;

	// Init as 4035
	vcnl403x_com_set_active_force_on_off(1);

    // Apply default settings
    vcnl403x_com_set_integration_time(VCNL403X_PS_IT_8T);
    vcnl403x_com_set_proximity_sensor_output_size(16);
    vcnl403x_com_set_sunlight_cancellation_on_off(1);
    vcnl403x_com_set_led_current(200);

	// Gesture stuff
	vcnl403x_com_set_gesture_interrupt_on_off(1);
	vcnl403x_com_set_gesture_mode_on_off(1);

    // Start sensor
    retval = vcnl403x_com_set_proximity_sensor_on_off(1);
    if (retval != 0) return -14;

	// Do a measurement
	uint16_t reg_content = 0;

	vcnl403x_trigger_measurement();

	sleep_func(20);

	// Read VCNL403X_PS2_DATA
    retval = vcnl403x_com_register_read_word(VCNL403X_PS2_DATA, &reg_content);
    if (retval != 0) return -5;

	if (reg_content == 0)
	{
		// No data for second channel -> 4030x00 version
		g_vcnl403x_variant = VCNL403X_VARIANT_SINGLE_PS;
		vcnl403x_com_set_proximity_sensor_on_off(0);
		vcnl403x_com_set_active_force_on_off(0);
		vcnl403x_com_set_gesture_mode_on_off(0);
		vcnl403x_com_set_gesture_interrupt_on_off(0);
		vcnl403x_com_set_proximity_sensor_on_off(1);
	}
	else g_vcnl403x_variant = VCNL403X_VARIANT_GESTURE_PS;


    return 0;
}

int vcnl403x_get_variant(vcnl403x_variant_t *variant)
{
    if (variant == 0) return -1;

    *variant = g_vcnl403x_variant;
    return 0;
}

int vcnl403x_get_proximity_channel_count(uint8_t *channel_count)
{
    if (channel_count == 0) return -1;

    if (g_vcnl403x_variant == VCNL403X_VARIANT_GESTURE_PS)
    {
        *channel_count = 3;
    }
    else
    {
        *channel_count = 1;
    }

    return 0;
}

int vcnl403x_trigger_measurement(void)
{
    // Trigger only in force mode (gesture variant)
    if (g_vcnl403x_variant == VCNL403X_VARIANT_GESTURE_PS)
    {
        return vcnl403x_com_trigger_gesture_sequence();
    }

    return 0;
}

int vcnl403x_get_proximity_data(uint16_t *ps1, uint16_t *ps2, uint16_t *ps3)
{
    if ((ps1 == 0) || (ps2 == 0) || (ps3 == 0)) return -1;

    if (g_vcnl403x_variant == VCNL403X_VARIANT_GESTURE_PS)
    {
        vcnl403x_com_read_ps1(ps1);
        vcnl403x_com_read_ps2(ps2);
        vcnl403x_com_read_ps3(ps3);
    }
    else
    {
        // VCNL4030: read PS1 only, remaining channels are 0
        vcnl403x_com_read_ps1(ps1);
        *ps2 = 0;
        *ps3 = 0;
    }

    return 0;
}