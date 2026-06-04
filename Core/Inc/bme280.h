/*
 * bme280.h
 *
 *  Created on: May 24, 2026
 *      Author: tomas
 */

#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

// Rejestry - prefiks REG
#define BME280_REG_ID           0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_DATA_START   0xF7
#define BME280_REG_CALIB1_START 0x88
#define BME280_REG_CALIB2_START 0xE1

// Maski bitów - prefiks MASK lub BIT
#define BME280_STATUS_MEASURING  0x08
#define BME280_STATUS_IM_UPDATE  0x01
#define BME280_CHIP_ID           0x60

#define BME280_I2C_ADDR_LOW (0x76 << 1)
#define BME280_I2C_ADDR_HIGH (0x77 << 1)

typedef int32_t BME280_S32_t;

typedef struct {
	// temp
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;

	// pressure
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;

	// humidity
	uint8_t dig_H1;
	int16_t dig_H2;
	uint8_t dig_H3;
	int16_t dig_H4;
	int16_t dig_H5;
	int8_t dig_H6;
} BME280_CalibData;

typedef enum {
	BME280_SLEEP_MODE = 0x00U,
	BME280_FORCED_MODE = 0x01U,
	BME280_NORMAL_MODE = 0x03U
} BME280_Mode;

typedef enum {
	BME280_OSRS_SKIPPED = 0x00U,
	BME280_OSRS_X1 = 0x01U,
	BME280_OSRS_X2 = 0x02U,
	BME280_OSRS_X4 = 0x03U,
	BME280_OSRS_X8 = 0x04U,
	BME280_OSRS_X16 = 0X05U
} BME280_Oversampling;

// BME280 device descriptor
// Holds configuration, calibration data and runtime state
typedef struct {
	I2C_HandleTypeDef *hi2c;
	uint8_t address;

	BME280_Oversampling osrs;
	BME280_Mode mode;

	BME280_CalibData calib_data;
	BME280_S32_t t_fine;

} BME280_HandleTypeDef;

HAL_StatusTypeDef BME280_Init(BME280_HandleTypeDef *bme);
HAL_StatusTypeDef BME280_ReadMeasurements(BME280_HandleTypeDef *bme,
		float *temp, float *pressure, float *humidity);
HAL_StatusTypeDef BME280_SetOversampling(BME280_HandleTypeDef *bme,
		BME280_Oversampling osrs);
HAL_StatusTypeDef BME280_TriggerForcedMeasurement(BME280_HandleTypeDef *bme);
HAL_StatusTypeDef BME280_ReadConfig(BME280_HandleTypeDef *bme);
HAL_StatusTypeDef BME280_IsMeasurementReady(BME280_HandleTypeDef *bme,
		uint8_t *ready);

#endif
