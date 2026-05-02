#ifndef __TASK_EEPROM_H__
#define __TASK_EEPROM_H__
#include "main.h"

#if defined(FREERTOS)

#include "devices.h"
#include "drivers.h"
#include "rtos_events.h"


#define HI_SCORE_ADDRESS 0x1000

extern QueueHandle_t Queue_EEPROM_Requests;

/* Functions used to interact with the EEPROM */
bool system_sensors_eeprom_write(QueueHandle_t return_queue, uint16_t address,
                                 uint8_t data);
bool system_sensors_eeprom_read(QueueHandle_t return_queue, uint16_t address,
                                uint8_t *data);

/* Function used to initialize resources for the EEPROM task */
bool task_eeprom_resources_init(SemaphoreHandle_t *spi_semaphore,
                                cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin);

#endif /* FREERTOS */

#endif /* __TASK_EEPROM_H__ */