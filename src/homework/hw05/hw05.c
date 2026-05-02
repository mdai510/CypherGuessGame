/**
 * @file hw05.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-10-08
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "hw05.h"

#if defined(HW05)

char APP_DESCRIPTION[] = "ECE353 S26 HW05";

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
static cyhal_i2c_t *I2C_Monarch_Obj;
static cyhal_spi_t *SPI_Monarch_Obj;
static SemaphoreHandle_t I2C_Semaphore;
static SemaphoreHandle_t SPI_Semaphore;
static SemaphoreHandle_t LCD_Semaphore;
static QueueHandle_t xQueue_Request_I2C;
QueueHandle_t Queue_Cap_Touch_Coords;
QueueHandle_t xQueue_Request_LCD;
QueueHandle_t xQueue_Response_LCD;
QueueHandle_t xQueue_Response_I2C;
QueueHandle_t Queue_Light_Sensor_Responses;
QueueHandle_t Queue_Response_Cap_Touch;
QueueHandle_t Queue_Guess_Received;
QueueHandle_t Queue_Guess_Response_Received;
EventGroupHandle_t ECE353_RTOS_Events;
TaskHandle_t xHandleCapTouch;

// game state
int my_guess_cnt = 0;
static uint8_t hi_score = 0;
static int selected_cypher[4] = {
    0}; // stores the currently selected cypher for other board to guess,
        // initialized to all 0s
static int guess[4] = {
    0}; // stores the current guess for the active player, initialized to all 0s

// amb light static globals
static bool is_dark = false;
static uint16_t cur_bg_color;

uint32_t opp_randnum = 0;

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/

/**
 * @brief Discovers the opponent's board by sending a discovery message
 *
 * @param sequence_num used to keep track of the sequence of messages this board
 * sends
 * @param is_first true if this board goes first in the game, otherwise false
 */
void discover_board(uint16 *sequence_num, bool *is_first) {
  while (true) {
    // Seed with tick count — varies per board since clocks drift
    srand(xTaskGetTickCount());
    uint32_t randnum = (uint32_t)rand();

    // send discovery message
    ipc_send_discovery(*sequence_num, randnum);

    ipc_wait_for_ack(TIMEOUT);
    // wait for discovery message from other board, retry after timeout
    EventBits_t event =
        xEventGroupWaitBits(ECE353_RTOS_Events, IPC_DISCOVERY_RECEIVED, pdTRUE,
                            pdFALSE, pdMS_TO_TICKS(500));
    if (event & IPC_DISCOVERY_RECEIVED) {
      if (randnum == opp_randnum) {
        // Exact tie: retry with a fresh random number
        continue;
      }
      *is_first = randnum > opp_randnum;
      break;
    }
  }
}

/**
 * @brief Initializes all of the Semaphore objects
 *
 */
static void semaphores_init(void) {
  I2C_Semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(I2C_Semaphore);

  SPI_Semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(SPI_Semaphore);

  LCD_Semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(LCD_Semaphore);
}

/**
 * @brief Initializes all of the Queue objects
 *
 */
static void queues_init(void) {
  xQueue_Request_LCD = xQueueCreate(1, sizeof(lcd_msg_request_t));
  xQueue_Response_LCD = xQueueCreate(1, sizeof(lcd_msg_response_t));

  xQueue_Request_I2C = xQueueCreate(1, sizeof(device_request_msg_t));
  xQueue_Response_I2C = xQueueCreate(1, sizeof(device_response_msg_t));

  Queue_Light_Sensor_Responses = xQueueCreate(1, sizeof(device_response_msg_t));
  Queue_Response_Cap_Touch = xQueueCreate(1, sizeof(device_response_msg_t));

  Queue_Cap_Touch_Coords = xQueueCreate(1, sizeof(touch_coords_t));

  Queue_Guess_Received = xQueueCreate(1, sizeof(uint32_t));
  Queue_Guess_Response_Received = xQueueCreate(1, sizeof(uint16_t));
}

/**
 * @brief Prints the inputted message to the screen
 *
 * @param c_msg a null-terminated message to be printed to the screen
 */
