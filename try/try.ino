/**********************************************************************/
/***  fichier: main.c                                               ***/
/***  tout le programme sauf la partie config                       ***/
/**********************************************************************/




// je déclare les libs dans le .c et non dans le .h car sinon bug du makefile jcpa pq 
#include <SoftwareSerial.h> //UART GPS 
#include <Wire.h> //communication led 
#include "DS1307.h" // clock RTC //bizarre 
#include <SPI.h> //pour communication SPI ( carte SD) 
#include <SD.h> //pour la carte SD 
#include <avr/pgmspace.h> //PROGMEM library
//

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
#define VERSION 1.1
#define NUM_LOT 12


//PIN à définir plus tard 
#define GREENPUSHPIN 5
#define REDPUSHPIN 9

#define REDLEDPIN 8
#define GREENLEDPIN 6
#define BLUELEDPIN 5

#define CHIPSELECTPIN 4

#define LUMIN_PIN 5
#define TEMP_AIR_PIN 4
#define HYGR_PIN 2
#define PRESSURE_PIN 2

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
  short pinCap;

} Parametre;

Parametre capteurs[NB_CAPTORS];  

volatile int firstLoop = 0; //sert pour le mod config du début

volatile boolean modConfig = false;   //faire bool quand j'ai temps
volatile boolean modEco = false;
volatile boolean modMaintenance = false;

volatile boolean redPush = false;
volatile boolean greenPush = false;
volatile short greenTimer = 0;
volatile short redTimer = 0;

long SIZE_SD_CARD_IN_KB = SIZE_OF_SD * 1000L;
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

//include "main.h"
#include "ft_configuration.h"
#include "ft_time.h"

SoftwareSerial SoftSerial(2, 3); // Serial already used for serial communication GPS connected on D2 port on Grove Shield
DS1307 clock;//define a object of DS1307 class RTC Clock on I2C port on Grove Shield


void RGB_color(short red_light_value, short green_light_value, short blue_light_value)
 {
  analogWrite(REDLEDPIN, red_light_value);
  analogWrite(GREENLEDPIN, green_light_value);
  analogWrite(BLUELEDPIN, blue_light_value);
}

void ft_error_led(short r1, short g1, short b1, short r2, short g2, short b2, short hz1, short hz2)
{
  RGB_color(r1, g1, b1);
  delay(hz1 * 1000);
  RGB_color(r2, g2, b2);
  delay(hz2 * 2000);
}

void ledMod()
{
  if(modConfig == true)
    RGB_color(0, 0, 255); // BLUE
  else if(modMaintenance == true)
    RGB_color(255, 122, 0);  // ORANGE
  else
    RGB_color(0, 255, 0);    //  GREEN 
}

void ft_checkSizeSDcard() //compte le nombre de fichier 
{
  int i;
  i = 0;
  File root;
  root = SD.open("/");
 
  while(true)
  {
    File entry = dir.openNextFile();
    if(!entry)
    {
      SIZE_SD_CARD_IN_KB = SIZE_SD_CARD_IN_KB - ( long(SIZE_OF_CLUSTER_IN_KB * i));
      return;
    }
  i++;
  }
}

void eventRedPush()
{
  redPush = !redPush;

  if(redPush == true)
    redTimer = getClockInSec();

  if( (redPush == false) && ( getClockInSec() > (redTimer + 5) ) )
  {
    if(firstLoop == 0)
      modConfig = true;
    else
    {
      if(modConfig == false)
      {
        if(modEco == false)
        {
          if(modMaintenance == false)
          {
            modMaintenance = true;
            ledMod();
            endSD();
            Serial.println(F("[END] mod Standard, [BEGIN] mod Maintenance."));
            //goto begin;
          }
          else 
          {
            modMaintenance = false;
            ledMod();
            initSD();
            Serial.println(F("[END] mod Maintenance, [BEGIN] mod Standard."));
            //goto begin;
          }
        }
        else 
        {
          modEco = false;
          compteurEco = 0;    
          LOG_INTERVALL = LOG_INTERVALL / 2;
          ledMod();
          Serial.println(F("[END] mod Eco, [BEGIN] mod Standard."));
        }
      }
      else
        modConfig = false;
    }
  }
}

