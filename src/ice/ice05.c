/**
 * @file ex03.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-06-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(ICE05)
#include "drivers.h"
#include "task_buttons.h" 

char APP_DESCRIPTION[] = "ECE353: ICE 05 - FreeRTOS Event Groups";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/
// ADD CODE for Event Group Bit Definitions

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
// ADD CODE for Event Group Handle
EventGroupHandle_t ECE353_RTOS_Events;

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/
void task_buzzer();

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
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    /* ADD CODE Initialize the buttons */
    rslt = buttons_init_gpio();
    if(rslt != CY_RSLT_SUCCESS){
        printf("Failed to initialize buttons GPIO\n");
        CY_ASSERT(0);
    }

    /* ADD CODE Initialize the buzzer */
    rslt = buzzer_init(0.5, 2000);
    if(rslt != CY_RSLT_SUCCESS){
        printf("Failed to initialize buzzer\n");
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
    /* ADD CODE Create the event group */
    ECE353_RTOS_Events = xEventGroupCreate();
    if(ECE353_RTOS_Events == NULL){
        printf("Failed to create event group\n");
    }

    /* ADD CODE Register the tasks with FreeRTOS*/
    task_buttons_init();
    xTaskCreate(task_buzzer, "Task Buzzer", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    
    /* ADD CODE Start the scheduler*/
    vTaskStartScheduler();

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}
#endif