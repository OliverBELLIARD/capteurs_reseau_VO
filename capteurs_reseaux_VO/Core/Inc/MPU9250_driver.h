/**
  ******************************************************************************
  * @file    MPU9250_driver.h
  * @brief   This file contains all the function prototypes for
  *          the MPU9250_driver.c file
  ******************************************************************************
  *
  *  Created on: Oct 25, 2024
  *      Author: oliver
  *      Source: https://github.com/MarkSherstan/STM32-MPU6050-MPU9250-I2C-SPI
  *
  ******************************************************************************
  */
#ifndef INC_MPU9250_DRIVER_H_
#define INC_MPU9250_DRIVER_H_

// Libs
#include <stdint.h>
#include <math.h>
#include "i2c.h"

// Constants
#define RAD2DEG 57.2957795131

// Defines
#define WHO_AM_I_6050_ANS 0x68
#define WHO_AM_I_9250_ANS 0x71
#define WHO_AM_I          0x75
#define AD0_LOW           0x68
#define AD0_HIGH          0x69
#define GYRO_CONFIG       0x1B
#define ACCEL_CONFIG      0x1C
#define PWR_MGMT_1        0x6B
#define ACCEL_XOUT_H      0x3B
#define I2C_TIMOUT_MS     1000

// Full scale ranges
enum gyroscopeFullScaleRange
{
    GFSR_250DPS,
    GFSR_500DPS,
    GFSR_1000DPS,
    GFSR_2000DPS
};

enum accelerometerFullScaleRange
{
    AFSR_2G,
    AFSR_4G,
    AFSR_8G,
    AFSR_16G
};

// Structures
extern struct RawData
{
    int16_t ax, ay, az, gx, gy, gz;
} rawData;

extern struct SensorData
{
    float ax, ay, az, gx, gy, gz;
} sensorData;

extern struct GyroCal
{
    float x, y, z;
} gyroCal;

extern struct Attitude
{
    float r, p, y;
} attitude;

// Variables
extern uint8_t _addr;
extern float _dt, _tau;
extern float aScaleFactor, gScaleFactor;

// Functions
uint8_t MPU_begin(I2C_HandleTypeDef *I2Cx, uint8_t addr, uint8_t aScale, uint8_t gScale, float tau, float dt);
void MPU_calibrateGyro(I2C_HandleTypeDef *I2Cx, uint16_t numCalPoints);
void MPU_calcAttitude(I2C_HandleTypeDef *I2Cx);
void MPU_readRawData(I2C_HandleTypeDef *I2Cx);
void MPU_readProcessedData(I2C_HandleTypeDef *I2Cx);
void MPU_writeGyroFullScaleRange(I2C_HandleTypeDef *I2Cx, uint8_t gScale);
void MPU_writeAccFullScaleRange(I2C_HandleTypeDef *I2Cx, uint8_t aScale);

#endif /* INC_MPU9250_DRIVER_H_ */