void eventGreenPush()
{
  greenPush = !greenPush;

  if(greenPush == true)
    greenTimer = getClockInSec();

  if( (greenPush == false) && ( getClockInSec() > (greenTimer + 5) ))
  {
    if(modConfig == false)
    {
      modEco = true;
      ledMod();
      LOG_INTERVALL = LOG_INTERVALL * 2;
      initSD();
      Serial.println(F("[END] mod Standard, [BEGIN] mod Eco."));
    }
  }
}

void initSD()
{
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print(F("Initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(CHIPSELECTPIN)) 
  {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    while(1);
    // LED intermittente rouge et blanche (fréquence 1Hz, durée 2 fois plus longue pour le blanc)

  }
  ft_checkSizeSDcard();
  Serial.println(F("card initialized."));
}

void endSD()
{
  fichier.close();                
  fichier2.close();
  //SD.end(CHIPSELECTPIN); //useless: ça existe pas
  Serial.println(F("You can retire SD card."));
}

String ft_processValue(short value, short indCap)
{
  if(value < capteurs[indCap].lowValue)
  {
    switch(indCap)
    {
      case 0:
      return("faible");
      case 1:
      return("erreur"); // définition du seuil de température de l'air (en °C) en dessous duquel le capteur se mettra en erreur.
      case 2:
      return("pasCompte");
      case 3:
      return("erreur");
    }
  }
  if(value < capteurs[indCap].highValue)
  {
    switch(indCap)
    {
      case 0:
      return("forte");
      case 1:
      return("erreur");
      case 2:
      return("pasCompte");
      case 3:
      return("erreur");
    }
  }
  switch(indCap)
  {
     case 0:
     return("moyenne");
     case 1 || 2 || 3:
     return(String(value)); 
  }
  return("ERROR");
}



String getNameFileNext(char *nameFile)
{
  //Problème de cette fonction c'est qu'elle gère qu'a "xx_9.log" ensuite ça plante
  // maybe use endWith ? 
  //replace().La fonction String replace () vous permet de remplacer toutes les occurrences d’un caractère donné par un autre caractère. Vous pouvez également utiliser replace pour remplacer les sous-chaînes d’une chaîne par une autre sous-chaîne.

  String nameFileNew = String(nameFile);

  while(SD.exists(nameFileNew) == true)
  {
    nameFile[7]++;
    nameFileNew = String(nameFile);
  }
  return(nameFileNew);
}

void ft_CheckSDcardFull()
{
  while(modConfig == false && SIZE_SD_CARD_IN_KB <= 0)  
    ft_error_led(255, 0, 0, 255, 255, 255, 1, 1);
  ledMod();
}

String ft_acqui_GPS()
{
  short compteurTimeout; 
  short timer;
  boolean gpsDone;
  String gpsData; 

  compteurTimeout = 0;
  timer = 0;
  gpsDone = false;
  gpsData = "";

  
  if(modEco == true)
    compteurEco++;
  if(compteurEco % 2 == 0) //mode normal compteurEco = 0 donc ça s'éxecute à chaque fois sinon 1/2
  {
    //code acquisition gps
    while(1)
    {
      timer = getClockInSec();
      while( (getClockInSec() > (timer + TIMEOUT) ) && (gpsDone == false) )
      {
        if(SoftSerial.available() )
        {
          gpsData = SoftSerial.readStringUntil('\n');
          if(gpsData.startsWith("$GPGGA",0))
            gpsDone = true; 
        }
        else
          ft_error_led(255, 0, 0, 255, 255, 0, 1, 1); // LED intermittente rouge et jaune (fréquence 1Hz,durée identique pour les 2 couleurs)
      }
      if(compteurTimeout >= 2) //si compteur timeout == 2 quitté et retourné NA
        return("NA");
      else if(gpsDone == false)//si TIMEOUT incrémenté compteur timeout et refaire boucle
      {
        compteurTimeout++;
        continue;
      }
      else 
        return(gpsData); //sinon ça veux dire que c'est bon et retourné la valeur
    }
  }
  else
    return("ECO");
}

String ft_acqui_Capteur(short i)
{
  short compteurTimeout; 
  short timer;
  String tempString;

  tempString = "";
  timer = 0;
  compteurTimeout = 0;


  timer = getClockInSec();
  if(capteurs[i].working != 0)
  {
    while((tempString == "PasCompte" || tempString == "") && (getClockInSec() > (timer + TIMEOUT)))
    tempString = ft_processValue(analogRead(capteurs[i].pinCap), i);
    if(tempString == "PasCompte")
    {
      if(compteurTimeout >= 2)
        return("NA");
      else 
      {
        compteurTimeout++;
            //                              mettre erreur led/*LED intermittente rouge et verte (fréquence 1Hz, durée 2 fois plus longue pour le vert)Données reçues d’un capteur incohérentes -<vérification matérielle requise*/
            ft_error_led(255, 0, 0, 0, 255, 0, 1, 2);
            ledMod();
          }
      }
      else if(tempString == "erreur")
      {
        ft_error_led(255, 0, 0, 0, 255, 0, 1, 1);
        ledMod();
        return("NA");
          //                              LED intermittente rouge et verte (fréquence 1Hz,durée identique pour les 2 couleurs) Erreur accès aux données d’un capteur
      }
      else
        return(tempString);
  }
  else
    dataString += "OFF";
  return("ERROR");
}


void ft_SaveDataSDcard(String dataString)
{
    nameFile = getNameFile();
    File fichier = SD.open(nameFile);       //pas sur qu'on puisse ouvrir deux fois un fichier faudra peut etre le close puis le reopen 
    while(1)
    {
        if(fichier.size() < (FILE_MAX_SIZE + sizeOf(dataString)) ) //unsigned long = File.size(); donc peut etre prob de compa avec short (true=faire cast)
        {
          File fichier = SD.open(nameFile, FILE_WRITE);
          fichier.println(dataString);
          fichier.close();
        }
        else
        {
          nameFile2 = getNameFileNext(nameFile);  //récupère le fichier xx_n existant pas
          File fichier = SD.open(nameFile, FILE_READ);  //fichier xx_0 en mode read
          File fichier2 = SD.open(nameFile2, FILE_WRITE) // fichier xx_n en mode write
          while (fichier.available())          //lire données du 1 pour écrire dans le 2eme
            fichier2.write(fichier.read());
          fichier.close();                    // on ferme les deux 
          fichier2.close();
          SIZE_SD_CARD_IN_KB = SIZE_SD_CARD_IN_KB - long(SIZE_OF_CLUSTER_IN_KB);
          if(!SD.remove(nameFile))           //et on suppr le premier, xx_0
          {
            ft_error_led(255, 0, 0, 255, 255, 255, 2, 1); //Erreur d’accès ou d’écriture sur la carte SD
            Serial.println(F("Erreur suppression fichier"));
            ledMod();
          }
          continue;
      }
    break;
  }
}

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  initSD(); // initialisation carte SD

  SoftSerial.begin(9600); // Open SoftwareSerial for GPS

  //Initialize Clock
  clock.begin();
  //LED intermittente rouge et bleue (fréquence 1Hz, durée identique pour les 2 couleurs)

  ft_reset(); //init clock + variable de mesure (utilisé en cas de RESET)

  //Initialize Buttons
  pinMode(REDPUSHPIN, INPUT);
  pinMode(GREENPUSHPIN, INPUT);

  //Initialize LEDS
  pinMode(REDLEDPIN, OUTPUT);
  pinMode(GREENLEDPIN, OUTPUT);  
  pinMode(BLUELEDPIN, OUTPUT); 

  //Initialize Events
  attachInterrupt(digitalPinToInterrupt(REDPUSHPIN), eventRedPush, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GREENPUSHPIN), eventGreenPush, CHANGE);
  
  ledMod();


  while(digitalRead(REDPUSHPIN) == 1 ); //bloquer le programme tant que utilisateur a pas lacher le bouton rouge 

  if(modConfig == true)
    //configuration();    // Wesh on peux la mettre dans le setup avec un condi sur le bouton ? //non c'est relou // ah si enfaite :)
  firstLoop = 1;
}