static inline void print_message(const char *c_msg) {
  lcd_msg_request_t request = {0};
  lcd_msg_t msg = {0};
  lcd_msg_response_t response = {0};

  msg.command = LCD_CMD_PRINT_MESSAGE;
  snprintf(msg.payload.message, 32, "%s", c_msg);
  request.msg = msg;
  request.return_queue = xQueue_Response_LCD;

  xQueueSend(xQueue_Request_LCD, &request, portMAX_DELAY);
  xQueueReceive(xQueue_Response_LCD, &response, portMAX_DELAY);
}

/**
 * @brief Draws the cypher tiles to the screen
 *
 * @param current_index the currently selected cypher tile
 * @param do_highlight true if the selected cypher tile should be inverted
 * @param is_selected true if the cypher tiles should print the selected cypher
 * tiles, otherwise false
 */
static void draw_cypher_tiles(uint32_t current_index, bool do_highlight,
                              bool is_selected) {
  lcd_msg_request_t request = {0};
  lcd_msg_t msg = {0};
  lcd_msg_response_t response = {0};

  // Draws the 4 cypher tiles
  request.return_queue = xQueue_Response_LCD;
  for (int col = 0; col < 4; ++col) {
    // Invert tile if it is the currently selected cypher and highlighting is
    // enabled
    msg.command = (do_highlight && col == current_index)
                      ? LCD_CMD_DRAW_TILE_INVERTED
                      : LCD_CMD_DRAW_TILE;
    msg.payload.tile.row = LCD_TILE_ROW_CYPHER;
    msg.payload.tile.col = col;
    msg.payload.tile.number = is_selected ? selected_cypher[col] : guess[col];
    msg.payload.tile.color_fg = LCD_COLOR_RED;
    msg.payload.tile.color_bg = cur_bg_color;

    request.msg = msg;
    xQueueSend(xQueue_Request_LCD, &request, portMAX_DELAY);
    xQueueReceive(xQueue_Response_LCD, &response, portMAX_DELAY);
  }
}

/**
 * @brief Draws the initial number tiles to the screen
 *
 */
static void draw_init_tiles(void) {
  lcd_msg_request_t request = {0};
  lcd_msg_t msg = {0};
  lcd_msg_response_t response = {0};

  // Draws the 0-3 tiles
  request.return_queue = xQueue_Response_LCD;
  for (int col = 0; col < 4; ++col) {
    msg.command = LCD_CMD_DRAW_TILE;
    msg.payload.tile.row = LCD_TILE_ROW_NUM_0_3;
    msg.payload.tile.col = col;
    msg.payload.tile.number = col;
    msg.payload.tile.color_fg = LCD_COLOR_GREEN;
    msg.payload.tile.color_bg = cur_bg_color;

    request.msg = msg;
    xQueueSend(xQueue_Request_LCD, &request, portMAX_DELAY);
    xQueueReceive(xQueue_Response_LCD, &response, portMAX_DELAY);
  }

  // Draws the 4-7 tiles
  for (int col = 0; col < 4; ++col) {
    msg.command = LCD_CMD_DRAW_TILE;
    msg.payload.tile.row = LCD_TILE_ROW_NUM_4_7;
    msg.payload.tile.col = col;
    msg.payload.tile.number = col + 4;
    msg.payload.tile.color_fg = LCD_COLOR_GREEN;
    msg.payload.tile.color_bg = cur_bg_color;

    request.msg = msg;
    xQueueSend(xQueue_Request_LCD, &request, portMAX_DELAY);
    xQueueReceive(xQueue_Response_LCD, &response, portMAX_DELAY);
  }
}

/**
 * @brief Clears the screen and sets the background color to
            the currently selected background color determined by the light
 sensor task
 *
 */
static void clear_screen(void) {
  lcd_msg_request_t request;
  lcd_msg_response_t response;

  request.msg.command = LCD_CMD_CLEAR_SCREEN;
  request.msg.payload.color = cur_bg_color;
  request.return_queue = xQueue_Response_LCD;

  xQueueSend(xQueue_Request_LCD, &request, portMAX_DELAY);
  xQueueReceive(xQueue_Response_LCD, &response, portMAX_DELAY);
}

/**
 * @brief Determines the selected tile based on the coordinates returned from
            the cap touch task.
 *
 * @param t_coords the touch-point coordinates returned from the cap touch task
 * @param selected_tile the selected tile, 0-7 corresponds to the number tiles
 and 8-11 corresponds to the cypher tiles
 * @return true if the coordinates correspond to an actual tile, otherwise false
 */
