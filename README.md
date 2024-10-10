<img width="1104" alt="Screenshot 2024-10-10 at 08 27 51" src="https://github.com/user-attachments/assets/ae6aea65-0050-4f10-aea6-8499c9085bf0"># ENSEA_ESE_2425_Networks
TP Bus et Réseaux

## 1. Présentation!
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

#### La table des adresses I2C Possible du composants:
L'adresse de l'appareil sur 7 bits est 111011x.
- 1110110 (0x76), SDO à la masse
- 1110111 (0x77), SDO sur VDD_{IO}
