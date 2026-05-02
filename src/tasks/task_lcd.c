/**
 * @file task_lcd.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-18
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "task_lcd.h"

#if defined(ECE353_FREERTOS)

/* FreeRTOS Queue for LCD messages */
static QueueHandle_t Queue_Requests = NULL;

static inline uint8_t print_char(char c, uint32_t x_pos, uint32_t y_pos) {

  uint16_t index = c - Consolas_20ptFontInfo.start_char;
  FONT_CHAR_INFO info = Consolas_20ptDescriptors[index];
  const uint8_t *bitmap = Consolas_20ptBitmaps + info.offset;

  lcd_draw_image(x_pos, y_pos, info.width, info.height, bitmap, LCD_COLOR_WHITE,
                 LCD_COLOR_BLACK, false);

  return info.width;
}

/* LCD Task */
void task_lcd(void *pvParameters) {
  (void)pvParameters; // Unused parameter

  lcd_msg_request_t lcd_request;
  lcd_msg_response_t response = LCD_MSG_RESPONSE_SUCCESS;
  bool status = false;

  while (1) {
    xQueueReceive(Queue_Requests, &lcd_request, portMAX_DELAY);

    switch (lcd_request.msg.command) {
    case LCD_CMD_PRINT_MESSAGE:
      status = master_mind_handle_msg(&lcd_request.msg);
      break;
    case LCD_CMD_DRAW_TILE:
      status = master_mind_handle_msg(&lcd_request.msg);
      break;
    case LCD_CMD_DRAW_TILE_INVERTED:
      status = master_mind_handle_msg(&lcd_request.msg);
      break;
    case LCD_CMD_CLEAR_SCREEN:
      status = master_mind_handle_msg(&lcd_request.msg);
    default:
      status = false;
      break;
    }

    response = status ? LCD_MSG_RESPONSE_SUCCESS : LCD_MSG_RESPONSE_ERROR;
    xQueueSend(lcd_request.return_queue, &response, portMAX_DELAY);
  }
}

/* LCD Task Initialization */
bool task_lcd_resources_init(QueueHandle_t queue_request) {

  BaseType_t result;

  if (queue_request == NULL) {
    return false;
  }
  Queue_Requests = queue_request;

  /* Create the LCD Task */
  result = xTaskCreate(task_lcd,            // Task function
                       "LCD Task",          // Task name
                       TASK_LCD_STACK_SIZE, // Stack size
                       NULL,                // Task parameters
                       TASK_LCD_PRIORITY,   // Task priority
                       NULL                 // Task handle
  );

  if (result != pdPASS) {
    return false;
  }

  return true;
}
#endif