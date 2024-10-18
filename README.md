# ***Projet Capteurs & reseaux***
> *Auteurs : Oliver BELLIARD & Valérian PRIOU*  
> *Enseignant encadrant : Christophe BARES*  
> *Boite utilisée : n°6*

# 1. Présentation
Ensemble de TP de Bus & Réseaux. Ces TP seront réalisés en C pour la partie STM32, et Python pour la partie Raspberry Pi.
L'échelonnement des TP est le suivant:  
1. Interrogation des capteurs par le bus I²2C  
2. Interfaçage STM32 <-> Raspberry Pi  
3. Interface Web sur Raspberry Pi  
4. Interface API Rest & pilotage d'actionneur par bus CAN  

# 2. TP 1 - Bus I2C
## 2.1. Capteur BMP280
À partir de la datasheet du [BMP280](https://moodle.ensea.fr/mod/resource/view.php?id=1910), on identifie les éléments suivants:
1. les adresses I²C possibles pour ce composant :  
Ce composant dispose de deux adresses sur 7 bits 111011x disponibles en slave mode: 0x76 et 0x77. On peut changer cette adresse en mettant à 0 ou 1 le bit "SDO".

2. le registre et la valeur permettant d'identifier ce composant :  
Le registre permettant d'identifier ce composant est à l'adresse 0xD0,  il doit contenir un chip_id à la valeur "0x58".

3. le registre et la valeur permettant de placer le composant en mode normal :  
Le mode normal peut être activé en modifiant le registre de control à l'adresse
0xF4, c'est un registre de 2 bits qu'il faut mettre à la valeur "11".

4. les registres contenant l'étalonnage du composant :  
Les registres d'étalonnage du composant sont situés aux adresses "0x88, ..., 0xA1"

5. les registres contenant la température (ainsi que le format) :  
Les registres contenant les valeurs de température sont présents aux adresses
"0xFA, 0xFB, 0xFC". La température est codée sur 20 bits, d'où la necessité d'avoir 
plusieurs adresses. FA le MSB, FB le LSB et FC le XLSB.

6. les registres contenant la pression (ainsi que le format) :  
Les registres contenant les valeurs de pression sont présents aux adresses
"0xF7, 0xF8, 0xF9". La pression est codée sur 20 bits, d'où la necessité d'avoir 
plusieurs adresses. F7 le MSB, F8 le LSB et F9 le XLSB.

7. les fonctions permettant le calcul de la température et de la pression compensées, en format entier 32 bits :    
Les fonctions permettant les calculs compensés de la température et de la pression
sont données dans la datasheet page 22. On doit se servir de coefficients de compensation
qui sont détaillés dans le code qui y est donné :

```C
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BMP280_S32_t t_fine;
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
{
BMP280_S32_t var1, var2, T;
var1 = ((((adc_T>>3) – ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
var2 = (((((adc_T>>4) – ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) – ((BMP280_S32_t)dig_T1))) >> 12) *
((BMP280_S32_t)dig_T3)) >> 14;
t_fine = var1 + var2;
T = (t_fine * 5 + 128) >> 8;
return T;
}
“”–
// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BMP280_U32_t bmp280_compensate_P_int64(BMP280_S32_t adc_P)
{
BMP280_S64_t var1, var2, p;
var1 = ((BMP280_S64_t)t_fine) – 128000;
var2 = var1 * var1 * (BMP280_S64_t)dig_P6;
var2 = var2 + ((var1*(BMP280_S64_t)dig_P5)<<17);
var2 = var2 + (((BMP280_S64_t)dig_P4)<<35);
var1 = ((var1 * var1 * (BMP280_S64_t)dig_P3)>>8) + ((var1 * (BMP280_S64_t)dig_P2)<<12);
var1 = (((((BMP280_S64_t)1)<<47)+var1))*((BMP280_S64_t)dig_P1)>>33;
if (var1 == 0)
{
return 0; // avoid exception caused by division by zero
}
p = 1048576-adc_P;
p = (((p<<31)-var2)*3125)/var1;
var1 = (((BMP280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
var2 = (((BMP280_S64_t)dig_P8) * p) >> 19;
p = ((p + var1 + var2) >> 8) + (((BMP280_S64_t)dig_P7)<<4);
return (BMP280_U32_t)p;
```

## 2.2. Setup du STM32
### Configuration du STM32

Sous STM32CubeIDE, mettre en place une configuration adaptée à votre carte de développement STM32.

Pour ce TP, vous aurez besoin des connections suivantes:
- d'une liaison I²C. Si possible, on utilisera les broches compatibles avec l'empreinte arduino (broches PB8 et PB9) [(Doc nucleo 64)](https://moodle.ensea.fr/mod/resource/view.php?id=1911)
- d'une UART sur USB (UART2 sur les broches PA2 et PA3) [(Doc nucleo 64)](https://moodle.ensea.fr/mod/resource/view.php?id=1911)
- d'une liaison UART indépendante pour la communication avec le Raspberry (TP2)
- d'une liaison CAN (TP4)

Prenez soin de bien choisir les ports associés à chacun de ces bus.

### Redirection du print

Afin de pouvoir facilement déboguer votre programme STM32, faites en sorte que la fonction printf renvoie bien ses chaînes de caractères sur la liaison UART sur USB, en ajoutant le code suivant au fichier `stm32f4xx_hal_msp.c` :  

```c
    /* USER CODE BEGIN PV */
    extern UART_HandleTypeDef huart2;
    /* USER CODE END PV */

    /* USER CODE BEGIN Macro */
    #ifdef __GNUC__ /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf    set to 'Yes') calls __io_putchar() */
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
    #else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
    #endif /* __GNUC__ */
    /* USER CODE END Macro */

    /* USER CODE BEGIN 1 */
    /**
    * @brief  Retargets the C library printf function to the USART.
    * @param  None
    * @retval None
    */
    PUTCHAR_PROTOTYPE
    {
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART2 and Loop until the end of transmission */
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

    return ch;
    }
    /* USER CODE END 1 */
```

Il est aussi possibe de réécrire la fonction `__io_putchar` en reprenant son prototype, c'est la méthode que nous avons choisi :
```c
    /* Private user code ---------------------------------------------------------*/
    /* USER CODE BEGIN 0 */
    int __io_putchar(int ch)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

        return ch;
    }

    /* USER CODE END 0 */
```
Cette approche permet de ne pas aller modifier d'autres fichiers et de centraliser cette unique fonction de liaison série spécialisée dans le `main.c`.  

### Test de la chaîne de compilation et communication UART sur USB
> Testez ce printf avec un programme de type echo.  

Pour voir le retour de la lisaison série USB (USART2) nous utilisons le programme "minicom" sur linux (Ubuntu) qui permet de tout gérer depuis un terminal. Une fois le paquet installé, il suffit de lancer la communication avec la commande suivante sur un terminal :
```bash
minicom -D dev/ttyACM0
```

Ici notre périphérique est le `ttyACM0` mais il peut varier selon les postes. Une fois lancée la commande et la carte redémarée, on observe sur notre terminal :

```
Welcome to minicom 2.9

OPTIONS: I18n 
Port /dev/ttyACM0, 09:36:39

Press CTRL-A Z for help on special keys


=== TP Capteurs & Réseaux ===

```

## 2.3. Communication I²C
### Primitives I²C sous STM32_HAL

L'API HAL (Hardware Abstraction Layer) fournit par ST propose entre autres 2 primitives permettant d'interagir avec le bus I²C en mode Master:

    HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)

    HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)

