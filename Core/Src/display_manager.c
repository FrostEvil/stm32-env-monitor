/*
 * display_manager.c
 *
 *  Created on: May 27, 2026
 *      Author: tomas
 */

#include "main.h"
#include "ssd1306.h"
#include "system_config.h"
#include <stdio.h>
#include <string.h>
#include "display_manager.h"
#include "alarm.h"

char text_buffer[64];
uint8_t first_status_update = 1;
AlarmStatus_t prev_overall_status = ALARM_NORMAL;

void PrintStartingScreen(SSD1306_HandleTypeDef *ssd) {
	char *text_layout[4] =
			{ "Temperature:", "Pressure:", "Humidity:", "Status:" };

	for (uint8_t i = 0; i < 4; i++) {
		SSD1306_SetCursor(ssd, 0, i * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_layout[i]);
	}
}

void PrintMeasurement(SSD1306_HandleTypeDef *ssd, uint8_t line,
		float *measurement) {

	snprintf(text_buffer, sizeof(text_buffer), "%.1f", *measurement);
	uint8_t y = line * SSD1306_LINE_HIGHT;

	SSD1306_SetCursor(ssd, X_OFFSET, y);
	SSD1306_Print(ssd, text_buffer);
}

void PrintStatus(SSD1306_HandleTypeDef *ssd, uint8_t line,
		AlarmStatus_t overall_status) {
	SSD1306_FillRectangle(ssd, X_OFFSET, line * SSD1306_LINE_HIGHT,
			(SSD1306_WIDTH - X_OFFSET), SSD1306_LINE_HIGHT,
			SSD1306_COLOR_BLACK);
	SSD1306_UpdateScreen(ssd);

	switch (overall_status) {
	case ALARM_ERROR:
		snprintf(text_buffer, sizeof(text_buffer), "ERROR");
		break;
	case ALARM_WARNING:
		snprintf(text_buffer, sizeof(text_buffer), "WARNING");
		break;
	default:
		snprintf(text_buffer, sizeof(text_buffer), "NORMAL");
	}

	uint8_t y = line * SSD1306_LINE_HIGHT;

	SSD1306_SetCursor(ssd, X_OFFSET, y);
	SSD1306_Print(ssd, text_buffer);

}

HAL_StatusTypeDef PrintStatement(SSD1306_HandleTypeDef *ssd, uint8_t line,
		AlarmState_t *alarm_state) {
	snprintf(text_buffer, sizeof(text_buffer), "Statement: ");
	size_t len = strlen(text_buffer);

	if (alarm_state->temperature_status == ALARM_ERROR) {
		len = strlen(text_buffer);
		snprintf(&text_buffer[len], sizeof(text_buffer) - len, "temperature ");
	}

	if (alarm_state->pressure_status == ALARM_ERROR) {
		len = strlen(text_buffer);
		snprintf(&text_buffer[len], sizeof(text_buffer) - len, "pressure ");
	}

	if (alarm_state->humidity_status == ALARM_ERROR) {
		len = strlen(text_buffer);
		snprintf(&text_buffer[len], sizeof(text_buffer) - len, "humidity ");

	}

	len = strlen(text_buffer);
	snprintf(&text_buffer[len], sizeof(text_buffer) - len, "error!");

	SSD1306_SetCursor(ssd, 0, line * SSD1306_LINE_HIGHT);
	SSD1306_Print(ssd, text_buffer);

	return HAL_OK;
}

HAL_StatusTypeDef DisplayStartingScreen(SSD1306_HandleTypeDef *ssd) {
	SSD1306_CleanData(ssd);
	PrintStartingScreen(ssd);
	SSD1306_UpdateScreen(ssd);

	return HAL_OK;
	//TODO: add new, unique TypeDefStatus structure
}

HAL_StatusTypeDef DisplayMeasurements(SensorData_t *sensor_data,
		SSD1306_HandleTypeDef *ssd, AlarmState_t *alarm_error) {

	SSD1306_UpdateArea(ssd, X_OFFSET, 0, (SSD1306_WIDTH - X_OFFSET),
			4 * SSD1306_LINE_HIGHT);

	PrintMeasurement(ssd, 0, &sensor_data->temperature);
	PrintMeasurement(ssd, 1, &sensor_data->pressure);
	PrintMeasurement(ssd, 2, &sensor_data->humidity);

	if (first_status_update == 1
			|| prev_overall_status != alarm_error->overall_status) {
		PrintStatus(ssd, 3, alarm_error->overall_status);
		first_status_update = 0;
	}

	SSD1306_UpdateScreen(ssd);

	if (alarm_error->overall_status == ALARM_ERROR) {
		SSD1306_UpdateArea(ssd, 0, 4 * SSD1306_LINE_HIGHT, 128,
				2 * SSD1306_LINE_HIGHT);

		PrintStatement(ssd, 4, alarm_error);
		SSD1306_UpdateScreen(ssd);
	} else {
		SSD1306_UpdateArea(ssd, 0, 4 * SSD1306_LINE_HIGHT, 128,
				2 * SSD1306_LINE_HIGHT);
		SSD1306_FillRectangle(ssd, 0, 4 * SSD1306_LINE_HIGHT,
		SSD1306_WIDTH - 1, 2 * SSD1306_LINE_HIGHT, SSD1306_COLOR_BLACK);
		SSD1306_UpdateScreen(ssd);
	}

	prev_overall_status = alarm_error->overall_status;

	return HAL_OK;
}

HAL_StatusTypeDef PrintErrorBlink(SSD1306_HandleTypeDef *ssd, uint8_t line,
		float *measurement, uint8_t *blink_on) {

	if (*blink_on) {
		SSD1306_FillRectangle(ssd, X_OFFSET, line * SSD1306_LINE_HIGHT,
				(SSD1306_WIDTH - X_OFFSET), SSD1306_LINE_HIGHT,
				SSD1306_COLOR_BLACK);
	} else {
		PrintMeasurement(ssd, line, measurement);
	}

	return HAL_OK;
}

HAL_StatusTypeDef DisplayUpdateErrorBlink(SSD1306_HandleTypeDef *ssd,
		SensorData_t *sensor_data, AlarmState_t *alarm_state) {

	SSD1306_UpdateArea(ssd, X_OFFSET, 0, (SSD1306_WIDTH - X_OFFSET),
			3 * SSD1306_LINE_HIGHT);

	uint8_t blink_on = ((HAL_GetTick()) / 1000) % 2;

	if (alarm_state->temperature_status == ALARM_ERROR) {
		PrintErrorBlink(ssd, 0, &sensor_data->temperature, &blink_on);
	}
	if (alarm_state->pressure_status == ALARM_ERROR) {
		PrintErrorBlink(ssd, 1, &sensor_data->pressure, &blink_on);
	}

	if (alarm_state->humidity_status == ALARM_ERROR) {
		PrintErrorBlink(ssd, 2, &sensor_data->humidity, &blink_on);
	}
	SSD1306_UpdateScreen(ssd);
	return HAL_OK;
}

