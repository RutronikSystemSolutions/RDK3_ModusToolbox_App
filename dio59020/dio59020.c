/*
 * DIO59020.c
 *
 *  Created on: Jun 10, 2021
 *      Authors: GDR, IUS
 */

#include "dio59020.h"

#include "stdlib.h"


static dio59020_read_func_t i2c_read_bytes;
static dio59020_write_func_t i2c_write_bytes;

void dio59020_init(dio59020_read_func_t read, dio59020_write_func_t write)
{
	i2c_read_bytes = read;
	i2c_write_bytes = write;
}


/*Check if DIO59020 device is responsive*/
uint8_t dio_online(void)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, IC_INFO_REG, &data, 1);
	if(result == 0)
	{
		if(data == IC_INFO_D_VALUE)
		{
			return 1;
		}
	}

	return 0;
}


/*Read the MONITOR Register*/
uint32_t dio_monitor_read(dio_monitor_reg_status_t* status)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, MONITOR_REG, &data, 1);

	status->CV_status = 		(_Bool)(data & CV);
	status->VBUS_VALID_status = (_Bool)(data & VBUS_VALID) >> 1;
	status->IBUS_status = 		(_Bool)(data & IBUS) >> 2;
	status->CHG_status = 		(_Bool)(data & CHG) >> 3;
	status->T_100_status = 		(_Bool)(data & T_100) >> 4;
	status->LINCHG_status = 	(_Bool)(data & LINCHG) >> 5;
	status->VBAT_CMP_status = 	(_Bool)(data & VBAT_CMP) >> 6;
	status->ITERM_status = 		(_Bool)(data & ITERM) >> 7;

	return result;
}

uint32_t dio_monitor_read_raw(uint8_t* register_content)
{
	uint32_t result;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, MONITOR_REG, register_content, 1);

	return result;
}

/*Disables the charger, puts into the hi-impedance mode*/
uint32_t dio_charger_disable(void)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	if(result == 0)
	{
		data |= CTRL1_CE;
		data |= CTRL1_HZ_MODE;

		result = i2c_write_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	}

	return result;
}

/*Enables the charger, exits from the hi-impedance mode*/
uint32_t dio_charger_enable(void)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	if(result == 0)
	{
		data &= ~CTRL1_CE;
		data &= ~CTRL1_HZ_MODE;

		result = i2c_write_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	}

	return result;
}

/*Sets the input current limit*/
uint32_t dio_set_current_lim(input_curr_lim_t curr_limit)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	if(result == 0)
	{
		data &= ~CTRL1_INLIM;
		data |= (curr_limit << 6);

		result = i2c_write_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	}

	return result;
}

/*Enables the Boost Mode*/
uint32_t dio_booster_enable(void)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	if(result == 0)
	{
		data |= CTRL1_OPA_MODE;

		result = i2c_write_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	}

	return result;
}

/*Disables the Boost Mode*/
uint32_t dio_booster_disable(void)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	if(result == 0)
	{
		data &= ~CTRL1_OPA_MODE;

		result = i2c_write_bytes(DIO_SLAVE_ADDR, CONTROL1, &data, 1);
	}

	return result;
}

/*Disables STAT pin*/
uint32_t dio_stat_pin_disable(void)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL0, &data, 1);
	if(result == 0)
	{
		data &= ~CTRL0_EN_STAT;

		result = i2c_write_bytes(DIO_SLAVE_ADDR, CONTROL0, &data, 1);
	}

	return result;
}

/*Enables STAT pin*/
uint32_t dio_stat_pin_enable(void)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL0, &data, 1);
	if(result == 0)
	{
		data |= CTRL0_EN_STAT;

		result = i2c_write_bytes(DIO_SLAVE_ADDR, CONTROL0, &data, 1);
	}

	return result;
}

/*Reads the charger status*/
uint32_t dio_get_status(charge_stat_t* status)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL0, &data, 1);
	if(result == 0)
	{
		*status = (data & CTRL0_STAT) >> 4;
	}

	return result;
}

/*Reads the voltage boost status*/
uint32_t dio_get_boost(_Bool* boost_mode)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL0, &data, 1);
	if(result == 0)
	{
		*boost_mode = (_Bool)((data & CTRL0_BOOST) >> 3);
	}

	return result;
}

/*Reads the charger faults*/
uint32_t dio_get_fault(chrg_fault_t* charger_fault)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, CONTROL0, &data, 1);
	if(result == 0)
	{
		data &= CTRL0_CHG_FAULT;

		*charger_fault = data;
	}

	return result;
}

/*Configures OTG pin*/
uint32_t dio_otg_pin_config(_Bool otg_pl, _Bool otg_en)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, OREG, &data, 1);
	if(result == 0)
	{
		data &= ~OREG_OTG_EN;
		data &= ~OREG_OTG_PL;
		data |= otg_en;
		data |= (otg_pl << 1);

		result = i2c_write_bytes(DIO_SLAVE_ADDR, OREG, &data, 1);
	}

	return result;
}

/*Sets the maximum battery charge current*/
uint32_t dio_set_batt_current(batt_chrg_curr_t curr_vref)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, IBAT, &data, 1);
	if(result == 0)
	{
		data &= ~IBAT_ICHRG;
		data |= (curr_vref << 4);

		result = i2c_write_bytes(DIO_SLAVE_ADDR, IBAT, &data, 1);
	}

	return result;
}

/*Sets the battery charge termination current*/
uint32_t dio_set_batt_term_curr(batt_term_curr_t curr_vref)
{
	uint32_t result;
	uint8_t data = 0;

	result = i2c_read_bytes(DIO_SLAVE_ADDR, IBAT, &data, 1);
	if(result == 0)
	{
		data &= ~IBAT_ITERM;
		data |= curr_vref;

		result = i2c_write_bytes(DIO_SLAVE_ADDR, IBAT, &data, 1);
	}

	return result;
}
