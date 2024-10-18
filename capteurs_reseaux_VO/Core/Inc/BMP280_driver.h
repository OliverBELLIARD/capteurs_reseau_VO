#include <stdio.h>
#include <stdlib.h>

#define BMP280_ADDR 0x77 << 1		// Doc bst-bmp280-ds001 page 28
#define BMP280_REG_ID 0xD0			// Doc bst-bmp280-ds001 page 24
#define BMP280_REG_MODE 0xF4		// Doc bst-bmp280-ds001 page 15
#define BMP280_REG_FILTER 0xF5		// Doc bst-bmp280-ds001 page 13
#define BMP280_REG_CALIBRATION 0x88	// Doc bst-bmp280-ds001 page 24, calib25 to calib00, 0x88…0xA1
#define BMP280_REG_TEMPERATURE 0xFA	// Temperature MSB register: 0xFA...0xFC. Doc bst-bmp280-ds001 page 24
#define BMP280_REG_PRESSURE 0xF7	// Pressure MSB register: 0xF7...0xF9, Doc bst-bmp280-ds001 page 24
#define BMP280_CONFIG ((0b010<<5)|(0b101<<2)|(0b11)) // Temperature oversampling x2|Pressure oversampling x16|normal mode

#define BUFF_SIZE 10


int BMP280_CheckID(void);
int BMP280_Config(void);
