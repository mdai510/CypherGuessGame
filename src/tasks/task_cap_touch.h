#ifndef __TASK_CAP_TOUCH_H__
#define __TASK_CAP_TOUCH_H__

#include "main.h"

#if defined(FREERTOS)

#include "cap_touch.h"
#include "drivers.h"


#include "cyhal_spi.h"
#include "devices.h"
#include "task_console.h"


#define TASK_CAP_TOUCH_STACK_SIZE (configMINIMAL_STACK_SIZE * 5)
#define TASK_CAP_TOUCH_PRIORITY (tskIDLE_PRIORITY + 1U)

extern QueueHandle_t Queue_Request_Cap_Touch;

/* Function used to initialize resources for the IMU task */
bool task_cap_touch_resources_init(QueueHandle_t queue_request,
                                   SemaphoreHandle_t *i2c_semaphore,
                                   cyhal_i2c_t *i2c_obj);

#endif
#endif