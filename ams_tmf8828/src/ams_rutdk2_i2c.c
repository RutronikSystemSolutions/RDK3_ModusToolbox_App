/*
 * ams_rutdk2_i2c.h
 *
 *  Created on: April 2022
 *      Author: A.Heder
 */

#include "ams_rutdk2_i2c.h"
#include <stdlib.h>


#include "hal/hal_i2c.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

int32_t ams_i2c_write_block(uint8_t slave_addr, uint8_t reg, const uint8_t * buf, uint32_t len)
{
	uint8_t* tosend = malloc(len + 1);
	tosend[0] = reg;
	for(int i= 0; i < len; ++i)
		tosend[i + 1] = buf[i];

	int8_t retval = hal_i2c_write(slave_addr, tosend, (uint16_t)(len + 1));

	free(tosend);
	return retval;
}

int32_t ams_i2c_read_block(uint8_t slave_addr, uint8_t reg, uint8_t * buf, uint32_t len)
{
	// Write the register
	if (hal_i2c_write(slave_addr, &reg, sizeof(reg)) != 0) return -1;
	return hal_i2c_read(slave_addr, buf, (uint16_t) len);
}