bool select_cypher_num(touch_coords_t t_coords, uint8_t *selected_tile) {
  // get cypher num from cap touch coordinates
  uint16_t x = t_coords.x;
  uint16_t y = t_coords.y;
  lcd_row_t row;
  lcd_col_t col;

  // get row
  if (y >= lcd_area_code_top() && y < lcd_area_code_top() + TILE_H) {
    row = LCD_TILE_ROW_CYPHER;
  } else if (y >= lcd_area_num0_3_top() && y < lcd_area_num0_3_top() + TILE_H) {
    row = LCD_TILE_ROW_NUM_0_3;
  } else if (y >= lcd_area_num4_7_top() && y < lcd_area_num4_7_top() + TILE_H) {
    row = LCD_TILE_ROW_NUM_4_7;
  } else {
    return false; // not in any tile row
  }
  // get col
  if (x < LCD_MARGIN || x >= LCD_W - LCD_MARGIN) {
    return false; // not in any tile column
  }
  col = (x - LCD_MARGIN) / (TILE_W + LCD_MARGIN);
  if (col >= 4) {
    return false; // not in any tile column
  }
  // calculate selected tile num
  if (row == LCD_TILE_ROW_CYPHER) {
    *selected_tile = col + 8; // cypher tiles are numbered 8-11
  } else if (row == LCD_TILE_ROW_NUM_0_3) {
    *selected_tile = col; // num 0-3 tiles are numbered 0-3
  } else if (row == LCD_TILE_ROW_NUM_4_7) {
    *selected_tile = col + 4; // num 4-7 tiles are numbered 4-7
  } else {
    return false; // not in any tile row
  }

  return true;
}

/**
 * @brief Polls the light sensor used to update the UI from light to dark mode
 * and vice-versa
 *
 * @param pvParameters UNUSED
 */
void task_light_sensor_poll(void *pvParameters) {
  (void)pvParameters;
  uint16_t light_reading;
  int consecutive_count = 0;
  bool prev_is_dark = false;
  task_console_printf("Starting Light Sensor Polling Task\n\r");

  // if sensor is same state for 2 consecutive polls, update is_dark and set
  // event to update background color
  while (1) {
    system_sensors_get_light(Queue_Light_Sensor_Responses, &light_reading);
    bool current_is_dark = light_reading < LIGHT_THRESHOLD;

    if (current_is_dark == prev_is_dark) {
      consecutive_count++;
    } else {
      prev_is_dark = current_is_dark;
      consecutive_count = 1;
    }

    if (consecutive_count >= 2 && prev_is_dark != is_dark) {
      is_dark = prev_is_dark;
      cur_bg_color = is_dark ? LCD_COLOR_BLACK : LCD_COLOR_WHITE;
      xEventGroupSetBits(ECE353_RTOS_Events, LIGHT_SENSOR_UPDATED);
      // task_console_printf("Ambient light changed, is_dark: %d\n\r", is_dark);
    }

    // task_console_printf("Ambient Light: %d, is_dark: %d\n\r", amb_light,
    // is_dark);
    vTaskDelay(pdMS_TO_TICKS(500)); // Poll at 2Hz
  }
}

/**
 * @brief Reads from the cap touch device upon task notification
 *
 * @param pvParameters UNUSED
 */
void task_cap_touch_poll(void *pvParameters) {
  (void)pvParameters;
  device_request_msg_t request = {.device = DEVICE_CAP_TOUCH,
                                  .operation = DEVICE_OP_READ,
                                  .response_queue = Queue_Response_Cap_Touch};
  device_response_msg_t response;
  touch_coords_t coords;
  task_console_printf("Starting Cap Touch Polling Task\n\r");

  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    xQueueSend(Queue_Request_Cap_Touch, &request, NO_TIMEOUT);
    if (xQueueReceive(Queue_Response_Cap_Touch, &response,
                      pdMS_TO_TICKS(100)) != pdTRUE)
      continue;

    coords = (response.status == DEVICE_OPERATION_STATUS_READ_SUCCESS)
                 ? (touch_coords_t){response.payload.cap_touch[0],
                                    response.payload.cap_touch[1]}
                 : (touch_coords_t){UINT16_MAX, UINT16_MAX};

    xQueueSend(Queue_Cap_Touch_Coords, &coords, NO_TIMEOUT);
  }
}

/**
 * @brief Reads the high-score from the EEPROM and prints the corresponding
 * message
 *
 * @param msg the message that is determined by the high-score saved in the
 * EEPROM
 */
