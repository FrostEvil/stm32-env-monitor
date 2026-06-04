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

char text_buffer[64];

MeasurementStatus_t measurement_status = { .temperature_status = DISPLAY_NORMAL,
		.pressure_status = DISPLAY_NORMAL, .humidity_status = DISPLAY_NORMAL,
		.error_flag = 0 };

DisplayStatus_t display_status = DISPLAY_NORMAL;

void UpdateMeasurementStatus(float measurement,
		volatile const Thresholds_t *thresholds, DisplayStatus_t *status) {
	if (measurement <= thresholds->min_error
			|| measurement >= thresholds->max_error) {
		*status = DISPLAY_ERROR;

	} else if (measurement <= thresholds->min_warning
			|| measurement >= thresholds->max_warning) {
		*status = DISPLAY_WARNING;
	} else {
		*status = DISPLAY_NORMAL;
	}
}

void UpdateDisplayStatus() {
	if (measurement_status.temperature_status == DISPLAY_ERROR
			|| measurement_status.pressure_status == DISPLAY_ERROR
			|| measurement_status.humidity_status == DISPLAY_ERROR) {
		display_status = DISPLAY_ERROR;
		measurement_status.error_flag = 1;

	} else if (measurement_status.temperature_status == DISPLAY_WARNING
			|| measurement_status.pressure_status == DISPLAY_WARNING
			|| measurement_status.humidity_status == DISPLAY_WARNING) {
		display_status = DISPLAY_WARNING;

	} else {
		display_status = DISPLAY_NORMAL;
		measurement_status.error_flag = 0;
	}
}

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

void PrintStatus(SSD1306_HandleTypeDef *ssd, uint8_t line) {
	switch (display_status) {
	case DISPLAY_NORMAL:
		snprintf(text_buffer, sizeof(text_buffer), "NORMAL");
		break;
	case DISPLAY_ERROR:
		snprintf(text_buffer, sizeof(text_buffer), "ERROR");
		break;
	case DISPLAY_WARNING:
		snprintf(text_buffer, sizeof(text_buffer), "WARNING");
		break;
	case DISPLAY_INFO:
		snprintf(text_buffer, sizeof(text_buffer), "INFO");
		break;
	}

	uint8_t y = line * SSD1306_LINE_HIGHT;

	SSD1306_SetCursor(ssd, X_OFFSET, y);
	SSD1306_Print(ssd, text_buffer);

}

HAL_StatusTypeDef PrintStatement(SSD1306_HandleTypeDef *ssd, uint8_t line) {
	snprintf(text_buffer, sizeof(text_buffer), "Statement: ");
	size_t len = strlen(text_buffer);

	if (measurement_status.temperature_status == DISPLAY_ERROR) {
		len = strlen(text_buffer);
		snprintf(&text_buffer[len], sizeof(text_buffer) - len, "temperature ");
	}

	if (measurement_status.pressure_status == DISPLAY_ERROR) {
		len = strlen(text_buffer);
		snprintf(&text_buffer[len], sizeof(text_buffer) - len, "pressure ");
	}

	if (measurement_status.humidity_status == DISPLAY_ERROR) {
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
		volatile SystemConfig_t *system_config, SSD1306_HandleTypeDef *ssd,
		uint8_t *error_flag) {

	UpdateMeasurementStatus(sensor_data->temperature,
			&system_config->temperature,
			&measurement_status.temperature_status);
	UpdateMeasurementStatus(sensor_data->pressure, &system_config->pressure,
			&measurement_status.pressure_status);
	UpdateMeasurementStatus(sensor_data->humidity, &system_config->humidity,
			&measurement_status.humidity_status);

	UpdateDisplayStatus();

	*error_flag = measurement_status.error_flag;

	SSD1306_UpdateArea(ssd, X_OFFSET, 0, (SSD1306_WIDTH - X_OFFSET),
			4 * SSD1306_LINE_HIGHT);

	PrintMeasurement(ssd, 0, &sensor_data->temperature);
	PrintMeasurement(ssd, 1, &sensor_data->pressure);
	PrintMeasurement(ssd, 2, &sensor_data->humidity);

	PrintStatus(ssd, 3);

	SSD1306_UpdateScreen(ssd);

	if (measurement_status.error_flag == 1) {
		SSD1306_UpdateArea(ssd, 0, 4 * SSD1306_LINE_HIGHT, 128,
				2 * SSD1306_LINE_HIGHT);

		PrintStatement(ssd, 4);
		SSD1306_UpdateScreen(ssd);
	}

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
		SensorData_t *sensor_data) {

	SSD1306_UpdateArea(ssd, X_OFFSET, 0, (SSD1306_WIDTH - X_OFFSET),
			3 * SSD1306_LINE_HIGHT);

	uint8_t blink_on = ((HAL_GetTick()) / 1000) % 2;

	if (measurement_status.temperature_status == DISPLAY_ERROR) {
		PrintErrorBlink(ssd, 0, &sensor_data->temperature, &blink_on);
	}
	if (measurement_status.pressure_status == DISPLAY_ERROR) {
		PrintErrorBlink(ssd, 1, &sensor_data->pressure, &blink_on);
	}

	if (measurement_status.humidity_status == DISPLAY_ERROR) {
		PrintErrorBlink(ssd, 2, &sensor_data->humidity, &blink_on);
	}
	SSD1306_UpdateScreen(ssd);
	return HAL_OK;
}

//HAL_StatusTypeDef DisplayUpdateErrorBlink(SSD1306_HandleTypeDef *ssd,
//		SensorData_t *sensor_data) {
//
//	uint8_t blink_on = ((HAL_GetTick()) / 1000) % 2;
//
//	if (measurement_status.temperature_status == DISPLAY_ERROR) {
//		if (blink_on) {
//
//			SSD1306_FillRectangle(ssd, X_OFFSET, 0 * SSD1306_LINE_HIGHT,
//					(SSD1306_WIDTH - X_OFFSET), SSD1306_FONT_HEIGHT,
//					SSD1306_COLOR_BLACK);
//		} else {
//			PrintMeasurement(ssd, 0, &sensor_data->temperature);
//		}
//	}
//	if (measurement_status.pressure_status == DISPLAY_ERROR) {
//		if (blink_on) {
//			SSD1306_FillRectangle(ssd, X_OFFSET, 1 * SSD1306_LINE_HIGHT,
//					(SSD1306_WIDTH - X_OFFSET), SSD1306_FONT_HEIGHT,
//					SSD1306_COLOR_BLACK);
//		} else {
//			PrintMeasurement(ssd, 1, &sensor_data->pressure);
//		}
//	}
//
//	if (measurement_status.humidity_status == DISPLAY_ERROR) {
//		if (blink_on) {
//			SSD1306_FillRectangle(ssd, X_OFFSET, 2 * SSD1306_LINE_HIGHT,
//					(SSD1306_WIDTH - X_OFFSET), SSD1306_FONT_HEIGHT,
//					SSD1306_COLOR_BLACK);
//		} else {
//			PrintMeasurement(ssd, 2, &sensor_data->humidity);
//		}
//	}
//
//	SSD1306_UpdateScreen(ssd);
//
//	return HAL_OK;
//
//}
