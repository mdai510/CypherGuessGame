#ifndef __CAP_TOUCH_H__
#define __CAP_TOUCH_H__

#include "cy_pdl.h"
#include "cybsp.h"
#include "cyhal.h"
#include "stdio.h"

#define FT6X06_I2C_ADDR 0x38

#define FT6X06_FOCALTECH_ID_R 0xA8
#define FT6X06_DEFAULT_ID 0x11

#define FT6X06_TD_STATUS 0x02
#define FT6X06_P1_XH 0x03
#define FT6X06_P1_XL 0x04
#define FT6X06_P1_YH 0x05
#define FT6X06_P1_YL 0x06

#endif
