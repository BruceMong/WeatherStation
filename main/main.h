/**********************************************************************/
/***  fichier: main.h                                               ***/
/***  en-tete de main.c                                             ***/
/**********************************************************************/

#ifndef MAIN_H
#define MAIN_H

//#include <SoftwareSerial.h>

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
#define SIZE_OF_SD 256
#define SIZE_OF_CLUSTER_IN_KB 4

#define NB_CAPTORS 4


typedef struct
{
  String name;      //ptetre enlever progmem si on peux pas modif Progmem (a test)
  short working;
  String lowName;
  short lowValue;
  String highName;
  short highValue;
  short maxDom;
  short minDom;

  short value;
  short compteurTimeout;

} Parametre;

Parametre capteurs[NB_CAPTORS];  
if(NB_CAPTORS >= 1) capteurs[0];
if(NB_CAPTORS >= 2) capteurs[1];
if(NB_CAPTORS >= 3) capteurs[2];
if(NB_CAPTORS >= 4) capteurs[3];
  //module complémentaire à incorporer au projet
if(NB_CAPTORS >= 5) capteurs[4];
if(NB_CAPTORS >= 6) capteurs[5];
if(NB_CAPTORS >= 7) capteurs[6];
if(NB_CAPTORS >= 8) capteurs[7];

volatile int firstLoop = 0; //sert pour le mod config du début

volatile boolean modConfig = false;   //faire bool quand j'ai temps
volatile boolean modEco = false;
volatile boolean modMaintenance = false;

volatile boolean redPush = false;
volatile boolean greenPush = false;
volatile short greenTimer = 0;
volatile short redTimer = 0;

int SIZE_SD_CARD = SIZE_OF_SD * 1000;
short FILE_MAX_SIZE; //ptetre pas volatile du coup // oui faut mettre const sinon plant
short LOG_INTERVALL; 
short TIMEOUT;    // si short impossible penser a casté en short pour la suite 

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
