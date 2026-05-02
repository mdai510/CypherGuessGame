/**
 * @file task_buttons.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "task_buttons.h"
#include "task_console.h"

 #ifdef ECE353_FREERTOS
 /**
  * @brief 
  * Task used to debounce button presses (SW1, SW2, SW3).  
  * The falling edge of the button press is detected by de-bouncing
  * the button for 30mS. Each button should be sampled every 15mS.
  *
  * When a button press is detected, the corresponding event is set in
  * in the event group ECE353_RTOS_Events.
  *
  * @param arg 
  * Unused parameter
  */
 void task_buttons(void *arg)
 {
    (void)arg; // Unused parameter
    static uint8_t button_counts[3] = {0,0,0};

    while (1)
    {
        // Monitor button SW1
        if ((PORT_BUTTON_SW1->IN & MASK_BUTTON_PIN_SW1) == 0) {
          button_counts[0]++;
          if (button_counts[0] == 2) {
            //printf("SW1 Pressed\n\r");
            xEventGroupSetBits(ECE353_RTOS_Events, SW1_PRESSED);
            //task_console_printf("BUTTON SW1 PRESSED\n\r");
          }
        } else {
          button_counts[0] = 0;
        }

        // Monitor button SW2
        if ((PORT_BUTTON_SW2->IN & MASK_BUTTON_PIN_SW2) == 0) {
          button_counts[1]++;
          if (button_counts[1] == 2) {
            xEventGroupSetBits(ECE353_RTOS_Events, SW2_PRESSED);
            //task_console_printf("BUTTON SW2 PRESSED\n\r");
          }
        } else {
          button_counts[1] = 0;
        }

        // Monitor button SW3
        if ((PORT_BUTTON_SW3->IN & MASK_BUTTON_PIN_SW3) == 0) {
          button_counts[2]++;
          if (button_counts[2] == 2) {
            xEventGroupSetBits(ECE353_RTOS_Events, SW3_PRESSED);
            //task_console_printf("BUTTON SW3 PRESSED\n\r");
          }
        } else {
          button_counts[2] = 0;
        }

        // Debounce delay
        //delay 15mS
        vTaskDelay(pdMS_TO_TICKS(15));
    }
 }

 /* Button Task Initialization */
bool task_buttons_init(void){

    BaseType_t result;

    //initialize button GPIOs
    buttons_init_gpio();

    // Create the button task
    result = xTaskCreate(
        task_buttons, 
        "Button Task", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY + 1, 
        NULL
    );

    if(result != pdPASS)
    {
        return false;
    }

    return true;
}
#endif