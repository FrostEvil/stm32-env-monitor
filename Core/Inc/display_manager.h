/*
 * display_manager.h
 *
 *  Created on: May 27, 2026
 *      Author: tomas
 */

#ifndef INC_DISPLAY_MANAGER_H_
#define INC_DISPLAY_MANAGER_H_

#include "ssd1306.h"
#include "main.h"
#include "alarm.h"

HAL_StatusTypeDef DisplayStartingScreen(SSD1306_HandleTypeDef *ssd);
HAL_StatusTypeDef DisplayMeasurements(SensorData_t *sensor_data,
		SSD1306_HandleTypeDef *ssd, AlarmState_t *alarm_error,
		uint8_t config_changed);
HAL_StatusTypeDef DisplayUpdateErrorBlink(SSD1306_HandleTypeDef *ssd,
		SensorData_t *sensor_data, AlarmState_t *alarm_state);
HAL_StatusTypeDef DisplayInfo(SSD1306_HandleTypeDef *ssd, uint8_t line,
		ChangedParam_t changed_param);
HAL_StatusTypeDef DisplaySettings(SSD1306_HandleTypeDef *ssd,
		volatile SystemConfig_t *system_config);
HAL_StatusTypeDef HideDisplayInfo(SSD1306_HandleTypeDef *ssd, uint8_t line,
		AlarmStatus_t overall_status);
#endif /* INC_DISPLAY_MANAGER_H_ */

