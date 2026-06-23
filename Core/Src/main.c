/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "bme280.h"
#include "ssd1306.h"
#include "system_config.h"
#include "display_manager.h"
#include <stdio.h>
#include <string.h>
#include "uart_protocol.h"
#include "alarm.h"
#include "button.h"
#include "iwdg.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

Env_Monitor_HandleTypeDef env_monitor = { 0 };
volatile AppButtons_t app_buttons = { 0 };

volatile uint8_t measurement_flag = 0;
volatile uint8_t time_counter = 0;

volatile uint8_t display_flag = 0;
volatile uint8_t display_time_counter = 0;

volatile uint8_t display_settings_time_counter = 0;
volatile uint8_t update_settings_display = 0;

char tx_sensor_data_buffer[64];
uint8_t first_measurement_flag = 0;
volatile uint8_t tx_busy = 0;

uint8_t rx_data = 0;
volatile uint8_t rx_command_ready = 0;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_I2C1_Init();
	MX_TIM10_Init();
	MX_TIM11_Init();
	MX_IWDG_Init();
	/* USER CODE BEGIN 2 */

	HAL_TIM_Base_Start_IT(&htim10);
	HAL_TIM_Base_Start_IT(&htim11);
	HAL_UART_Receive_IT(&huart2, &rx_data, 1);

	AlarmState_t alarm_state = { ALARM_NORMAL };
	ChangedParam_t changed_param = 0x00;

	env_monitor.bme.hi2c = &hi2c1;
	env_monitor.bme.address = BME280_I2C_ADDR_LOW;
	env_monitor.bme.mode = BME280_SLEEP_MODE;
	env_monitor.bme.osrs = BME280_OSRS_X1;

	env_monitor.sensor_data.temperature = 0;
	env_monitor.sensor_data.pressure = 0;
	env_monitor.sensor_data.humidity = 0;
	env_monitor.sensor_data.valid = 1; //Default 1 to call BME280_TriggerForcedMeasurement() function for the first time
	env_monitor.sensor_data.timestamp_ms = HAL_GetTick();

	env_monitor.ssd.hi2c = &hi2c1;
	env_monitor.ssd.address = SSD1306_I2C_ADDR;

	env_monitor.system_config.temperature.min_warning = -5.0f;
	env_monitor.system_config.temperature.max_warning = 30.0f;
	env_monitor.system_config.temperature.min_error = -10.0f;
	env_monitor.system_config.temperature.max_error = 35.0f;

	env_monitor.system_config.pressure.min_warning = 995.0f;
	env_monitor.system_config.pressure.max_warning = 1050.0f;
	env_monitor.system_config.pressure.min_error = 900.0f;
	env_monitor.system_config.pressure.max_error = 1100.0f;

	env_monitor.system_config.humidity.min_warning = 20.0f;
	env_monitor.system_config.humidity.max_warning = 70.0f;
	env_monitor.system_config.humidity.min_error = 10.0f;
	env_monitor.system_config.humidity.max_error = 85.0f;

	env_monitor.system_config.measurement_interval_s = 5;
	env_monitor.system_config.display_interval_s = 1;
	env_monitor.system_config.alarm_hysteresis = 2;

	uint8_t measurement_in_progress = 0;
	uint8_t config_changed = 0;
	uint8_t settings_showed = 0;
	uint8_t display_info_showed = 0;
	uint8_t init_counter = 0;
	uint8_t reset_counter = 0;

	uint32_t prev_tick_slot = 0;
	uint32_t curr_tick_slot = 0;
	uint8_t trigger_forced_counter = 0;

	uint8_t unsuccessful_measurements = 0;

	while (init_counter <= 5) {
		HAL_IWDG_Refresh(&hiwdg);

		if (BME280_Init(&env_monitor.bme) == HAL_OK
				&& SSD1306_Init(&env_monitor.ssd) == SSD1306_OK) {
			break;
		} else {
			init_counter++;
			HAL_Delay(500);
		}

	}

	if (init_counter > 5) {

		while (reset_counter <= 5) {
			HAL_IWDG_Refresh(&hiwdg);

			HAL_GPIO_WritePin(Red_LED_GPIO_Port, Red_LED_Pin,
					(HAL_GetTick() / 250) % 2);
			HAL_Delay(500);
			reset_counter++;
		}

		NVIC_SystemReset();

	}

	DisplayStartingScreen(&env_monitor.ssd);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		if (Button_IsPressed(&app_buttons.buzzer_mute_btn)) {
			BuzzerOff();
		}

		if (Button_IsPressed(&app_buttons.config_ack_btn)) {
			config_changed = 0;
			display_info_showed = 0;
			HideDisplayInfo(&env_monitor.ssd, 3, alarm_state.overall_status);
		}

		if (Button_IsPressed(&app_buttons.settings_view_btn)) {
			settings_showed = 1;
		}

		if (Button_IsPressed(&app_buttons.settings_close_btn)) {
			settings_showed = 0;
			DisplayStartingScreen(&env_monitor.ssd);
		}

		if (config_changed == 1 && display_info_showed == 0) {
			DisplayInfo(&env_monitor.ssd, 3, changed_param);
			display_info_showed = 1;
		}

		if (settings_showed == 1 && update_settings_display == 1) {
			DisplaySettings(&env_monitor.ssd, &env_monitor.system_config);
			update_settings_display = 0;
		}

		if (rx_command_ready == 1) {
			UART_Task(&rx_command_ready, &env_monitor, &huart2, &tx_busy,
					&config_changed, &changed_param);

		}

		if (first_measurement_flag && display_flag
				&& alarm_state.overall_status == ALARM_ERROR
				&& settings_showed == 0) {
			if (DisplayUpdateErrorBlink(&env_monitor.ssd,
					&env_monitor.sensor_data, &alarm_state) == HAL_OK) {
				display_flag = 0;
			}

		}

		if (measurement_flag) {

			curr_tick_slot = HAL_GetTick() / 5000;

			if (measurement_in_progress == 0) {

				if (BME280_TriggerForcedMeasurement(&env_monitor.bme)
						== HAL_OK) {
					measurement_in_progress = 1;
					trigger_forced_counter = 0;
				} else {

					HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,
							(HAL_GetTick() / 500) % 2);

					if (curr_tick_slot != prev_tick_slot) {
						trigger_forced_counter++;

						if (BME280_TriggerForcedMeasurement(&env_monitor.bme)
								== HAL_OK) {
							measurement_in_progress = 1;
							trigger_forced_counter = 0;

						}
					}
					prev_tick_slot = curr_tick_slot;
				}

				if (trigger_forced_counter >= 5) {

					NVIC_SystemReset();
				}
			}

			if (BME280_IsMeasurementReady(&env_monitor.bme,
					&env_monitor.sensor_data.valid) == HAL_OK
					&& env_monitor.sensor_data.valid) {
				if (BME280_ReadMeasurements(&env_monitor.bme,
						&env_monitor.sensor_data.temperature,
						&env_monitor.sensor_data.pressure,
						&env_monitor.sensor_data.humidity) == HAL_OK) {

					if (unsuccessful_measurements > 0) {

						if (tx_busy == 0) {
							snprintf(tx_sensor_data_buffer,
									sizeof(tx_sensor_data_buffer),
									"I2C issue, %d unsuccessful measurements!",
									unsuccessful_measurements);

							if (HAL_UART_Transmit_IT(&huart2,
									(uint8_t*) tx_sensor_data_buffer,
									strlen(tx_sensor_data_buffer)) == HAL_OK) {
								tx_busy = 1;
							}
						}

						unsuccessful_measurements = 0;
					}

					first_measurement_flag = 1;

					UpdateAlarmState(&env_monitor.system_config,
							&env_monitor.sensor_data, &alarm_state);

					if (settings_showed == 0) {
						DisplayMeasurements(&env_monitor.sensor_data,
								&env_monitor.ssd, &alarm_state, config_changed);
					}

					UpdateAlarmIndicators(alarm_state.overall_status);
					env_monitor.sensor_data.timestamp_ms = HAL_GetTick();
					snprintf(tx_sensor_data_buffer,
							sizeof(tx_sensor_data_buffer),
							"T:%.1f;H:%.1f;P:%.1f;UPT_MS:%lu;\r\n",
							env_monitor.sensor_data.temperature,
							env_monitor.sensor_data.humidity,
							env_monitor.sensor_data.pressure,
							env_monitor.sensor_data.timestamp_ms);

					if (tx_busy == 0) {
						if (HAL_UART_Transmit_IT(&huart2,
								(uint8_t*) tx_sensor_data_buffer,
								strlen(tx_sensor_data_buffer)) == HAL_OK) {
							tx_busy = 1;
							measurement_in_progress = 0;
							measurement_flag = 0;
						}
					}

				} else {
					unsuccessful_measurements++;

					if (unsuccessful_measurements >= 10) {
						NVIC_SystemReset();
					}

				}

			}
		}

		HAL_IWDG_Refresh(&hiwdg);
	}

	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	if (htim == &htim10) {

		time_counter++;
		display_time_counter++;
		display_settings_time_counter++;

		if (time_counter >= env_monitor.system_config.measurement_interval_s) {
			measurement_flag = 1;
			time_counter = 0;
		}

		if (display_time_counter
				>= env_monitor.system_config.display_interval_s) {
			display_flag = 1;
			display_time_counter = 0;
		}

		if (display_settings_time_counter >= 3) {
			update_settings_display = 1;
			display_settings_time_counter = 0;
		}
	}

	if (htim == &htim11) {
		app_buttons.buzzer_mute_btn.gpio_state = HAL_GPIO_ReadPin(
		buzzer_mute_btn_GPIO_Port, buzzer_mute_btn_Pin);
		app_buttons.config_ack_btn.gpio_state = HAL_GPIO_ReadPin(
		config_ack_btn_GPIO_Port,
		config_ack_btn_Pin);
		app_buttons.settings_view_btn.gpio_state = HAL_GPIO_ReadPin(
		settings_view_btn_GPIO_Port, settings_view_btn_Pin);
		app_buttons.settings_close_btn.gpio_state = HAL_GPIO_ReadPin(
		settings_close_btn_GPIO_Port, settings_close_btn_Pin);

		Button_Debounce(&app_buttons.buzzer_mute_btn);
		Button_Debounce(&app_buttons.config_ack_btn);
		Button_Debounce(&app_buttons.settings_view_btn);
		Button_Debounce(&app_buttons.settings_close_btn);

	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart == &huart2) {
		UART_ProcessRxData(&rx_data, &rx_command_ready);
		HAL_UART_Receive_IT(&huart2, &rx_data, 1);
	}

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart2) {
		tx_busy = 0;
	}
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
