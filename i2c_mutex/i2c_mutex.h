/*
 * i2c_mutex.h
 *
 *  Created on: 15.07.2026
 *      Author: DC1
 */

#ifndef I2C_MUTEX_I2C_MUTEX_H_
#define I2C_MUTEX_I2C_MUTEX_H_

#include "cy_pdl.h"
#include <stdbool.h>

void i2c_mutex_init(void);
void i2c_mutex_take(void);
bool i2c_mutex_try_take(void);
void i2c_mutex_give(void);


#endif /* I2C_MUTEX_I2C_MUTEX_H_ */
