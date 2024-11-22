/**
 ******************************************************************************
 * @file           : BMP280_driver.c
 * @brief          : Driver for the BMP280
 ******************************************************************************
 *  Created on: Oct 18, 2024
 *      Author: oliver
 */
#include "main.h"
#include "i2c.h"
#include "BMP280_driver.h"

#define VERBOSE 0

I2C_HandleTypeDef* hi2c_user;

/**
 * @brief Check the BMP280 sensor ID.
 *
 * Sends a command to retrieve the BMP280 sensor's ID and checks if the
 * response matches the expected ID. Prints the ID if successful.
 *
 * @return int Returns EXIT_SUCCESS if ID is read successfully, otherwise returns EXIT_FAILURE.
 */
int BMP280_Check_id(void)
{
	uint8_t buff[BUFF_SIZE];	// Buffer for the I2C communication
	HAL_StatusTypeDef ret; 		// I2C operations status
	hi2c_user = &hi2c1;			// I2C Handler used by the user

	buff[0] = BMP280_REG_ID;

	ret = HAL_I2C_Master_Transmit(hi2c_user, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Transmit failure\r\n");
		return EXIT_FAILURE;
	}
	ret = HAL_I2C_Master_Receive(hi2c_user, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Receive failure\r\n");
		return EXIT_FAILURE;
	}

	printf("BMP280 ID: 0x%X\r\n", buff[0]);

	return EXIT_SUCCESS;
}

/**
 * @brief Configure the BMP280 sensor.
 *
 * Sets up the BMP280 with the specified configuration. The configuration is
 * sent and confirmed by checking the response from the sensor.
 *
 * @return int Returns EXIT_SUCCESS if configuration is confirmed, otherwise returns EXIT_FAILURE.
 */
