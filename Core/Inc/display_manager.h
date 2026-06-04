/*
 * display_manager.h
 *
 *  Created on: May 27, 2026
 *      Author: tomas
 */

#ifndef INC_DISPLAY_MANAGER_H_
#define INC_DISPLAY_MANAGER_H_

#define X_OFFSET 80

typedef enum {
	DISPLAY_NORMAL = 0x00U,
	DISPLAY_ERROR = 0x01U,
	DISPLAY_WARNING = 0x02U,
	DISPLAY_INFO = 0x03U
} DisplayStatus_t;

 typedef struct {
	DisplayStatus_t temperature_status;
	DisplayStatus_t pressure_status;
	DisplayStatus_t humidity_status;
	uint8_t error_flag;
} MeasurementStatus_t;


HAL_StatusTypeDef DisplayStartingScreen(SSD1306_HandleTypeDef *ssd);
HAL_StatusTypeDef DisplayMeasurements(SensorData_t *sensor_data,
		volatile SystemConfig_t *system_config, SSD1306_HandleTypeDef *ssd, uint8_t *error_flag);
HAL_StatusTypeDef DisplayUpdateErrorBlink(SSD1306_HandleTypeDef *ssd, SensorData_t *sensor_data);
#endif /* INC_DISPLAY_MANAGER_H_ */

