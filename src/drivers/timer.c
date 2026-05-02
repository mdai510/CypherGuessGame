#include "timer.h"
#include "cyhal_timer.h"
#include "cyhal_timer_impl.h"
#include <complex.h>

cy_rslt_t timer_init(cyhal_timer_t *timer_obj, cyhal_timer_cfg_t *timer_cfg, uint32_t ticks, void *Handler)
{
    cy_rslt_t rslt;

    timer_cfg->period = ticks;  //set timer period
    timer_cfg->direction = CYHAL_TIMER_DIR_UP;  //set timer to count up
    timer_cfg->is_compare = false; //disable compare mode
    timer_cfg->is_continuous = true; //enable continuous mode
    timer_cfg->value = 0;   //initialize timer value to 0

    rslt = cyhal_timer_init(timer_obj, NC, NULL); //initialize timer
    if (rslt != CY_RSLT_SUCCESS){
        return rslt; // Return if initialization failed 
    }
    rslt = cyhal_timer_configure(timer_obj, timer_cfg); //configure timer
    if (rslt != CY_RSLT_SUCCESS){
        return rslt; // Return if configuration failed 
    }
    rslt = cyhal_timer_set_frequency(timer_obj, 100000000); //set timer frequency to 100MHz
    if (rslt != CY_RSLT_SUCCESS){
        return rslt; // Return if setting frequency failed 
    }
    cyhal_timer_register_callback(timer_obj, Handler, NULL); //register callback function
    cyhal_timer_enable_event(timer_obj, CYHAL_TIMER_IRQ_TERMINAL_COUNT, 3, true); //enable terminal count interrupt
    rslt = cyhal_timer_start(timer_obj); //start timer
    if(rslt != CY_RSLT_SUCCESS){
        return rslt; // Return if starting timer failed 
    }

    return rslt; // Return the result of the initialization
}