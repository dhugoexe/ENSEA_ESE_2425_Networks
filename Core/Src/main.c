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
#include "BMP280.h"
//#include "MPU9250.h"
#include "string.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define size_Calib 26
#define MAX_BUFFER_SIZE 100
#define BUFFER_SIZE 64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

char rxBuffer[MAX_BUFFER_SIZE];
char spr_buffer[50];
CAN_TxHeaderTypeDef TxHeader;
uint8_t adresse = 0x77 << 1;
uint8_t data[2];
int16_t x, y, z;
int Tempinit = 26 ;
uint8_t TxData[8]; // Tableau de données pour envoyer des instructions au moteur
uint8_t angle;
uint8_t rxData;
uint8_t data_Calib[size_Calib];
uint8_t buffer[1];
int8_t Val_temp;
//uint8_t Registre = 0xD0;
uint8_t Registre_Masc = 0xF4;
HAL_StatusTypeDef status;
int32_t coefficientK=1 ;
uint32_t TxMailbox;
uint16_t rxIndex = 0;
char rx_buffer[BUFFER_SIZE];
volatile uint8_t rx_index = 0;
char command_buffer[BUFFER_SIZE];
char response_buffer[64]; // Pour stocker la réponse à envoyer

//uint8_t BMP280_TRIM_REG_MSB = 0x88;
//uint8_t BMP280_TEMP_REG_MSB = 0xFA;
//uint8_t BMP280_PRES_REG_MSB = 0xF7;
/*
uint8_t buffer[1];
HAL_StatusTypeDef status;

uint8_t buf[3];
int32_t  compensatePressure;
uint8_t tab[3];
int32_t Temp, compensateTemperature;

uint8_t data_Calib[size_Calib];
uint8_t data[2];
int16_t x, y, z;


 */
// Configure le registre de contrôle (BMP280_CTRL_MEAS_REG)


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//***********Code ACCELEROMETRE**************
void MPU9250_Init() {
	uint8_t  MPU9250_data[2];
	MPU9250_data[0] = 0x6B;     // PWR_MGMT_1
	MPU9250_data[1] = 0x00;     // Activation du  capteur
	// Adresse I2C du MPU= 0x68
	HAL_I2C_Master_Transmit(&hi2c2, 0x68 << 1, data, 2, HAL_MAX_DELAY);

	// Configurer la plage d'accélération à ±4g
	MPU9250_data[0] = 0x1C;     // Adresse du registre ACCEL_CONFIG
	MPU9250_data[1] = 0x08;     // Réglage de la plage à ±4g
	HAL_I2C_Master_Transmit(&hi2c2, 0x68<< 1, data, 2, HAL_MAX_DELAY);
}

void MPU9250_Data(int16_t *x, int16_t *y, int16_t *z) {

	uint8_t reg_ACCEL_XOUT_H = 0x3B;
	uint8_t data_read[6];


	// Envoyer l'adresse du registre de départ
	HAL_I2C_Master_Transmit(&hi2c2, 0x68<< 1, &reg_ACCEL_XOUT_H, 1, HAL_MAX_DELAY);

	// Recevoir les 6 octets de données des axes X, Y, Z
	HAL_I2C_Master_Receive(&hi2c2, 0x68 << 1, data_read, 6, HAL_MAX_DELAY);

	// Construire les valeurs brutes pour chaque axe
	*x = (int16_t)((data_read[0] << 8) | data_read[1]);
	*y = (int16_t)((data_read[2] << 8) | data_read[3]);
	*z = (int16_t)((data_read[4] << 8) | data_read[5]);
}


