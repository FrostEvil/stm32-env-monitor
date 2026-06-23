/*
 * uart_protocol.h
 *
 *  Created on: Jun 10, 2026
 *      Author: tomas
 */

#ifndef INC_UART_PROTOCOL_H_
#define INC_UART_PROTOCOL_H_

#include "stm32f4xx_hal.h"
#include "system_config.h"
#include "main.h"

#define RX_RING_BUFFER_SIZE 64
#define RX_COMMAND_BUFFER_SIZE 64

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef struct {
	const char *command_name;
	HAL_StatusTypeDef (*handler)(char *param,
			Env_Monitor_HandleTypeDef *env_monitor, float *parsed_param);
} Command_t;

typedef struct {
	const Command_t *command;
	char *param;
} ParseCommand_t;

void UART_ProcessRxData(uint8_t *rx_data, volatile uint8_t *rx_command_ready);
void UART_Task(volatile uint8_t *rx_command_ready,
		Env_Monitor_HandleTypeDef *env_monitor, UART_HandleTypeDef *huart,
		volatile uint8_t *tx_busy, uint8_t *config_changed,
		ChangedParam_t *changed_param);

#endif /* INC_UART_PROTOCOL_H_ */
