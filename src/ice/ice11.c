/**
 * @file ex03.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-06-30
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "main.h"

#if defined(ICE11)
#include "drivers.h"
#include "rtos_events.h"
#include "task_buttons.h"
#include "task_ipc.h"

char APP_DESCRIPTION[] = "ECE353: ICE 11 - FreeRTOS IPC Rx/Tx";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
EventGroupHandle_t ECE353_RTOS_Events = NULL;

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/

/* This function will be used to discover other board. This function should not
 * return until the discovery is complete.  The discovery is complete when we
 * receive a discovery message from the other board OR we send a discovery
 * message that is Acked by the other board */
void discover_board(uint16_t *sequence_num) {
  bool discovery_complete = false;

  while (discovery_complete == false) {
    /* ADD CODE */
    /* Send a discovery message */
    ipc_send_discovery(*sequence_num);

    /* Wait for the ack */
    if (ipc_wait_for_ack(1000)) {
      printf("ACK to discovery message received.\n\r");
      discovery_complete = true;
    }
    else{
      printf("No response to discovery message, retrying...\n\r");
    }

    //check if discovery message received from other board
    EventBits_t events = xEventGroupWaitBits(ECE353_RTOS_Events, IPC_DISCOVERY_RECEIVED, pdTRUE, pdFALSE, pdMS_TO_TICKS(1000));
    if(events & IPC_DISCOVERY_RECEIVED){
        printf("Discovery message received from other board.\n\r");
        //send ack back to other board
        ipc_send_ack(*sequence_num);
        discovery_complete = true;
    }
  }
}

/**
 * @brief
 * This task will be used to verify the functionality of the IPC UART
 * specification by ....
 *
 * @param arg
 * Unused parameter
 */
void task_system_control(void *arg) {
  (void)arg; // Unused parameter
  EventBits_t events;

  uint16_t sequence_num = 0;

  /* Begin the discovery process. */
  discover_board(&sequence_num);

  while (1) {
    // Wait for SW1, SW2,or SW3 to be pressed.  If you have not gotten
    // task_buttons.c working yet, you will need to do so before you can proceed
    // with this task.
    events =
        xEventGroupWaitBits(ECE353_RTOS_Events,
                            SW1_PRESSED | SW2_PRESSED |
                                SW3_PRESSED,
                            pdTRUE, pdFALSE, portMAX_DELAY);

    if (events & SW1_PRESSED) {
      /* ADD CODE */

      /* Send the active player message  */
      ipc_send_active_player(sequence_num);

      /* Wait for the ack */
      if(ipc_wait_for_ack(1000)){
        printf("ACK received for active player message.\n\r");
      }
      else{
        printf("No response to active player message.\n\r");
      }

      /* Print out a message indicating if the ACK was received */
      printf("Recieved ACK for sequence number %d\n\r", sequence_num);
      sequence_num++;

    } else if (events & SW2_PRESSED) {
      /* ADD CODE */

      /* Send the inactive player message  */
      ipc_send_inactive_player(sequence_num);

      /* Wait for the ack */
      if(ipc_wait_for_ack(1000)){
        printf("ACK received for inactive player message.\n\r");
      }
      else{
        printf("No response to inactive player message.\n\r");
      }

      /* Print out a message indicating if the ACK was received */
      printf("Recieved ACK for sequence number %d\n\r", sequence_num);
      sequence_num++;

    } else if (events & SW3_PRESSED) {
      /* ADD CODE */

      /* Send the status message with an error code  */
      ipc_send_status(sequence_num, IPC_STATUS_CRC_FAIL); //example error code for now

      /* Wait for the ack */
      if(ipc_wait_for_ack(1000)){
        printf("ACK received for status message.\n\r");
      }
      else{
        printf("No response to status message.\n\r");
      }

      /* Print out a message indicating if the ACK was received */
      printf("Recieved ACK for sequence number %d\n\r", sequence_num);
      sequence_num++;

    } else {
      printf("Unknown Event!\n\r\n\r");
    }
  }
}

/**
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 */
void app_init_hw(void) {
  cy_rslt_t rslt;

  console_init();
  printf("**************************************************\n\r");
  printf("* %s\n\r", APP_DESCRIPTION);
  printf("* Date: %s\n\r", __DATE__);
  printf("* Time: %s\n\r", __TIME__);
  printf("* Name:%s\n\r", NAME);
  printf("**************************************************\n\r");

  // Initialize the LEDs
  rslt = leds_init_gpio();
  if (rslt != CY_RSLT_SUCCESS) {
    printf("LED initialization failed!\n\r");
    for (int i = 0; i < 10000; i++)
      ;
    CY_ASSERT(0);
  }

  rslt = buttons_init_gpio();

  if (rslt != CY_RSLT_SUCCESS) {
    printf("Button initialization failed!\n\r");
    for (int i = 0; i < 10000; i++)
      ;
    CY_ASSERT(0);
  }
}

/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */
void app_main(void) {
  // Initialize the EventGroup
  ECE353_RTOS_Events = xEventGroupCreate();

  if (!task_buttons_init()) {
    printf("Button initialization failed!\n\r");
    for (int i = 0; i < 10000; i++)
      ;
    CY_ASSERT(0);
  }

  if (!task_ipc_init()) {
    printf("IPC initialization failed!\n\r");
    for (int i = 0; i < 10000; i++)
      ;
    CY_ASSERT(0);
  }

  // Create the System Control Task
  xTaskCreate(task_system_control, "System Control",
              configMINIMAL_STACK_SIZE * 5, NULL, 2, NULL);

  /* Start the scheduler*/
  vTaskStartScheduler();

  /* Will never reach this loop once the scheduler starts */
  while (1) {
  }
}
#endif