void Data_Accel(int16_t x, int16_t y, int16_t z) {
	printf("\r\n Acceleration X: %d , Y: %d , Z: %d \r\n", x * 4, y * 4, z * 4);  // ±4g
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
	MX_I2C2_Init();
	MX_CAN1_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	printf("TESTtp1 Hedia et Hugo \r\n");

	uint8_t buf[1];

	buffer[0] = 0xD0;
	printf("Hello world\r\n");
	if (HAL_I2C_Master_Transmit(&hi2c2, BMP280_adresse, buffer, 1, HAL_MAX_DELAY) != 0) {
		printf("Problem in I2C Transmission\r\n");
	}

	if (HAL_I2C_Master_Receive(&hi2c2, BMP280_adresse, buffer, 1,HAL_MAX_DELAY) != 0) {
		printf("Problem in I2C Reception \r\n");
	}

	printf("Identifiant: 0x%x", buffer[0]);

	if (buf[0] == 0x58) {
		printf("Verification Ok\r\n");

	} else {
		printf("Verification not Ok!\r\n");

	}


	printf("\r\nConfigure BMP280\r\n");
	uint8_t buffer2[3];


	buffer2[0] = 0xF4;
	buffer2[1] = (0b010 << 5) | (0b101 << 2) | (0b11);
	printf("Configure...\r\n");

	if (HAL_I2C_Master_Transmit(&hi2c2, BMP280_adresse, buffer2, 2, HAL_MAX_DELAY) != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	if (HAL_I2C_Master_Receive(&hi2c2, BMP280_adresse, buffer2, 1, HAL_MAX_DELAY) != 0) {
		printf("Problem with I2C Receive\r\n");
	}


	if (buffer2[0] == buffer2[1]) {
		printf("Verification Ok\r\n");
	} else {
		printf("Verification not Ok!\r\n");
		//return 1;
	}

	//printf("etalonage\r\n");
	BMP280_get_trimming(data_Calib);
	BMP280_Temperateur();
	BMP280_Pression();
	//char[50] reponse[]
	//HAL_UART_Transmit(&huart1, (uint8_t*)response, strlen(response));

	HAL_UART_Receive_IT(&huart1, (uint8_t*)&rx_buffer[rx_index], 1);

	//HAL_Delay(1000);

	printf("\r\n Affichage de MPU \r\n");
	MPU9250_Data(&x, &y, &z);  // ou MPU9250_Read_Raw_Data pour le MPU9250
	Data_Accel(x, y, z);
	//printf("Test\r\n");
	//HAL_UART_Receive_IT(&huart3, &Data, 1);

	// Démarrer le module CAN

	if (HAL_CAN_Start(&hcan1) != HAL_OK)
	{
		printf("erreur CAN\r\n");
		//Gestion d'erreur
		Error_Handler();
	}

	// printf("activation de CAN\r\n");

	//Manuel Mode


	TxHeader.StdId = 0x61;          // ID standard du message
	//TxHeader.ExtId = 0;              // ID étendu non utilisé ici
	TxHeader.IDE = CAN_ID_STD;       // Trame standard
	TxHeader.RTR = CAN_RTR_DATA;     // Trame de données
	TxHeader.DLC = 2;                // Taille des données (1 octet dans ce cas)
	TxHeader.TransmitGlobalTime = DISABLE;


	/*
   TxHeader.StdId = 0x61;          // ID standard du message
   //TxHeader.ExtId = 0;              // ID étendu non utilisé ici
   TxHeader.IDE = CAN_ID_STD;       // Trame standard
   TxHeader.RTR = CAN_RTR_DATA;     // Trame de données
   TxHeader.DLC = 2;                // Taille des données (1 octet dans ce cas)
   TxHeader.TransmitGlobalTime = DISABLE;
	 */
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{


		//mode manuel
		/*
	      TxData[0]=90;// sens 1
	  	  HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	  	  HAL_Delay(1000);

	  	  TxData[0]=0;// sens 2
	  	  HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	  	  HAL_Delay(1000);

		 */
		// calcule d'angle
		angle = (Tempinit-BMP280_Temperateur())*coefficientK ;
		// TxData[1]=1;

		//calcule vitesse
		/*
	     uint8_t vitesse = BMP280_Temperateur()*0.5;
	     if (vitesse > 255) vitesse = 255;

	     // Préparer les données pour l'envoi sur le bus CAN
	     TxData[0] = vitesse;  // Vitesse du moteur calculée
		 TxData[1]=1;

	 	  HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	 	  //HAL_Delay(1000);
	       printf("rotation moteur sens1\r\n");

		 */

		Val_temp=BMP280_Temperateur();

		int vitesse = Val_temp * 255;
		printf("angle=%i\r\n",vitesse);

		//TxData[0] =90;// Commande pour déplacer le moteur de 90°

		TxData[1]=1;
		TxData[0]=vitesse;
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
		HAL_Delay(1000);
		printf("rotation moteur sens1\r\n");

		/*
	 else{
	      TxData[0]=-angle;
	      HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	      //HAL_Delay(1000);
	      printf("rotation moteur sens2\r\n");
	    }
		 */
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		char received_char = rx_buffer[rx_index];

		// Ajouter le caractère au buffer de commande
		if (received_char == '\r' || received_char == '\n') {
			command_buffer[rx_index] = '\0';  // Terminer la commande
			process_command(command_buffer);  // Traiter la commande reçue
			rx_index = 0;                     // Réinitialiser l'index
		} else {
			command_buffer[rx_index++] = received_char;

			// Empêcher le dépassement du buffer
			if (rx_index >= BUFFER_SIZE) {
				rx_index = 0;
			}
		}

		// Relancer la réception d'un autre caractère
		HAL_UART_Receive_IT(&huart1, (uint8_t*)&rx_buffer[rx_index], 1);
	}
}

void process_command(char *command) {
	if (strncmp(command, "GET_T", 5) == 0) {
		sprintf(response_buffer, "T=10_C\r\n"); // Exemple de température
	}
	/*
    else if (strncmp(command, "GET_P", 5) == 0) {
        sprintf(response_buffer, "P=%dPa\r\n", 102300); // Exemple de pression
    } else if (strncmp(command, "SET_K=", 6) == 0) {
        int new_k = atoi(&command[6]);
        sprintf(response_buffer, "SET_K=OK\r\n");
    } else if (strncmp(command, "GET_K", 5) == 0) {
        sprintf(response_buffer, "K=%.5f\r\n", 12.34);
    } else if (strncmp(command, "GET_A", 5) == 0) {
        sprintf(response_buffer, "A=%.4f\r\n", 125.7);
    } else {
        sprintf(response_buffer, "Unknown command\r\n");
    }
	 */
	// Envoyer la réponse sur UART3
	HAL_UART_Transmit(&huart1, (uint8_t*)response_buffer, strlen(response_buffer), -1);
}

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
