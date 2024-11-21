

#include "stdio.h"
#include "stdlib.h"

#include "main.h"
#include "BMP280.h"

extern I2C_HandleTypeDef hi2c2;

int32_t NONcompensatePression;
int32_t compensatePression;
int32_t compensateTemperateur;
int32_t NONcompensateTemperateur;
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

int32_t t_fine;


int32_t BMP280_Temperateur() {
	uint8_t *buffer;
	uint8_t registre = 0xFA;
	if (HAL_I2C_Master_Transmit(&hi2c2, BMP280_adresse, &registre, 1, HAL_MAX_DELAY) != 0) {
			printf("Problem in Transmition I2C\r\n");
		}

		buffer = (uint8_t*) malloc(3);
		if (HAL_I2C_Master_Receive(&hi2c2, BMP280_adresse, buffer, 3, HAL_MAX_DELAY) != 0) {
			printf("Problem in Reciption I2C\r\n");
		}

	NONcompensateTemperateur = ((int32_t) (buffer[0]) << 12) | ((int32_t) (buffer[1]) << 4) | ((int32_t) (buffer[2]) >> 4);
	compensateTemperateur =bmp280_compensate_T_int32(NONcompensateTemperateur);
	free(buffer);

	    printf("TemperatureNonCompens: %ld \r\n" , NONcompensateTemperateur);
		printf("TemperatureCompens: %ld °C\r\n", compensateTemperateur);
		return NONcompensateTemperateur;
}

int BMP280_Pression() {

	uint8_t *buffer;
	uint8_t registre = 0xF7;


	if (HAL_I2C_Master_Transmit(&hi2c2, BMP280_adresse, &registre, 1, HAL_MAX_DELAY) != 0) {
				printf("Problem in Transmition I2C\r\n");
			}

			buffer = (uint8_t*) malloc(3);
			if (HAL_I2C_Master_Receive(&hi2c2, BMP280_adresse, buffer, 3, HAL_MAX_DELAY) != 0) {
				printf("Problem in Reception I2C\r\n");
			}

	NONcompensatePression = ((int32_t) (buffer[0]) << 12) | ((int32_t) (buffer[1]) << 4) | ((int32_t) (buffer[2]) >> 4);

	compensatePression=bmp280_compensate_P_int32(NONcompensatePression);
	free(buffer);

	        printf("PressionNonCompens: %ld \r\n" , NONcompensatePression);
			printf("PressionCompens: %ld hPa \r\n", compensatePression);
			return compensatePression;


	return 0;
}


// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
int32_t t_fine;

int32_t bmp280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) *
			((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	//printf("TemperatureCompens: %ld\r\n", T);

	return T/100;
}
uint32_t bmp280_compensate_P_int32(int32_t adc_P)
{
	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
	var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
		p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
	return p/100;
}


void  BMP280_get_trimming(uint8_t data_Calib[size_Calib])
{
	uint8_t registre = 0x88;
    uint8_t* p = data_Calib;
    int i = 0;

    if (HAL_I2C_Master_Transmit(&hi2c2, BMP280_adresse, &registre, 1, HAL_MAX_DELAY) == HAL_OK)
    {
        if (HAL_I2C_Master_Receive(&hi2c2, BMP280_adresse, data_Calib, size_Calib, HAL_MAX_DELAY) == HAL_OK)
        {
            printf("Calibration data received\r\n");

            // Create an array of pointers to your calibration data variables
            uint16_t* dig_values[] = { &dig_T1, &dig_T2, &dig_T3, &dig_P1, &dig_P2, &dig_P3,
                                       &dig_P4, &dig_P5, &dig_P6, &dig_P7, &dig_P8, &dig_P9 };

            // Use a loop to copy data into calibration variables
         //1 ére solution
            /*for (i = 0; i < 12; i++)
            {
                memcpy(dig_values[i], p, 2);
                p += 2;
            }*/

          //2 eme solution

            dig_T1 = (p[1] << 8) | p[0];
            p += 2;
            dig_T2 = (p[1] << 8) | p[0];
            p += 2;
            dig_T3 = (p[1] << 8) | p[0];
            p += 2;
            dig_P1 = (p[1] << 8) | p[0];
            p += 2;
            dig_P2 = (p[1] << 8) | p[0];
            p += 2;
            dig_P3 = (p[1] << 8) | p[0];
            p += 2;
            dig_P4 = (p[1] << 8) | p[0];
            p += 2;
            dig_P5 = (p[1] << 8) | p[0];
            p += 2;
            dig_P6 = (p[1] << 8) | p[0];
            p += 2;
            dig_P7 = (p[1] << 8) | p[0];
            p += 2;
            dig_P8 = (p[1] << 8) | p[0];
            p += 2;
            dig_P9 = (p[1] << 8) | p[0];



            // Display the calibration data
            for (i = 0; i < size_Calib; i++)
            {
                printf("calib %d = 0x%x\n\r", i, data_Calib[i]);
            }
        }
    }
}
