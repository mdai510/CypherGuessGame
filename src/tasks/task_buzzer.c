#include "main.h"

#ifdef FREERTOS

#include "task_buzzer.h"

/**
 * @brief 
 * Task used to control the buzzer based on button events.
 * 
 * SW1 -- Turn buzzer on
 * SW2 -- Turn buzzer off
 *
 * @param arg 
 * Unused parameter
 */
void task_buzzer(void *arg)
{
    EventBits_t event_bits;

    (void)arg; // Unused parameter
    while (1)
    {
        event_bits = xEventGroupWaitBits(RTOS_Events, SW1_PRESSED | SW2_PRESSED | SW3_PRESSED, pdTRUE, pdFALSE, portMAX_DELAY);
        if(event_bits & SW1_PRESSED){
            buzzer_on();
        }
        else if(event_bits & SW2_PRESSED){
            buzzer_off();
        }
    }
}
#endif