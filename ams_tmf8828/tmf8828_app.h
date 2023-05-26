/*
 * tmf8828_app.h
 *
 *  Created on: 14 Apr 2023
 *      Author: jorda
 */

#ifndef AMS_TMF8828_TMF8828_APP_H_
#define AMS_TMF8828_TMF8828_APP_H_

#include <stdint.h>

#include "tmf882x.h"
#include "platform_wrapper.h"


#define TMF8828_MODE_3X3		0
#define TMF8828_MODE_8X8		1
#define TMF8828_MODE_INVALID	2

typedef int8_t (*tmf8828_read_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef int8_t (*tmf8828_write_func_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

typedef struct {
    uint32_t result_num;         /* increments with every new bunch of results */
    uint32_t temperature;        /* temperature */
    uint32_t ambient_light;      /* ambient light level */
    uint32_t photon_count;       /* photon count */
    uint32_t ref_photon_count;   /* reference photon count */
    uint32_t sys_ticks;          /* system ticks */
    uint32_t valid_results;      /* number of valid results */
    uint32_t num_results;        /* number of results */
    struct tmf882x_meas_result results[TMF882X_MAX_MEAS_RESULTS];
} tmf8828_results_t;

void tmf8828_app_init(tmf8828_read_func_t read, tmf8828_write_func_t write);

/**
 * @brief Check if the AMS TOF board is available or not
 *
 * @retval 0 Board is not available
 * @retval 1 Board is available
 */
int tmf8828_app_is_board_available();

/**
 * @brief Check if the application is in mode 8x8
 *
 * @retval 0 Not mode 8x8 but mode 3x3 (9 values)
 * @retval 1 Mode 8x8 (64 values)
 */
uint16_t tmf8828_app_is_mode_8x8();

int tmf8828_app_init_measurement();

/**
 * @brief Request a new mode
 * The change will be performed by the next call to tmf8828_app_do (asynchronous)
 *
 * @param [in] mode TMF8828_MODE_3X3 (0) or TMF8828_MODE_8X8 (1)
 */
void tmf8828_app_request_new_mode(uint8_t mode);

int tmf8828_app_do();

void tmpf8828_on_new_result(struct platform_ctx *ctx, struct tmf882x_msg_meas_results *result_msg);

tmf8828_results_t* tmpf8828_get_last_results();

uint16_t* tmpf8828_get_last_8x8_results();

#endif /* AMS_TMF8828_TMF8828_APP_H_ */
