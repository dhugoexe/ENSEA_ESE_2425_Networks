## 1. Présentation
Le but de cette série de TP est de mettre en place l'ensemble des composants suivant: </br>

<img width="1104" alt="Screenshot 2024-10-10 at 08 27 51" src="https://github.com/user-attachments/assets/ae6aea65-0050-4f10-aea6-8499c9085bf0"># ENSEA_ESE_2425_Networks
</br>
Ces TP seront réalisés en C pour la partie STM32, et Python pour la partie Raspberry Pi. 

## TP1 - Bus I2C

<img width="685" alt="Screenshot 2024-10-10 at 09 02 18" src="https://github.com/user-attachments/assets/cdede810-5f6f-49b4-87c8-87503ddc1646">

La première étape est de mettre en place la communication entre le microcontrôleur et les capteurs (température, pression, accéléromètre...) via  le bus I²C.
</br>
Le capteur comporte 2 composants I²C, qui partagent le même bus. Le STM32 jouera le rôle de Master sur le bus.
</br>
Le code du STM32 sera écrit en langage C, en utilisant la bibliothèque HAL.

### 1. Le Capteur BMP280 

Le BMP280 est un capteur de pression et température développé par Bosch.  A partir de la datasheet du BMP280, on identifie les éléments suivants:

#### Adresses I²C possibles

- 0x76 (SDO connecté à GND)
- 0x77 (SDO connecté à V_DDIO)

#### Identification du composant

- Registre : 0xD0
- Valeur : 0x58

#### Mode normal

- Registre : 0xF4
- Valeur : '11' dans les bits 1 et 0

#### Registres d'étalonnage

- Plage : 0x88 à 0xA1
- Noms : calib00 à calib25

#### Registres de température

- Registres : 0xFA (temp_msb), 0xFB (temp_lsb), 0xFC (temp_xlsb)
- Format : 20 bits répartis sur 3 registres

#### Registres de pression

- Registres : 0xF7 (press_msb), 0xF8 (press_lsb), 0xF9 (press_xlsb)
- Format : 20 bits répartis sur 3 registres
  
#### Fonctions de calcul (format entier 32 bits)

- Température : bmp280_compensate_T_int32(BMP280_S32_t adc_T)
- Pression : bmp280_compensate_P_int32(BMP280_S32_t adc_P)

Note : La fonction de température doit être appelée avant celle de pression.



### 2.2 Setup du STM32
#### Configuration du STM32
- d'une liaison I²C. Si possible, on utilisera les broches compatibles avec l'empreinte arduino (broches PB8 et PB9) (Doc nucleo 64)
- d'une UART sur USB (UART2 sur les broches PA2 et PA3) (Doc nucleo 64)
- d'une liaison UART indépendante pour la communication avec le Raspberry (TP2)
- d'une liaison CAN (TP4)

```
 printf("\r\nTest\r\n");
```

  ![image](https://github.com/user-attachments/assets/634696e5-3728-4918-9c58-26e95e8df641)


  ```
  status = HAL_I2C_Master_Transmit(&hi2c2, BSP280_ADDR, idRegister, sizeof(idRegister), -1);
	do{
		printf("status: %i \r\n", status);
		status = HAL_I2C_Master_Receive(&hi2c2, BSP280_ADDR, dataBuffer, sizeof(dataBuffer), -1);
		HAL_Delay(100);

	}while(status != HAL_OK);

	printf("Res: %i \r\n", dataBuffer[0]);
  ```

