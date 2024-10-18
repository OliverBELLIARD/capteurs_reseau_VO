#include "main.h"
#include "i2c.h"
#include "BMP280_driver.h"


int BMP280_Check_id(void)
{
	uint8_t buff[BUFF_SIZE];
	HAL_StatusTypeDef ret; 		// I2C operations status

	buff[0] = BMP280_REG_ID;

	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
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
	uint8_t buff[BUFF_SIZE];
	HAL_StatusTypeDef ret; 		// I2C operations status

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
/*
 * Update the calibration parameters.
 */
void BMP280_calibration(void)
{
	uint8_t buff[BUFF_SIZE];
	uint8_t receive_buf[24];
	HAL_StatusTypeDef ret; 		// I2C operations status

	buff[0]= BMP280_REG_CALIBRATION;

	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Transmit failure\r\n");
	}

	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, receive_buf, 24, HAL_MAX_DELAY);
	if(ret != HAL_OK){
		printf("I2C Receive failure\r\n");
	}

	printf("Current calibration values:\r\n");
	for(int i=0;i<24;i++){
		printf("calib %2d = 0x%x\r\n",i, receive_buf[i]);
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
}

/* Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
 * t_fine carries fine temperature as global value
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

/* Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
 * Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
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

int BMP280_Write_Reg(uint8_t reg, uint8_t value) {
	uint8_t buff[BUFF_SIZE];
	HAL_StatusTypeDef ret;

	buff[0] = reg;
	buff[1] = value;

	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, buff, 2, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buff, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Receive\r\n");
	}

	if (buff[0] == value) {
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}

uint8_t* BMP280_Read_Reg(uint8_t reg, uint8_t length) {
	uint8_t *buf;
	HAL_StatusTypeDef ret;

	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, &reg, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	buf = (uint8_t*) malloc(length);
	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buf, length,
			HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Receive\r\n");
	}

	return buf;
}

BMP280_S32_t BMP280_get_temperature() {
	uint8_t *buf;
	BMP280_S32_t adc_T;

	buf = BMP280_Read_Reg(BMP280_REG_TEMP_MSB, BMP280_LEN_TEMP);

	adc_T = ((BMP280_S32_t) (buf[0]) << 12) | ((BMP280_S32_t) (buf[1]) << 4)
			| ((BMP280_S32_t) (buf[2]) >> 4);

	free(buf);

	printf("Temperature: ");
	printf("0X%05lX", adc_T);
	printf("\r\n");

	adc_T = BMP280_compensate_T_int32(adc_T);
	printf("Compensated temperature: ");
	printf("0X%05lX", adc_T);
	printf("\r\n");

	return adc_T;
}

BMP280_S32_t BMP280_get_pressure() {
	uint8_t *buf;
	BMP280_S32_t adc_P;

	buf = BMP280_Read_Reg(BMP280_REG_PRES_MSB, BMP280_LEN_PRES);

	adc_P = ((BMP280_S32_t) (buf[0]) << 12) | ((BMP280_S32_t) (buf[1]) << 4)
			| ((BMP280_S32_t) (buf[2]) >> 4);

	free(buf);

	printf("Pressure:    0x");
	printf("%05lX", adc_P);
	printf("\r\n");

	adc_P = BMP280_compensate_P_int64(adc_P);
	printf("Compensated pressure:    0x");
	printf("%05lX", adc_P);
	printf("\r\n");

	return adc_P;
}
