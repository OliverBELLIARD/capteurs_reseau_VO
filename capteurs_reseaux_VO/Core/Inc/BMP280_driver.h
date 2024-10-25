/*
 * MPU9250_driver.h
 *
 *  Created on: Oct 18, 2024
 *      Author: oliver
 */

#ifndef INC_BMP280_DRIVER_H_
#define INC_BMP280_DRIVER_H_

#include <stdio.h>
#include <stdlib.h>

#define BMP280_ADDR 0x77 << 1		// Doc bst-bmp280-ds001 page 28
#define BMP280_REG_ID 0xD0			// Doc bst-bmp280-ds001 page 24
#define BMP280_REG_MODE 0xF4		// Doc bst-bmp280-ds001 page 15
#define BMP280_REG_FILTER 0xF5		// Doc bst-bmp280-ds001 page 13
#define BMP280_REG_CALIBRATION 0x88	// Doc bst-bmp280-ds001 page 24, calib25 to calib00, 0x88…0xA1

#define BMP280_REG_TEMP_MSB 0xFA	// Temperature MSB register: 0xFA...0xFC. Doc bst-bmp280-ds001 page 24
#define BMP280_LEN_TEMP 3

#define BMP280_REG_PRES_MSB 0xF7	// Pressure MSB register: 0xF7...0xF9, Doc bst-bmp280-ds001 page 24
#define BMP280_LEN_PRES 3

#define BMP280_CONFIG ((0b010<<5)|(0b101<<2)|(0b11)) // Temperature oversampling x2|Pressure oversampling x16|normal mode

#define BUFF_SIZE 10


typedef uint32_t BMP280_U32_t;
typedef int32_t BMP280_S32_t;
typedef int64_t BMP280_S64_t;

int BMP280_Check_id(void);
int BMP280_Config(void);
void BMP280_calibration(void);
BMP280_S32_t BMP280_compensate_T_int32(BMP280_S32_t);
BMP280_U32_t BMP280_compensate_P_int64(BMP280_S32_t);
int BMP280_Write_Reg(uint8_t, uint8_t);
uint8_t* BMP280_Read_Reg(uint8_t, uint8_t);
BMP280_S32_t BMP280_get_temperature(void);
BMP280_S32_t BMP280_get_pressure(void);

#endif /* INC_BMP280_DRIVER_H_ */
