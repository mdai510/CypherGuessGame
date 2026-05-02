#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <stdio.h>
#include "cybsp.h"
#include "cyhal_gpio.h"
#include "cyhal_timer.h" 
#include "pins.h"
#include "events.h"
#include "timer.h"

typedef enum {
    BUTTON_SW1 = 0,
    BUTTON_SW2,
    BUTTON_SW3,
} button_t;

typedef enum {
    BUTTON_STATE_FALLING_EDGE = 0,
    BUTTON_STATE_LOW,
    BUTTON_STATE_HIGH,
    BUTTON_STATE_RISING_EDGE,
} button_state_t;

// Function to initialize the buttons
cy_rslt_t buttons_init_gpio(void);

// Function to initialize the timer for button debouncing
cy_rslt_t buttons_init_timer(void);

// Function to read the state of a specific button
button_state_t buttons_get_state(button_t button);



#endif