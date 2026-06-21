/*
 * alarm.c
 *
 *  Created on: Jun 17, 2026
 *      Author: tomas
 */

#include "alarm.h"


uint8_t buzzer_on_flag = 1;

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

void UpdateAlarmIndicators(AlarmStatus_t overall_status) {

	switch (overall_status) {
	case ALARM_ERROR:
		HAL_GPIO_WritePin(Red_LED_GPIO_Port, Red_LED_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, GPIO_PIN_RESET);

		if (buzzer_on_flag) {
			HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
		}
		break;

	case ALARM_WARNING:
		HAL_GPIO_WritePin(Red_LED_GPIO_Port, Red_LED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
		buzzer_on_flag = 1;
		break;

	default:
		HAL_GPIO_WritePin(Red_LED_GPIO_Port, Red_LED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, GPIO_PIN_SET);

		HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
		buzzer_on_flag = 1;
	}
}

void BuzzerOff() {

	HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
	buzzer_on_flag = 0;

}
