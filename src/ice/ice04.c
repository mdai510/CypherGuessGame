/**
 * @file ice04.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(ICE04)
#include "drivers.h"
#include <stdio.h>

char APP_DESCRIPTION[] = "ECE353: ICE 04 - PWM Buzzer";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
cyhal_pwm_t pwm_obj_red;
cyhal_pwm_t pwm_obj_green;
cyhal_pwm_t pwm_obj_blue;

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
    cy_rslt_t rslt;

    console_init();
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");
    
    //initialize buttons
    rslt = buttons_init_gpio();
    if(rslt != CY_RSLT_SUCCESS){
        printf("Failed to initialize Buttons\n\r");
        CY_ASSERT(0);
    }

    //initialize button timer
    rslt = buttons_init_timer();
    if(rslt != CY_RSLT_SUCCESS){
        printf("Failed to initialize Buttons Timer\n\r");
        CY_ASSERT(0);
    }

    //initialize led PWMs
    rslt = leds_init_pwm(&pwm_obj_red, &pwm_obj_green, &pwm_obj_blue);
    if(rslt != CY_RSLT_SUCCESS)
    {
        printf("Failed to initialize PWM\n\r");
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
    static uint8_t duty_cycle_red = 0;
    static uint8_t duty_cycle_green = 0;
    static uint8_t duty_cycle_blue = 0;

    while(1)
    {
        if(ECE353_Events.sw1){
            ECE353_Events.sw1 = 0;

            duty_cycle_red += 10;
            if(duty_cycle_red > 100){
                duty_cycle_red = 0;
            }
            cyhal_pwm_set_duty_cycle(&pwm_obj_red, duty_cycle_red, 10000);
            printf("Red LED Duty Cycle: %d%%\n\r", duty_cycle_red);
        }
        if(ECE353_Events.sw2){
            ECE353_Events.sw2 = 0;

            duty_cycle_green += 10;
            if(duty_cycle_green > 100){
                duty_cycle_green = 0;
            }
            cyhal_pwm_set_duty_cycle(&pwm_obj_green, duty_cycle_green, 10000);
            printf("Green LED Duty Cycle: %d%%\n\r", duty_cycle_green);
        }
        if(ECE353_Events.sw3){
            ECE353_Events.sw3 = 0;

            duty_cycle_blue += 10;
            if(duty_cycle_blue > 100){
                duty_cycle_blue = 0;
            }
            cyhal_pwm_set_duty_cycle(&pwm_obj_blue, duty_cycle_blue, 10000);
            printf("Blue LED Duty Cycle: %d%%\n\r", duty_cycle_blue);
        }
    }
}
#endif
