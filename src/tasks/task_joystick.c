
#include "main.h"

#ifdef FREERTOS
#include "drivers.h"
#include "task_joystick.h"

/* Message lookup table for joystick positions */
const char *const joystick_pos_names[] = {
    "Center",     "Left",        "Right",      "Up",         "Down",
    "Upper Left", "Upper Right", "Lower Left", "Lower Right"};

/**
 * @brief
 *  Task used to monitor the joystick
 * @param arg
 */
void task_joystick(void *arg) {
  (void)arg;
  device_request_msg_t request = {0};

  while (1) {
    xQueueReceive(xQueue_Request_Joystick, &request, portMAX_DELAY);

    device_response_msg_t response;
    response.device = DEVICE_JOYSTICK;
    response.status = DEVICE_OPERATION_STATUS_READ_SUCCESS;
    response.payload.joystick = joystick_get_pos();

    xQueueSend(request.response_queue, &response, portMAX_DELAY);
  }
}

bool task_joystick_init(void) {
  /* Create the Queue used to send Joystick Positions*/
  /* Create the joystick task */
  xTaskCreate(task_joystick, "Joystick Task", 5 * configMINIMAL_STACK_SIZE,
              NULL, tskIDLE_PRIORITY + 1, NULL);

  return true;
}
#endif /* FREERTOS */