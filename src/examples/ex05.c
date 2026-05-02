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

#if defined(EX05)

#include "drivers.h"

char APP_DESCRIPTION[] = "ECE353: Example 05 - FreeRTOS Tasks";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
volatile bool buzzer_enable = false;

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/
void task_button_sw1(void *arg);
void task_button_sw2(void *arg);
void task_buzzer(void *arg);

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
void task_button_sw1(void *arg)
{
    (void)arg; // Unused parameter
    uint32_t button_cnt = 0;
    printf("Task SW1 Created\n\r");
    while(1)
    {
        //check the button
        if((PORT_BUTTON_SW1->IN & MASK_BUTTON_PIN_SW1) == 0){
            button_cnt++;
            if(button_cnt == 2){
                printf("Button SW1 Pressed - Enabled Buzzer\n\r");
                //set global var
                buzzer_enable = true;
            }
        } 
        else {
          button_cnt = 0;
        }

        //delay 15ms
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

void task_button_sw2(void *arg)
{
  (void)arg; // Unused parameter
  uint32_t button_cnt = 0;
  printf("Task SW2 Created\n\r");
  while (1) {
    // check the button
    if ((PORT_BUTTON_SW2->IN & MASK_BUTTON_PIN_SW2) == 0) {
      button_cnt++;
      if (button_cnt == 2) {
        printf("Button SW2 Pressed - Disabled Buzzer\n\r");
        // set global var
        buzzer_enable = false;
      }
    } 
    else {
      button_cnt = 0;
    }

    // delay 15ms
    vTaskDelay(pdMS_TO_TICKS(15));
  }
}

void task_buzzer(void *arg)
{
    (void)arg; // Unused parameter
    printf("Task Buzzer Created\n\r");

    while (1)
    {
        //delay 100ms
        vTaskDelay(pdMS_TO_TICKS(100));

        if(buzzer_enable){
          buzzer_on();
        }
        else{
          buzzer_off();
        }
    }
}

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

    /* Initialize the buttons */
    rslt = buttons_init_gpio();
    if (rslt != CY_RSLT_SUCCESS) {
      printf("Failed to initialize buttons GPIO\n");
      CY_ASSERT(0);
    }
    /* Initialize the buzzer */
    rslt = buzzer_init(0.5, 2000);
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
    /* Register the tasks with FreeRTOS*/
    xTaskCreate(task_button_sw1, "SW1 Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(task_button_sw2, "SW2 Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(task_buzzer, "Buzzer Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

    /* Start the scheduler*/
    vTaskStartScheduler();

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}
#endif