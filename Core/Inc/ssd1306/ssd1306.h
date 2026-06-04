/*
 * SSD1306.h
 *
 *  Created on: May 14, 2026
 *      Author: asiac
 */

#ifndef INC_SSD1306_H_
#define INC_SSD1306_H_

#include <stdint.h>
#include <stm32f4xx_hal.h>
#include <ssd1306_font.h>
#include <ssd1306_defs.h>

typedef enum {
	SSD1306_COLOR_BLACK = 0x00U, SSD1306_COLOR_WHITE = 0x01U
} SSD1306_Color;

typedef enum {
	SSD1306_OK = 0x00U, SSD1306_ERROR = 0x01U
} SSD1306_Status;

typedef enum {
	SSD1306_INVERSE_DISPLAY_OFF = 0xA6U, SSD1306_INVERSE_DISPLAY_ON = 0xA7U
} SSD1306_InverseDisplay;

// Main SSD1306 driver handle structure.
typedef struct {
	I2C_HandleTypeDef *hi2c;
	uint8_t address;

	// Framebuffer: each byte stores vertical column of 8 pixels.
	uint8_t buffer[SSD1306_BUFFER_SIZE];

	uint8_t column_address_start;
	uint8_t column_address_end;
	uint8_t page_address_start;
	uint8_t page_address_end;
	uint8_t contrast;
	SSD1306_InverseDisplay inverse_display;
	uint8_t cursor_x;
	uint8_t cursor_y;

} SSD1306_HandleTypeDef;

// init
SSD1306_Status SSD1306_Init(SSD1306_HandleTypeDef *ssd);

// display update
SSD1306_Status SSD1306_UpdateScreen(SSD1306_HandleTypeDef *ssd);
SSD1306_Status SSD1306_UpdateArea(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t width, uint8_t height);
SSD1306_Status SSD1306_CleanData(SSD1306_HandleTypeDef *ssd);

// draw
SSD1306_Status SSD1306_DrawPixel(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, SSD1306_Color color);
SSD1306_Status SSD1306_DrawLine(SSD1306_HandleTypeDef *ssd, uint8_t x1,
		uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_Color color);
SSD1306_Status SSD1306_DrawRectangle(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t width, uint8_t height, SSD1306_Color color);
SSD1306_Status SSD1306_FillRectangle(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t width, uint8_t height, SSD1306_Color color);
void SSD1306_FillScreen(SSD1306_HandleTypeDef *ssd, SSD1306_Color color);
SSD1306_Status SSD1306_DrawCircle(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t r, SSD1306_Color color);

// text
SSD1306_Status SSD1306_DrawChar(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, uint8_t c);
SSD1306_Status SSD1306_DrawString(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y, const char *buffer);
SSD1306_Status SSD1306_SetCursor(SSD1306_HandleTypeDef *ssd, uint8_t x,
		uint8_t y);
SSD1306_Status SSD1306_NewLine(SSD1306_HandleTypeDef *ssd);
SSD1306_Status SSD1306_Print(SSD1306_HandleTypeDef *ssd, const char *text);

// display control
HAL_StatusTypeDef SSD1306_SetContrast(SSD1306_HandleTypeDef *ssd,
		uint8_t contrast);
HAL_StatusTypeDef SSD1306_InvertDisplay(SSD1306_HandleTypeDef *ssd,
		SSD1306_InverseDisplay inverse_display);

#endif /* INC_SSD1306_H_ */
