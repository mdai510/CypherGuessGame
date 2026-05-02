/**
 * @file ice01.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(ICE01)
#include "drivers.h"

char APP_DESCRIPTION[] = "ECE353: ICE 01 - Memory Mapped IO - GPIO";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

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
    console_init();
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    //Initialize pins as inputs for buttons
    printf("Initializing GPIO for Buttons...\n\r");
    if(buttons_init_gpio() != CY_RSLT_SUCCESS)
    {
        printf("ERROR: Initializing Buttons\n\r");
        CY_ASSERT(0);
    }

    //initialize pins as outputs for RED, BLUE, and GREEN LEDs
    printf("Initializing GPIO for RGB LEDs...\n\r");
    if(leds_init_gpio() != CY_RSLT_SUCCESS)
    {
        printf("ERROR: Initializing LEDs\n\r");
        CY_ASSERT(0);
    }
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

    while(1)
    {
        button_state_t btn1_state = buttons_get_state(BUTTON_SW1);
        button_state_t btn2_state = buttons_get_state(BUTTON_SW2);
        button_state_t btn3_state = buttons_get_state(BUTTON_SW3);
        if(btn1_state == BUTTON_STATE_FALLING_EDGE) {
            printf("Button 1 Pressed\n\r");
            leds_set_state(LED_RED, LED_STATE_ON);
        }
        else if(btn1_state == BUTTON_STATE_RISING_EDGE) {
            printf("Button 1 Released\n\r");
            leds_set_state(LED_RED, LED_STATE_OFF);
        }
        if(btn2_state == BUTTON_STATE_FALLING_EDGE) {
            printf("Button 2 Pressed\n\r");
            leds_set_state(LED_GREEN, LED_STATE_ON);
        }
        else if(btn2_state == BUTTON_STATE_RISING_EDGE) {
            printf("Button 2 Released\n\r");
            leds_set_state(LED_GREEN, LED_STATE_OFF);
        }
        if(btn3_state == BUTTON_STATE_FALLING_EDGE) {
            printf("Button 3 Pressed\n\r");
            leds_set_state(LED_BLUE, LED_STATE_ON);
        }
        else if(btn3_state == BUTTON_STATE_RISING_EDGE) {
            printf("Button 3 Released\n\r");
            leds_set_state(LED_BLUE, LED_STATE_OFF);
        }

        /* Sleep for 100mS */
        cyhal_system_delay_ms(100);
    }
}
#endif
