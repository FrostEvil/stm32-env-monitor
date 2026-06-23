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

#define X_OFFSET 80

char text_buffer[64];

static void ClearArea(SSD1306_HandleTypeDef *ssd, uint8_t x, uint8_t y, uint8_t width,
		uint8_t height) {
	SSD1306_FillRectangle(ssd, x, y, width, height, SSD1306_COLOR_BLACK);

}

static void PrintStartingScreen(SSD1306_HandleTypeDef *ssd) {
	char *text_layout[4] =
			{ "Temperature:", "Pressure:", "Humidity:", "Status:" };

	for (uint8_t i = 0; i < 4; i++) {
		SSD1306_SetCursor(ssd, 0, i * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_layout[i]);
	}
}

static void PrintMeasurement(SSD1306_HandleTypeDef *ssd, uint8_t line,
		float *measurement) {

	snprintf(text_buffer, sizeof(text_buffer), "%.1f", *measurement);
	uint8_t y = line * SSD1306_LINE_HIGHT;

	SSD1306_SetCursor(ssd, X_OFFSET, y);
	SSD1306_Print(ssd, text_buffer);
}

static void PrintStatus(SSD1306_HandleTypeDef *ssd, uint8_t line,
		AlarmStatus_t overall_status) {

	ClearArea(ssd, X_OFFSET, line * SSD1306_LINE_HIGHT,
			(SSD1306_WIDTH - X_OFFSET), SSD1306_LINE_HIGHT);

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

static void PrintInfoStatus(SSD1306_HandleTypeDef *ssd, uint8_t line) {
	SSD1306_FillRectangle(ssd, X_OFFSET, line * SSD1306_LINE_HIGHT,
			(SSD1306_WIDTH - X_OFFSET), SSD1306_LINE_HIGHT,
			SSD1306_COLOR_BLACK);

	snprintf(text_buffer, sizeof(text_buffer), "INFO");
	uint8_t y = line * SSD1306_LINE_HIGHT;

	SSD1306_SetCursor(ssd, X_OFFSET, y);
	SSD1306_Print(ssd, text_buffer);
}

static void PrintChangedParam(SSD1306_HandleTypeDef *ssd, uint8_t line,
		ChangedParam_t changed_param) {

	snprintf(text_buffer, sizeof(text_buffer), "Statement: ");
	size_t len = strlen(text_buffer);

	switch (changed_param) {
	case PARAM_TEMP_MIN_ERROR:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_temp_min_error changed! ");
		break;
	case PARAM_TEMP_MAX_ERROR:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_temp_max_error changed! ");
		break;
	case PARAM_TEMP_MIN_WARNING:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_temp_min_warning changed! ");
		break;
	case PARAM_TEMP_MAX_WARNING:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_temp_max_warning changed! ");
		break;

	case PARAM_PRESS_MIN_ERROR:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_press_min_error changed! ");
		break;
	case PARAM_PRESS_MAX_ERROR:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_press_max_error changed! ");
		break;
	case PARAM_PRESS_MIN_WARNING:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_press_min_warning changed! ");
		break;
	case PARAM_PRESS_MAX_WARNING:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_press_max_warning changed! ");
		break;

	case PARAM_HUM_MIN_ERROR:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_hum_min_error changed! ");
		break;
	case PARAM_HUM_MAX_ERROR:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_hum_max_error changed! ");
		break;
	case PARAM_HUM_MIN_WARNING:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_hum_min_warning changed! ");
		break;
	case PARAM_HUM_MAX_WARNING:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_hum_max_warning changed! ");
		break;

	case PARAM_INTERVAL:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_internal changed! ");
		break;
	case PARAM_OSRS:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_osrs changed! ");
		break;
	case PARAM_INVERSE_DISPLAY:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_inverse_display changed! ");
		break;
	case PARAM_CONTRAST:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_contrast changed! ");
		break;
	case PARAM_HYSTERESIS:
		snprintf(&text_buffer[len], sizeof(text_buffer) - len,
				"param_hysteresis changed! ");
		break;
	}

	SSD1306_SetCursor(ssd, 0, line * SSD1306_LINE_HIGHT);
	SSD1306_Print(ssd, text_buffer);
}

static HAL_StatusTypeDef PrintErrorStatement(SSD1306_HandleTypeDef *ssd, uint8_t line,
		AlarmState_t *alarm_state) {

	ClearArea(ssd, 0, 4 * SSD1306_LINE_HIGHT, SSD1306_WIDTH - 1,
			2 * SSD1306_LINE_HIGHT);
	SSD1306_UpdateScreen(ssd);

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

}

HAL_StatusTypeDef DisplayMeasurements(SensorData_t *sensor_data,
		SSD1306_HandleTypeDef *ssd, AlarmState_t *alarm_error,
		uint8_t config_changed) {

	SSD1306_UpdateArea(ssd, X_OFFSET, 0, (SSD1306_WIDTH - X_OFFSET),
			4 * SSD1306_LINE_HIGHT);

	PrintMeasurement(ssd, 0, &sensor_data->temperature);
	PrintMeasurement(ssd, 1, &sensor_data->pressure);
	PrintMeasurement(ssd, 2, &sensor_data->humidity);

	if (config_changed == 0) {

		PrintStatus(ssd, 3, alarm_error->overall_status);

		if (alarm_error->overall_status == ALARM_ERROR) {
			SSD1306_UpdateArea(ssd, 0, 4 * SSD1306_LINE_HIGHT, 128,
					2 * SSD1306_LINE_HIGHT);

			PrintErrorStatement(ssd, 4, alarm_error);
			SSD1306_UpdateScreen(ssd);
		} else {
			SSD1306_UpdateArea(ssd, 0, 4 * SSD1306_LINE_HIGHT, SSD1306_WIDTH,
					2 * SSD1306_LINE_HIGHT);
			ClearArea(ssd, 0, 4 * SSD1306_LINE_HIGHT, SSD1306_WIDTH - 1,
					2 * SSD1306_LINE_HIGHT);
			SSD1306_UpdateScreen(ssd);
		}

	}
	SSD1306_UpdateScreen(ssd);

	return HAL_OK;
}

HAL_StatusTypeDef DisplayInfo(SSD1306_HandleTypeDef *ssd, uint8_t line,
		ChangedParam_t changed_param) {

	SSD1306_UpdateArea(ssd, 0, line * SSD1306_LINE_HIGHT, SSD1306_WIDTH,
	SSD1306_HEIGHT);

	PrintInfoStatus(ssd, line);
	SSD1306_UpdateScreen(ssd);

	PrintChangedParam(ssd, line + 1, changed_param);
	SSD1306_UpdateScreen(ssd);

	return HAL_OK;
}

HAL_StatusTypeDef HideDisplayInfo(SSD1306_HandleTypeDef *ssd, uint8_t line,
		AlarmStatus_t overall_status) {
	SSD1306_UpdateArea(ssd, 0, line * SSD1306_LINE_HIGHT, SSD1306_WIDTH,
	SSD1306_HEIGHT);
	PrintStatus(ssd, 3, overall_status);

	ClearArea(ssd, 0, (line + 1) * SSD1306_LINE_HIGHT, SSD1306_WIDTH - 1,
			2 * SSD1306_LINE_HIGHT);
	SSD1306_UpdateScreen(ssd);

	return HAL_OK;
}

static HAL_StatusTypeDef PrintErrorBlink(SSD1306_HandleTypeDef *ssd, uint8_t line,
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

HAL_StatusTypeDef DisplaySettings(SSD1306_HandleTypeDef *ssd,
		volatile SystemConfig_t *system_config) {
	SSD1306_UpdateArea(ssd, 0, 0, SSD1306_WIDTH,
	SSD1306_HEIGHT);

	switch ((HAL_GetTick() / 3000) % 4) {
	case 0:
		ClearArea(ssd, 0, 0, SSD1306_WIDTH - 1, SSD1306_HEIGHT - 1);

		snprintf(text_buffer, sizeof(text_buffer), "Temperature settings");
		SSD1306_SetCursor(ssd, 0, 0);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Minimum error: %.1f",
				system_config->temperature.min_error);
		SSD1306_SetCursor(ssd, 0, SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Minimum warning: %.1f",
				system_config->temperature.min_warning);
		SSD1306_SetCursor(ssd, 0, 2 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Maximum warning: %.1f",
				system_config->temperature.max_warning);
		SSD1306_SetCursor(ssd, 0, 3 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Maximum error: %.1f",
				system_config->temperature.max_error);
		SSD1306_SetCursor(ssd, 0, 4 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		break;

	case 1:
		ClearArea(ssd, 0, 0, SSD1306_WIDTH - 1, SSD1306_HEIGHT - 1);

		snprintf(text_buffer, sizeof(text_buffer), "Pressure settings");
		SSD1306_SetCursor(ssd, 0, 0);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Minimum error: %.1f",
				system_config->pressure.min_error);
		SSD1306_SetCursor(ssd, 0, SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Minimum warning: %.1f",
				system_config->pressure.min_warning);
		SSD1306_SetCursor(ssd, 0, 2 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Maximum warning: %.1f",
				system_config->pressure.max_warning);
		SSD1306_SetCursor(ssd, 0, 3 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Maximum error: %.1f",
				system_config->pressure.max_error);
		SSD1306_SetCursor(ssd, 0, 4 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		break;

	case 2:
		ClearArea(ssd, 0, 0, SSD1306_WIDTH - 1, SSD1306_HEIGHT - 1);

		snprintf(text_buffer, sizeof(text_buffer), "Humidity settings");
		SSD1306_SetCursor(ssd, 0, 0);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Minimum error: %.1f",
				system_config->humidity.min_error);
		SSD1306_SetCursor(ssd, 0, SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Minimum warning: %.1f",
				system_config->humidity.min_warning);
		SSD1306_SetCursor(ssd, 0, 2 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Maximum warning: %.1f",
				system_config->humidity.max_warning);
		SSD1306_SetCursor(ssd, 0, 3 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Maximum error: %.1f",
				system_config->humidity.max_error);
		SSD1306_SetCursor(ssd, 0, 4 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		break;

	case 3:
		ClearArea(ssd, 0, 0, SSD1306_WIDTH - 1, SSD1306_HEIGHT - 1);

		snprintf(text_buffer, sizeof(text_buffer), "Others");
		SSD1306_SetCursor(ssd, 0, 0);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer),
				"Measurement interval (s):%lu",
				system_config->measurement_interval_s);
		SSD1306_SetCursor(ssd, 0, SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Display interval (s) :%d",
				system_config->display_interval_s);
		SSD1306_SetCursor(ssd, 0, 2 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		snprintf(text_buffer, sizeof(text_buffer), "Hysteresis: %.1f",
				system_config->alarm_hysteresis);
		SSD1306_SetCursor(ssd, 0, 3 * SSD1306_LINE_HIGHT);
		SSD1306_Print(ssd, text_buffer);

		break;
	}

	SSD1306_UpdateScreen(ssd);
	return HAL_OK;
}