static inline void get_init_eeprom(char msg[50]) {
  (void)system_sensors_eeprom_read(xQueue_Response_I2C, HI_SCORE_ADDRESS,
                                   &hi_score);

  if (hi_score == RESET_SCORE) {
    snprintf(msg, 50, "Select Cypher: HiScr: N/A");
  } else {
    snprintf(msg, 50, "Select Cypher: HiScr: %d", hi_score);
  }
}

/**
 * @brief Draws the initial game board based
 *
 * @param msg the initial score message to be printed to the board
 */
static void display_init_ui(char msg[50]) {
  const uint32_t starting_index = 0;
  const bool highlight_tiles = true;
  const bool cypher_tiles = true;

  (void)system_sensors_get_light(xQueue_Response_I2C, &cur_bg_color);
  cur_bg_color =
      cur_bg_color >= LIGHT_THRESHOLD ? LCD_COLOR_WHITE : LCD_COLOR_BLACK;

  clear_screen();
  print_message(msg);
  draw_cypher_tiles(starting_index, highlight_tiles, cypher_tiles);
  draw_init_tiles();
}

/**
 * @brief Updates the message and cypher tiles
 *
 * @param msg the new message to be printed to the board
 * @param cypher_index the currently selected cypher tile
 * @param highlight_tile whether the currently selected cypher tile should be
 * inverted
 * @param draw_selected true if the selected cypher numbers are printed,
 * otherwise false
 */
static void update_msg_and_cypher(char msg[50], uint32_t cypher_index,
                                  bool highlight_tile, bool draw_selected) {

  print_message(msg);
  draw_cypher_tiles(cypher_index, highlight_tile, draw_selected);
}

/**
 * @brief Updates the UI based on the change detected by the light sensor
 *
 * @param msg the current message to be printed to the board
 * @param cur_cypher_index the currently selected cypher tile
 * @param highlight_tiles whether to invert the currently selected cypher tile
 */
static void update_ui(char msg[50], uint32_t cur_cypher_index,
                      bool highlight_tiles) {
  const bool DRAW_CYPHER_TILES = true;

  clear_screen();
  update_msg_and_cypher(msg, cur_cypher_index, highlight_tiles,
                        DRAW_CYPHER_TILES);
  draw_init_tiles();
}

/**
 * @brief The task that contains the game loop
 *
 * @param pvParameters UNUSED
 */
