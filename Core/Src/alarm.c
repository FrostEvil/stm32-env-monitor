/*
 * alarm.c
 *
 *  Created on: Jun 17, 2026
 *      Author: tomas
 */

#include "alarm.h"
#include "system_config.h"

AlarmStatus_t UpdateSingleAlarmState(float value, volatile Thresholds_t *limits,
		float hysteresis, AlarmStatus_t current_status) {

	switch (current_status) {
	case ALARM_ERROR:
		if (value >= limits->max_error - hysteresis
				|| value <= limits->min_error + hysteresis) {
			return ALARM_ERROR;
		}
		if (value >= limits->max_warning || value <= limits->min_warning) {
			return ALARM_WARNING;
		}
		return ALARM_NORMAL;

	case ALARM_WARNING:
		if (value >= limits->max_error || value <= limits->min_error) {
			return ALARM_ERROR;
		}
		if (value >= limits->max_warning - hysteresis
				|| value <= limits->min_warning + hysteresis) {
			return ALARM_WARNING;
		}
		return ALARM_NORMAL;

	default:
		if (value >= limits->max_error || value <= limits->min_error) {
			return ALARM_ERROR;
		}
		if (value >= limits->max_warning || value <= limits->min_warning) {
			return ALARM_WARNING;
		}
		return ALARM_NORMAL;
	}

}

void UpdateAlarmState(volatile SystemConfig_t *system_config,
		SensorData_t *sensor_data, AlarmState_t *alarm_state) {
	alarm_state->temperature_status = UpdateSingleAlarmState(
			sensor_data->temperature, &system_config->temperature,
			system_config->alarm_hysteresis, alarm_state->temperature_status);

	alarm_state->pressure_status = UpdateSingleAlarmState(sensor_data->pressure,
			&system_config->pressure, system_config->alarm_hysteresis,
			alarm_state->pressure_status);

	alarm_state->humidity_status = UpdateSingleAlarmState(sensor_data->humidity,
			&system_config->humidity, system_config->alarm_hysteresis,
			alarm_state->humidity_status);

	alarm_state->overall_status = alarm_state->temperature_status;

	if (alarm_state->pressure_status > alarm_state->overall_status) {
		alarm_state->overall_status = alarm_state->pressure_status;
	}

	if (alarm_state->humidity_status > alarm_state->overall_status) {
		alarm_state->overall_status = alarm_state->humidity_status;
	}
}

