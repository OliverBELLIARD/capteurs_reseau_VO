# ***Projet Capteurs & reseaux***
> *Auteurs : Oliver BELLIARD & Valérian PRIOU*  
> *Enseignant encadrant : Christophe BARES*

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
Ce composant dispose de deux adresses disponibles en slave mode: 0x76 et 0x77. On peut changer cette adresse en mettant à 0 ou 1 le bit "SDO".

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
qui sont détaillés dans le code qui y est donné.

## 2.2. Setup du STM32
### Configuration du STM32

Sous STM32CubeIDE, mettre en place une configuration adaptée à votre carte de développement STM32.

Pour ce TP, vous aurez besoin des connections suivantes:
- d'une liaison I²C. Si possible, on utilisera les broches compatibles avec l'empreinte arduino (broches PB8 et PB9) (Doc nucleo 64)
- d'une UART sur USB (UART2 sur les broches PA2 et PA3) (Doc nucleo 64)
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
Testez ce printf avec un programme de type echo.
