/*
 * ssd1306_draw.c
 *
 *  Created on: May 27, 2026
 *      Author: tomas
 */


/*
 * ssd1306_draw.c
 *
 *  Created on: May 21, 2026
 *      Author: tomas
 */

#include <stdlib.h>
#include <ssd1306.h>

// SSD1306 framebuffer uses page layout.
// Each byte represents 8 vertical pixels.
SSD1306_Status SSD1306_DrawPixel(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, SSD1306_Color color) {

	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}

	uint8_t page = y / 8;
	uint8_t bit_index = y % 8;
	uint16_t byte_index = page * SSD1306_WIDTH + x;

	if (color == SSD1306_COLOR_WHITE) {
		ssd->buffer[byte_index] |= 0x01 << bit_index;

	} else if (color == SSD1306_COLOR_BLACK) {
		ssd->buffer[byte_index] &= ~(0x01 << bit_index);

	}

	return SSD1306_OK;
}

// Bresenham line drawing algorithm.
SSD1306_Status SSD1306_DrawLine(SSD1306_HandleTypeDef *ssd, uint8_t x1,
		uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_Color color) {

	if (x1 >= SSD1306_WIDTH || x2 >= SSD1306_WIDTH || y1 >= SSD1306_HEIGHT
			|| y2 >= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}

	int16_t dx = abs((int16_t) x2 - (int16_t) x1);
	int16_t dy = abs((int16_t) y2 - (int16_t) y1);
	int8_t step_x = 0;
	int8_t step_y = 0;
	int16_t current_x = x1;
	int16_t current_y = y1;
	int16_t error = 0;

	if (x1 <= x2) {
		step_x = 1;
	} else {
		step_x = -1;
	}

	if (y1 <= y2) {
		step_y = 1;
	} else {
		step_y = -1;
	}

	if (dx == dy) {
		for (uint16_t i = 0; i <= dx; i++) {
			SSD1306_DrawPixel(ssd, current_x, current_y, color);
			current_x += step_x;
			current_y += step_y;
		}
	} else if (dx >= dy) {
		for (uint16_t i = 0; i <= dx; i++) {
			SSD1306_DrawPixel(ssd, current_x, current_y, color);
			current_x += step_x;
			error += dy;

			if (error >= dx) {
				current_y += step_y;
				error -= dx;
			}
		}
	} else {
		for (uint16_t i = 0; i <= dy; i++) {
			SSD1306_DrawPixel(ssd, current_x, current_y, color);
			current_y += step_y;
			error += dx;

			if (error >= dy) {
				current_x += step_x;
				error -= dy;
			}
		}
	}

	return SSD1306_OK;
}

// Bresenham line drawing algorithm.
SSD1306_Status SSD1306_DrawCircle(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t r, SSD1306_Color color) {
	int16_t min_x = (int16_t) x - (int16_t) r;
	int16_t max_x = (int16_t) x + (int16_t) r;
	int16_t min_y = (int16_t) y - (int16_t) r;
	int16_t max_y = (int16_t) y + (int16_t) r;

	if (min_x < 0|| min_y < 0 || max_x >= SSD1306_WIDTH
	|| max_y >= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}

	uint8_t current_x = 0;
	uint8_t current_y = r;

	int16_t d = 3 - 2 * r;

	while (current_x <= current_y) {

		SSD1306_DrawPixel(ssd, x + current_x, y + current_y, color);

		SSD1306_DrawPixel(ssd, x - current_x, y + current_y, color);

		SSD1306_DrawPixel(ssd, x + current_x, y - current_y, color);

		SSD1306_DrawPixel(ssd, x - current_x, y - current_y, color);

		SSD1306_DrawPixel(ssd, x + current_y, y + current_x, color);

		SSD1306_DrawPixel(ssd, x - current_y, y + current_x, color);

		SSD1306_DrawPixel(ssd, x + current_y, y - current_x, color);

		SSD1306_DrawPixel(ssd, x - current_y, y - current_x, color);

		if (d < 0) {
			d = d + 4 * current_x + 6;
		} else {
			d = d + 4 * (current_x - current_y) + 10;
			current_y--;
		}
		current_x++;
	}
	return SSD1306_OK;
}

SSD1306_Status SSD1306_DrawRectangle(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t width, uint8_t height, SSD1306_Color color) {

	if (width >= SSD1306_WIDTH || height >= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}

	uint16_t x2 = x + width - 1;
	uint16_t y2 = y + height - 1;

	if (x2 >= SSD1306_WIDTH || y2 >= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}

	SSD1306_DrawLine(ssd, x, y, x2, y, color);
	SSD1306_DrawLine(ssd, x2, y, x2, y2, color);
	SSD1306_DrawLine(ssd, x2, y2, x, y2, color);
	SSD1306_DrawLine(ssd, x, y2, x, y, color);

	return SSD1306_OK;
}

SSD1306_Status SSD1306_FillRectangle(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t width, uint8_t height, SSD1306_Color color) {

	if (width >= SSD1306_WIDTH || height >= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}

	uint16_t x2 = x + width - 1;
	uint16_t y2 = y + height - 1;
	uint8_t current_y = y;

	if (x2 >= SSD1306_WIDTH || y2 >= SSD1306_HEIGHT) {
		return SSD1306_ERROR;
	}

	for (uint8_t i = 0; i < height; i++) {
		SSD1306_DrawLine(ssd, x, current_y, x2, current_y, color);
		current_y++;
	}

	return SSD1306_OK;

}

void SSD1306_FillScreen(SSD1306_HandleTypeDef *ssd, SSD1306_Color color) {
	// 0x00 -> black, 0xFF -> white
	uint8_t fill_color = 0x00;

	if (color == SSD1306_COLOR_WHITE) {
		fill_color = 0xFF;
	}

	for (uint16_t i = 0; i < SSD1306_BUFFER_SIZE; i++) {
		ssd->buffer[i] = fill_color;
	}

}
