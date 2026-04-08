/* vcnl403x_com.c
 *
 *  Created on: 20 Feb 2026
 *      Author: dc1
 */

#include "vcnl403x_com.h"
#include "vcnl403x_hal_interface.h"
#include "vcnl403x_def.h"

static vcnl403x_read_register_func_t i2c_read_register;
static vcnl403x_write_func_t i2c_write_bytes;

void vcnl403x_com_init_hal(vcnl403x_read_register_func_t read, vcnl403x_write_func_t write)
{
    i2c_read_register = read;
    i2c_write_bytes = write;
}

/**
 * @brief Read register value over I2C
 * Access to the VCNL is limited to 2 bytes (1 word)
 */
int vcnl403x_com_register_read_word(uint8_t reg, uint16_t *buffer)
{
    return i2c_read_register(VCNL403X_ADDR, reg, (uint8_t *)buffer, sizeof(uint16_t));
}

/**
 * @brief Write a word value over I2C
 */
static int register_write_word(uint8_t reg, uint16_t data)
{
    uint8_t buffer[3] = { reg, (uint8_t)(data & 0xFF), (uint8_t)((data >> 8) & 0xFF) };

    int8_t result = i2c_write_bytes(VCNL403X_ADDR, buffer, sizeof(buffer));
    if (result != 0)
    {
        return -1;
    }

    return 0;
}

/**
 * @param [in] status 0: off, 1: on
 */
int vcnl403x_com_set_proximity_sensor_on_off(uint8_t status)
{
    const uint16_t ps_sd_mask = 0x1;	// first bit

	// Read content of the register
    uint16_t reg_content = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_1, &reg_content);
    if (retval != 0)
    {
        return -1;
    }

	// Mask the bit we want to set
    reg_content = reg_content & ~ps_sd_mask;

	// In case status is OFF, write 1
    if (status == 0)
        reg_content = reg_content | ps_sd_mask;

    retval = register_write_word(VCNL403X_PS_CONF_1, reg_content);
    if (retval != 0)
    {
        return -2;
    }

    return 0;
}

/**
 * @param [in] status 0: off, 1: on
 */
int vcnl403x_com_set_active_force_on_off(uint8_t status)
{
    const uint16_t ps_af_mask = (uint16_t)(0x1u << 3);

	// Read content of the register
    uint16_t reg_content = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_3, &reg_content);
    if (retval != 0)
    {
        return -1;
    }

	// Mask the bit we want to set
    reg_content = reg_content & ~ps_af_mask;

	// In case status is ON, write 1
    if (status == 1)
        reg_content = reg_content | ps_af_mask;

    retval = register_write_word(VCNL403X_PS_CONF_3, reg_content);
    if (retval != 0)
    {
        return -2;
    }

    return 0;
}

/**
 * @brief Set the integration time
 * @param [in] integration_time
 * 				0: 1T
 * 				1: 1.5T
 * 				2: 2T
 * 				3: 2.5T
 * 				4: 3T
 * 				5: 3.5T
 * 				6: 4T
 * 				7: 8T
 */
int vcnl403x_com_set_integration_time(uint8_t integration_time)
{
    const uint16_t ps_it_mask = (uint16_t)(0x7u << 1);

    if (integration_time > 7)
    {
        return -1;
    }

    uint16_t reg_content = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_1, &reg_content);
    if (retval != 0)
    {
        return -2;
    }

    reg_content = reg_content & ~ps_it_mask;
    reg_content = reg_content | ((uint16_t)integration_time << 1);

    retval = register_write_word(VCNL403X_PS_CONF_1, reg_content);
    if (retval != 0)
    {
        return -3;
    }

    return 0;
}

/**
 * @param [in] size in bits
 * 				Possible values: 12 or 16
 */
int vcnl403x_com_set_proximity_sensor_output_size(uint8_t size)
{
    const uint16_t ps_output_size_mask = (uint16_t)(0x1u << 11);	// bit 11

    if ((size != 12) && (size != 16))
    {
        return -1;
    }

	// Read content of the register
    uint16_t reg_content = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_1, &reg_content);
    if (retval != 0)
    {
        return -2;
    }

	// Mask the bit we want to set
    reg_content = reg_content & ~ps_output_size_mask;

	// Set output size bit
    if (size == 16)
        reg_content = reg_content | ps_output_size_mask;

    retval = register_write_word(VCNL403X_PS_CONF_1, reg_content);
    if (retval != 0)
    {
        return -3;
    }

    return 0;
}


int vcnl403x_com_set_sunlight_cancellation_on_off(uint8_t status)
{
    const uint16_t ps_sc_en_mask = 0x1;	// first bit

	// Read content of the register
    uint16_t reg_content = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_3, &reg_content);
    if (retval != 0)
    {
        return -1;
    }

	// Mask the bit we want to set
    reg_content = reg_content & ~ps_sc_en_mask;

	// In case status is ON, write 1
    if (status == 1)
        reg_content = reg_content | ps_sc_en_mask;

    retval = register_write_word(VCNL403X_PS_CONF_3, reg_content);
    if (retval != 0)
    {
        return -2;
    }

    return 0;
}

