/*
 * ssd1306_lowlevel.c
 *
 *  Created on: May 21, 2026
 *      Author: tomas
 */

#include "ssd1306.h"
#include "ssd1306_defs.h"
#include <string.h>

// Transmission buffer.
// First byte contains SSD1306 data/control prefix.
static uint8_t display_data_buffer[SSD1306_DISPLAY_BUFFER_SIZE];

HAL_StatusTypeDef SSD1306_WriteCommand(SSD1306_HandleTypeDef *ssd,
		uint8_t command) {
	HAL_StatusTypeDef transmit_status;
	uint8_t buffer[2] = { SSD1306_COMMAND_TYPE, command };

	transmit_status = HAL_I2C_Master_Transmit(ssd->hi2c, ssd->address, buffer,
			2, 100);

	return transmit_status;
}

HAL_StatusTypeDef SSD1306_WriteCommandWithArg(SSD1306_HandleTypeDef *ssd,
		uint8_t command, uint8_t arg) {
	HAL_StatusTypeDef transmit_status;
	uint8_t buffer[3];

	buffer[0] = SSD1306_COMMAND_TYPE;
	buffer[1] = command;
	memcpy(&buffer[2], &arg, 1);

	transmit_status = HAL_I2C_Master_Transmit(ssd->hi2c, ssd->address, buffer,
			3, 100);

	return transmit_status;
}

HAL_StatusTypeDef SSD1306_WriteCommandWithArgs(SSD1306_HandleTypeDef *ssd,
		uint8_t command, uint8_t *args, uint8_t args_size) {
	HAL_StatusTypeDef transmit_status;

	uint8_t buffer[10];

	if (args_size > 8) {
		return HAL_ERROR;
	}

	buffer[0] = SSD1306_COMMAND_TYPE;
	buffer[1] = command;
	memcpy(&buffer[2], args, args_size);
	transmit_status = HAL_I2C_Master_Transmit(ssd->hi2c, ssd->address, buffer,
			args_size + 2, 100);

	return transmit_status;
}

// Sends framebuffer content to display using page addressing.
SSD1306_Status SSD1306_WriteData(SSD1306_HandleTypeDef *ssd) {

	// Calculate first framebuffer byte for selected update area.
	uint16_t start_index = ssd->page_address_start * SSD1306_WIDTH
			+ ssd->column_address_start;
	uint8_t args_size = ssd->column_address_end - ssd->column_address_start + 1;
	uint8_t pages = ssd->page_address_end - ssd->page_address_start + 1;

	display_data_buffer[0] = SSD1306_DATA_TYPE;

	uint8_t column_args[2] = { ssd->column_address_start,
			ssd->column_address_end };

	if (SSD1306_WriteCommandWithArgs(ssd, SSD1306_COLUMN_ADDRESS, column_args,
			2) != HAL_OK) {
		return SSD1306_ERROR;
	}

	// Send framebuffer page by page.
	for (uint8_t i = 0; i < pages; i++) {
		uint8_t page_args[2] = { ssd->page_address_start + i,
				ssd->page_address_start + i };

		if (SSD1306_WriteCommandWithArgs(ssd, SSD1306_PAGE_ADDRESS, page_args,
				2) != HAL_OK) {
			return SSD1306_ERROR;
		}

		memcpy(&display_data_buffer[1], &ssd->buffer[start_index], args_size);

		if (HAL_I2C_Master_Transmit(ssd->hi2c, ssd->address,
				display_data_buffer, args_size + 1, 100) != HAL_OK) {
			return SSD1306_ERROR;
		}
		start_index += SSD1306_WIDTH;
	}

	ssd->column_address_start = SSD1306_COLUMN_START_ADDRESS_DEFAULT;
	ssd->column_address_end = SSD1306_COLUMN_END_ADDRESS_DEFAULT;

	ssd->page_address_start = SSD1306_PAGE_ADDRESS_START_DEFAULT;
	ssd->page_address_end = SSD1306_PAGE_ADDRESS_END_DEFAULT;

	return SSD1306_OK;

}

//COMMAND: Contrast x
HAL_StatusTypeDef SSD1306_SetContrast(SSD1306_HandleTypeDef *ssd,
		uint8_t contrast) {
	if (contrast > 100) {
		return HAL_ERROR;
	}

	uint8_t contrast_control = (255 * contrast + 50) / 100;
	HAL_StatusTypeDef contrast_control_tx_status;

	contrast_control_tx_status = SSD1306_WriteCommandWithArg(ssd,
	SSD1306_CONTRAST_CONTROL, contrast_control);

	if (contrast_control_tx_status != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;

}

//COMMAND: Inverse ON/ Inverse OFF
HAL_StatusTypeDef SSD1306_InvertDisplay(SSD1306_HandleTypeDef *ssd,
		SSD1306_InverseDisplay inverse_display) {
	HAL_StatusTypeDef inverse_display_tx_status;

	inverse_display_tx_status = SSD1306_WriteCommand(ssd, inverse_display);

	if (inverse_display_tx_status != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}
