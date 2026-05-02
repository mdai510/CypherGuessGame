 /**
 * @file hw02.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-10-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "hw02.h"

#if defined(HW02)

char APP_DESCRIPTION[] = "ECE353 S26 HW02";

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
EventGroupHandle_t ECE353_RTOS_Events;

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/

void task_hw02_system_control(void *pvParameters)
{
    (void)pvParameters; // Unused parameter
    static int cypher[4] = {0,0,0,0};
    static int cur_cypher_pos = 0;
    static int cur_col = 0;
    static int cur_row = 0;
    static int prev_col = 0;
    static int prev_row = 0;
    static bool changed_pos = false;
    EventBits_t events;

    static int cur_cypher_val = 0;
    joystick_position_t pos;
    device_request_msg_t device_request;
    device_response_msg_t device_response;

    /* Draw the initial master mind game board */
    //clear screen
    lcd_msg_t msg;
    msg.command = LCD_CMD_CLEAR_SCREEN;
    xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);

    //draw start message
    msg.command = LCD_CMD_PRINT_MESSAGE;
    snprintf(msg.payload.message, 32, "Select your Cypher");
    xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);

    //draw left most cypher tile
    msg.command = LCD_CMD_DRAW_TILE_INVERTED;
    msg.payload.tile.row = LCD_TILE_ROW_CYPHER;
    msg.payload.tile.col = 0;
    msg.payload.tile.number = 0;
    msg.payload.tile.color_fg = LCD_COLOR_BLACK;
    msg.payload.tile.color_bg = LCD_COLOR_RED;
    xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);

    //draw other cypher tiles
    for(int col = 1; col < 4; col++)
    {
        msg.command = LCD_CMD_DRAW_TILE_INVERTED;
        msg.payload.tile.row = LCD_TILE_ROW_CYPHER;
        msg.payload.tile.col = col;
        msg.payload.tile.number = 0;
        msg.payload.tile.color_fg = LCD_COLOR_RED;
        msg.payload.tile.color_bg = LCD_COLOR_BLACK;
        xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);
    }

    //draw first data entry tile
    msg.command = LCD_CMD_DRAW_TILE_INVERTED;
    msg.payload.tile.row = LCD_TILE_ROW_NUM_0_3;
    msg.payload.tile.col = 0;
    msg.payload.tile.number = 0;
    msg.payload.tile.color_fg = LCD_COLOR_BLACK;
    msg.payload.tile.color_bg = LCD_COLOR_GREEN;
    xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);

    //draw other data entry tiles
    for(int col = 1; col < 4; col++)
    {
        msg.command = LCD_CMD_DRAW_TILE_INVERTED;
        msg.payload.tile.row = LCD_TILE_ROW_NUM_0_3;
        msg.payload.tile.col = col;
        msg.payload.tile.number = col;
        msg.payload.tile.color_fg = LCD_COLOR_GREEN;
        msg.payload.tile.color_bg = LCD_COLOR_BLACK;
        xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);
    }
    for(int col = 0; col < 4; col++)
    {
        msg.command = LCD_CMD_DRAW_TILE_INVERTED;
        msg.payload.tile.row = LCD_TILE_ROW_NUM_4_7;
        msg.payload.tile.col = col;
        msg.payload.tile.number = col + 4;
        msg.payload.tile.color_fg = LCD_COLOR_GREEN;
        msg.payload.tile.color_bg = LCD_COLOR_BLACK;
        xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);
    }

    /* Allow the user to select 4 numbers for their cypher */
    //wait for joystick position messages and button presses
    while (1) {
        if(cur_cypher_pos < 4){
            //wait for joystick move or button press event
            events = xEventGroupWaitBits(ECE353_RTOS_Events, 
                                JOYSTICK_MOVED | SW1_PRESSED,
                                pdTRUE, pdFALSE, portMAX_DELAY);
            if(events & JOYSTICK_MOVED){
                xQueueReceive(Queue_Joystick, &pos, portMAX_DELAY);
                if (pos == JOYSTICK_POS_LEFT) {
                    if (cur_col > 0) {
                    prev_col = cur_col;
                    prev_row = cur_row;
                    cur_col--;
                    changed_pos = true;
                    }
                } else if (pos == JOYSTICK_POS_RIGHT) {
                    if (cur_col < 3) {
                    prev_col = cur_col;
                    prev_row = cur_row;
                    cur_col++;
                    changed_pos = true;
                    }
                } else if (pos == JOYSTICK_POS_UP) {
                    if (cur_row > 0) {
                    prev_row = cur_row;
                    prev_col = cur_col;
                    cur_row--;
                    changed_pos = true;
                    }
                } else if (pos == JOYSTICK_POS_DOWN) {
                    if (cur_row < 1) {
                    prev_row = cur_row;
                    prev_col = cur_col;
                    cur_row++;
                    changed_pos = true;
                    }
                }
                // update tile if diff pos
                if (changed_pos) {
                    // update prev tile to non-highlighted
                    msg.command = LCD_CMD_DRAW_TILE_INVERTED;
                    msg.payload.tile.row =
                        prev_row == 0 ? LCD_TILE_ROW_NUM_0_3 : LCD_TILE_ROW_NUM_4_7;
                    msg.payload.tile.col = prev_col;
                    msg.payload.tile.number = prev_row == 0 ? prev_col : prev_col + 4;
                    msg.payload.tile.color_fg = LCD_COLOR_GREEN;
                    msg.payload.tile.color_bg = LCD_COLOR_BLACK;
                    xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);
                    printf("Unhighlighting tile at row %d, col %d\n\r", prev_row, prev_col);

                    // update new tile to highlighted
                    msg.command = LCD_CMD_DRAW_TILE_INVERTED;
                    msg.payload.tile.row =
                        cur_row == 0 ? LCD_TILE_ROW_NUM_0_3 : LCD_TILE_ROW_NUM_4_7;
                    msg.payload.tile.col = cur_col;
                    msg.payload.tile.number = cur_row == 0 ? cur_col : cur_col + 4;
                    msg.payload.tile.color_fg = LCD_COLOR_BLACK;
                    msg.payload.tile.color_bg = LCD_COLOR_GREEN;
                    xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);
                    printf("Highlighting tile at row %d, col %d\n\r", cur_row, cur_col);
                    changed_pos = false;
                }
            }
            else if (events & SW1_PRESSED) {
                printf("Button Pressed with cypher num %d selected\n\r", cur_row == 0 ? cur_col : cur_col + 4);
                cur_cypher_val = (cur_row == 0) ? cur_col : cur_col + 4;
                cypher[cur_cypher_pos] = cur_cypher_val;
                // update cur cypher num and de-highlight it
                msg.command = LCD_CMD_DRAW_TILE_INVERTED;
                msg.payload.tile.row = LCD_TILE_ROW_CYPHER;
                msg.payload.tile.col = cur_cypher_pos;
                msg.payload.tile.number = cur_cypher_val;
                msg.payload.tile.color_fg = LCD_COLOR_RED;
                msg.payload.tile.color_bg = LCD_COLOR_BLACK;
                xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);
                cur_cypher_pos++;
                // highlight next cypher num tile
                if (cur_cypher_pos < 4) {
                    msg.command = LCD_CMD_DRAW_TILE_INVERTED;
                    msg.payload.tile.row = LCD_TILE_ROW_CYPHER;
                    msg.payload.tile.col = cur_cypher_pos;
                    msg.payload.tile.number = 0;
                    msg.payload.tile.color_fg = LCD_COLOR_BLACK;
                    msg.payload.tile.color_bg = LCD_COLOR_RED;
                    xQueueSend(xQueue_Request_LCD, &msg, portMAX_DELAY);
                }
            }
        }
    }
}


