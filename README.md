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
Sous l’outil STM32CubeIDE, nous avons effectué les connexions suivantes :
*Une liaison I²C1, en utilisant si possible les broches PB6 et PB7.

*Une liaison UART2 sur USB, connectée aux broches PA2 et PA3.

*Une liaison UART1 indépendante pour la communication avec le Raspberry Pi pour le TP2.

*Une liaison CAN1 pour le TP4.

![image](https://github.com/user-attachments/assets/4864395a-db0e-4ead-8cf9-d92a33377c31)

Dans ce TP, sous l’outil STM32CubeIDE, nous avons configuré les connexions suivantes :

*Deux interfaces USART pour afficher les données sur Tera Term.

![image](https://github.com/user-attachments/assets/a870d45a-b1d2-4db2-b43e-6c07b65dcca9)


*Une interface I2C1 pour établir la communication entre la carte STM32 et le capteur IMU 10DOF.

![image](https://github.com/user-attachments/assets/a949952a-e776-4b63-9788-61517a8b8235)

Ces connexions permettent d’assurer une transmission efficace des données pour l'affichage et la communication avec le capteur.

## Redirection du printf

Afin de simplifier le débogage de notre programme STM32, nous avons modifié le fichier stm32f4xx_hal_msp.c pour permettre à la fonction printf de transmettre ses chaînes de caractères via l’interface UART sur USB. Cette configuration rend possible l’affichage des messages de débogage directement sur le terminal.

![image](https://github.com/user-attachments/assets/0a4ec3ab-743a-4122-9dae-c0d78c3f8b29)


*Ce code définit la fonction PUTCHAR_PROTOTYPE, qui redirige chaque caractère de printf vers l’interface UART2. Lorsqu'un caractère est envoyé, la fonction utilise HAL_UART_Transmit pour l’écrire sur l’USART2, en attendant la fin de la transmission avant de passer au caractère suivant. Cela permet d’envoyer les sorties de printf sur UART, pour les afficher sur un terminal connecté par USB.

## Test de la chaine de compilation et communication UART sur USB

Pour pouvoir tester le programme de type écho, nous allons utiliser printf pour afficher le message suivant : "TESTtp1 Hedia et Hugo

![image](https://github.com/user-attachments/assets/ad31ac81-2c11-4da6-85fd-3c78fcba59a7)

L'API HAL (Hardware Abstraction Layer) de ST fournit des fonctions pour gérer le bus I²C en mode Master, simplifiant les échanges de données avec des périphériques I²C. Deux fonctions principales sont disponibles : 

*AL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout):  permet d’envoyer des données à un périphérique à une adresse donnée (DevAddress), 
*HAL_I2C_Master_Receive : permet de recevoir des données de ce même périphérique. 

Ces fonctions prennent en paramètres l'adresse du périphérique, un pointeur vers les données à transmettre ou recevoir (pData), la taille des données (Size), et un délai d'expiration (Timeout).

## Adresse:
HAL_I2C_Master_Transmit permet d'écire sur le bus, alors que HAL_I2C_Master_Receive permet de lire le bus. Ces 2 fonctions gère le bit R/W, mais il faut quand même lui laisser la place dans l'adresse I²C.
L'adresse I²C est officiellement sur 7 bits. L'API HAL du STM32 demande des adresses I²C sur 8bits, le LSB étant réservé au bit de R/W.

![image](https://github.com/user-attachments/assets/5c948f3f-9520-4077-99a6-b3a62a62f12a)

## Transmit:
Le maître drive seul le bus. Le premier octet transmis sera l'adresse de l'esclave, le second octet représente l'adresse du 1er registre à écrire. Les autres octets seront les données à transmettre.
L'esclave valide chaque octet en forçant le ACK à zéro (dominant) au bon moment.

![image](https://github.com/user-attachments/assets/4ee64dfc-ca60-41d0-91b1-2f3fb2ef1b9e)

## Recive:

Le maître drive le bus seulement pour l'adresse de l'esclave (1er octet). C'est ensuite l'esclave qui drive le bus pour permettre au maître de lire les données.
Le maître valide chaque octet en forçant le ACK à zéro (dominant) au bon moment, sauf le dernier (Non ACK ou NACK)
La sélection du registre à lire se fait en envoyant (transmit) auparavant l'adresse du registre à lire.

![image](https://github.com/user-attachments/assets/05ca7e6d-84c3-4104-a583-4253dc1b623a)

## Communication avec le BMP280

## *Identification du BMP280

![image](https://github.com/user-attachments/assets/7d2fcb21-14ee-435a-b68b-c3738b40383a)

Comme l'indique le tableau de la mémoire MAP, l'adresse du registre ID est 0x58, correspondant à la même valeur que celle obtenue dans Tera Term.

![image](https://github.com/user-attachments/assets/01a4f2e6-1cae-4f24-a7f4-bb6125334aea)




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
![image](https://github.com/user-attachments/assets/103ec964-def7-4f67-8a41-f51a49c39b00)


## TP2
![Capture d'écran 2024-11-12 181032](https://github.com/user-attachments/assets/7fe05d56-f98b-4827-ba57-e6b1ae369b58)

![Capture d'écran 2024-11-12 181532](https://github.com/user-attachments/assets/8657c7b7-f6c7-4c5b-bb61-f24a48c74135)

![Capture d'écran 2024-11-12 181805](https://github.com/user-attachments/assets/06bc0f44-fcbe-4310-811c-02cdfe0648b6)

![Capture d'écran 2024-11-12 182010](https://github.com/user-attachments/assets/9dbf1e90-e71b-41da-8ffb-30e3ade2b429)

![Capture d'écran 2024-11-12 190059](https://github.com/user-attachments/assets/d4de8b78-dbfa-4f81-8358-7dc915261580)






![image](https://github.com/user-attachments/assets/f747d370-21a3-4244-b433-55396e2bcd5a)


<img width="842" alt="Screenshot 2024-10-18 at 09 16 22" src="https://github.com/user-attachments/assets/2594276c-15a5-4f70-a4be-41b8cf8f4cb5">

