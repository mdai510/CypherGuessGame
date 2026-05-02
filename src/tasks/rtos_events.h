/**
 * @file rtos_events.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-13
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __RTOS_EVENTS_H__
#define __RTOS_EVENTS_H__
#include "main.h"

#ifdef ECE353_FREERTOS

/*******************************************************************************
 * Event Group for system events.
 ******************************************************************************/
extern EventGroupHandle_t ECE353_RTOS_Events;

/*******************************************************************************
 * Macros used to define the system events
 ******************************************************************************/
#define SW1_PRESSED (1 << 0)
#define SW2_PRESSED (1 << 1)
#define SW3_PRESSED (1 << 2)
#define IPC_ACK_RECEIVED (1 << 3)
#define IPC_DISCOVERY_RECEIVED (1 << 4)
#define LIGHT_SENSOR_UPDATED (1 << 5)
#define CAP_TOUCH_ENABLED (1 << 6)
#define IPC_RDY_RECEIVED (1 << 7)
#define IPC_GUESS_RECEIVED (1 << 8)
#define IPC_GUESS_RESPONSE_RECEIVED (1 << 9)
#define IPC_RESET_RECEIVED (1 << 10)
#define JOYSTICK_MOVED (1 << 11)

#endif // ECE353_FREERTOS

#endif // __RTOS_EVENTS_H__