/*************************************************
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 ************************************************/
void app_init_hw(void)
{
    cy_rslt_t rslt;

    console_init();
    // Set text color to black
    printf("\x1b[30m");
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    rslt = lcd_initialize();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("LCD initialization failed!\n\r");
        for(int i = 0; i < 100000; i++) {}
        CY_ASSERT(0);
    }

    /* Initialize the joystick */
    joystick_init();

    /* Initialize the buttons*/
    buttons_init_gpio();

}

/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */
void app_main(void)
{
    //delay
    CyDelay(100);
    /* Create the event group for RTOS events */
    ECE353_RTOS_Events = xEventGroupCreate();

    /* Create the FreeRTOS queues */
    xQueue_Request_LCD = xQueueCreate(5, sizeof(lcd_msg_request_t));
    if (xQueue_Request_LCD == NULL){
        printf("Failed to create LCD request queue\n");
        CY_ASSERT(0);
    }

    /* Create the FreeRTOS queues */


     /* Create the FreeRTOS tasks */
    xTaskCreate(
        task_hw02_system_control,
        "System Control Task",
        TASK_SYSTEM_CONTROL_STACK_SIZE,
        NULL,
        TASK_SYSTEM_CONTROL_PRIORITY,
        NULL
    );

    /* Create the FreeRTOS tasks */
    task_lcd_resources_init(xQueue_Request_LCD);
    task_buttons_init();
    task_joystick_init();

    /* Start the scheduler*/
    vTaskStartScheduler();

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}
#endif