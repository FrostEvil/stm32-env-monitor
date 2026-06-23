/*
 * system_config.h
 *
 *  Created on: May 27, 2026
 *      Author: tomas
 */

#ifndef INC_SYSTEM_CONFIG_H_
#define INC_SYSTEM_CONFIG_H_

typedef struct {
	float min_error;
	float max_error;
	float min_warning;
	float max_warning;
} Thresholds_t;

typedef struct {
	Thresholds_t temperature;
	Thresholds_t pressure;
	Thresholds_t humidity;
	uint32_t measurement_interval_s;
	uint8_t display_interval_s;
	float alarm_hysteresis;
} SystemConfig_t;

typedef enum {
	PARAM_TEMP_MIN_ERROR = 0x00U,
	PARAM_TEMP_MAX_ERROR = 0x01U,
	PARAM_TEMP_MIN_WARNING = 0x02U,
	PARAM_TEMP_MAX_WARNING = 0x03U,

	PARAM_PRESS_MIN_ERROR = 0x04U,
	PARAM_PRESS_MAX_ERROR = 0x05U,
	PARAM_PRESS_MIN_WARNING = 0x06U,
	PARAM_PRESS_MAX_WARNING = 0x07U,

	PARAM_HUM_MIN_ERROR = 0x08U,
	PARAM_HUM_MAX_ERROR = 0x09U,
	PARAM_HUM_MIN_WARNING = 0x0AU,
	PARAM_HUM_MAX_WARNING = 0x0BU,

	PARAM_INTERVAL = 0X0CU,
	PARAM_OSRS = 0X0DU,
	PARAM_INVERSE_DISPLAY = 0X0EU,
	PARAM_CONTRAST = 0X0FU,
	PARAM_HYSTERESIS = 0X10U,
} ChangedParam_t;

#endif /* INC_SYSTEM_CONFIG_H_ */
