/**
 * @file ice02.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(ICE03)
#include "drivers.h"
#include <stdio.h>

char APP_DESCRIPTION[] = "ECE353: ICE 03 - Timer Interrupts/Debounce Buttons";

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
    cy_rslt_t rslt;

    console_init();
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    //initialize leds
    rslt = leds_init_gpio();
    if(rslt != CY_RSLT_SUCCESS){
        printf("Failed to initialize LEDs\n\r");
    }
    rslt = buttons_init_gpio();
    if(rslt != CY_RSLT_SUCCESS){
        printf("Failed to initialize Buttons\n\r");
    }
    rslt = buttons_init_timer();
    if(rslt != CY_RSLT_SUCCESS){
        printf("Failed to initialize Buttons Timer\n\r");
    }
}

typedef enum {
    INIT,
    SW1_DET,
    SW2_DET_1,
    SW2_DET_2,
    SW3_DET
} State;

/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */
void app_main(void)
{
    static State current_state = INIT;
    leds_set_state(LED_RED, LED_STATE_ON);

    while(1)
    {
        if(ECE353_Events.sw1 || ECE353_Events.sw2 || ECE353_Events.sw3){
            switch(current_state){
                case INIT:
                    if(ECE353_Events.sw1){
                        printf("SW1 Pressed, transition from INIT to SW1_DET\n\r");
                        ECE353_Events.sw1 = 0;
                        current_state = SW1_DET;
                        leds_set_state(LED_BLUE, LED_STATE_ON);
                    }
                    else if(ECE353_Events.sw2){
                        printf("SW2 Pressed, stay in INIT\n\r");
                        ECE353_Events.sw2 = 0;
                    }
                    else if(ECE353_Events.sw3){
                        printf("SW3 Pressed, stay in INIT\n\r");
                        ECE353_Events.sw3 = 0;
                    }
                    break;
                case SW1_DET:
                    if(ECE353_Events.sw2){
                        printf("SW2 Pressed, transition from SW1_DET to SW2_DET_1\n\r");
                        ECE353_Events.sw2 = 0;
                        current_state = SW2_DET_1;
                        leds_set_state(LED_RED, LED_STATE_OFF);
                    }
                    else if(ECE353_Events.sw1){
                        printf("SW1 Pressed, transition from SW1_DET to INIT\n\r");
                        ECE353_Events.sw1 = 0;
                        current_state = INIT;
                        leds_set_state(LED_BLUE, LED_STATE_OFF);
                    }
                    else if (ECE353_Events.sw3){
                        printf("SW3 Pressed, transition from SW1_DET to INIT\n\r");
                        ECE353_Events.sw3 = 0;
                        current_state = INIT;
                        leds_set_state(LED_BLUE, LED_STATE_OFF);
                    }
                    break;
                case SW2_DET_1:
                    if(ECE353_Events.sw2){
                        printf("SW2 Pressed, transition from SW2_DET_1 to SW2_DET_2\n\r");
                        ECE353_Events.sw2 = 0;
                        current_state = SW2_DET_2;
                        leds_set_state(LED_GREEN, LED_STATE_ON);
                    }
                    else if(ECE353_Events.sw1){
                        printf("SW1 Pressed, transition from SW2_DET_1 to INIT\n\r");
                        ECE353_Events.sw1 = 0;
                        current_state = INIT;
                        leds_set_state(LED_BLUE, LED_STATE_OFF);
                        leds_set_state(LED_RED, LED_STATE_ON);
                    }
                    else if (ECE353_Events.sw3){
                        printf("SW3 Pressed, transition from SW2_DET_1 to INIT\n\r");
                        ECE353_Events.sw3 = 0;
                        current_state = INIT;
                        leds_set_state(LED_BLUE, LED_STATE_OFF);
                        leds_set_state(LED_RED, LED_STATE_ON);
                    }
                    break;
                case SW2_DET_2:
                    if(ECE353_Events.sw3){
                        printf("SW3 Pressed, transition from SW2_DET_2 to SW3_DET\n\r");
                        ECE353_Events.sw3 = 0;
                        current_state = SW3_DET;
                        leds_set_state(LED_BLUE, LED_STATE_OFF);
                    }
                    else if(ECE353_Events.sw1){
                        printf("SW1 Pressed, transition from SW2_DET_2 to INIT\n\r");
                        ECE353_Events.sw1 = 0;
                        current_state = INIT;
                        leds_set_state(LED_BLUE, LED_STATE_OFF);
                        leds_set_state(LED_RED, LED_STATE_ON);
                        leds_set_state(LED_GREEN, LED_STATE_OFF);
                    }
                    else if (ECE353_Events.sw2){
                        printf("SW2 Pressed, transition from SW2_DET_2 to INIT\n\r");
                        ECE353_Events.sw2 = 0;
                        current_state = INIT;
                        leds_set_state(LED_BLUE, LED_STATE_OFF);
                        leds_set_state(LED_RED, LED_STATE_ON);
                        leds_set_state(LED_GREEN, LED_STATE_OFF);
                    }
                    break;
                case SW3_DET:
                    if(ECE353_Events.sw1){
                        printf("SW1 Pressed, transition from SW3_DET to INIT\n\r");
                        ECE353_Events.sw1 = 0;
                        current_state = INIT;
                        leds_set_state(LED_RED, LED_STATE_ON);
                        leds_set_state(LED_GREEN, LED_STATE_OFF);
                    }
                    else if(ECE353_Events.sw2){
                        printf("SW2 Pressed, transition from SW3_DET to INIT\n\r");
                        ECE353_Events.sw2 = 0;
                        current_state = INIT;
                        leds_set_state(LED_RED, LED_STATE_ON);
                        leds_set_state(LED_GREEN, LED_STATE_OFF);
                    }
                    else if (ECE353_Events.sw3){
                        printf("SW3 Pressed, transition from SW3_DET to INIT\n\r");
                        ECE353_Events.sw3 = 0;
                        current_state = INIT;
                        leds_set_state(LED_RED, LED_STATE_ON);
                        leds_set_state(LED_GREEN, LED_STATE_OFF);
                    }
                    break;
                default:
                    printf("Unknown State!\n");
                    current_state = INIT;
                    break;
            }
        }
    }
}
#endif