/**
 * @param [in] current Current intensity to be set
 * 				Possible values are:
 * 				50 -> 50mA
 * 				75 -> 75mA
 * 				100 -> 100mA
 * 				120 -> 120mA
 * 				140 -> 140mA
 * 				160 -> 160mA
 * 				180 -> 180mA
 * 				200 -> 200mA
 */
int vcnl403x_com_set_led_current(uint8_t current)
{
    const uint16_t led_i_mask = (uint16_t)(0x7u << 8);	// bit 10, 9, 8

    if ((current != 50)
            && (current != 75)
            && (current != 100)
            && (current != 120)
            && (current != 140)
            && (current != 160)
            && (current != 180)
            && (current != 200))
    {
        return -1;
    }

	// Read content of the register
    uint16_t reg_content = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_3, &reg_content);
    if (retval != 0)
    {
        return -2;
    }

	// Mask the bit we want to set
    reg_content = reg_content & ~led_i_mask;

    uint16_t current_bit = 0;
    switch(current)
    {
        case 50:  current_bit = 0; break;
        case 75:  current_bit = 1; break;
        case 100: current_bit = 2; break;
        case 120: current_bit = 3; break;
        case 140: current_bit = 4; break;
        case 160: current_bit = 5; break;
        case 180: current_bit = 6; break;
        case 200: current_bit = 7; break;
        default:  current_bit = 0; break;
    }

	// Set new current
    reg_content = reg_content | (uint16_t)(current_bit << 8);

    retval = register_write_word(VCNL403X_PS_CONF_3, reg_content);
    if (retval != 0)
    {
        return -3;
    }

    return 0;
}

/**
 * @param [in] status 0: off, 1: on
 * GESTURE_MODE is bit 6 in PS_CONF2 
 */
int vcnl403x_com_set_gesture_mode_on_off(uint8_t status)
{
    // Bit 14 im 16-bit Wort (entspricht Bit 6 in PS_CONF2)
    const uint16_t gesture_mode_mask = (uint16_t)(0x01u << 14);
    uint16_t reg_content = 0;
    
    // WICHTIG: Adresse 0x03 lesen (enthält CONF1 und CONF2)
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_1, &reg_content);
    if (retval != 0) return -1;

    if (status) reg_content |= gesture_mode_mask;
    else        reg_content &= ~gesture_mode_mask;

    return register_write_word(VCNL403X_PS_CONF_1, reg_content);
}

/**
 * @param [in] status 0: off, 1: on
 * GESTURE_INT_EN is bit 7 in PS_CONF2 
 */
int vcnl403x_com_set_gesture_interrupt_on_off(uint8_t status)
{
    const uint16_t gesture_int_mask = (uint16_t)(0x1u << 15);

    uint16_t reg_content = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_1, &reg_content);
    if (retval != 0)
    {
        return -1;
    }

    reg_content = reg_content & ~gesture_int_mask;

    if (status == 1)
        reg_content = reg_content | gesture_int_mask;

    retval = register_write_word(VCNL403X_PS_CONF_1, reg_content);
    if (retval != 0)
    {
        return -2;
    }

    return 0;
}

/**
 * @brief Trigger one gesture/proximity sequence
 * PS_TRIG is bit 2 in PS_CONF3 
 */
int vcnl403x_com_trigger_gesture_sequence(void)
{
    const uint16_t ps_trig_mask = (uint16_t)(0x1u << 2);

    uint16_t reg_content = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_PS_CONF_3, &reg_content);
    if (retval != 0)
    {
        return -1;
    }

    // Write '1' to PS_TRIG, device auto clears it 
    reg_content = reg_content | ps_trig_mask;

    retval = register_write_word(VCNL403X_PS_CONF_3, reg_content);
    if (retval != 0)
    {
        return -2;
    }

    return 0;
}


/**
 * @brief Read INT_Flag (command 0x0D high byte).
 * Note: flags are cleared by reading INT_Flag.
 */
int vcnl403x_com_read_int_flag(uint8_t *int_flag)
{
    if (int_flag == 0)
    {
        return -1;
    }

    uint16_t word = 0;
    int retval = vcnl403x_com_register_read_word(VCNL403X_INT_FLAG, &word);
    if (retval != 0)
    {
        return -2;
    }

    *int_flag = (uint8_t)((word >> 8) & 0xFF);
    return 0;
}

int vcnl403x_com_read_ps1(uint16_t *data)
{
    return vcnl403x_com_register_read_word(VCNL403X_PS1_DATA, data);
}

int vcnl403x_com_read_ps2(uint16_t *data)
{
    return vcnl403x_com_register_read_word(VCNL403X_PS2_DATA, data);
}

int vcnl403x_com_read_ps3(uint16_t *data)
{
    return vcnl403x_com_register_read_word(VCNL403X_PS3_DATA, data);
}