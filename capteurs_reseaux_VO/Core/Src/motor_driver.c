/**
 ******************************************************************************
 * @file           : motor_driver.c
 * @brief          : Driver for the TP CAN bus – Stepper motor board
 ******************************************************************************
 *
 *  Created on: Nov 9, 2024
 *      Author: oliver
 *  Documentation: https://moodle.ensea.fr/mod/resource/view.php?id=1921
 *
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include "can.h"
#include "motor_driver.h"

#define TRUE  1
#define FALSE 0

int logs;	// Boolean to choose to display logs or not

/**
 * @brief Initialises the CAN communication
 */
void CAN_Init()
{
	HAL_StatusTypeDef status;
	logs = FALSE;

	status = HAL_CAN_Start(&hcan1);

	switch (status)
	{
	case HAL_OK:
		if (logs == TRUE) printf("CAN started successfully.\r\n");
		break;
	case HAL_ERROR:
		if (logs == TRUE) printf("Error: CAN start failed.\r\n");
		Error_Handler(); // Optional: Go to error handler
		break;
	case HAL_BUSY:
		if (logs == TRUE) printf("Warning: CAN is busy. Retry later.\r\n");
		// Optional: add retry logic if desired
		break;
	case HAL_TIMEOUT:
		if (logs == TRUE) printf("Error: CAN start timed out.\r\n");
		Error_Handler(); // Optional: Go to error handler
		break;
	default:
		if (logs == TRUE) printf("Unknown status returned from HAL_CAN_Start.\r\n");
		Error_Handler(); // Optional: Go to error handler
		break;
	}
}

/**
 * @brief Sends a CAN message with retry logic.
 *
 * This function attempts to send a message over the CAN bus to a specified
 * message ID (`msg_id`). If the CAN bus is busy, it will retry sending up to
 * a maximum number of attempts (`maxRetries`). In case of any other error
 * (such as timeout or general error), the function will call `Error_Handler()`
 * to manage the failure.
 *
 * @param uint8_t* aData	Pointer to the data buffer containing the message to send.
 *               			The data should be in the form of an array of `uint8_t`.
 * @param uint32_t size		Size of the data in bytes (must match the Data Length Code (DLC)
 * 				 			field in the CAN frame).
 * @param uint32_t msg_id	CAN message identifier (11-bit standard ID) that defines the
 *               			destination or type of the message being sent.
 *
 * @retval None
 */
void CAN_Send(uint8_t * aData, uint32_t size, uint32_t msg_id)
{
	HAL_StatusTypeDef status;
	CAN_TxHeaderTypeDef header;
	uint32_t txMailbox;
	int retryCount = 0;
	const int maxRetries = 5;

	// Initialiser le header
	header.StdId = msg_id;
	header.IDE = CAN_ID_STD;
	header.RTR = CAN_RTR_DATA;
	header.DLC = size;
	header.TransmitGlobalTime = DISABLE;

	// Pointer vers les variables locales
	CAN_TxHeaderTypeDef *pHeader = &header;
	uint32_t *pTxMailbox = &txMailbox;

	// Attempt to add the CAN message to the transmission mailbox with retry logic
	do {
		status = HAL_CAN_AddTxMessage(&hcan1, pHeader, aData, pTxMailbox);

		switch (status)
		{
		case HAL_OK:
			if (logs == TRUE)
			{
			printf("CAN message ");
			for (int i = 0; i<size; i++)
				printf(" 0x%X", aData[i]);
			printf(" sent successfully to  0x%X.\r\n", (unsigned int)msg_id);
			}
			return;  // Exit the function if the message was sent successfully

		case HAL_BUSY:
			retryCount++;
			if (logs == TRUE) printf("Warning: CAN bus is busy, retrying (%d/%d)...\r\n", retryCount, maxRetries);
			HAL_Delay(10);  // Optional: Add a small delay between retries
			break;

		case HAL_ERROR:
			if (logs == TRUE) printf("Error: Failed to send CAN message.\r\n");
			Error_Handler();  // Optional: Go to error handler for critical failure
			return;

		case HAL_TIMEOUT:
			if (logs == TRUE) printf("Error: CAN message send timed out.\r\n");
			Error_Handler();  // Optional: Go to error handler for timeout
			return;

		default:
			if (logs == TRUE) printf("Unknown status returned from HAL_CAN_AddTxMessage.\r\n");
			Error_Handler();  // Optional: Handle unexpected status
			return;
		}

	} while (status == HAL_BUSY && retryCount < maxRetries);

	if (retryCount == maxRetries)
	{
		if (logs == TRUE) printf("Error: Exceeded maximum retries for CAN message send.\r\n");
		Error_Handler();  // Optional: Go to error handler after max retries
	}
}

/**
 * @brief Sets up the Step motor with parameters
 * @param uint8_t direction	Rotation direction: Anti-clockwise (0x00) or Clockwise (0x01).
 * @param uint8_t steps		Number of steps, range: 0x01 to 0xFF (1 unit = 1°).
 * @param uint8_t speed		Speed of the motor, range: 0x01 = 1 ms / 1 kHz to 0xFF = 255ms / 4 Hz
 */
void MOT_Set_mode(uint8_t direction, uint8_t steps, uint8_t speed)
{
	uint8_t aData[3];

	aData[0] = direction;
	aData[1] = steps;
	aData[2] = speed;

	CAN_Send(aData, 3, MOT_MODE_MANUAL_ID);
}

/**
 * @brief Sets the current position of the motor as the origin.
 */
void MOT_Set_origin()
{
	uint8_t aData[2];

	aData[0] = 0;
	CAN_Send(aData, 1, MOT_INIT_POS_ID);
}

/**
 * @brief Rotates the Step Motor of an angle in a defined direction.
 * @param uint8_t angle Angle of rotation, range: 0x01 to 0xFF (1 unit = 1°).
 * @param uint8_t sign	Angle sign, can be positive (0x00) or negative (0x01).
 */
void MOT_Rotate(uint8_t angle, uint8_t sign)
{
	uint8_t aData[2];

	if (angle < MOT_ANGLE_MIN) angle = 0x00;
	if (angle > MOT_ANGLE_MAX) angle = 0xFF;

	aData[0] = angle;
	aData[1] = sign;

	CAN_Send(aData, MOT_ANGLE_SIZE, MOT_ANGLE_ID);
}
