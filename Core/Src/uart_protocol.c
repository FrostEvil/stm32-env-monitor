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

char rx_data_buffer[RX_RING_BUFFER_SIZE];
char rx_command_buffer[RX_COMMAND_BUFFER_SIZE];
uint8_t rx_head = 0;
uint8_t rx_tail = 0;
uint8_t command_index = 0;

uint8_t rx_data_overflow = 0;
uint8_t rx_command_overflow = 0;

ParseCommand_t parsed;
HAL_StatusTypeDef command_status;

HAL_StatusTypeDef SetTempMinErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					>= env_monitor->system_config.temperature.min_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.temperature.min_error = strtof(param,
	NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetTempMaxErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.temperature.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.temperature.max_error = strtof(param,
	NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetTempMinWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.temperature.min_error
			|| strtof(param, NULL)
					>= env_monitor->system_config.temperature.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.temperature.min_warning = strtof(param,
	NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetTempMaxWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.temperature.min_warning
			|| strtof(param, NULL)
					>= env_monitor->system_config.temperature.max_error) {
		return HAL_ERROR;
	}

	env_monitor->system_config.temperature.max_warning = strtof(param,
	NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetPressMinErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					>= env_monitor->system_config.pressure.min_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.pressure.min_error = strtof(param, NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetPressMaxErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.pressure.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.pressure.max_error = strtof(param, NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetPressMinWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.pressure.min_error
			|| strtof(param, NULL)
					>= env_monitor->system_config.pressure.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.pressure.min_warning = strtof(param,
	NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetPressMaxWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.pressure.min_warning
			|| strtof(param, NULL)
					>= env_monitor->system_config.pressure.max_error) {
		return HAL_ERROR;
	}

	env_monitor->system_config.pressure.max_warning = strtof(param,
	NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetHumMinErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					>= env_monitor->system_config.humidity.min_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.humidity.min_error = strtof(param, NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetHumMaxErr(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.humidity.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.humidity.max_error = strtof(param, NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetHumMinWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.humidity.min_error
			|| strtof(param, NULL)
					>= env_monitor->system_config.humidity.max_warning) {
		return HAL_ERROR;
	}

	env_monitor->system_config.humidity.min_warning = strtof(param,
	NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetHumMaxWar(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL
			|| strtof(param, NULL)
					<= env_monitor->system_config.humidity.min_warning
			|| strtof(param, NULL)
					>= env_monitor->system_config.humidity.max_error) {
		return HAL_ERROR;
	}

	env_monitor->system_config.humidity.max_warning = strtof(param,
	NULL);

	return HAL_OK;

}

HAL_StatusTypeDef SetInverseDisplay(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL) {
		return HAL_ERROR;
	}

	if (strcmp(param, "ON\r\n") == 0) {
		return SSD1306_InvertDisplay(&env_monitor->ssd,
				SSD1306_INVERSE_DISPLAY_ON);
	} else if (strcmp(param, "OFF\r\n") == 0) {
		return SSD1306_InvertDisplay(&env_monitor->ssd,
				SSD1306_INVERSE_DISPLAY_OFF);
	} else {
		return HAL_ERROR;
	}
}

HAL_StatusTypeDef SetInterval(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL || strtof(param, NULL) <= 2) {
		return HAL_ERROR;
	}

	env_monitor->system_config.measurement_interval_s = (uint8_t) strtof(param,
	NULL);

	return HAL_OK;
}

HAL_StatusTypeDef SetOsrs(char *param, Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL) {
		return HAL_ERROR;
	}

	switch ((uint8_t) strtof(param, NULL)) {
	case 0:
		BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_SKIPPED);
		return HAL_OK;
		break;
	case 1:
		BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X1);
		return HAL_OK;
		break;
	case 2:
		BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X2);
		return HAL_OK;
		break;
	case 4:
		BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X4);
		return HAL_OK;
		break;
	case 8:
		BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X8);
		return HAL_OK;
		break;
	case 16:
		BME280_SetOversampling(&env_monitor->bme, BME280_OSRS_X16);
		return HAL_OK;
		break;

	default:
		return HAL_ERROR;
	}

}

HAL_StatusTypeDef SetContrast(char *param,
		Env_Monitor_HandleTypeDef *env_monitor) {

	if (param == NULL || strtof(param, NULL) > 100) {
		return HAL_ERROR;
	}

	if(SSD1306_SetContrast(&env_monitor->ssd, (uint8_t) strtof(param, NULL)) == HAL_OK){
		return HAL_OK;
	}else{
		return HAL_ERROR;
	}

}

Command_t command_table[] = {
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
				"CMD:SET_CONTRAST:", SetContrast } };

void UART_SendMessage(UART_HandleTypeDef *huart) {
	if (command_status == HAL_OK) {
		HAL_UART_Transmit_IT(huart, (uint8_t*) "ACK\r\n", strlen("ACK\r\n"));
	} else {
		HAL_UART_Transmit_IT(huart, (uint8_t*) "ERR\r\n", strlen("ERR\r\n"));
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
		//TODO: Add rx_data_overflow handling
	}

}

void BuildCommand() {
	if (rx_head == rx_tail) {
		return;
	} // Is it neccessary?

	if (command_index >= RX_COMMAND_BUFFER_SIZE - 1) {
		rx_command_overflow = 1;
	} // Is it neccessary when command_index is increasing in while loop? and ParseCommand is executed
//i know that somewhere is '\n' and olso rx_command_buffer and rx_data_buffer has the same size

	if (rx_command_overflow != 1) {

		while (rx_data_buffer[rx_tail] != '\n') {
			rx_command_buffer[command_index] = rx_data_buffer[rx_tail];
			rx_tail = (rx_tail + 1) % RX_RING_BUFFER_SIZE;
			command_index++;
		}
		rx_command_buffer[command_index] = '\n';
		command_index++;
		rx_tail = (rx_tail + 1) % RX_RING_BUFFER_SIZE;
		rx_command_buffer[command_index] = '\0';

		command_index = 0;

	} else {
		//TODO: ADD rx_command_overflow HANDLING
	}
}

ParseCommand_t ParseCommand() {
	ParseCommand_t result = { 0 };

	for (uint8_t i = 0; i < ARRAY_SIZE(command_table); i++) {
		if (strncmp(rx_command_buffer, command_table[i].command_name,
				strlen(command_table[i].command_name)) == 0) {
			result.command = &command_table[i];
			result.param = &rx_command_buffer[strlen(
					command_table[i].command_name)];
		}
	}

	return result;
}

void ExecudeCommand(Env_Monitor_HandleTypeDef *env_monitor) {
	command_status = parsed.command->handler(parsed.param, env_monitor);
}

void UART_Task(volatile uint8_t *rx_command_ready,
		Env_Monitor_HandleTypeDef *env_monitor, UART_HandleTypeDef *huart) {
	BuildCommand();
	parsed = ParseCommand();
	ExecudeCommand(env_monitor);

	UART_SendMessage(huart);
	*rx_command_ready = 0;
}
