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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "BMP280_driver.h"
#include "MPU9250_driver.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRUE  1
#define FALSE 0
#define CAN_BUFF_LENGTH 100

// Step Motor Constants
#define MOT_MODE_MANUAL_ID 0x60
#define MOT_ANGLE_ID 0x61
#define MOT_INIT_POS_ID 0x62
#define MOT_ANGLE_POSITIVE 0x00
#define MOT_ANGLE_NEGATIVE 0x01
#define MOT_ANGLE_MIN 0x00
#define MOT_ANGLE_MAX 0xFF
#define MOT_ANGLE_SIZE 2

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
	BMP280_get_temperature();	// Acquisition de la température
	BMP280_get_pressure();		// Acquisition de la pression
}

void MPU9250_init()
{
	// Vérifie si l'IMU est configuré correctement et bloque si ce n'est pas le cas
	if (MPU_begin(&hi2c1, AD0_LOW, AFSR_4G, GFSR_500DPS, 0.98, 0.004) == TRUE)
	{
		printf("Centrale inertielle configurée correctement\r\n");
	}
	else
	{
		printf("ERREUR!\r\n");
	}

	// Calibre l'IMU
	printf("CALIBRATION EN COURS...\r\n");
	MPU_calibrateGyro(&hi2c1, 1500);
}

void CAN_Init()
{
	HAL_StatusTypeDef status;

	status = HAL_CAN_Start(&hcan1);

	switch (status)
	{
	case HAL_OK:
		printf("CAN started successfully.\r\n");
		break;
	case HAL_ERROR:
		printf("Error: CAN start failed.\r\n");
		Error_Handler(); // Optional: Go to error handler
		break;
	case HAL_BUSY:
		printf("Warning: CAN is busy. Retry later.\r\n");
		// Optional: add retry logic if desired
		break;
	case HAL_TIMEOUT:
		printf("Error: CAN start timed out.\r\n");
		Error_Handler(); // Optional: Go to error handler
		break;
	default:
		printf("Unknown status returned from HAL_CAN_Start.\r\n");
		Error_Handler(); // Optional: Go to error handler
		break;
	}
}

void CAN_Send(uint8_t * aData, uint32_t size, uint32_t msg_id)
{
	HAL_StatusTypeDef status;
	CAN_TxHeaderTypeDef * pHeader = NULL;
	uint32_t * pTxMailbox = NULL;
	int retryCount = 0;
	const int maxRetries = 5;

	pHeader->StdId = msg_id;
	pHeader->IDE = CAN_ID_STD;
	pHeader->RTR = CAN_RTR_DATA;
	pHeader->DLC = size;
	pHeader->TransmitGlobalTime = DISABLE;

	// Attempt to add the CAN message to the transmission mailbox with retry logic
	do {
		status = HAL_CAN_AddTxMessage(&hcan1, pHeader, aData, pTxMailbox);

		switch (status)
		{
		case HAL_OK:
			printf("CAN message ");
			for (int i = 0; i<size; i++)
				printf(" 0x%X", aData[i]);
			printf(" sent successfully to  0x%X.\r\n", (unsigned int)msg_id);
			return;  // Exit the function if the message was sent successfully

		case HAL_BUSY:
			retryCount++;
			printf("Warning: CAN bus is busy, retrying (%d/%d)...\r\n", retryCount, maxRetries);
			HAL_Delay(10);  // Optional: Add a small delay between retries
			break;

		case HAL_ERROR:
			printf("Error: Failed to send CAN message.\r\n");
			Error_Handler();  // Optional: Go to error handler for critical failure
			return;

		case HAL_TIMEOUT:
			printf("Error: CAN message send timed out.\r\n");
			Error_Handler();  // Optional: Go to error handler for timeout
			return;

		default:
			printf("Unknown status returned from HAL_CAN_AddTxMessage.\r\n");
			Error_Handler();  // Optional: Handle unexpected status
			return;
		}

	} while (status == HAL_BUSY && retryCount < maxRetries);

	if (retryCount == maxRetries)
	{
		printf("Error: Exceeded maximum retries for CAN message send.\r\n");
		Error_Handler();  // Optional: Go to error handler after max retries
	}
}

void MOT_Init()
{
	uint8_t aData[3];

	aData[0] = 0;
	aData[1] = 1;
	aData[2] = 1;
	CAN_Send(aData, 3, MOT_MODE_MANUAL_ID);

	aData[0] = 0;
	CAN_Send(aData, 1, MOT_INIT_POS_ID);
}

void MOT_Rotate(uint8_t  angle, uint8_t sign)
{
	uint8_t aData[2];

	if (angle < MOT_ANGLE_MIN) angle = 0x00;
	if (angle > MOT_ANGLE_MAX) angle = 0xFF;

	aData[0] = angle;
	aData[1] = sign;

	CAN_Send(aData, MOT_ANGLE_SIZE, MOT_ANGLE_ID);
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
	MX_USART1_UART_Init();
	MX_CAN1_Init();
	/* USER CODE BEGIN 2 */
	printf("\r\n=== TP Capteurs & Reseaux ===\r\n");

	CAN_Init();
	//MOT_Init();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		MOT_Rotate(90, MOT_ANGLE_POSITIVE);
		HAL_Delay(1000);
		MOT_Rotate(90, MOT_ANGLE_NEGATIVE);
		HAL_Delay(1000);
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
