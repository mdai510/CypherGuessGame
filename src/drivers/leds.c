#include "leds.h"

#define REG_OUT_LED_RG (*(volatile uint32_t *)(0x40310480)) // Address for P9 Output Register
#define REG_OUT_LED_B (*(volatile uint32_t *)(0x40310400)) // Address for P8 Output Register

static const int LED_masks[3] = {
    MASK_LED_RED,
    MASK_LED_BLUE,
    MASK_LED_GREEN
};

cy_rslt_t leds_init_gpio(void){
    cy_rslt_t result;
    
    result = cyhal_gpio_init(PIN_LED_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    if(result != CY_RSLT_SUCCESS) return result;
    
    result = cyhal_gpio_init(PIN_LED_BLUE, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    if (result != CY_RSLT_SUCCESS) return result;
    
    result = cyhal_gpio_init(PIN_LED_GREEN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    if (result != CY_RSLT_SUCCESS) return result;
    
    return CY_RSLT_SUCCESS;
}

void leds_set_state(led_t led, led_state_t state){
    if(led == LED_RED || led == LED_GREEN){
        if(state) REG_OUT_LED_RG |= LED_masks[led];
        else REG_OUT_LED_RG &= ~LED_masks[led];
    }
    else{
        if(state) REG_OUT_LED_B |= LED_masks[led];
        else REG_OUT_LED_B &= ~LED_masks[led];
    }
}

cy_rslt_t leds_init_pwm(cyhal_pwm_t *pwm_obj_red, cyhal_pwm_t *pwm_obj_green, cyhal_pwm_t *pwm_obj_blue){
    cy_rslt_t rslt;

    rslt = cyhal_pwm_init(pwm_obj_red, PIN_LED_RED, NULL);
    if (rslt != CY_RSLT_SUCCESS) return rslt;
    rslt = cyhal_pwm_set_duty_cycle(pwm_obj_red, 50, 10000);
    if (rslt != CY_RSLT_SUCCESS) return rslt;
    rslt = cyhal_pwm_start(pwm_obj_red);
    if (rslt != CY_RSLT_SUCCESS) return rslt;
    
    rslt = cyhal_pwm_init(pwm_obj_green, PIN_LED_GREEN, NULL);
    if (rslt != CY_RSLT_SUCCESS) return rslt;
    rslt = cyhal_pwm_set_duty_cycle(pwm_obj_green, 50, 10000);
    if (rslt != CY_RSLT_SUCCESS) return rslt;
    rslt = cyhal_pwm_start(pwm_obj_green);
    if (rslt != CY_RSLT_SUCCESS) return rslt;
    
    rslt = cyhal_pwm_init(pwm_obj_blue, PIN_LED_BLUE, NULL);
    if (rslt != CY_RSLT_SUCCESS) return rslt;
    rslt = cyhal_pwm_set_duty_cycle(pwm_obj_blue, 50, 10000);
    if (rslt != CY_RSLT_SUCCESS) return rslt;
    rslt = cyhal_pwm_start(pwm_obj_blue);
    if (rslt != CY_RSLT_SUCCESS) return rslt;

    return CY_RSLT_SUCCESS;
}