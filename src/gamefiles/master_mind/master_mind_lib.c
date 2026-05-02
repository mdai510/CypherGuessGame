#include "master_mind_lib.h"
#include "drivers.h"
#include "main.h"


#define COLORS_INVERTED true
#define IS_CENTERED true

static uint16_t bg_color = LCD_COLOR_BLACK;
static uint16_t x_pos = 0;

/**
 * @brief Prints a character in 20pt Font to the LCD
 *
 * @param c the character to be printed
 */
static inline void print_char(char c) {
  uint16_t fg_color =
      bg_color == LCD_COLOR_BLACK ? LCD_COLOR_WHITE : LCD_COLOR_BLACK;
  const uint16_t y_pos = LCD_MARGIN;

  uint16_t index = c - Consolas_20ptFontInfo.start_char;
  FONT_CHAR_INFO info = Consolas_20ptDescriptors[index];
  const uint8_t *bitmap = Consolas_20ptBitmaps + info.offset;

  lcd_draw_image(x_pos, y_pos, info.width, info.height, bitmap, fg_color,
                 bg_color, !IS_CENTERED);

  x_pos += info.width;
}

/**
 * @brief Prints the number associated with each tile in the center of said tile
 *
 * @param number the tile-'s number
 * @param fg_color the foreground color
 * @param bg_color the background color
 * @param tile_rect the tile's rectangle object
 */
static inline void print_tile_number(uint8_t number, uint16_t fg_color,
                                     uint16_t bg_color, lcd_rect_t tile_rect) {

  FONT_CHAR_INFO info = FONT_CHAR_INFO_LARGE_NUMBERS[number];
  const uint8_t *const bitmap = FONT_NUM_LARGE_BITMAPS + info.offset;

  lcd_draw_image(tile_rect.cx, tile_rect.cy, info.width, info.height, bitmap,
                 fg_color, bg_color, IS_CENTERED);
}

/**
 * @brief Draws a tile to the LCD
 *
 * @param tile the tile to be drawn
 * @param inverted true number and tile's colors need to be swapped, else false
 * @return _Bool true if the tile was able to be drawn, otherwise false
 */
static _Bool draw_tile(lcd_tile_t tile, _Bool inverted) {
  lcd_rect_t tile_rect;
  uint16_t bg_color, fg_color;

  // bg_color == tile's color
  // fg_color == number's color
  bg_color = inverted ? tile.color_fg : tile.color_bg;
  fg_color = inverted ? tile.color_bg : tile.color_fg;

  _Bool is_proper_rect = lcd_tile_rect(&tile_rect, tile.row, tile.col);
  if (!is_proper_rect)
    return false;

  // We do not need to draw the tile to the screen if it is the same color as
  // the LCD screen, thus saving us resources
  lcd_draw_rectangle(tile_rect.cx, tile_rect.cy, tile_rect.w, tile_rect.h,
                     bg_color, IS_CENTERED);

  print_tile_number(tile.number, fg_color, bg_color, tile_rect);

  return true;
}

/**
 * @brief
 * This function will parse an LCD message and perform the appropriate action
 * @param msg
 * @return true
 * @return false
 */
bool master_mind_handle_msg(lcd_msg_t *msg) {
  switch (msg->command) {
  case LCD_CMD_PRINT_MESSAGE:
    const char *const message = msg->payload.message;
    const size_t message_len = strlen(message);
    x_pos = 0;

    for (size_t i = 0; i < message_len; ++i) {
      print_char(message[i]);
    }

    return true;
  case LCD_CMD_DRAW_TILE:
    lcd_tile_t tile = msg->payload.tile;
    return draw_tile(tile, !COLORS_INVERTED);
  case LCD_CMD_DRAW_TILE_INVERTED:
    tile = msg->payload.tile;
    return draw_tile(tile, COLORS_INVERTED);
  case LCD_CMD_CLEAR_SCREEN:
    lcd_clear_screen(msg->payload.color);
    bg_color = msg->payload.color;
    return true;
  default:
    break;
  }
  return false;
}