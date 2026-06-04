/*
 * ssd1306_internal.h
 *
 *  Created on: May 21, 2026
 *      Author: tomas
 */

#ifndef INC_SSD1306_INTERNAL_H_
#define INC_SSD1306_INTERNAL_H_

HAL_StatusTypeDef SSD1306_WriteCommand(SSD1306_HandleTypeDef *ssd,
		uint8_t command);
HAL_StatusTypeDef SSD1306_WriteCommandWithArg(SSD1306_HandleTypeDef *ssd,
		uint8_t command, uint8_t args, uint8_t args_size);
HAL_StatusTypeDef SSD1306_WriteCommandWithArgs(
		SSD1306_HandleTypeDef *ssd, uint8_t command, uint8_t *args,
		uint8_t args_size);
SSD1306_Status SSD1306_WriteData(SSD1306_HandleTypeDef *ssd);
#endif /* INC_SSD1306_INTERNAL_H_ */