void task_hw05_system_control(void *pvParameters) {
  (void)pvParameters;

  EventBits_t event;

  // Game State Variables
  game_state_t game_state = STATE_INIT;
  bool goes_first;

  // UI Variables
  char cur_msg[50] = {0};
  uint32_t cypher_index = 0;
  bool highlight_cypher_tiles = true;
  touch_coords_t coords;
  uint8_t prev_selected_tile = INVALID_SELECTION;
  uint8_t selected_tile;

  // IPC Variables
  uint16_t inactive_payload;
  uint32_t encoded_guess;
  uint8_t decoded_guess[4];
  uint8_t exact_match_cnt;
  uint8_t digit_match_cnt;
  uint8_t opp_exact_match_cnt;
  uint8_t opp_digit_match_cnt;
  uint16_t sequence_num = 0;

  while (1) {
    event = xEventGroupWaitBits(ECE353_RTOS_Events, LIGHT_SENSOR_UPDATED,
                                pdTRUE, pdFALSE, NO_TIMEOUT);
    if (event & LIGHT_SENSOR_UPDATED) {
      update_ui(cur_msg, cypher_index, highlight_cypher_tiles);
    }

    // wait for discover message (other board reset), if received go back to
    // init state
    event = xEventGroupWaitBits(ECE353_RTOS_Events, IPC_DISCOVERY_RECEIVED,
                                pdTRUE, pdFALSE, NO_TIMEOUT);
    if (event & IPC_DISCOVERY_RECEIVED) {
      game_state = STATE_INIT;
    }

    switch (game_state) {
    case STATE_INIT:
      exact_match_cnt = 0;
      digit_match_cnt = 0;
      my_guess_cnt = 0;
      get_init_eeprom(cur_msg);
      display_init_ui(cur_msg);
      discover_board(&sequence_num, &goes_first);
      xEventGroupClearBits(ECE353_RTOS_Events, IPC_DISCOVERY_RECEIVED);

      game_state = STATE_ENTER_CYPHER;
      break;
    case STATE_ENTER_CYPHER:
      event = xEventGroupWaitBits(ECE353_RTOS_Events, SW1_PRESSED, pdTRUE,
                                  pdFALSE, NO_TIMEOUT);
      if (event & SW1_PRESSED) {
        game_state = STATE_WAIT_OPP_READY;
        highlight_cypher_tiles = false;
        snprintf(cur_msg, sizeof(cur_msg), "Waiting for Opponent...         ");
        update_msg_and_cypher(cur_msg, cypher_index, highlight_cypher_tiles,
                              true);
        break;
      }

      event = xEventGroupWaitBits(ECE353_RTOS_Events, SW3_PRESSED, pdTRUE,
                                  pdFALSE, NO_TIMEOUT);
      if (event & SW3_PRESSED && hi_score != RESET_SCORE) {
        hi_score = RESET_SCORE;
        system_sensors_eeprom_write(xQueue_Response_I2C, HI_SCORE_ADDRESS,
                                    hi_score);
        snprintf(cur_msg, 50, "Select Cypher: HiScr: N/A         ");
        print_message(cur_msg);
      }

      xTaskNotifyGive(xHandleCapTouch);
      if (xQueueReceive(Queue_Cap_Touch_Coords, &coords, NO_TIMEOUT) != pdTRUE)
        break;

      // When no touch is detected
      if (coords.x == UINT16_MAX && coords.y == UINT16_MAX) {
        prev_selected_tile = INVALID_SELECTION;
        break;
      }

      if (select_cypher_num(coords, &selected_tile)) {
        if (selected_tile != prev_selected_tile) {
          // Current Selected Tile is a Cypher Tile
          if (selected_tile >= 8) {
            cypher_index = selected_tile - 8;
            draw_cypher_tiles(cypher_index, highlight_cypher_tiles, true);
          } else {
            // Current Selected Tile is a Number Tile
            if (cypher_index >= 0 && cypher_index < 4) {
              selected_cypher[cypher_index] = selected_tile;
              cypher_index = (cypher_index + 1) % 4;
              draw_cypher_tiles(cypher_index, highlight_cypher_tiles, true);
            }
          }
        }

        prev_selected_tile = selected_tile;
      } else {
        prev_selected_tile = INVALID_SELECTION;
      }

      break;

    case STATE_WAIT_OPP_READY:
      ipc_send_rdy(sequence_num++);
      ipc_wait_for_ack(pdMS_TO_TICKS(TIMEOUT));

      event = xEventGroupWaitBits(ECE353_RTOS_Events, IPC_RDY_RECEIVED, pdTRUE,
                                  pdFALSE, NO_TIMEOUT);
      if ((event & IPC_RDY_RECEIVED) != IPC_RDY_RECEIVED)
        break;

      cypher_index = 0;
      memset(guess, 0, sizeof(guess));

      if (goes_first) {
        game_state = STATE_MY_TURN;
        highlight_cypher_tiles = true;
        snprintf(cur_msg, sizeof(cur_msg), "My Turn: Exact-0 Digit-0         ");
        update_msg_and_cypher(cur_msg, cypher_index, highlight_cypher_tiles,
                              false);
      } else {
        game_state = STATE_OPP_TURN;
        highlight_cypher_tiles = false;
        snprintf(cur_msg, sizeof(cur_msg),
                 "Opp Turn: Exact-0 Digit-0         ");
        update_msg_and_cypher(cur_msg, cypher_index, highlight_cypher_tiles,
                              false);
      }

      break;

    case STATE_MY_TURN:
      event = xEventGroupWaitBits(ECE353_RTOS_Events, SW1_PRESSED, pdTRUE,
                                  pdFALSE, NO_TIMEOUT);
      if (event & SW1_PRESSED) {
        uint32_t encoded_guess =
            (guess[0] << 24) | (guess[1] << 16) | (guess[2] << 8) | guess[3];
        ipc_send_active_player(sequence_num++, encoded_guess);
        ipc_wait_for_ack(pdMS_TO_TICKS(TIMEOUT));
        ++my_guess_cnt;
        game_state = STATE_WAIT_FEEDBACK;
        break;
      }

      xTaskNotifyGive(xHandleCapTouch);
      if (xQueueReceive(Queue_Cap_Touch_Coords, &coords, pdMS_TO_TICKS(100)) !=
          pdTRUE)
        break;

      if (coords.x == UINT16_MAX && coords.y == UINT16_MAX) {
        prev_selected_tile = INVALID_SELECTION;
        break;
      }

      if (select_cypher_num(coords, &selected_tile)) {
        if (selected_tile != prev_selected_tile) {
          // Current Selected Tile is a Cypher Tile
          if (selected_tile >= 8) {
            cypher_index = selected_tile - 8;
            draw_cypher_tiles(cypher_index, highlight_cypher_tiles, false);
          } else {
            // Current Selected Tile is a Number Tile
            if (cypher_index >= 0 && cypher_index < 4) {
              guess[cypher_index] = selected_tile;
              cypher_index = (cypher_index + 1) % 4;
              draw_cypher_tiles(cypher_index, highlight_cypher_tiles, false);
            }
          }
        }

        prev_selected_tile = selected_tile;
      } else {
        prev_selected_tile = INVALID_SELECTION;
      }

      break;

    case STATE_WAIT_FEEDBACK:
      event =
          xEventGroupWaitBits(ECE353_RTOS_Events, IPC_GUESS_RESPONSE_RECEIVED,
                              pdTRUE, pdFALSE, NO_TIMEOUT);
      if ((event & IPC_GUESS_RESPONSE_RECEIVED) != IPC_GUESS_RESPONSE_RECEIVED)
        break;

      xQueueReceive(Queue_Guess_Response_Received, &inactive_payload,
                    pdMS_TO_TICKS(100));
      exact_match_cnt = (inactive_payload >> 8) & 0xFF;
      digit_match_cnt = (uint8_t)inactive_payload & 0xFF;

      highlight_cypher_tiles = false;
      cypher_index = 0;

      if (exact_match_cnt == 4) {
        snprintf(cur_msg, sizeof(cur_msg), "You Win in %d guess(es)!         ",
                 my_guess_cnt);
        print_message(cur_msg);

        if (my_guess_cnt < hi_score) {
          hi_score = my_guess_cnt;
          (void)system_sensors_eeprom_write(xQueue_Response_I2C,
                                            HI_SCORE_ADDRESS, hi_score);
        }

        game_state = STATE_GAME_OVER;
      } else {
        snprintf(cur_msg, sizeof(cur_msg),
                 "Opp Turn: Exact-%d Digit-%d        ", exact_match_cnt,
                 digit_match_cnt);
        print_message(cur_msg);
        game_state = STATE_OPP_TURN;
      }

      break;

    case STATE_OPP_TURN:
      opp_digit_match_cnt = 0;
      opp_exact_match_cnt = 0;

      event = xEventGroupWaitBits(ECE353_RTOS_Events, IPC_GUESS_RECEIVED,
                                  pdTRUE, pdFALSE, NO_TIMEOUT);
      if ((event & IPC_GUESS_RECEIVED) != IPC_GUESS_RECEIVED)
        break;

      xQueueReceive(Queue_Guess_Received, &encoded_guess, pdMS_TO_TICKS(100));

      decoded_guess[0] = (uint8_t)(encoded_guess >> 24) & 0xFF;
      decoded_guess[1] = (uint8_t)(encoded_guess >> 16) & 0xFF;
      decoded_guess[2] = (uint8_t)(encoded_guess >> 8) & 0xFF;
      decoded_guess[3] = (uint8_t)encoded_guess & 0xFF;

      for (int i = 0; i < 4; ++i) {
        if (decoded_guess[i] == selected_cypher[i]) {
          ++opp_exact_match_cnt;
        } else if (decoded_guess[i] == selected_cypher[(i + 1) % 4] ||
                   decoded_guess[i] == selected_cypher[(i + 2) % 4] ||
                   decoded_guess[i] == selected_cypher[(i + 3) % 4]) {
          ++opp_digit_match_cnt;
        }
      }

      ipc_send_inactive_player(sequence_num++, opp_exact_match_cnt,
                               opp_digit_match_cnt);
      ipc_wait_for_ack(TIMEOUT);

      if (opp_exact_match_cnt == 4) {
        uint8_t offset = goes_first ? 0 : 1;
        snprintf(cur_msg, sizeof(cur_msg), "You Lose! Opp guesses: %d        ",
                 my_guess_cnt + offset);
        print_message(cur_msg);
        game_state = STATE_GAME_OVER;
      } else {
        snprintf(cur_msg, sizeof(cur_msg), "My Turn: Exact-%d Digit-%d       ",
                 exact_match_cnt, digit_match_cnt);
        print_message(cur_msg);

        memset(guess, 0, sizeof(guess));
        highlight_cypher_tiles = true;
        draw_cypher_tiles(0, highlight_cypher_tiles, false);
        game_state = STATE_MY_TURN;
      }

      break;

    case STATE_GAME_OVER:
      event = xEventGroupWaitBits(ECE353_RTOS_Events, SW1_PRESSED, pdTRUE,
                                  pdFALSE, NO_TIMEOUT);
      if (event & SW1_PRESSED) {
        game_state = STATE_INIT;

        memset(selected_cypher, 0, sizeof(selected_cypher));
        memset(guess, 0, sizeof(guess));
        cypher_index = 0;
        my_guess_cnt = 0;
        highlight_cypher_tiles = true;

        ipc_send_rst(sequence_num++);
        ipc_wait_for_ack(pdMS_TO_TICKS(TIMEOUT));
        break;
      }

      event = xEventGroupWaitBits(ECE353_RTOS_Events, IPC_RESET_RECEIVED,
                                  pdTRUE, pdFALSE, NO_TIMEOUT);
      if (event & IPC_RESET_RECEIVED) {
        game_state = STATE_INIT;

        memset(selected_cypher, 0, sizeof(selected_cypher));
        memset(guess, 0, sizeof(guess));
        cypher_index = 0;
        my_guess_cnt = 0;

        break;
      }

      break;

    default:
      task_console_printf("Game Entered Unknown State\n\r");
      for (int i = 0; i < 100000; ++i)
        ;
      CY_ASSERT(0);
    }
  };
}

