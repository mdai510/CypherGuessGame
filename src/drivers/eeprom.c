/**
 * @file eeprom.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2023-10-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "eeprom.h"
#include "cyhal_hw_types.h"
#include <sys/types.h>


/** Determine if the EEPROM is busy writing the last
 *  transaction to non-volatile storage
 *
 * @param spi_obj -- pointer to the SPI object
 * @param cs_pin  -- chip select pin for the EEPROM
 */
void eeprom_wait_for_write(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin)
{
	// ADD CODE
	uint8_t tx_buffer[2];
	uint8_t rx_buffer[2];
	tx_buffer[0] = EEPROM_CMD_RDSR; // Read Status Register command
	tx_buffer[1] = 0xFF; // dummy byte

	while(1)
	{
		//pull CS low to select the EEPROM
		cyhal_gpio_write(cs_pin, 0);

		//send command over SPI
		cyhal_spi_transfer(spi_obj, tx_buffer, 2, rx_buffer, 2, 0xFF);

		//pull CS high to deselect the EEPROM
		cyhal_gpio_write(cs_pin, 1);

		//check if WIP bit is set (bit 0 of status register)
		if((rx_buffer[1] & 0x01) == 0)
		{
			break; // WIP bit is clear, write is complete
		}
	}
}

/** Enables Writes to the EEPROM
 *
 * @param spi_obj -- pointer to the SPI object
 * @param cs_pin  -- chip select pin for the EEPROM
 *
 */
void eeprom_write_enable(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin)
{
	// ADD CODE
	uint8_t tx_buffer[2];
	tx_buffer[0] = EEPROM_CMD_WREN; // Write Enable command
	tx_buffer[1] = 0xFF; // dummy byte

	//pull CS low to select the EEPROM
	cyhal_gpio_write(cs_pin, 0);

	//send command over SPI
	cyhal_spi_transfer(spi_obj, tx_buffer, 2, NULL, 0, 0xFF);

	//pull CS high to deselect the EEPROM
	cyhal_gpio_write(cs_pin, 1);
}

/** Disable Writes to the EEPROM
 *
 * @param spi_obj -- pointer to the SPI object
 * @param cs_pin  -- chip select pin for the EEPROM
 *
 */
void eeprom_write_disable(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin)
{
	// ADD CODE
	uint8_t tx_buffer[2];
	tx_buffer[0] = EEPROM_CMD_WRDI; // Write Disable command
	tx_buffer[1] = 0xFF; // dummy byte

	//pull CS low to select the EEPROM
	cyhal_gpio_write(cs_pin, 0);

	//send command over SPI
	cyhal_spi_transfer(spi_obj, tx_buffer, 2, NULL, 0, 0xFF);

	//pull CS high to deselect the EEPROM
	cyhal_gpio_write(cs_pin, 1);
}

/** Writes a single byte to the specified address
 *
 * @param address -- 16 bit address in the EEPROM
 * @param data    -- value to write into memory
 *
 */
void eeprom_write_byte(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin, uint16_t address, uint8_t data)
{
	// ADD CODE
	uint8_t tx_buffer[4];
	tx_buffer[0] = EEPROM_CMD_WRITE; // Write command
	tx_buffer[1] = (address >> 8) & 0xFF; // high byte of address
	tx_buffer[2] = address & 0xFF; // low byte of address
	tx_buffer[3] = data; // data byte to write

	// Set the Write Enable Latch (WEL) before issuing the write command
	eeprom_write_enable(spi_obj, cs_pin);

	// pull CS low to select the EEPROM
	cyhal_gpio_write(cs_pin, 0);

	// send command and data over SPI
	cyhal_spi_transfer(spi_obj, tx_buffer, 4, NULL, 0, 0xFF);

	// pull CS high to deselect the EEPROM
	cyhal_gpio_write(cs_pin, 1);

	// Wait for the internal write cycle to complete before returning
	eeprom_wait_for_write(spi_obj, cs_pin);
}

/** Reads a single byte to the specified address
 *
 * @param address -- 16 bit address in the EEPROM
 *
 */
uint8_t eeprom_read_byte(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin, uint16_t address)
{
	// ADD CODE
	uint8_t tx_buffer[4];
	tx_buffer[0] = EEPROM_CMD_READ; // Read command
	tx_buffer[1] = (address >> 8) & 0xFF; // high byte of address
	tx_buffer[2] = address & 0xFF; // low byte of address
	tx_buffer[3] = 0xFF; // dummy byte
	uint8_t rx_buffer[4];

	// pull CS low to select the EEPROM
	cyhal_gpio_write(cs_pin, 0);

	// send command and address over SPI, read data byte
	cyhal_spi_transfer(spi_obj, tx_buffer, 4, rx_buffer, 4, 0xFF);

	// pull CS high to deselect the EEPROM
	cyhal_gpio_write(cs_pin, 1);

	return rx_buffer[3]; // return the read byte
}