

#ifndef SRC_BMP280_SIMPLE_H_
#define SRC_BMP280_SIMPLE_H_


static const uint8_t BMP280_adresse = 0x77 << 1;


#define size_Calib 26



void  BMP280_get_trimming(uint8_t data_Calib[size_Calib]);
uint32_t bmp280_compensate_P_int32(int32_t adc_P);
int32_t bmp280_compensate_T_int32(int32_t adc_T);
int32_t BMP280_Temperateur();
int BMP280_Pression();



#endif

