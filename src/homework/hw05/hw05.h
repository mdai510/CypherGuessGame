/**
 * @file hw05.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2026-03-23
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef __HW05_H__
#define __HW05_H__

#include "main.h"

#if defined(HW05)

#include "drivers.h"
#include "rtos_events.h"
#include "task_buttons.h"
#include "task_cap_touch.h"
#include "task_console.h"
#include "task_eeprom.h"
#include "task_ipc.h"
#include "task_lcd.h"
#include "task_light_sensor.h"


#define TASK_SYSTEM_CONTROL_STACK_SIZE (configMINIMAL_STACK_SIZE * 5)
#define TASK_SYSTEM_CONTROL_PRIORITY (tskIDLE_PRIORITY + 1U)

#define LIGHT_THRESHOLD 150
#define RESET_SCORE 0xFF
#define INVALID_SELECTION 12
#define NO_TIMEOUT (TickType_t)0
#define LAST_NUMBER_TILE 8
#define FIRST_NUMBER 0

typedef struct {
  uint16_t x;
  uint16_t y;
} touch_coords_t;

// state machine enum
typedef enum {
  STATE_INIT,
  STATE_ENTER_CYPHER,
  STATE_WAIT_OPP_READY,
  STATE_MY_TURN,
  STATE_WAIT_FEEDBACK,
  STATE_OPP_TURN,
  STATE_GAME_OVER
} game_state_t;

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
void task_hw05_system_control(void *pvParameters);

#endif

#endif /* __HW05_H__ */