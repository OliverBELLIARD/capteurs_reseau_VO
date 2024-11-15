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
#include "can.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BMP280_driver.h"
#include "MPU9250_driver.h"
#include "motor_driver.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRUE  1
#define FALSE 0
#define SERIAL_BUFF_SIZE 100

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t serial_buff[SERIAL_BUFF_SIZE];

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
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

	return ch;
}

void BMP280_init()
{
	BMP280_Check_id();			// Identification du BMP280
	BMP280_Config();			// Configuration du BMP280
	BMP280_calibration();		// Mise à jour des paramètres d'étalonage

	//BMP280_get_temperature();	// Acquisition de la température
	//BMP280_get_pressure();		// Acquisition de la pression
}

void MPU9250_init()
{
	// Vérifie si l'IMU est configuré correctement et bloque si ce n'est pas le cas
	if (MPU_begin(&hi2c3, AD0_LOW, AFSR_4G, GFSR_500DPS, 0.98, 0.004) == TRUE)
	{
		printf("Centrale inertielle configurée correctement\r\n");
	}
	else
	{
		printf("ERREUR!\r\n");
	}

	// Calibre l'IMU
	printf("CALIBRATION EN COURS...\r\n");
	MPU_calibrateGyro(&hi2c3, 1500);
}

void MOT_Init()
{
	CAN_Init();
	MOT_Set_mode(MOT_MODE_ANTICLOCKWISE, 1, 1);
	MOT_Set_origin();
}

void RaspberryPI_Request(char * msg)
{
	if (!strcmp("GET_T", msg)) {
		printf("%d\r\n", (int) BMP280_get_temperature());
	}
	if (!strcmp("GET_P", msg)) {
		printf("%d\r\n", (int) BMP280_get_pressure());
	}
	if (!strcmp("SET_K", msg)) {
	}
	if (!strcmp("GET_K", msg)) {
	}
	if (!strcmp("GET_A", msg)) {
		MPU_calcAttitude(&hi2c3);
		printf("A:%.1f;%.1f;%.1f\r\n", attitude.r, attitude.p, attitude.y);
	}
	else {
		printf("Unknown request: %s\r\n", msg);
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        // Process received data
        serial_buff[Size] = '\0'; // Null-terminate the received string
        printf("Received (%d bytes): %s\r\n", Size, serial_buff);

        // Restart DMA Reception
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, serial_buff, SERIAL_BUFF_SIZE);
    }
}

int angle = 0;
/**
 * @brief  Period elapsed callback in non-blocking mode.
 * @param  htim: TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	float proportional_coeff = 0.001;

	if (htim->Instance == TIM2) // Check if the interrupt is from Timer 2
	{
		// Asservissement du moteur proportionel à la température
		angle += (int)(BMP280_get_temperature() * proportional_coeff)%360;

		if (angle>180) MOT_Rotate(angle-180, MOT_ANGLE_NEGATIVE);
		if (angle<=180) MOT_Rotate(angle, MOT_ANGLE_POSITIVE);
	}
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
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_USART1_UART_Init();
	MX_CAN1_Init();
	MX_I2C3_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
	printf("\r\n=== TP Capteurs & Reseaux ===\r\n");
	BMP280_init();
	MOT_Init();
	MPU9250_init();

	// Enable Timer 2 IT
	HAL_TIM_Base_Start_IT(&htim2);

	// Start USART1 DMA reception
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, serial_buff, SERIAL_BUFF_SIZE);
	// Enable UART IDLE interrupt
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

	//HAL_UART_DMAResume(&huart1);
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
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 80;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
