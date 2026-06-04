/*
 * SSD1306.c
 *
 *  Created on: May 14, 2026
 *      Author: tomas
 */

#include "ssd1306.h"
#include "ssd1306_internal.h"
#include <ssd1306_font.h>
#include <ssd1306_defs.h>

// Clear local framebuffer without updating display.
static void SSD1306_CleanFramebuffer(SSD1306_HandleTypeDef *ssd) {
	for (uint16_t i = 0; i < SSD1306_BUFFER_SIZE; i++) {
		ssd->buffer[i] = 0x00;
	}

}

SSD1306_Status SSD1306_CleanData(SSD1306_HandleTypeDef *ssd) {
	SSD1306_CleanFramebuffer(ssd);
	return SSD1306_WriteData(ssd);
}

SSD1306_Status SSD1306_UpdateScreen(SSD1306_HandleTypeDef *ssd) {
	return SSD1306_WriteData(ssd);
}

// Configure partial display update area.
// Data is sent during next SSD1306_UpdateScreen() call.
SSD1306_Status SSD1306_UpdateArea(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t width, uint8_t height) {
	 uint16_t end_x = x + width - 1;
	    uint16_t end_y = y + height - 1;

	    if (end_x >= SSD1306_WIDTH ||
	        end_y >= SSD1306_HEIGHT) {
	        return SSD1306_ERROR;
	    }

	    ssd->page_address_start = y / 8;
	    ssd->page_address_end = end_y / 8;

	    ssd->column_address_start = x;
	    ssd->column_address_end = end_x;

	    return SSD1306_OK;
}

// Initialize SSD1306 display controller.
SSD1306_Status SSD1306_Init(SSD1306_HandleTypeDef *ssd) {

	if (SSD1306_WriteCommand(ssd,
	SSD1306_DISPLAY_OFF) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommandWithArg(ssd,
	SSD1306_DIVIDE_RATIO_AND_OSC_FREQ,
	SSD1306_CLOCK_SETTINGS_DEFAULT, 1) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommandWithArg(ssd,
	SSD1306_MULTIPLEX_RATIO,
	SSD1306_MULTIPLEX_RATIO_DEFAULT, 1) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommandWithArg(ssd,
	SSD1306_DISPLAY_OFFSET,
	SSD1306_DISPLAY_OFFSET_DEFAULT, 1) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommand(ssd, SSD1306_START_LINE) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommandWithArg(ssd,
	SSD1306_CHARGE_PUMP,
	SSD1306_CHARGE_PUMP_ENABLE, 1) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommandWithArg(ssd,
	SSD1306_MEMORY_ADDRESSING_MODE,
	SSD1306_HORIZONTAL_ADDRESSING_MODE, 1) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommand(ssd,
	SSD1306_SEGMENT_RE_MAP) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommand(ssd,
	SSD1306_COM_OUTPUT_SCAN_DIRECTION) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommandWithArg(ssd,
	SSD1306_COM_PINS_HARDWARE_CONF,
	SSD1306_COM_PINS_HARDWARE_CONF_DEFAULT, 1) != HAL_OK) {
		return SSD1306_ERROR;
	}

	ssd->contrast = SSD1306_CONTRAST_CONTROL_DEFAULT;
	if (SSD1306_WriteCommandWithArg(ssd,
	SSD1306_CONTRAST_CONTROL, SSD1306_CONTRAST_CONTROL_DEFAULT, 1) != HAL_OK) {
		return SSD1306_ERROR;
	}
	ssd->contrast = SSD1306_CONTRAST_CONTROL_DEFAULT;

	if (SSD1306_WriteCommand(ssd,
	SSD1306_ENTIRE_DISPLAY_ON) != HAL_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommand(ssd,
	SSD1306_NORMAL_DISPLAY) != HAL_OK) {
		return SSD1306_ERROR;
	}
	ssd->inverse_display = SSD1306_INVERSE_DISPLAY_OFF;

	ssd->column_address_start = SSD1306_COLUMN_START_ADDRESS_DEFAULT;
	ssd->column_address_end = SSD1306_COLUMN_END_ADDRESS_DEFAULT;
	ssd->page_address_start = SSD1306_PAGE_ADDRESS_START_DEFAULT;
	ssd->page_address_end = SSD1306_PAGE_ADDRESS_END_DEFAULT;

	SSD1306_CleanFramebuffer(ssd);

	if (SSD1306_WriteData(ssd) != SSD1306_OK) {
		return SSD1306_ERROR;
	}

	if (SSD1306_WriteCommand(ssd, SSD1306_DISPLAY_ON) != HAL_OK) {
		return SSD1306_ERROR;
	}

	ssd->cursor_x = 0;
	ssd->cursor_y = 0;

	return SSD1306_OK;

}
