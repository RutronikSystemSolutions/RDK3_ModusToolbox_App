/*
 * pca9554_app.c
 *
 *  Created on: Jan 12, 2023
 *      Author: Gintaras
 */

#include "pca9554_app.h"
#include "i2c_mutex.h"

cy_rslt_t pca9554_configure(void)
{
    PCA9554_RET_TYPE result;

    i2c_mutex_take();

    result = pca9554_write_config_pol(PCA9554_DEV_ADDRESS, PCA9554_POL_CONFIG);
    if (result == PCA9554_COM_ERR)
    {
        i2c_mutex_give();
        return 1;
    }

    result = pca9554_write_config_dir(PCA9554_DEV_ADDRESS, PCA9554_DIR_CONFIG);
    if (result == PCA9554_COM_ERR)
    {
        i2c_mutex_give();
        return 1;
    }

    result = pca9554_pins_write(PCA9554_DEV_ADDRESS, PCA9554_OUT_CONFIG);
    if (result == PCA9554_COM_ERR)
    {
        i2c_mutex_give();
        return 1;
    }

    i2c_mutex_give();
    return CY_RSLT_SUCCESS;
}

cy_rslt_t pca9554_dir_set(uint8_t dir)
{
    PCA9554_RET_TYPE result;

    i2c_mutex_take();

    result = pca9554_pins_write(PCA9554_DEV_ADDRESS, dir);
    if (result == PCA9554_COM_ERR)
    {
        i2c_mutex_give();
        return 1;
    }

    i2c_mutex_give();
    return CY_RSLT_SUCCESS;
}