#include "task_cap_touch.h"

#if defined(FREERTOS)
#include "rtos_events.h"

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
QueueHandle_t Queue_Request_Cap_Touch = NULL;
static SemaphoreHandle_t *I2C_Semaphore = NULL;
static cyhal_i2c_t *I2C_Obj = NULL;

/**
 * @brief Reads the status register to get the number of touch points, then
 * reads the x and y registers if there are 1-2 touch points
 *
 * @param touch_points the number of touch points
 * @param x x coordinate
 * @param y y coordinate
 * @return cy_rslt_t if there are any errors reading from the I2C bus
 */
static cy_rslt_t read_coords(uint8_t *touch_points, uint16_t *x, uint16_t *y) {
  cy_rslt_t rslt = CY_RSLT_SUCCESS;
  uint8_t rx_buffer[5] = {0};
  uint8_t x_MSB, x_LSB;
  uint8_t y_MSB, y_LSB;

  // Better to read all the registers at once
  rslt = cyhal_i2c_master_mem_read(I2C_Obj, FT6X06_I2C_ADDR, FT6X06_TD_STATUS,
                                   sizeof(uint8_t), rx_buffer,
                                   5 * sizeof(uint8_t), 0);

  if (rslt != CY_RSLT_SUCCESS)
    return rslt;

  *touch_points = rx_buffer[0] & 0x0F;
  if (*touch_points != 1 && *touch_points != 2)
    return rslt;

  x_MSB = rx_buffer[1] & 0x0F;
  x_LSB = rx_buffer[2];
  *x = ((uint16_t)x_MSB << 8) | x_LSB;

  y_MSB = rx_buffer[3] & 0x0F;
  y_LSB = rx_buffer[4];
  *y = ((uint16_t)y_MSB << 8) | y_LSB;

  return CY_RSLT_SUCCESS;
}

/**
 * @brief Get the ID of the I2C device
 *
 * @param id the id of the device
 * @return cy_rslt_t CY_RSLT_SUCCESS if there are no errors reading from the I2C
 * Bus
 */
static cy_rslt_t get_id(uint8_t *id) {
  cy_rslt_t rslt = CY_RSLT_SUCCESS;

  rslt = i2c_read_u8(I2C_Obj, FT6X06_I2C_ADDR, FT6X06_FOCALTECH_ID_R, id);
  if (rslt != CY_RSLT_SUCCESS)
    return rslt;

  return CY_RSLT_SUCCESS;
}

/**
 * @brief This rotates the display since mine isn't in the correct orientation
 *
 * @param x x coordinate
 * @param y y coordinate
 */
static inline void rotate(uint16_t *x, uint16_t *y) {
  uint16_t temp = *x;

  *x = *y;
  *y = 240 - temp;
}

/**
 * @brief The task that receives a request, sees if there are any touch points,
 * then returns the coordinates of the points
 *
 * @param params UNUSED
 */
void task_cap_touch(void *params) {
  (void)params;

  device_request_msg_t request;
  device_response_msg_t response;
  cy_rslt_t rslt = CY_RSLT_SUCCESS;
  uint8_t touch_points;
  uint16_t x, y;

  task_console_printf("Starting Cap Touch Task\n\r");

  while (1) {
    xQueueReceive(Queue_Request_Cap_Touch, &request, portMAX_DELAY);

    if (request.operation != DEVICE_OP_READ) {
      task_console_printf("Invalid Operation\n\r");
      continue;
    }

    if (request.device != DEVICE_CAP_TOUCH) {
      task_console_printf("Invalid Device\n\r");
      continue;
    }

    xSemaphoreTake(*I2C_Semaphore, portMAX_DELAY);

    rslt = read_coords(&touch_points, &x, &y);

    xSemaphoreGive(*I2C_Semaphore);

    if (rslt != CY_RSLT_SUCCESS) {
      task_console_printf("FT6X06 Unable to Read Status\n\r");
      vTaskDelay(pdMS_TO_TICKS(1000));
      vTaskSuspend(NULL);
    }

    if (touch_points != 1 && touch_points != 2) {
      response.device = DEVICE_CAP_TOUCH;
      response.status = DEVICE_OPERATION_STATUS_READ_FAILURE;
    } else {
      response.device = DEVICE_CAP_TOUCH;
      response.status = DEVICE_OPERATION_STATUS_READ_SUCCESS;

      rotate(&x, &y);

      response.payload.cap_touch[0] = x;
      response.payload.cap_touch[1] = y;
    }

    xQueueSend(request.response_queue, &response, portMAX_DELAY);
  }
}

bool task_cap_touch_resources_init(QueueHandle_t queue_request,
                                   SemaphoreHandle_t *i2c_semaphore,
                                   cyhal_i2c_t *i2c_obj) {
  if (queue_request == NULL || i2c_semaphore == NULL || i2c_obj == NULL) {
    return false;
  }

  /* Save the resources */
  Queue_Request_Cap_Touch = queue_request;
  I2C_Semaphore = i2c_semaphore;
  I2C_Obj = i2c_obj;

  /* ADD CODE */
  return xTaskCreate(task_cap_touch, "Cap Touch Task",
                     TASK_CAP_TOUCH_STACK_SIZE, NULL, TASK_CAP_TOUCH_PRIORITY,
                     NULL) == pdPASS;
}
#endif /* FREERTOS */