/**
 * @file ex01.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-06-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(EX01)
#include "drivers.h"

char APP_DESCRIPTION[] = "ECE353: Example 01 - Intro to C";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/
#define REG_OUT_LED_GREEN  (*(volatile uint32_t *)(0x40310480)) //Address for P9.2 LED Register

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/


/**
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 */
void app_init_hw(void)
{
    cy_rslt_t rslt_r, rslt_b, rslt_g;
    console_init();
    
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    //Initialize pins as outputs for RED, BLUE, and GREEN LEDs
    printf("Initializing GPIO for RGB LEDs...\n\r");
    rslt_r = cyhal_gpio_init(PIN_LED_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    rslt_b = cyhal_gpio_init(PIN_LED_BLUE, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    rslt_g = cyhal_gpio_init(PIN_LED_GREEN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    if(rslt_r != CY_RSLT_SUCCESS || rslt_b != CY_RSLT_SUCCESS || rslt_g != CY_RSLT_SUCCESS)
    {
        printf("GPIO Init Failed\n\r");
        CY_ASSERT(0);
    }
    printf("Starting main application loop...\n\r");
}

/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */
void app_main(void)
{
    /* Enter Infinite Loop*/
    while (1)
    {
        //Turn Green LED ON
        //cyhal_gpio_write(PIN_LED_GREEN, 1);
        REG_OUT_LED_GREEN |= MASK_LED_GREEN;  // Set P9.2 High

        //Delay for 500ms
        cyhal_system_delay_ms(500);

        //Turn Green LED OFF
        //cyhal_gpio_write(PIN_LED_GREEN, 0);
        REG_OUT_LED_GREEN &= ~MASK_LED_GREEN; // Set P9.2 Low

        //Delay for 500ms
        cyhal_system_delay_ms(500);
    }
}
#endif