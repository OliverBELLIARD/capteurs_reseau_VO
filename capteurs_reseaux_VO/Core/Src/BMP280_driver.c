#include "main.h"
#include "i2c.h"
#include "BMP280_driver.h"


uint8_t buff[BUFF_SIZE];	// used buffer on I2C transmissions
HAL_StatusTypeDef ret; 		// I2C operations status


int BMP280_CheckID(void)
{
	buff[0] = BMP280_REG_ID;

	ret = HAL_I2C_Master_Transmit(&hi2c1,BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Transmit failure\r\n");
		return EXIT_FAILURE;
	}
	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Receive failure\r\n");
		return EXIT_FAILURE;
	}

	printf("BMP280 ID: 0x%X\r\n", buff[0]);

	return EXIT_SUCCESS;
}

int BMP280_Config(void)
{
	buff[0]= BMP280_REG_MODE;
	buff[1]= BMP280_CONFIG;

	ret = HAL_I2C_Master_Transmit(&hi2c1,BMP280_ADDR, buff, 2, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Transmit failure\r\n");
		return EXIT_FAILURE;
	}

	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Receive failure\r\n");
		return EXIT_FAILURE;
	}

	if(buff[0] == BMP280_CONFIG){
		printf("La config envoyée reçue avec succès\r\n");
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}
