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

#endif /* INC_SYSTEM_CONFIG_H_ */