void loop()
{
  //begin: 

  ft_CheckSDcardFull(); // bloque si SD Pleine (sauf si mode maintenance)

  //**** Acquisition 

  String dataString = "";
  //char* dataString;
  //dataString = malloc(sizeof(char) * (NB_CAPTORS * 4) + 8 + 8);


  //horloge
  dataString += getTime();
  dataString += ";";

  //gps
  dataString += ft_acqui_GPS();
  dataString += ";";

  //Capteurs
  short i = 0;
  while(i < (NB_CAPTORS - 1) )
  {
    dataString += ft_acqui_Capteur(i);
    dataString += ";";
    i++;
  }

  // save data in SDcard || affichage serial
  if(modMaintenance == false)
    ft_SaveDataSDcard(dataString);
  else
    Serial.println(dataString);

  delay(LOG_INTERVALL * 1000);
}










//--------------------------------------------------------------------------------------------------------------











void ft_reset()
{
  LOG_INTERVALL = 10; //min
  TIMEOUT = 30; //sc
  FILE_MAX_SIZE = 2048; //octet 

  clock.fillByYMD(2013,1,19);//Jan 19,2013
  clock.fillByHMS(15,28,30);//15:28 30"
    clock.fillDayOfWeek(SAT);//Saturday
  clock.setTime();//write time to the RTC chip

  if(NB_CAPTORS >= 1) capteurs[0] = {"LUMIN", 1, "LUMIN_LOW", 255, "LUMIN_HIGH", 768, 0, 1023, LUMIN_PIN, 0};
  if(NB_CAPTORS >= 2) capteurs[1] = {"TEMP_AIR", 1, "MIN_TEMP_AIR", -10, "MAX_TEMP_AIR", 60, -40, 85, TEMP_AIR_PIN, 0};
  if(NB_CAPTORS >= 3) capteurs[2] = {"HYGR", 1, "HYGR_MINT", 0, "HYGR_MAXT", 50, -40, 85, HYGR_PIN, 0};
  if(NB_CAPTORS >= 4) capteurs[3] = {"PRESSURE", 1, "PRESSURE_MIN", 850, "PRESSURE_MAX", 1080, 300, 1100, PRESSURE_PIN, 0};
  //module complémentaire à incorporer au projet
 // if(NB_CAPTORS >= 5) capteurs[4] = {"TEMP_EAU"}; //à def
 // if(NB_CAPTORS >= 6) capteurs[5] = {"FORCE_COURANT"}; //à def
 // if(NB_CAPTORS >= 7) capteurs[6] = {"FORCE_VENT"}; //à def
 // if(NB_CAPTORS >= 8) capteurs[7] = {"TAUX_PARTICULE"}; //à def


  if(modConfig == true) Serial.println(F("[DONE] Reset des variables effectué"));
}











