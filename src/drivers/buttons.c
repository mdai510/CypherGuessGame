#include "buttons.h"
#include "cyhal_timer.h"
#include "events.h"
#include "pins.h"

#define REG_IN_BUTTON (*(volatile uint32_t *)(0x40310310)) // Address for P6 Input Register

static const int mask_pins[3] = {
  MASK_BUTTON_PIN_SW1,
  MASK_BUTTON_PIN_SW2,
  MASK_BUTTON_PIN_SW3
};

cy_rslt_t buttons_init_gpio(void){
  cy_rslt_t result;

  result = cyhal_gpio_init(PIN_BUTTON_SW1, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
  if(result != CY_RSLT_SUCCESS) return result;

  result = cyhal_gpio_init(PIN_BUTTON_SW2, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
  if (result != CY_RSLT_SUCCESS) return result;

  result = cyhal_gpio_init(PIN_BUTTON_SW3, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
  if (result != CY_RSLT_SUCCESS) return result;

  //printf("button gpio initialized\n\r");

  return CY_RSLT_SUCCESS;
}

static cyhal_timer_t button_timer;
static cyhal_timer_cfg_t button_timer_cfg;

// Timer interrupt handler for button debouncing for 25mS
static void button_timer_handler(void* arg, cyhal_timer_event_t event){
  static uint8_t button_counts[3] = {0, 0, 0};

  uint8_t sw1 = PORT_BUTTON_SW1->IN & MASK_BUTTON_PIN_SW1;
  uint8_t sw2 = PORT_BUTTON_SW2->IN & MASK_BUTTON_PIN_SW2;
  uint8_t sw3 = PORT_BUTTON_SW3->IN & MASK_BUTTON_PIN_SW3;

  if(sw1 == 0){
    button_counts[0]++;
    if(button_counts[0] == 5){
      Events.sw1 = 1;
    }
  }
  else{
    button_counts[0] = 0;
  }

  if(sw2 == 0){
    button_counts[1]++;
    if(button_counts[1] == 5){
      Events.sw2 = 1;
    }
  }
  else{
    button_counts[1] = 0;
  }

  if(sw3 == 0){
    button_counts[2]++;
    if(button_counts[2] == 5){
      Events.sw3 = 1;
    }
  }
  else{
    button_counts[2] = 0;
  }
}

cy_rslt_t buttons_init_timer(void){
  //initialize the timer for button debouncing for a period of 5mS
  //to get 5mS with a 100MHz timer, we need 500,000 ticks
  return timer_init(&button_timer, &button_timer_cfg, 500000, button_timer_handler);
}

button_state_t buttons_get_state(button_t button) {
  // Assume buttons are not pressed at startup (active low)
  //static variable to hold previous levels of all buttons
  static bool prev_lvls[3] = {1, 1, 1}; 
  bool current_lvl = REG_IN_BUTTON & mask_pins[button];

  button_state_t cur_state;
  if(!prev_lvls[button] && !current_lvl) cur_state = BUTTON_STATE_LOW;
  else if(prev_lvls[button] && current_lvl) cur_state = BUTTON_STATE_HIGH;
  else if(prev_lvls[button] && !current_lvl) cur_state = BUTTON_STATE_FALLING_EDGE;
  else cur_state = BUTTON_STATE_RISING_EDGE;

  prev_lvls[button] = current_lvl;
  return cur_state;
}