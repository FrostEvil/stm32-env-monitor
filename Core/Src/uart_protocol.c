/*
 * uart_protocol.c
 *
 *  Created on: Jun 10, 2026
 *      Author: tomas
 */

#include "uart_protocol.h"
#include <stdint.h>
#include <string.h>
#include "usart.h"
#include <stdlib.h>
#include "system_config.h"
#include "ssd1306.h"
#include "bme280.h"

static char rx_data_buffer[RX_RING_BUFFER_SIZE];
static char rx_command_buffer[RX_COMMAND_BUFFER_SIZE];
static uint8_t rx_head = 0;
static uint8_t rx_tail = 0;
static uint8_t command_index = 0;

static uint8_t rx_data_overflow = 0;
static uint8_t rx_command_overflow = 0;

static ParseCommand_t parsed;

static HAL_StatusTypeDef SetTempMinErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param
					>= env_monitor->system_config.temperature.min_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.temperature.min_error = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetTempMaxErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param
					<= env_monitor->system_config.temperature.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.temperature.max_error = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetTempMinWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param <= env_monitor->system_config.temperature.min_error
			|| *parsed_param
					>= env_monitor->system_config.temperature.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.temperature.min_warning = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetTempMaxWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param
					<= env_monitor->system_config.temperature.min_warning
			|| *parsed_param
					>= env_monitor->system_config.temperature.max_error) {
		return HAL_ERROR;
	}

	env_monitor->system_config.temperature.max_warning = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetPressMinErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param
					>= env_monitor->system_config.pressure.min_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.pressure.min_error = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetPressMaxErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param
					<= env_monitor->system_config.pressure.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.pressure.max_error = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetPressMinWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param <= env_monitor->system_config.pressure.min_error
			|| *parsed_param
					>= env_monitor->system_config.pressure.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.pressure.min_warning = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetPressMaxWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param <= env_monitor->system_config.pressure.min_warning
			|| *parsed_param >= env_monitor->system_config.pressure.max_error) {
		return HAL_ERROR;
	}

	env_monitor->system_config.pressure.max_warning = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetHumMinErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param
					>= env_monitor->system_config.humidity.min_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.humidity.min_error = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetHumMaxErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param
					<= env_monitor->system_config.humidity.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.humidity.max_error = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetHumMinWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param <= env_monitor->system_config.humidity.min_error
			|| *parsed_param
					>= env_monitor->system_config.humidity.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.humidity.min_warning = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetHumMaxWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL
			|| *parsed_param <= env_monitor->system_config.humidity.min_warning
			|| *parsed_param >= env_monitor->system_config.humidity.max_error) {
		return HAL_ERROR;
	}

	env_monitor->system_config.humidity.max_warning = *parsed_param;

	return HAL_OK;

}

static HAL_StatusTypeDef SetInverseDisplay(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {
	(void) parsed_param;

	if (param == NULL) {
		return HAL_ERROR;
	}

	if (strcmp(param, "ON") == 0) {
		return SSD1306_InvertDisplay(&env_monitor->ssd,
				SSD1306_INVERSE_DISPLAY_ON);
	} else if (strcmp(param, "OFF") == 0) {
		return SSD1306_InvertDisplay(&env_monitor->ssd,
				SSD1306_INVERSE_DISPLAY_OFF);
	} else {
		return HAL_ERROR;
	}
}

static HAL_StatusTypeDef SetInterval(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL || *parsed_param <= 2) {
		return HAL_ERROR;
	}

	env_monitor->system_config.measurement_interval_s =
			(uint32_t) *parsed_param;

	return HAL_OK;
}

static HAL_StatusTypeDef SetOsrs(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL) {
		return HAL_ERROR;
	}

	switch ((uint8_t) *parsed_param) {
	case 0:
		return BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_SKIPPED);

	case 1:
		return BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X1);

	case 2:
		return BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X2);

	case 4:
		return BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X4);

	case 8:
		return BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X8);

	case 16:
		return BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X16);

	default:
		return HAL_ERROR;
	}

}

