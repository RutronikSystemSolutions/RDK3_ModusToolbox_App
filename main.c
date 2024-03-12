/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the RDK3 Throughput Measurement
*              Server Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*  Created on: 2022-12-21
*  Company: Rutronik Elektronische Bauelemente GmbH
*  Address: Jonavos g. 30, Kaunas 44262, Lithuania
*  Author: GDR
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"


#include "hal/hal_i2c.h"
#include "hal/hal_sleep.h"
#include "sht4x/sht4x.h"
#include "bmp581/bmp581.h"

#include "host_main.h"
#include "notification_fabric.h"
#include "rutronik_application.h"

static cyhal_timer_t sensor_timer;
static uint8_t timer_interrupt = 0;

static void sensor_timer_isr(void *callback_arg, cyhal_timer_event_t event)
{
	(void) callback_arg;
	(void) event;

	timer_interrupt = 1;
}

static cy_rslt_t sensor_timer_init(void)
{
	const uint32_t timer_frequency_hz = 10000;
	const uint32_t isr_frequency_hz = 100;

	const uint8_t priority = 6;
	cyhal_timer_cfg_t configuration;
	cy_rslt_t result;

	configuration.compare_value = 0;
	configuration.period = (timer_frequency_hz / isr_frequency_hz);
	configuration.direction = CYHAL_TIMER_DIR_UP;
	configuration.is_compare = false;
	configuration.is_continuous = true;
	configuration.value = 0;

	result = cyhal_timer_init(&sensor_timer, NC, NULL);
	if (result != CY_RSLT_SUCCESS) return result;

	result = cyhal_timer_configure(&sensor_timer, &configuration);
	if (result != CY_RSLT_SUCCESS) return result;

	result = cyhal_timer_set_frequency(&sensor_timer, timer_frequency_hz);
	if (result != CY_RSLT_SUCCESS) return result;

	cyhal_timer_register_callback(&sensor_timer, sensor_timer_isr, NULL);

	cyhal_timer_enable_event(&sensor_timer, CYHAL_TIMER_IRQ_TERMINAL_COUNT, priority, true);

	result = cyhal_timer_start(&sensor_timer);
	return result;
}

int main(void)
{
    cy_rslt_t result;
    int8_t res = 0;
    rutronik_application_t rutronik_app;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    __enable_irq(); /* Enable global interrupts. */

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(KITPROG_TX, KITPROG_RX, CY_RETARGET_IO_BAUDRATE);
    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    printf("------------------------- \r\n");
    printf("Starting RDK3 application \r\n");
    printf("Enabling control over Smartphone App \r\n");
    printf("------------------------- \r\n");

    /* Initialize the User LEDs */
    result = cyhal_gpio_init(LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    result |= cyhal_gpio_init(LED2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    result |= cyhal_gpio_init(LED3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {CY_ASSERT(0);}
    cyhal_gpio_write((cyhal_gpio_t)LED1, CYBSP_LED_STATE_ON);
    cyhal_gpio_write((cyhal_gpio_t)LED2, CYBSP_LED_STATE_OFF);
    cyhal_gpio_write((cyhal_gpio_t)LED3, CYBSP_LED_STATE_OFF);

    // Initialize I2C master
    res = hal_i2c_init();
    if (res != 0)
    {
    	CY_ASSERT(0);
    }

    // Initialise timer
    res = sensor_timer_init();
    printf("sensor timer init retval: %d \r\n", res);


    /*Charger control*/
    result = cyhal_gpio_init(CHR_DIS, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    if (result != CY_RSLT_SUCCESS)
    {CY_ASSERT(0);}
    cyhal_gpio_write((cyhal_gpio_t)CHR_DIS, false); /*Charger ON*/

    /* Configure USER_BTN */
    result = cyhal_gpio_init(USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {CY_ASSERT(0);}


    rutronik_application_init(&rutronik_app);

    Ble_Init(&rutronik_app);

    uint32_t counter = 0;

    for (;;)
    {
    	if (timer_interrupt)
    	{
    		timer_interrupt = 0;

    		rutronik_application_do(&rutronik_app);

    		if ((counter % 10) == 0)
			{
    			cyhal_gpio_toggle(LED1);
			}
    		counter++;
    	}



    	host_main_do();
    }
}

/* [] END OF FILE */
