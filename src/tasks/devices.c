/**
 * @file devices.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-10-27
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "devices.h"

#if defined(ECE353_FREERTOS)
#include "task_console.h"
#include "task_eeprom.h"
#include "task_imu.h"
#include "task_io_expander.h"
#include "task_light_sensor.h"
#include "task_temp_sensor.h"


/**
 * @brief Parses the EEPROM request
 *
 * @param request the EEPROM request
 * @return true if it was parsed correctly
 * @return false if it was missing parts
 */
static bool parse_eeprom(device_request_msg_t *request) {
  char *token, *endptr;
  uint16_t addr = 0;
  uint8_t value = 0;

  if ((token = strtok(NULL, " ")) == NULL)
    return false;

  if (strcmp(token, "R") == 0) {
    if ((token = strtok(NULL, " ")) == 0)
      return false;

    addr = (uint16_t)strtoul(token, &endptr, 16);
    if (addr == 0 && endptr == token)
      return false;

    request->operation = DEVICE_OP_READ;
  } else if (strcmp(token, "W") == 0) {
    if ((token = strtok(NULL, " ")) == 0)
      return false;

    addr = (uint16_t)strtoul(token, &endptr, 16);
    if (addr == 0 && endptr == token)
      return false;

    if ((token = strtok(NULL, " ")) == 0)
      return false;

    value = (uint8_t)strtoul(token, &endptr, 16);
    if (value == 0 && endptr == token)
      return false;

    request->operation = DEVICE_OP_WRITE;
  } else {
    return false;
  }

  request->device = DEVICE_EEPROM;
  request->address = addr;
  request->value = value;

  return true;
}

/**
 * @brief Creates a CAP_TOUCH request
 *
 * @param request the request
 */
static inline void parse_cap_touch(device_request_msg_t *request) {
  request->device = DEVICE_CAP_TOUCH;
  request->operation = DEVICE_OP_READ;
}

/**
 * @brief Parses the string from the CLI into an EEPROM or CAP_TOUCH request
 *
 * @param data the data from the CLI
 * @param request the type of request (EEPROM OR CAP_TOUCH)
 * @return true if it was parsed correctly
 * @return false if an unknown request was inputted
 */
bool parse_cli_data(char *data, device_request_msg_t *request) {
  char *token = NULL;
  if (data == NULL || request == NULL)
    return false;

  if (*data == '\n') {
    data++;
  }
  if ((token = strtok(data, " ")) == NULL)
    return false;

  if (strcmp(token, "EEPROM") == 0)
    return parse_eeprom(request);
  else if (strcmp(token, "CAP_TOUCH") == 0) {
    parse_cap_touch(request);
    return true;
  }

  return false;
}

#endif /* ECE353_FREERTOS */
