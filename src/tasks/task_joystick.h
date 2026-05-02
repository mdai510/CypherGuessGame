#ifndef __TASK_JOYSTICK_H__
#define __TASK_JOYSTICK_H__
#include "devices.h"
#include "drivers.h"
#include "main.h"
#include "rtos_events.h"
#include <complex.h>


#ifdef FREERTOS
extern QueueHandle_t xQueue_Request_Joystick;

void task_joystick(void *arg);

bool task_joystick_init(void);

#endif /* FREERTOS */
#endif /* __TASK_JOYSTICK_H__ */