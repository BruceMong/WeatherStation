/**********************************************************************/
/***  fichier: main.h                                               ***/
/***  en-tete de main.c                                             ***/
/**********************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include <SoftwareSerial.h> //UART GPS 
#include <Wire.h> //communication led 
#include "DS1307.h" // clock RTC //bizarre 
#include <SPI.h> //pour communication SPI ( carte SD) 
#include <SD.h> //pour la carte SD 
#include <avr/pgmspace.h> //PROGMEM library

//inclusion du define de la version + numérot de lot via makefile (avant compil)
//exmple aperçu :
//#define DEBUG
//#define VERSION 1.1
//#define NUM_LOT 12

//PIN à définir plus tard 
#define GREENPUSHPIN PIN
#define REDPUSHPIN PIN

#define REDLEDPIN PIN
#define GREENLEDPIN PIN
#define BLUELEDPIN PIN

#define CHIPSELECTPIN PIN

#define LUMIN_PIN PIN
#define TEMP_AIR_PIN PIN
#define HYGR_PIN PIN
#define PRESSURE_PIN PIN

//info SD card
#define SIZE_OF_SD_IN_KB 256000
#define SIZE_OF_CLUSTER_IN_KB 4

typedef struct
{
  PROGMEM String name;      //ptetre enlever progmem si on peux pas modif Progmem (a test)
  PROGMEM short working;
  PROGMEM String lowName;
  PROGMEM short lowValue;
  PROGMEM String highName;
  PROGMEM short highValue;
  PROGMEM short maxDom;
  PROGMEM short minDom;

  short value;
  short compteurTimeout;

} Parametre;

Parametre capteurs[NB_CAPTORS];  

volatile int firstLoop = 0; //sert pour le mod config du début

volatile int modConfig = 0;   //faire bool quand j'ai temps
volatile int modEco = 0;
volatile int modMaintenance = 0;

volatile boolean redPush = false;
volatile boolean greenPush = false;
volatile short greenTimer = 0;
volatile short redTimer = 0;

const PROGMEM int SIZE_SD_CARD = SIZE_OF_SD_IN_KB;
const PROGMEM short FILE_MAX_SIZE; //ptetre pas volatile du coup // oui faut mettre const sinon plant
const PROGMEM short LOG_INTERVALL; 
const PROGMEM short TIMEOUT;    // si short impossible penser a casté en short pour la suite 

volatile int compteurEco = 0;  //sert pour faire le modulo 2 (mesure) voir shéma

String nameFile2;   //def en global pour pouvoir les fermer en cas d'interruption
String nameFile;
File fichier;
File fichier2;

// il faut faire précéder la déclaration de la fonction du mot-clef extern, qui signifie que cette fonction est définie dans un autre fichier.
extern void RGB_color(short red_light_value, short green_light_value, short blue_light_value);
extern void ft_error_led(short r1, short g1, short b1, short r2, short g2, short b2, short hz1, short hz2);
extern void ledMod();
extern void ft_checkSizeSDcard(); //compte le nombre de fichier 
extern void eventRedPush();
extern void eventGreenPush();
extern void initSD();
extern void endSD();
extern String ft_processValue(short value, short indCap);
extern String getNameFileNext(char *nameFile);
extern void ft_CheckSDcardFull(); 
extern String ft_acqui_GPS();
extern String ft_acqui_Capteur(short i);
extern void ft_SaveDataSDcard(String dataString);
extern void setup(); 
extern void loop();

#endif