int BMP280_Config(void)
{
	uint8_t buff[BUFF_SIZE];	// Buffer for the I2C communication
	HAL_StatusTypeDef ret; 		// I2C operations status
	hi2c_user = &hi2c1;			// I2C Handler used by the user

	buff[0]= BMP280_REG_MODE;
	buff[1]= BMP280_CONFIG;

	ret = HAL_I2C_Master_Transmit(hi2c_user,BMP280_ADDR, buff, 2, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Transmit failure\r\n");
		return EXIT_FAILURE;
	}

	ret = HAL_I2C_Master_Receive(hi2c_user, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Receive failure\r\n");
		return EXIT_FAILURE;
	}

	if(buff[0] == BMP280_CONFIG){
		printf("La config BMP280 envoyée reçue avec succès\r\n");
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;
uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;

BMP280_S32_t t_fine;

/**
 * @brief Update the calibration parameters of BMP280.
 *
 * Reads calibration data from the BMP280 sensor and stores it in global
 * variables for temperature and pressure compensation calculations.
 */
int BMP280_calibration(void)
{
	uint8_t buff[BUFF_SIZE];
	uint8_t receive_buf[24];
	HAL_StatusTypeDef ret; 		// I2C operations status

	buff[0]= BMP280_REG_CALIBRATION;

	ret = HAL_I2C_Master_Transmit(hi2c_user, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Transmit failure\r\n");
		return EXIT_FAILURE;
	}

	ret = HAL_I2C_Master_Receive(hi2c_user, BMP280_ADDR, receive_buf, 24, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Receive failure\r\n");
		return EXIT_FAILURE;
	}

	if (VERBOSE) {
		printf("Current calibration values:\r\n");
		for(int i=0;i<24;i++){
			printf("calib %2d = 0x%x\r\n",i, receive_buf[i]);
		}
	}

	dig_T1 = receive_buf[0]|(receive_buf[1]<<8);
	dig_T2 = receive_buf[2]|(receive_buf[3]<<8);
	dig_T3 = receive_buf[4]|(receive_buf[5]<<8);
	dig_P1 = receive_buf[6]|(receive_buf[7]<<8);
	dig_P2 = receive_buf[8]|(receive_buf[9]<<8);
	dig_P3 = receive_buf[10]|(receive_buf[11]<<8);
	dig_P4 = receive_buf[12]|(receive_buf[13]<<8);
	dig_P5 = receive_buf[14]|(receive_buf[15]<<8);
	dig_P6 = receive_buf[16]|(receive_buf[17]<<8);
	dig_P7 = receive_buf[18]|(receive_buf[19]<<8);
	dig_P8 = receive_buf[20]|(receive_buf[21]<<8);
	dig_P9 = receive_buf[22]|(receive_buf[23]<<8);

	return EXIT_SUCCESS;
}

/**
 * @brief Compensate temperature reading from BMP280.
 *
 * Compensates the raw temperature data read from the sensor to provide a
 * temperature value in degrees Celsius, with a resolution of 0.01°C.
 * t_fine carries fine temperature as global value.
 *
 * @param adc_T Raw ADC temperature value.
 * @return BMP280_S32_t Compensated temperature in degrees Celsius, scaled by 100 (e.g., 5123 represents 51.23°C).
 */
BMP280_S32_t BMP280_compensate_T_int32(BMP280_S32_t adc_T)
{
	BMP280_S32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) * ((BMP280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;

	return T;
}

/**
 * @brief Compensate pressure reading from BMP280.
 *
 * Compensates the raw pressure data read from the sensor to provide a
 * pressure value in Pascals (Pa) in Q24.8 format (24 integer bits and 8 fractional bits).
 *
 * @param adc_P Raw ADC pressure value.
 * @return BMP280_U32_t Compensated pressure in Pascals as a unsigned 32 bit integer
 * 						(Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa).
 */
BMP280_U32_t BMP280_compensate_P_int64(BMP280_S32_t adc_P)
{
	BMP280_S64_t var1, var2, p;
	var1 = ((BMP280_S64_t)t_fine) - 128000;
	var2 = var1 * var1 * (BMP280_S64_t)dig_P6;
	var2 = var2 + ((var1*(BMP280_S64_t)dig_P5)<<17);
	var2 = var2 + (((BMP280_S64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (BMP280_S64_t)dig_P3)>>8) + ((var1 * (BMP280_S64_t)dig_P2)<<12);
	var1 = (((((BMP280_S64_t)1)<<47)+var1))*((BMP280_S64_t)dig_P1)>>33;
	if (var1 == 0)
	{
		return EXIT_FAILURE; // avoid exception caused by division by zero
	}

	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((BMP280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((BMP280_S64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((BMP280_S64_t)dig_P7)<<4);
	return (BMP280_U32_t)p;
}

/**
 * @brief Write a value to a BMP280 register.
 *
 * Writes a specified value to a BMP280 register and verifies if the value
 * was successfully written.
 *
 * @param reg Register address.
 * @param value Value to write to the register.
 * @return int Returns EXIT_SUCCESS if value is successfully written, otherwise returns EXIT_FAILURE.
 */
int BMP280_Write_Reg(uint8_t reg, uint8_t value) {
	uint8_t buff[BUFF_SIZE];
	HAL_StatusTypeDef ret;

	buff[0] = reg;
	buff[1] = value;

	ret = HAL_I2C_Master_Transmit(hi2c_user, BMP280_ADDR, buff, 2, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	ret = HAL_I2C_Master_Receive(hi2c_user, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Receive\r\n");
	}

	if (buff[0] == value) {
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}

/**
 * @brief Read data from a BMP280 register.
 *
 * Reads a specified number of bytes from a BMP280 register and returns
 * a dynamically allocated buffer containing the data.
 *
 * @param reg Register address to read from.
 * @param length Number of bytes to read.
 * @return uint8_t* Pointer to buffer with read data. The caller is responsible for freeing the buffer.
 */
uint8_t* BMP280_Read_Reg(uint8_t reg, uint8_t length) {
	uint8_t *buf;
	HAL_StatusTypeDef ret;

	ret = HAL_I2C_Master_Transmit(hi2c_user, BMP280_ADDR, &reg, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	buf = (uint8_t*) malloc(length);
	ret = HAL_I2C_Master_Receive(hi2c_user, BMP280_ADDR, buf, length,
			HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Receive\r\n");
	}

	return buf;
}

/**
 * @brief Get the compensated temperature in degrees Celsius.
 *
 * Reads the raw temperature data from the BMP280, compensates it, and
 * returns the result. Prints both raw and compensated temperature values.
 *
 * @return BMP280_S32_t Compensated temperature in degrees Celsius, scaled by 100.
 */
BMP280_S32_t BMP280_get_temperature() {
	uint8_t *buf;
	BMP280_S32_t adc_T;

	buf = BMP280_Read_Reg(BMP280_REG_TEMP_MSB, BMP280_LEN_TEMP);

	adc_T = ((BMP280_S32_t) (buf[0]) << 12) | ((BMP280_S32_t) (buf[1]) << 4)
							| ((BMP280_S32_t) (buf[2]) >> 4);

	free(buf);

	if (VERBOSE) {
		printf("Temperature: ");
		printf("0x%05lX = %d°C", adc_T, adc_T);
		printf("\r\n");
	}

	if (VERBOSE) {
		adc_T = BMP280_compensate_T_int32(adc_T);
		printf("Compensated temperature: ");
		printf("0x%05lX = %d°C", adc_T, adc_T);
		printf("\r\n");
	}

	return adc_T;
}

/**
 * @brief Get the compensated pressure in Pascals.
 *
 * Reads the raw pressure data from the BMP280, compensates it, and
 * returns the result. Prints both raw and compensated pressure values.
 *
 * @return BMP280_S32_t Compensated pressure in Pascals.
 */
BMP280_S32_t BMP280_get_pressure() {
	uint8_t *buf;
	BMP280_S32_t adc_P;

	buf = BMP280_Read_Reg(BMP280_REG_PRES_MSB, BMP280_LEN_PRES);

	adc_P = ((BMP280_S32_t) (buf[0]) << 12) | ((BMP280_S32_t) (buf[1]) << 4)
							| ((BMP280_S32_t) (buf[2]) >> 4);

	free(buf);

	if (VERBOSE) {
		printf("Pressure: ");
		printf("0x%05lX = %ld Pa", adc_P, adc_P);
		printf("\r\n");
	}

	if (VERBOSE) {
		adc_P = BMP280_compensate_P_int64(adc_P);
		printf("Compensated pressure: ");
		printf("0x%05lX = %ld Pa", adc_P, adc_P);
		printf("\r\n");
	}

	return adc_P;
}