/*************************************************
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 ************************************************/
void app_init_hw(void) {
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

  I2C_Monarch_Obj = i2c_init(PIN_I2C_SDA, PIN_I2C_SCL);

  if (!I2C_Monarch_Obj) {
    printf("I2C Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  SPI_Monarch_Obj = spi_init(PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCLK);
  if (!SPI_Monarch_Obj) {
    printf("SPI Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  rslt = buttons_init_gpio();
  if (rslt != CY_RSLT_SUCCESS) {
    printf("Button Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  rslt = cyhal_gpio_init(PIN_SPI_EEPROM_CS, CYHAL_GPIO_DIR_OUTPUT,
                         CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) {
    printf("EEPROM CS Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  rslt = lcd_initialize();
  if (rslt != CY_RSLT_SUCCESS) {
    printf("LCD Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
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
  ECE353_RTOS_Events = xEventGroupCreate();

  semaphores_init();
  queues_init();

  if (!task_console_init()) {
    printf("Console Task Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  if (!task_button_init()) {
    printf("Button Task Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  if (!task_ipc_init()) {
    printf("IPC Task Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  if (!task_eeprom_resources_init(&SPI_Semaphore, SPI_Monarch_Obj,
                                  PIN_SPI_EEPROM_CS)) {
    printf("EEPROM Task Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  if (!task_cap_touch_resources_init(xQueue_Request_I2C, &I2C_Semaphore,
                                     I2C_Monarch_Obj)) {
    printf("Cap Touch Task Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  if (!task_lcd_resources_init(xQueue_Request_LCD)) {
    printf("LCD Task Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  if (!task_light_sensor_resources_init(&I2C_Semaphore, I2C_Monarch_Obj)) {
    printf("Light Sensor Task Initialization Failed\n\r");
    for (int i = 0; i < 100000; ++i)
      ;
    CY_ASSERT(0);
  }

  xTaskCreate(task_hw05_system_control, "HW05 System Control",
              TASK_SYSTEM_CONTROL_STACK_SIZE, NULL,
              TASK_SYSTEM_CONTROL_PRIORITY, NULL);

  // create polling tasks for cap touch and light sensor
  xTaskCreate(task_light_sensor_poll, "Light Sensor Polling",
              configMINIMAL_STACK_SIZE * 5, NULL, tskIDLE_PRIORITY + 1, NULL);

  xTaskCreate(task_cap_touch_poll, "Cap Touch Polling",
              configMINIMAL_STACK_SIZE * 5, NULL, tskIDLE_PRIORITY + 1,
              &xHandleCapTouch);

  /* Start the scheduler*/
  vTaskStartScheduler();

  /* Will never reach this loop once the scheduler starts */
  while (1) {
  }
}
#endif