où:

    I2C_HandleTypeDef hi2c: structure stockant les informations du contrôleur I²C

    uint16_t DevAddress: adresse I³C du périphérique Slave avec lequel on souhaite interagir.

    uint8_t *pData: buffer de données

    uint16_t Size: taille du buffer de données

    uint32_t Timeout: peut prendre la valeur HAL_MAX_DELAY

### Communication avec le BMP280

#### Identification du BMP280

L'identification du BMP280 consiste en la lecture du registre ID

En I²C, la lecture se déroule de la manière suivante :

1. envoyer l'adresse du registre ID
2. recevoir 1 octet correspondant au contenu du registre

Vérifiez que le contenu du registre correspond bien à la datasheet.
Vérifiez à l'oscilloscope que la formes des trames I²C est conforme.

#### Configuration du BMP280

Avant de pouvoir faire une mesure, il faut configurer le BMP280.

Pour commencer, nous allons utiliser la configuration suivante: mode normal, Pressure oversampling x16, Temperature oversampling x2

En I²C, l'écriture dans un registre se déroule de la manière suivante:

1. envoyer l'adresse du registre à écrire, suivi de la valeur du registre
2. si on reçoit immédiatement, la valeur reçu sera la nouvelle valeur du registre

Vérifiez que la configuration a bien été écrite dans le registre.

#### Récupération de l'étalonnage, de la température et de la pression

Récupérez en une fois le contenu des registres qui contiennent l'étalonnage du BMP280.

Dans la boucle infinie du STM32, récupérez les valeurs de la température et de la pression. Envoyez sur le port série le valeurs 32 bit non compensées de la pression de la température.

#### Calcul des températures et des pression compensées

Retrouvez dans la datasheet du STM32 le code permettant de compenser la température et la pression à l'aide des valeurs de l'étalonnage au format entier 32 bits (on utilisera pas les flottants pour des problèmes de performance).

Transmettez sur le port série les valeurs compensés de température et de pression sous un format lisible.

# 3. TP2 - Interfaçage STM32 - Raspberry
## 3.1. Mise en route du Raspberry PI Zéro
Le Pi Zero est suffisamment peu puissant pour être alimenté par le port USB de l'ordinateur.

### Préparation du Raspberry
Téléchargez l'image "Raspberry Pi OS (32-bit) Lite" et installez la sur la carte SD, disponible à cette adresse:  https://www.raspberrypi.org/downloads/raspberry-pi-os/.  

Pour l'installation sur la carte SD, nous avons utilise : Rpi_Imager: https://www.raspberrypi.com/software/  

Rpi_imager va nous permettre de choisir l'image, de la télécharger et de la configurer.

Configuration réseau du routeur utilisé en TP :  
SSID : ESE_Bus_Network  
Password : ilovelinux  

Pour activer le port série sur connecteur GPIO, sur la partition boot, modifiez le fichier config.txt pour ajouter à la fin les lignes suivantes :  
```
enable_uart=1  
dtoverlay=disable-bt  
```
### Premier démarrage
On installe la carte SD dans le Raspberry puis on branche l'alimentation.

> utilisateur : voese  
> mdp : voese  
> IP : 192.168.88.230

Pour nous connecter en SSH au Raspberry nous utilisons la commande :
```bash
ssh voese@192.168.88.230
```
  
On utilisez ssh pour vous connecter à votre Raspberry.  
Comment le Raspberry a obtenu son adresse IP ? 

Le Raspberry Pi a obtenu son adresse IP via le réseau Wi-Fi auquel il s'est connecté, spécialement paramétré pour le TP : ESE_Bus_Network.  
Celui-ci est de la forme "192.168.88.XXX" avec les "X" définis par l'ordre auquel on est arrivé sur le réseau  
par rapport aux autres élèves de la classe.
