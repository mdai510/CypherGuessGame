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
#include "task_console.h"

#ifdef ECE353_FREERTOS
/**
 * @brief
 * This function is the event handler for the console UART.
 *
 * The ISR will receive characters from the UART and store them in a console
 * buffer until the user presses the ENTER key.  At that point, the ISR will
 * send a task notification to the console task to process the received string.
 *
 * The ISR will also echo the received character back to the console.
 *
 * @param handler_arg
 * @param event
 */
void console_event_handler(void *handler_arg, cyhal_uart_event_t event) {
  (void)handler_arg;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint8_t c;

  if ((event & CYHAL_UART_IRQ_RX_NOT_EMPTY) == CYHAL_UART_IRQ_RX_NOT_EMPTY) {
    // ADD CODE
    // Read in character
    cyhal_uart_getc(&cy_retarget_io_uart_obj, &c, portMAX_DELAY);

    // Echo character to the hardware FiFo
    cyhal_uart_putc(&cy_retarget_io_uart_obj, c);

    // If character is backspace (0x8) or delete key (0x7F), remove last
    // character from array
    if (c == 0x8 || c == 0x7F) {
      produce_console_buffer->data[--produce_console_buffer->index] = '\0';
    }

    // Else if character is '\n' or '\r', NULL terminate string and send
    // task notification and swap buffer pointers

    else if ((c == '\r' || c == '\n') && produce_console_buffer->index != 0) {
      produce_console_buffer->data[produce_console_buffer->index] = '\0';
      produce_console_buffer->index = 0;
      console_buffer_t *temp = produce_console_buffer;
      produce_console_buffer = consume_console_buffer;
      consume_console_buffer = temp;

      vTaskNotifyGiveFromISR(TaskHandle_Console_Rx, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    // Else add character to buffer and increment the index
    else if ((c != '\r' || c != '\n') &&
             produce_console_buffer->index < CONSOLE_MAX_MESSAGE_LENGTH - 2) {
      produce_console_buffer->data[produce_console_buffer->index++] = c;
    }
  }
  if ((event & CYHAL_UART_IRQ_TX_EMPTY) == CYHAL_UART_IRQ_TX_EMPTY) {
    /* ADD CODE */
    // Check if circular buffer is empty, disable TX Empty interrupts
    // Else transmit next character
    if (circular_buffer_empty(circular_buffer_tx)) {
      cyhal_uart_enable_event(&cy_retarget_io_uart_obj, CYHAL_UART_IRQ_TX_EMPTY,
                              INT_PRIORITY_CONSOLE, false);
    } else {
      char c;
      circular_buffer_remove(circular_buffer_tx, &c);
      cyhal_uart_putc(&cy_retarget_io_uart_obj, c);
    }
  } else {
  }
}

/**
 * @brief
 * This function initializes the console tasks and resources.
 * @return true
 * @return false
 */
bool task_console_init(void) {
  /* Register a function for the UART ISR*/
  cyhal_uart_register_callback(&cy_retarget_io_uart_obj, // UART object
                               console_event_handler,    // Event handler
                               NULL                      // Handler argument
  );

  /* Initialize UART Rx Resources */
  if (!task_console_resources_init_rx()) {
    return false; // Initialization failed
  }

  /* Initialize UART Tx Resources */
  if (!task_console_resources_init_tx()) {
    return false; // Initialization failed
  } else {
    // Enable UART Rx Interrupts
    cyhal_uart_enable_event(&cy_retarget_io_uart_obj,
                            CYHAL_UART_IRQ_RX_NOT_EMPTY, 7, true);
  }

  return true; // Initialization successful
}
#endif