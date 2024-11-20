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

En I²C, la lecture se déroule de la manière suivante:

1-envoyer l'adresse du registre ID

2-recevoir 1 octet correspondant au contenu du registre

![image](https://github.com/user-attachments/assets/7d2fcb21-14ee-435a-b68b-c3738b40383a)

Comme l'indique le tableau de la mémoire MAP, l'adresse du registre ID est 0x58, correspondant à la même valeur que celle obtenue dans Tera Term.

![image](https://github.com/user-attachments/assets/6d9db1a0-615f-4edc-8ee1-434ac4d2bce0)


## Configuration du BMP280

Pour commencer, nous allons utiliser la configuration suivante: mode normal, Pressure oversampling x16, Temperature oversampling x2

En I²C, l'écriture dans un registre se déroule de la manière suivante:

1-envoyer l'adresse du registre à écrire, suivi de la valeur du registre

2-si on reçoit immédiatement, la valeur reçu sera la nouvelle valeur du registre

![Capture d'écran 2024-11-10 123838](https://github.com/user-attachments/assets/42a5cd92-1fc5-4ca3-8cc8-7079497a4f7e)


## Récupération de l'étalonnage, de la température et de la pression

Les paramètres de calibrage sont programmés dans la mémoire non volatile (NVM) des dispositifs lors de leur fabrication et ne peuvent pas être modifiés par l'utilisateur. Chaque valeur de compensation est un entier de 16 bits, stocké en complément à deux, et nécessite deux mots de 8 bits pour être représenté. Les registres de 8 bits, nommés calib00 à calib25, sont situés aux adresses mémoire de 0x88 à 0xA1. Les valeurs de compensation pour la température sont nommées dig_T# et celles pour la pression, dig_P#,comme le montre le tableau suivant:

![image](https://github.com/user-attachments/assets/d7ff87c1-2723-4d3d-a4ac-e59513f85355)

![image](https://github.com/user-attachments/assets/c6a8c4d9-b60f-43e5-b81a-2f4940301df8)

l'affichage de la pression a partir de la fonction BMP280_Pression, récupère la mesure de pression non compensée d’un capteur BMP280 via une communication I2C, puis calcule la pression compensée. Elle commence par définir une adresse de registre (0xF7) et l'envoie au capteur BMP280. En cas d’erreur de transmission, un message d’erreur est affiché. Ensuite, un tampon est alloué pour stocker les 3 octets de données de pression reçues du capteur. En cas d’erreur de réception, un message d’erreur est également affiché. meme pour la temperateur mais a partir de registre (0xFA)

![image](https://github.com/user-attachments/assets/77ed4bd4-7f32-4c0a-b4dc-94592510151f)

## Calcul des températures et des pression compensées

pour la calcule des températeur et des pressn compensées on a utulisé la datasheet du BMP280 le code permettant de compenser la température et la pression à l'aide des valeurs de l'étalonnage au format entier 32 bits.

![image](https://github.com/user-attachments/assets/0d31e23f-4fc2-4e91-b62f-3794b6df1d5f)

## Interfaçage de l'accléromètre

![image](https://github.com/user-attachments/assets/9a0f80ea-2985-48af-b3cd-68db98a38505)

## Conclusion

À partir de ce TP, nous avons configuré les capteurs BMP280 et MPU9250 pour afficher la température ainsi que la pression, en versions compensée et non compensée.

## TP2

L'objectif de ce TP est de mettre en place une communication entre une carte STM32 et un Raspberry Pi Zéro via une liaison série UART. Cette communication permettra de contrôler et monitorer différents périphériques de la carte STM32 depuis le Raspberry Pi.

  ### 3.1 Configuration du rasperry
Nous avons utilisé Rpi_imager pour installer Raspberry Pi OS Lite (32-bit) sur la carte SD. L'avantage de Rpi_imager est qu'il nous a permis de configurer directement :

- Le hostname : rasperrypi-911
- L'utilisateur : hugodevaux
- Le mot de passe : .......
- La connexion WiFi
- L'activation SSH

  ### 3.2 Premier démarrage
Après avoir inséré la carte SD et alimenté le Raspberry, nous avons :

Attendu environ 1 minute pour laisser le système démarrer
Utilisé nmap pour scanner le réseau et trouver l'IP du Raspberry : 192.168.88.235
Remarque: pour ce genre de configuration, il est judicieux d'attribué une IP statique au composant afin que l'IP de connexion soit toujours là même.

Ensuite, on se connecte en SSH :

<img width="842" alt="Screenshot 2024-10-18 at 09 16 22" src="https://github.com/user-attachments/assets/2594276c-15a5-4f70-a4be-41b8cf8f4cb5">


## Port Série
Nous allons boucler le port série du Raspberry Pi en connectant la pin RX (GPIO15) à la pin TX (GPIO14).

![image](https://github.com/user-attachments/assets/45bc34ab-8509-4189-92c6-9a34baec4366)

Pour tester la communication sur le port série de votre Raspberry Pi, nous allons utiliser un logiciel appelé minicom. Il vous permettra d'envoyer et de recevoir des données via le port série.

![image](https://github.com/user-attachments/assets/107fcfbc-4cf6-42c7-bcd4-4db3fb35de5c)

nous allons utulisé la commande ls /dev/tty* pour affiche tous les périphériques de type terminal (consoles, ports série, etc.) qui sont accessibles sur votre système.

![Capture d'écran 2024-11-12 190059](https://github.com/user-attachments/assets/d4de8b78-dbfa-4f81-8358-7dc915261580)

Une fois dans minicom configurer le port série en pressant CTRL+A suivi de O.

![Capture d'écran 2024-11-12 181532](https://github.com/user-attachments/assets/270eff34-5d9f-4a3e-8548-150095baa6f4)

en suite ona rentrée sur Serial Port Setup pour déactiver le contrôle de flux matériel (on utilise pas les lignes RTS/CTS).

![Capture d'écran 2024-11-15 163328](https://github.com/user-attachments/assets/3ffb7ec3-6701-4dcc-b29c-b3c53c0d57a0)


pour ecrire sur le minicom nous allons cliquer sur CNTRL+A E, Résultat Les caractères tapés s'affichaient à l'écran, confirmant le bon fonctionnement du port série.

![Capture d'écran 2024-11-12 182010](https://github.com/user-attachments/assets/9dbf1e90-e71b-41da-8ffb-30e3ade2b429)


### 3.3 Communication avec le STM32


On implémente le protocole suviant de communication entre le Raspberry et la STM32:

<img width="571" alt="Screenshot 2024-11-15 at 11 24 19" src="https://github.com/user-attachments/assets/c2e72882-f284-4d6e-9af0-fa283d20f40b">

### 3.4. Commande depuis Python

Pour établir la communication avec le port série, nous avons utilisé la bibliothèque pyserial. Cette solution permet d'interfacer efficacement le protocole de communication série avec Python, facilitant ainsi son intégration ultérieure dans une API REST. L'implémentation est disponible dans le fichier API/serial.py.

Environnement de développement
Choix d'une approche efficace
Pour optimiser le développement sur Raspberry Pi, nous avons délibérément évité l'utilisation d'éditeurs en ligne de commande comme VIM ou NANO, qui peuvent ralentir le processus de développement. À la place, nous avons opté pour une solution plus moderne et ergonomique.
Configuration du développement à distance
Nous avons mis en place un système de développement à distance utilisant PyCharm. Cependant, plutôt que d'utiliser la fonctionnalité de développement à distance native de PyCharm qui aurait nécessité :

L'installation d'un serveur PyCharm sur le Raspberry Pi
Une configuration complexe
Des ressources système supplémentaires
Une surface d'attaque de sécurité accrue

Nous avons privilégié une approche plus légère basée sur SFTP.
Solution SFTP retenue
Cette approche présente plusieurs avantages :

Synchronisation automatique des fichiers de code uniquement
Absence de fichiers de configuration IDE sur le Raspberry Pi
Maintenance simplifiée
Meilleure sécurité
Consommation minimale des ressources du Raspberry Pi

La configuration SFTP permet un déploiement ciblé et efficace, ne transférant que les fichiers nécessaires à l'exécution du code.
Ci-dessous, la configuration SFTP:

<img width="563" alt="Screenshot 2024-11-15 at 11 32 55" src="https://github.com/user-attachments/assets/2101b1d2-86c5-4715-9d88-71b28de23629">
<img width="602" alt="Screenshot 2024-11-15 at 11 33 17" src="https://github.com/user-attachments/assets/56eb06b4-c047-48cf-9b2e-e1abd6de528d">

## TP3 - Interface REST
L'objectif de ce TP est de développer une interface REST sur le Raspberry Pi, permettant d'assurer une communication efficace et standardisée avec d'autres systèmes ou dispositifs via des requêtes HTTP.
## 4.1 Installation du serveur Python
## Installation
Créez votre propre utilisateur différent de pi, remplacer XXX par le nom de votre choix, avec les droits de sudo et  d'accès au port série (dialout):

sudo adduser XXX
sudo usermod -aG sudo XXX
sudo usermod -aG dialout XXX

*sudo usermod -aG sudo XXX:Cette commande ajoute l'utilisateur XXX au groupe sudo, ce qui lui accorde les droits administratifs.
L'option -aG signifie : ajouter (a) au groupe (G) sans modifier l'appartenance actuelle de l'utilisateur à d'autres groupes.

*sudo usermod -aG dialout XXX:Cette commande ajoute l'utilisateur XXX au groupe dialout. Ce groupe donne l'autorisation d'accéder aux ports série (par exemple, /dev/ttyS0 ou /dev/ttyUSB0), ce qui est nécessaire pour utiliser des périphériques connectés au port série.

installation de pip pour python3 sur le Raspberry:
*sudo apt update
-Cette commande met à jour la liste des paquets disponibles sur le Raspberry Pi.
-Cela garantit que les versions des logiciels installés ou disponibles sont les plus récentes.

*sudo apt install python3-pip
-Cette commande installe pip pour Python 3.
-pip est un outil essentiel pour gérer les bibliothèques Python et installer des dépendances nécessaires pour vos projets (comme Flask, NumPy, etc.).
-L'installation se fait avec sudo car elle modifie les paquets système, ce qui nécessite des droits administratifs.



On commence par faire une route API, simple retournant simplement un message de bienvenue: 
![image](https://github.com/user-attachments/assets/ab1e3d64-7ff6-4e4e-850a-363a423d2575)

### 1.1 Exemple de route

```
@app.route('/api/welcome/', methods=['GET', 'POST', 'DELETE'])
def api_welcome():
    global welcome

    if request.method == 'GET':
        return jsonify({'data': welcome})

    elif request.method == 'POST':
        # Change sentence
        received_data = request.get_json()
        if 'sentence' in received_data:
            welcome = received_data['sentence']
            return jsonify({'data': welcome})
        abort(400)

    elif request.method == 'DELETE':
        # Delete sentence
        welcome = ""
        return jsonify({'data': welcome})
```


Avec Flask (et FastAPI), on utilise pour chaque fonction un décodeur `@app.route`, pour indiquer qu'en cas de requête HTTP à l'url indiqué dans le décodeur, il faut executer cette fonction.
Pa exemple `@app.route('/api/welcome/', methods=['GET', 'POST', 'DELETE'])`, indique que l'on autorise les méthodes GET, POST et DELETE, pour l'url baseUrl/api/welcome.
Le décodeur est associé à la fonction, `welcome()`, qui sera executé à l'url.
e role du fragment <int:index> dans l'url indique que cette partie de l'url correspond à une variable, ici on passe un entier. Utilisé pour faire des GET pour obtenir une donnée précise, ne nécéssitant pas de POST. 

Ensuite, conformément au proptocole web (HTTP), on indique le type de retour:
![image](https://github.com/user-attachments/assets/e164d4ae-227b-4d9e-8a26-4b59a2d408bc)

![image](https://github.com/user-attachments/assets/18ea298b-eb2e-448d-a3af-274b879fb00e)



![image](https://github.com/user-attachments/assets/b7a74638-c4d6-48cd-a77f-b82b6db066fc)
![image](https://github.com/user-attachments/assets/39673bcc-b56e-4dff-a2a1-07241945a1f3)
![image](https://github.com/user-attachments/assets/45cdbf3e-986b-4c73-a15b-86b125bb6462)


![image](https://github.com/user-attachments/assets/588a33a7-e7a4-4beb-b79a-538624618db4)

![image](https://github.com/user-attachments/assets/44034d09-ede3-4fe4-838d-3082284b2ae6)
![image](https://github.com/user-attachments/assets/8444fca0-bc42-4912-a97a-fcdbeb6ab412)
![image](https://github.com/user-attachments/assets/68cf3182-87c7-4f6e-8ee9-bfd45670d524)

### API CRUD 

On ajoute les méthodes HTTP suivantes:
<img width="525" alt="Screenshot 2024-11-15 at 15 08 29" src="https://github.com/user-attachments/assets/43d60f2e-803e-422f-b095-8a63ae820b55">

Ci-dessous, des exemples d'utilisations:

#### GET
<img width="1155" alt="Screenshot 2024-11-15 at 15 06 29" src="https://github.com/user-attachments/assets/1fc4abdd-f4ff-42fb-adfe-472143ee74ac">

#### POST 
<img width="1155" alt="Screenshot 2024-11-15 at 15 06 53" src="https://github.com/user-attachments/assets/cacdc7c9-040c-4afe-82bf-e5f69eef13d6">

#### DELETE
<img width="1155" alt="Screenshot 2024-11-15 at 15 07 10" src="https://github.com/user-attachments/assets/07d6a2e0-e898-4e99-942b-0e5932ca2cc1">


Remarque: tout le codes des autres méthodes se trouve dans le fichier `api/hello.py`. Nous avons utiliser HTTPie comme client HTTP pour effectué les tests.
## TP4 Bus CAN

L'objectif de ce tp Développement d'une API Rest et mise en place d'un périphérique sur bus CAN

![image](https://github.com/user-attachments/assets/9905232c-8348-4857-b2c9-54bdc756f4a8)

Les cartes STM32L476 sont équipées d'un contrôleur CAN intégré. Pour pouvoir les utiliser, il faut leur adjoindre un Tranceiver CAN. Ce rôle est dévolu à un TJA1050. Ce composant est alimenté en 5V, mais possède des E/S compatibles 3,3V.

Afin de faciliter sa mise en œuvre, ce composant a été installé sur une carte fille (shield) au format Arduino, qui peut donc s'insérer sur les cartes nucléo64:

![image](https://github.com/user-attachments/assets/c4954c0c-7073-4c1c-a7b3-50dbe391ba07)

Ce shield possède un connecteur subd9, qui permet de connecter un câble au format CAN. Pour rappel, le brochage de ce connecteur est le suivant:

![image](https://github.com/user-attachments/assets/9e181161-c346-4580-a5ca-4e5ff186d56d)

Nous allons utiliser le bus CAN pour piloter un module moteur pas-à-pas. Ce module s'alimente en +12V.
pour piloté se moteur on a le Mode de calcul automatique de l'angle : il vous suffit d'envoyer une trame CAN configurée avec l'angle du moteur, puis le moteur tournera jusqu'à atteindre le nouvel angle.

Composition de la trame :

* Donnée D0 : Angle souhaité, entre 0° et 180° (si >180°, l'angle sera limité à 180°)
* Donnée D1 : Représente le signe de l'angle

Et le Mode manuel :
Ce mode vous permet d'envoyer une trame CAN complexe pour contrôler :

Le nombre de pas
Le sens de rotation
La vitesse du moteur
![image](https://github.com/user-attachments/assets/849f8872-cc77-4abd-95be-ddb0f23f650f)

La carte moteur est un peu capricieuse et ne semble tolérer qu'une vitesse CAN de 500kbit/s. Pensez à régler CubeMx en conséquence.
Edit 2022: Il semble que ce soit surtout le ratio seg2/(seg1+seg2), qui détermine l'instant de décision, qui doit être aux alentours de 87%. pour cela on a coniguré le péripherque CAN a partir de site http://www.bittiming.can-wiki.info/ 

Pour notre projet, nous avons une fréquence de 42 MHz et une vitesse de communication de la carte moteur de 500 kbit/s, comme indiqué dans l'image ci-dessous.

![image](https://github.com/user-attachments/assets/eab2da42-3152-44b9-acbc-d370b974d251)

 D'après le tableau de calcul, pour obtenir un débit de 500 kbit/s, il est nécessaire de configurer le Time Quanta dans le segment de bit 1 à 11 et dans le segment de bit 2 à 2.

 ![image](https://github.com/user-attachments/assets/87ec74f5-8a07-42af-a370-6fc435600828)

Dans cette étape, nous avons activé le CAN1 et l'avons configuré en fonction des résultats obtenus précédemment:
 
 ![image](https://github.com/user-attachments/assets/3e35490f-6559-4e8f-8326-05d6874f2a8b)

 ## Pilotage du moteur

 pour que  le moteur tourne de 90° dans un sens, puis de 90° dans l'autre, avec une période de 1 seconde.nous allons utulisé les primitives HAL de CAN:

* HAL_StatusTypeDef HAL_CAN_Start (CAN_HandleTypeDef * hcan):La fonction HAL_CAN_Start de la bibliothèque HAL pour STM32 permet de démarrer le périphérique CAN. Elle permet d'initialise et d'active la communication CAN pour permettre l'envoi et la réception de messages.
  
![image](https://github.com/user-attachments/assets/ad566af7-0148-4d0e-a9ae-1ab9e296cedc)


*aprés on a utulisé La variable TxHeader est une structure contenant les champs suivants, que vous devez remplir avant de faire appel à HAL_CAN_AddTxMessage:

![image](https://github.com/user-attachments/assets/8c20652d-e2d2-4e4b-bdba-febec0b7d801)

Voici un exemple de code simple pour commander le moteur et le faire tourner de 90° dans les deux sens :

![image](https://github.com/user-attachments/assets/4de9d2c3-1e82-4a14-81d1-fb89ef911127)

## Interfaçage avec le capteur
Dans cette partie on a calcule la vitesse et l'angle a partir de la valeur de temperateur

![image](https://github.com/user-attachments/assets/b3ff8186-35a8-46e0-9a30-9ae5e426e84b)

Le programme ajuste la vitesse et le sens de rotation d’un moteur en fonction de la température mesurée. La valeur de température est convertie en vitesse, envoyée via le bus CAN, et un message de confirmation est affiché sur un terminal série. Ce type de contrôle est courant dans les systèmes embarqués nécessitant un ajustement dynamique basé sur des capteurs environnementaux.










 

 



