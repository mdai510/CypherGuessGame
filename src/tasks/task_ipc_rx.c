#include "main.h"
#include <stdbool.h>

#if defined(FREERTOS)
#include "task_ipc.h"

static const char *const STATUS_MSGS[3] = {"OK", "CRC FAIL",
                                           "INVALID MESSAGE TYPE"};

#define IPC_CMD_BASE 0xC0
#define IPC_STATUS_BASE 0xE0

/* Globals */
TaskHandle_t TaskHandle_IPC_Rx = NULL;

/* Use a double buffering strategy for IPC packets */
static volatile ipc_packet_t IPC_Rx_Buffer0;
static volatile ipc_packet_t IPC_Rx_Buffer1;

volatile ipc_packet_t *volatile IPC_Rx_Produce_Buffer = &IPC_Rx_Buffer0;
volatile ipc_packet_t *volatile IPC_Rx_Consume_Buffer = &IPC_Rx_Buffer1;

extern uint32_t opp_randnum;

/**
 * @brief
 *
 * This task is used to process received IPC packets.  The task will block
 * on a FreeRTOS Task Notification.  When a notification is received,
 * the task will process the IPC packet stored in the consume buffer.
 *
 * For validation purposes, the task will print out the contents of the
 * received IPC packet to the console.
 *
 * @param arg
 * Unused parameter
 */
void task_ipc_rx(void *param) {
  while (1) {
    // Wait for a FreeRTOS Task Notification
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (validate_packet((ipc_packet_t *)IPC_Rx_Consume_Buffer) == true) {
      ipc_cmd_t cmd = IPC_Rx_Consume_Buffer->cmd;
      uint8_t status_num =
          IPC_Rx_Consume_Buffer->payload.status - IPC_STATUS_BASE;
      uint16_t sequence_num = IPC_Rx_Consume_Buffer->sequence_num;

      const char *status_msg = NULL;

      // Process the received IPC packet
      switch (cmd) {
      case IPC_CMD_DISCOVERY:
        opp_randnum = IPC_Rx_Consume_Buffer->payload.randnum;
        xEventGroupSetBits(RTOS_Events, IPC_DISCOVERY_RECEIVED);
        ipc_send_ack(sequence_num);
        break;

      case IPC_CMD_ACK:
        printf("Ack Received with Sequence Number: %d!\n\r", sequence_num);
        xEventGroupSetBits(RTOS_Events, IPC_ACK_RECEIVED);
        break;

      case IPC_CMD_INACTIVE_PLAYER:
        printf("Received Inactive Player Command\n\r");
        xEventGroupSetBits(RTOS_Events, IPC_GUESS_RESPONSE_RECEIVED);
        uint16_t inactive_payload =
            IPC_Rx_Consume_Buffer->payload.inactive_player;
        xQueueSend(Queue_Guess_Response_Received, &inactive_payload,
                   portMAX_DELAY);
        ipc_send_ack(sequence_num);
        break;
      case IPC_CMD_ACTIVE_PLAYER:
        printf("Received Active Player Command\n\r");
        xEventGroupSetBits(RTOS_Events, IPC_GUESS_RECEIVED);
        uint32_t guess_encoded = IPC_Rx_Consume_Buffer->payload.active_player;
        xQueueSend(Queue_Guess_Received, &guess_encoded, portMAX_DELAY);
        ipc_send_ack(sequence_num);
        break;
      case IPC_CMD_RDY:
        printf("Received Ready Command\n\r");
        xEventGroupSetBits(RTOS_Events, IPC_RDY_RECEIVED);
        ipc_send_ack(sequence_num);
        break;
      case IPC_CMD_RST:
        printf("Received Reset Command\n\r");
        xEventGroupSetBits(RTOS_Events, IPC_RESET_RECEIVED);
        ipc_send_ack(sequence_num);
        break;
      case IPC_CMD_STATUS:
        status_msg = status_num < sizeof(STATUS_MSGS) / sizeof(char *)
                         ? STATUS_MSGS[status_num]
                         : "UNKNOWN";

        printf("Received %s Status Message\n\r", status_msg);
        ipc_send_ack(sequence_num);
        break;

      default:
        printf("Unknown Command Received\n\r");
        break;
      }
    } else {
      printf("Invalid IPC packet received!\n\r");
      ipc_send_status(0, IPC_STATUS_CRC_FAIL);
      ipc_wait_for_ack(TIMEOUT);
    }
  }
}

bool task_ipc_resources_init_rx(void) {
  // Create the IPC Rx Task
  BaseType_t task_ipc_rx_status = xTaskCreate(
      task_ipc_rx,       // Function that implements the task.
      "IPC Rx Task",     // Text name for the task.
      IPC_STACK_SIZE,    // Stack size in words, not bytes.
      NULL,              // Parameter passed into the task.
      IPC_PRIORITY,      // Priority at which the task is created.
      &TaskHandle_IPC_Rx // Used to pass out the created task's handle.
  );

  if (task_ipc_rx_status != pdPASS) {
    return false;
  }

  return true;
}

#endif /* FREERTOS */