/*
 * ssd1306_text.c
 *
 *  Created on: May 21, 2026
 *      Author: tomas
 */

#include <stdlib.h>
#include <string.h>
#include <ssd1306.h>

// Draw single ASCII character from font table.
SSD1306_Status SSD1306_DrawChar(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t c) {
	if (c < 32 || c > 126) {
		return SSD1306_ERROR;
	}

	const uint8_t *font_char = font[c - 32];
	SSD1306_Color color;
	uint8_t current_x = x;
	uint8_t current_y = y;
	uint8_t pixel_state;

	for (uint8_t i = 0; i < SSD1306_FONT_WIDTH; i++) {

		for (uint8_t j = 0; j < SSD1306_FONT_HEIGHT; j++) {
			pixel_state = (font_char[i] >> j) & 0x01;
			if (pixel_state == 0x01) {
				color = SSD1306_COLOR_WHITE;
			} else {
				color = SSD1306_COLOR_BLACK;
			}
			if (SSD1306_DrawPixel(ssd, current_x, current_y, color)
					== SSD1306_ERROR) {
				return SSD1306_ERROR;
			}
			current_y++;
		}
		current_y = y;
		current_x++;
	}

	return SSD1306_OK;
}

SSD1306_Status SSD1306_DrawString(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, const char *buffer) {

	if (buffer == NULL) {
		return SSD1306_ERROR;
	}

	uint8_t current_x = x;
	uint8_t current_y = y;
	size_t buffer_length = strlen(buffer);
	char current_char;

	for (size_t i = 0; i < buffer_length; i++) {
		current_char = buffer[i];

		// Handle newline character.
		if (current_char == '\n') {
			current_y += SSD1306_FONT_HEIGHT + SSD1306_LINE_SPACING;
			current_x = 0;
			continue;
		}

		if (current_y + SSD1306_FONT_HEIGHT + SSD1306_LINE_SPACING
				>= SSD1306_HEIGHT) {
			return SSD1306_ERROR;
		}

		// Move to next line if current line is full.
		if ((current_x + SSD1306_FONT_WIDTH + SSD1306_FONT_SPACING)
				>= SSD1306_WIDTH) {
			current_y += SSD1306_FONT_HEIGHT + SSD1306_LINE_SPACING;
			current_x = 0;
		}

		if (SSD1306_DrawChar(ssd, current_x, current_y, current_char)
				== SSD1306_ERROR) {
			return SSD1306_ERROR;
		}

		current_x += SSD1306_FONT_WIDTH + SSD1306_FONT_SPACING;

	}

	return SSD1306_OK;
}

SSD1306_Status SSD1306_SetCursor(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y) {
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}

	ssd->cursor_x = x;
	ssd->cursor_y = y;

	return SSD1306_OK;
}

SSD1306_Status SSD1306_NewLine(SSD1306_HandleTypeDef *ssd) {

	if (ssd->cursor_y + SSD1306_FONT_HEIGHT + SSD1306_LINE_SPACING
			>= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}
	ssd->cursor_y += SSD1306_FONT_HEIGHT + SSD1306_LINE_SPACING;
	ssd->cursor_x = 0;

	return SSD1306_OK;

}

// Print text using internal cursor position.
SSD1306_Status SSD1306_Print(SSD1306_HandleTypeDef *ssd, const char *text) {
	if (text == NULL) {
		return SSD1306_ERROR;
	}

	size_t text_length = strlen(text);

	for (size_t i = 0; i < text_length; i++) {

		if (text[i] == '\n') {
			SSD1306_NewLine(ssd);
			continue;
		}

		if (ssd->cursor_x + SSD1306_FONT_WIDTH + SSD1306_FONT_SPACING
				>= SSD1306_WIDTH) {
			if (SSD1306_NewLine(ssd) != SSD1306_OK) {
				return SSD1306_ERROR;
			}
		}
		if (SSD1306_DrawChar(ssd, ssd->cursor_x, ssd->cursor_y, text[i])
				!= SSD1306_OK) {
			return SSD1306_ERROR;
		}
		ssd->cursor_x += SSD1306_FONT_WIDTH + SSD1306_FONT_SPACING;
	}
	return SSD1306_OK;
}
