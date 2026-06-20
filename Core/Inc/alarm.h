/*
 * alarm.h
 *
 *  Created on: Jun 17, 2026
 *      Author: tomas
 */

#ifndef INC_ALARM_H_
#define INC_ALARM_H_

#include "main.h"
#include "system_config.h"

typedef enum {
	ALARM_NORMAL = 0x00U, ALARM_WARNING = 0x01U, ALARM_ERROR = 0x02U
} AlarmStatus_t;

typedef struct {
	AlarmStatus_t overall_status;
	AlarmStatus_t temperature_status;
	AlarmStatus_t pressure_status;
	AlarmStatus_t humidity_status;
} AlarmState_t;

void UpdateAlarmState(volatile SystemConfig_t *system_config, SensorData_t *sensor_data,
		AlarmState_t *alarm_state);
void UpdateAlarmIndicators(AlarmStatus_t overall_status);
#endif /* INC_ALARM_H_ */
