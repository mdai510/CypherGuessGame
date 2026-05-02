/**
 * @file task_console_rx.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-21
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "main.h"
#include "task_cap_touch.h"
#include "task_eeprom.h"


#ifdef ECE353_FREERTOS
#include "cyhal_uart.h"
#include "devices.h"
#include "drivers.h"
#include "task_console.h"

/**
 * @brief
 * This file contains the implementation of the console receive (Rx) task.
 * The task is responsible for processing incoming console commands and
 * controlling the state of the LEDs accordingly.
 *
 * The task uses a double buffer to process the incoming console commands.
 * The supported commands will be "RED_ON" and "RED_OFF" to control the red LED.
 */

/* ADD CODE */
/* Global Variables */
console_buffer_t console_buffer1;
console_buffer_t console_buffer2;

// Pointers to the above buffers that switch based on which buffer thenISR
// is writing to
console_buffer_t *produce_console_buffer;
console_buffer_t *consume_console_buffer;

TaskHandle_t TaskHandle_Console_Rx;

static QueueHandle_t Queue_Rx_Response;

/**
 * @brief
 * This function is the bottom half task for receiving console input.
 *
 * It waits for a task notification from the ISR indicating that a new
 * command has been received. The task then processes the command and
 * controls the state of the LEDs accordingly.
 *
 * @param param Unused parameter
 */
void task_console_rx(void *param) {
  (void)param; // Unused parameter
  device_request_msg_t request;
  device_response_msg_t response;
  uint16_t x, y;
  char *cmd = NULL;

  while (1) {
    /* ADD CODE */
    // Wait indefinitely for a Task Notification
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Process data in consume_console_buffer
    cmd = consume_console_buffer->data;

    if (!parse_cli_data(cmd, &request))
      continue;

    request.response_queue = Queue_Rx_Response;

    switch (request.device) {
    case DEVICE_EEPROM:
      xQueueSend(Queue_EEPROM_Requests, &request, portMAX_DELAY);
      xQueueReceive(Queue_Rx_Response, &response, portMAX_DELAY);

      switch (request.operation) {
      case DEVICE_OP_READ:
        task_console_printf("EEPROM READ: ADDR=0x%04X, Value=0x%02X\r\n",
                            request.address, response.payload.eeprom);
        break;
      case DEVICE_OP_WRITE:
        task_console_printf("EEPROM WRITE: Addr=0x%04X, Value=0x%02X\r\n",
                            request.address, request.value);
        break;
      }
      break;
    case DEVICE_CAP_TOUCH:
      xQueueSend(Queue_Request_Cap_Touch, &request, portMAX_DELAY);
      xQueueReceive(Queue_Rx_Response, &response, portMAX_DELAY);

      switch (response.status) {
      case DEVICE_OPERATION_STATUS_READ_SUCCESS:
        x = response.payload.cap_touch[0];
        y = response.payload.cap_touch[1];

        task_console_printf("Cap Touch: Sensor 0=%d, Sensor 1=%d\n\r", x, y);
        break;
      case DEVICE_OPERATION_STATUS_READ_FAILURE:
        task_console_printf("Failed to read Capacitive Touch data\n\r");
        break;
      default:
        break;
      }
    default:
      break;
    }
  }
}

/**
 * @brief
 * This function initializes the resources for the console Rx task.
 * @return true if resources were initialized successfully
 * @return false if resource initialization failed
 */
bool task_console_resources_init_rx(void) {
  BaseType_t rslt;

  Queue_Rx_Response = xQueueCreate(1, sizeof(device_response_msg_t));

  console_buffer1.data = (char *)pvPortMalloc(CONSOLE_MAX_MESSAGE_LENGTH);
  console_buffer2.data = (char *)pvPortMalloc(CONSOLE_MAX_MESSAGE_LENGTH);

  if (console_buffer1.data == NULL || console_buffer2.data == NULL)
    return pdFALSE;

  produce_console_buffer = &console_buffer1;
  consume_console_buffer = &console_buffer2;

  produce_console_buffer->index = 0;
  consume_console_buffer->index = 0;

  rslt =
      xTaskCreate(task_console_rx, "Console RX", 2 * configMINIMAL_STACK_SIZE,
                  NULL, tskIDLE_PRIORITY + 1, &TaskHandle_Console_Rx);

  return (rslt == pdPASS); // Resources initialized successfully
}
#endif