static HAL_StatusTypeDef SetContrast(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {

	if (param == NULL || *parsed_param > 100) {
		return HAL_ERROR;
	}

	return SSD1306_SetContrast(&env_monitor->ssd, (uint8_t) *parsed_param);
}

static HAL_StatusTypeDef SetHysteresis(char *param,
		Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param) {
	if (param == NULL || *parsed_param <= 0 || *parsed_param > 5) {
		return HAL_ERROR;
	}

	if (*parsed_param
			>= (env_monitor->system_config.temperature.max_error
					- env_monitor->system_config.temperature.min_error) / 2
			|| *parsed_param
					>= (env_monitor->system_config.temperature.max_warning
							- env_monitor->system_config.temperature.min_warning)
							/ 2
			|| *parsed_param
					>= (env_monitor->system_config.pressure.max_error
							- env_monitor->system_config.pressure.min_error) / 2
			|| *parsed_param
					>= (env_monitor->system_config.pressure.max_warning
							- env_monitor->system_config.pressure.min_warning)
							/ 2
			|| *parsed_param
					>= (env_monitor->system_config.humidity.max_error
							- env_monitor->system_config.humidity.min_error) / 2
			|| *parsed_param
					>= (env_monitor->system_config.humidity.max_warning
							- env_monitor->system_config.humidity.min_warning)
							/ 2) {
		return HAL_ERROR;
	}

	env_monitor->system_config.alarm_hysteresis = *parsed_param;

	return HAL_OK;
}

static Command_t command_table[] = {
		{ "CMD:SET_TEMPERATURE_MIN_ERROR:", SetTempMinErr }, {
				"CMD:SET_TEMPERATURE_MAX_ERROR:", SetTempMaxErr }, {
				"CMD:SET_TEMPERATURE_MIN_WARNING:", SetTempMinWar }, {
				"CMD:SET_TEMPERATURE_MAX_WARNING:", SetTempMaxWar },

		{ "CMD:SET_PRESSURE_MIN_ERROR:", SetPressMinErr }, {
				"CMD:SET_PRESSURE_MAX_ERROR:", SetPressMaxErr }, {
				"CMD:SET_PRESSURE_MIN_WARNING:", SetPressMinWar }, {
				"CMD:SET_PRESSURE_MAX_WARNING:", SetPressMaxWar },

		{ "CMD:SET_HUMIDITY_MIN_ERROR:", SetHumMinErr }, {
				"CMD:SET_HUMIDITY_MAX_ERROR:", SetHumMaxErr }, {
				"CMD:SET_HUMIDITY_MIN_WARNING:", SetHumMinWar }, {
				"CMD:SET_HUMIDITY_MAX_WARNING:", SetHumMaxWar },

		{ "CMD:SET_INTERVAL:", SetInterval }, { "CMD:SET_OSRS:", SetOsrs }, {
				"CMD:SET_INVERSE_DISPLAY:", SetInverseDisplay }, {
				"CMD:SET_CONTRAST:", SetContrast }, { "CMD:SET_HYSTERESIS:",
				SetHysteresis } };

void UART_SendMessage(UART_HandleTypeDef *huart,
		HAL_StatusTypeDef *command_status, volatile uint8_t *tx_busy,
		uint8_t *config_changed) {
	if (parsed.command == NULL) {
		if (*tx_busy == 0) {
			HAL_UART_Transmit_IT(huart, (uint8_t*) "UNKNOWN COMMAND\r\n",
					strlen("UNKNOWN COMMAND\r\n"));
		}
		return;
	}

	if (*command_status == HAL_OK) {
		if (*tx_busy == 0) {
			HAL_UART_Transmit_IT(huart, (uint8_t*) "ACK\r\n",
					strlen("ACK\r\n"));
			*config_changed = 1;
		}
	} else {
		if (*tx_busy == 0) {
			HAL_UART_Transmit_IT(huart, (uint8_t*) "ERR\r\n",
					strlen("ERR\r\n"));
		}
	}
}

void UART_ProcessRxData(uint8_t *rx_data, volatile uint8_t *rx_command_ready) {

	if ((rx_head + 1) % RX_RING_BUFFER_SIZE == rx_tail) {
		rx_data_overflow = 1;
	}

	if (rx_data_overflow != 1) {
		rx_data_buffer[rx_head] = *rx_data;
		rx_head = (rx_head + 1) % RX_RING_BUFFER_SIZE;

		if (*rx_data == '\n') {
			*rx_command_ready = 1;
		}

	} else {
		rx_head = 0;
		rx_tail = 0;
		*rx_command_ready = 1;
		rx_data_overflow = 0;
	}

}

static void BuildCommand() {
	if (rx_command_overflow != 1) {

		while (rx_data_buffer[rx_tail] != '\n') {

			if (rx_head == rx_tail
					|| command_index >= (RX_COMMAND_BUFFER_SIZE - 1)) {
				rx_command_overflow = 1;
				break;
			}

			if (rx_data_buffer[rx_tail] != '\r') {
				rx_command_buffer[command_index] = rx_data_buffer[rx_tail];
				command_index++;
			}
			rx_tail = (rx_tail + 1) % RX_RING_BUFFER_SIZE;

		}

		rx_tail = (rx_tail + 1) % RX_RING_BUFFER_SIZE;
		rx_command_buffer[command_index] = '\0';

		command_index = 0;

	} else {
		rx_head = 0;
		rx_tail = 0;
		command_index = 0;
		rx_command_buffer[command_index] = '\0';
		rx_command_overflow = 0;
	}
}

static ParseCommand_t ParseCommand(ChangedParam_t *changed_param) {
	ParseCommand_t result = { 0 };

	for (uint8_t i = 0; i < ARRAY_SIZE(command_table); i++) {
		if (strncmp(rx_command_buffer, command_table[i].command_name,
				strlen(command_table[i].command_name)) == 0) {
			result.command = &command_table[i];
			result.param = &rx_command_buffer[strlen(
					command_table[i].command_name)];
			*changed_param = i;
		}
	}

	return result;
}

static void ExecuteCommand(Env_Monitor_HandleTypeDef *env_monitor,
		float *parsed_param, HAL_StatusTypeDef *command_status) {
	*command_status = parsed.command->handler(parsed.param, env_monitor,
			parsed_param);
}

void UART_Task(volatile uint8_t *rx_command_ready,
		Env_Monitor_HandleTypeDef *env_monitor, UART_HandleTypeDef *huart,
		volatile uint8_t *tx_busy, uint8_t *config_changed,
		ChangedParam_t *changed_param) {
	float parsed_param = 0;
	HAL_StatusTypeDef command_status = HAL_ERROR;

	BuildCommand();
	parsed = ParseCommand(changed_param);
	if (parsed.command == NULL) {
		UART_SendMessage(huart, &command_status, tx_busy, config_changed);
	} else {
		parsed_param = strtof(parsed.param, NULL);

		ExecuteCommand(env_monitor, &parsed_param, &command_status);
		UART_SendMessage(huart, &command_status, tx_busy, config_changed);
	}

	*rx_command_ready = 0;
}