//------------------------------------------------------------------------------







int getClockInSec() //ou short 
{
  return((clock.hour * 3600) + (clock.minute * 60) + clock.second ) ;
}

String getNameFile()
{
  //200531_0.LOG (Année=20, mois=05, jour=31, numéro de révision=0)
  String nameFile = "";
  nameFile += String(clock.year, DEC);
  nameFile += String(clock.month, DEC);
  nameFile += String(clock.dayOfMonth, DEC);

  nameFile += "_0.LOG";
  return(nameFile);
}

String getTime()
{
    String time="";
    clock.getTime();
    time+=String(clock.hour, DEC);
    time+=String(":");
    time+=String(clock.minute, DEC);
    time+=String(":");
    time+=String(clock.second, DEC);
    time+=String("  ");
    time+=String(clock.month, DEC);
    time+=String("/");
    time+=String(clock.dayOfMonth, DEC);
    time+=String("/");
    time+=String(clock.year+2000, DEC);
    time+=String(" ");
    time+=String(clock.dayOfMonth);
    time+=String("*");
    switch (clock.dayOfWeek)// Friendly printout the weekday
    {
        case MON:
        time+=String("MON");
        break;
        case TUE:
        time+=String("TUE");
        break;
        case WED:
        time+=String("WED");
        break;
        case THU:
        time+=String("THU");
        break;
        case FRI:
        time+=String("FRI");
        break;
        case SAT:
        time+=String("SAT");
        break;
        case SUN:
        time+=String("SUN");
        break;
    }
    time+=String(" ");
   return(time);
}
