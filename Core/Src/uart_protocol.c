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
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					>= system_config->temperature.min_warning) {
		return HAL_ERROR;
	}

	system_config->temperature.min_error = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetTempMaxErr(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->temperature.max_warning) {
		return HAL_ERROR;
	}

	system_config->temperature.max_error = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetTempMinWar(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->temperature.min_error
			|| (float) strtoul(param, NULL, 10)
					>= system_config->temperature.max_warning) {
		return HAL_ERROR;
	}

	system_config->temperature.min_warning = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetTempMaxWar(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->temperature.min_warning
			|| (float) strtoul(param, NULL, 10)
					>= system_config->temperature.max_error) {
		return HAL_ERROR;
	}

	system_config->temperature.max_warning = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetPressMinErr(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					>= system_config->pressure.min_warning) {
		return HAL_ERROR;
	}

	system_config->pressure.min_error = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetPressMaxErr(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->pressure.max_warning) {
		return HAL_ERROR;
	}

	system_config->pressure.max_error = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetPressMinWar(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->pressure.min_error
			|| (float) strtoul(param, NULL, 10)
					>= system_config->pressure.max_warning) {
		return HAL_ERROR;
	}

	system_config->pressure.min_warning = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetPressMaxWar(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->pressure.min_warning
			|| (float) strtoul(param, NULL, 10)
					>= system_config->pressure.max_error) {
		return HAL_ERROR;
	}

	system_config->pressure.max_warning = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetHumMinErr(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					>= system_config->humidity.min_warning) {
		return HAL_ERROR;
	}

	system_config->humidity.min_error = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetHumMaxErr(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->humidity.max_warning) {
		return HAL_ERROR;
	}

	system_config->humidity.max_error = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetHumMinWar(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->humidity.min_error
			|| (float) strtoul(param, NULL, 10)
					>= system_config->humidity.max_warning) {
		return HAL_ERROR;
	}

	system_config->humidity.min_warning = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetHumMaxWar(char *param,
		volatile SystemConfig_t *system_config) {

	if (param == NULL
			|| (float) strtoul(param, NULL, 10)
					<= system_config->humidity.min_warning
			|| (float) strtoul(param, NULL, 10)
					>= system_config->humidity.max_error) {
		return HAL_ERROR;
	}

	system_config->humidity.max_warning = (float) strtoul(param, NULL, 10);

	return HAL_OK;

}

HAL_StatusTypeDef SetInverseDisplay(char *param,
		volatile SystemConfig_t *system_config) {
	return HAL_OK;
}

HAL_StatusTypeDef SetInterval(char *param,
		volatile SystemConfig_t *system_config) {
	return HAL_OK;
}

HAL_StatusTypeDef SetOsrs(char *param, volatile SystemConfig_t *system_config) {
	return HAL_OK;
}

HAL_StatusTypeDef SetContrast(char *param,
		volatile SystemConfig_t *system_config) {
	return HAL_OK;
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

void ExecudeCommand(volatile SystemConfig_t *system_config) {
	command_status = parsed.command->handler(parsed.param, system_config);
}

void UART_Task(volatile uint8_t *rx_command_ready,
		volatile SystemConfig_t *system_config, UART_HandleTypeDef *huart) {
	BuildCommand();
	parsed = ParseCommand();
	ExecudeCommand(system_config);

	UART_SendMessage(huart);
	*rx_command_ready = 0;
}
