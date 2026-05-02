#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>
#include <stdbool.h>

#include "cy_pdl.h"
#include "cy_result.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cyhal_hw_types.h"
#include "cyhal_spi.h"
#include "cyhal_gpio.h"
#include "pins.h"

/* Macros */
#define SPI_FREQ			10000000 // 10 MHz


/* Public Global Variables */
extern cyhal_spi_t mSPI;

/**
 * @brief Initialize the SPI interface used on the board.
 *
 * @return cyhal_spi_t* Returns a pointer to the initialized SPI object.
 *         NULL if initialization failed.
 */
cyhal_spi_t * spi_init(
    cyhal_gpio_t mosi,
    cyhal_gpio_t miso,
    cyhal_gpio_t clk
);

#endif // __SPI_H__ 