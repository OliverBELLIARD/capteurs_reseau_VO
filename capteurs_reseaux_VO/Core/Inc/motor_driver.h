/**
  ******************************************************************************
  * @file    motor_driver.h
  * @brief   This file contains all the function prototypes for
  *          the motor_driver.c file
  ******************************************************************************
  *
  *  Created on: Nov 9, 2024
  *      Author: oliver
  *
  ******************************************************************************
  */

#ifndef INC_MOTOR_DRIVER_H_
#define INC_MOTOR_DRIVER_H_

// Step Motor Config Constants
#define MOT_MODE_MANUAL_ID 0x60
#define MOT_MODE_CLOCKWISE 0x01
#define MOT_MODE_ANTICLOCKWISE 0x00

// Step Motor initial position
#define MOT_INIT_POS_ID 0x62

// Step Motor Angle constants
#define MOT_ANGLE_ID 0x61
#define MOT_ANGLE_POSITIVE 0x00
#define MOT_ANGLE_NEGATIVE 0x01
#define MOT_ANGLE_MIN 0x00
#define MOT_ANGLE_MAX 0xFF
#define MOT_ANGLE_SIZE 2

#define MOT_REACTION_TIME_MIN 15

void CAN_Init();
void CAN_Send(uint8_t*, uint32_t, uint32_t);
void MOT_Set_mode(uint8_t, uint8_t, uint8_t);
void MOT_Set_origin();
void MOT_Rotate(uint8_t, uint8_t);

#endif /* INC_MOTOR_DRIVER_H_ */
