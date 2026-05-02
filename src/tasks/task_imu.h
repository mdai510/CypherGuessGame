#ifndef __TASK_IMU_H__
#define __TASK_IMU_H__

#include "main.h"
#if defined(FREERTOS)
#include "cyhal_spi.h"
#include "devices.h"
#include "drivers.h"


extern QueueHandle_t Queue_IMU_Request;

bool system_sensors_get_imu(QueueHandle_t return_queue, int16_t imu_data[3]);

/* Function used to initialize resources for the IMU task */
bool task_imu_resources_init(void *spi_semaphore, cyhal_spi_t *spi_obj,
                             cyhal_gpio_t cs_pin);

#endif /* FREERTOS */

#endif /* __TASK_IMU_H__ */