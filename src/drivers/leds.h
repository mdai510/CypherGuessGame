#ifndef __LEDS_H__
#define __LEDS_H__

#include <stdio.h>
#include "cybsp.h"
#include "cyhal_gpio.h"
#include "pins.h"
#include "cyhal_pwm.h"

typedef enum {
    LED_RED = 0,
    LED_BLUE,
    LED_GREEN,  
}led_t;

typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON,
} led_state_t;

// Function to initialize the LEDs
cy_rslt_t leds_init_gpio(void);

// Function to set the state of a specific LED
void leds_set_state(led_t led, led_state_t state);

// Function to initialize PWM for the RGB LEDs
cy_rslt_t leds_init_pwm(cyhal_pwm_t *pwm_obj_red, cyhal_pwm_t *pwm_obj_green, cyhal_pwm_t *pwm_obj_blue);

#endif