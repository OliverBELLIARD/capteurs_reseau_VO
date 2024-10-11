/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BMP280_ADDR 0x77 << 1		// Doc bst-bmp280-ds001 page 28
#define BMP280_REG_ID 0xD0			// Doc bst-bmp280-ds001 page 24
#define BMP280_REG_MODE 0xF4		// Doc bst-bmp280-ds001 page 15
#define BMP280_REG_FILTER 0xF5		// Doc bst-bmp280-ds001 page 13
#define BMP280_REG_CALIBRATION 0x88	// Doc bst-bmp280-ds001 page 24, calib25 to calib00, 0x88…0xA1
#define BMP280_REG_TEMPERATURE 0xFA	// Temperature MSB register: 0xFA...0xFC. Doc bst-bmp280-ds001 page 24
#define BMP280_REG_PRESSURE 0xF7	// Pressure MSB register: 0xF7...0xF9, Doc bst-bmp280-ds001 page 24
#define BMP280_CONFIG ((0b010<<5)|(0b101<<2)|(0b11)) // Temperature oversampling x2|Pressure oversampling x16|normal mode

#define BUFF_SIZE 10

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t buff[BUFF_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

	return ch;
}

HAL_StatusTypeDef ret; // Status des opérations I2C

int BMP280_CheckID(void){
	buff[0] = BMP280_REG_ID;

	ret = HAL_I2C_Master_Transmit(&hi2c1,BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C failure\r\n");
		return EXIT_FAILURE;
	}
	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C failure\r\n");
		return EXIT_FAILURE;
	}

	printf("BMP280 ID: 0x%X\r\n", buff[0]);

	return EXIT_SUCCESS;
}

int BMP280_Config(void){
	buff[0]= BMP280_REG_MODE;
	buff[1]= BMP280_CONFIG;

	ret = HAL_I2C_Master_Transmit(&hi2c1,BMP280_ADDR, buff, 2, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C failure\r\n");
		return EXIT_FAILURE;
	}

	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C failure\r\n");
		return EXIT_FAILURE;
	}

	if(buff[0] == BMP280_CONFIG){
		printf("La config envoyée reçue avec succès\r\n");
		return EXIT_SUCCESS;
	}

	return 1;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

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
	/* USER CODE BEGIN 2 */
	printf("\r\n=== TP Capteurs & Reseaux ===\r\n");

	/* Identification du BMP280 */
	buff[0] = BMP280_REG_ID;
	HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);

	HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	printf("Contenu du registre 0x%02X (id) : 0x%02X\r\n", BMP280_REG_ID, buff[0]);

	/* Configuration du BMP280 */
	// Configuration du mode

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
