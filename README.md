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
1. les adresses I²C possibles pour ce composant.  
2. le registre et la valeur permettant d'identifier ce composant  
3. le registre et la valeur permettant de placer le composant en mode normal  
4. les registres contenant l'étalonnage du composant  
5. les registres contenant la température (ainsi que le format)  
6. les registres contenant la pression (ainsi que le format)  
7. les fonctions permettant le calcul de la température et de la pression compensées, en format entier 32 bits.  
