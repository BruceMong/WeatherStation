/**********************************************************************/
/***  fichier: main.c                                               ***/
/***  tout le programme sauf la partie config                       ***/
/**********************************************************************/

// je déclare les libs dans le .c et non dans le .h car sinon bug du makefile jcpa pq 
#include <SoftwareSerial.h> //UART GPS 
#include <Wire.h> //communication led 
#include <DS1307.h> // clock RTC //bizarre 
#include <SPI.h> //pour communication SPI ( carte SD) 
#include <SD.h> //pour la carte SD 
#include <avr/pgmspace.h> //PROGMEM library
//


#include "main.h"
#include "ft_configuration.h"
#include "ft_time.h"

SoftwareSerial SoftSerial(2, 3); // Serial already used for serial communication GPS connected on D2 port on Grove Shield
DS1307 clock(CLOCK_PIN_A, CLOCK_PIN_B); //define a object of DS1307 class RTC Clock on I2C port on Grove Shield



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
    configuration();    // Wesh on peux la mettre dans le setup avec un condi sur le bouton ? //non c'est relou // ah si enfaite :)
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






short ft_findNum(char *msg)
{
  short i; 
  short valeur;
  short nega = 1;

  i = 0;
  valeur = 0;

  while(msg[i] != '=')
    i++;
  i++;
  if(msg[i] == '-')
  {
    nega = -1; 
    i++;
  }
  valeur = extractNum(msg, i);
  return(valeur * nega);
}

char* newStrWithoutNum(char *str)
{
  char *newStr;
  int i = 0;

  while(str[i] != '=')
  {
    if(str[i] == '\0')
      return("WRONG");
    i++;
  }
  //newStr = malloc(sizeof(char) * i);
  newStr = malloc(sizeof(char) * i + sizeof(char)); //pas de +1 car vire le '=' //ah si finalement car on part de i =0 
  i = 0;
  while(str[i] != '=')
  {
    newStr[i] = str[i]; 
    i++;
  }
  newStr[i] = '\0';
  return(newStr);
}

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

boolean ft_checkArguValid(char* str)
{
  int i;

  i = 0;
  while(str[i] != '=')
    i++;
  i++;
  if(str[i] == '-')
    i++;
  while(str[i] != '\0')
  {
    if( str[i] < 48 || str[i] > 57)
      return(false);
    i++;
  }
  return(true);
}

void ft_beforeModifCapt(Parametre variable, short idVar, char* str)  //variable est un pointeur de l'adresse du capteur à traiter 
{ 
  if(ft_checkArguValid(str) == false)
  {
    Serial.print(F("[ERROR] Argument de "));
    Serial.print(nameVa);
    Serial.println(F(" non valide, veuillez entrer une valeur numérique (0-9)"));
  }
  else
  {
    if(idVar == 1)
      ft_modifCapteurs( variable.name, &variable.working, ft_findNum(str), 0, 1);
    else if(idVar == 2)
      ft_modifCapteurs( variable.lowName, &variable.lowValue, ft_findNum(str), variable.maxDom, variable.minDom);
    else if(idVar == 3)
      ft_modifCapteurs( variable.highName, &variable.highValue, ft_findNum(str), variable.maxDom, variable.minDom);
  }
  return;
}

    
    
//copy paste en haut 

void ft_modifCapteurs(char* name, short *valuePara, short valueInsere, short max, short min)
{

  if(valueInsere >= min && valueInsere <= max)
  {
    lastValue = *valuePara;
    *valuePara = valueInsere;
    Serial.print(F("[DONE] Changement de "));
    Serial.print(name);
    Serial.print(F("="));
    Serial.print(lastValue);
    Serial.print(F( "a"));
    Serial.print(name);
    Serial.print(F("="));
    Serial.println(valuePara);
  }       
  else
  {
    Serial.println(F("[ERROR] Valeur entree:"));
    Serial.print(valueInsere);
    Serial.print(F("est hors du domaine de definition {"));
    Serial.print(min);
    Serial.print(F( ","));
    Serial.print(max);
    Serial.println(F("} de "));
    Serial.println(name);  
  }
  return();
}

short ft_strlength(char *str)
{
  int i; 

  i = 0;
  while(str[i] != '\0')
    i++;
  return(i);
}

short ft_isCaraNum(char c)
{
  if(c >= 48 && c <= 57)
    return(0);
  return(1);
}

short ft_isCaraColon(char c)
{
  if(c == ':')
    return(0);
  return(1);
}

short ft_checkStrDate(char* str, short size)
{
  short i;
  short verif;

  verif = 0;
  i = 0;
  if(ft_strlength(str) == size)
  {
    while(i < size && verif == 0)
    {
      if(i == 2 || i == 5)
        verif = ft_isCaraColon();
      else 
        verif = ft_isCaraNum();
    }
    if (verif == 0)
      return(0);
  }
  return(1);

}

short extractNum(char *str, int s)
{
  short num;

  num = 0;
  while(str[s] != ':' || str[s] != '\0')
  {
    num = num * 10 + (str[i] - 48);
    s++;
  }
  return(num);
}

void ft_changeClock(char *str)
{
  short hour; 
  short minute;
  short seconde;  // ex: 10:04:06

  hour = extractNum(str, 0);
  minute = extractNum(str, 3);
  seconde = extractNum(str, 6);

  if(hour <= 23 && minute <= 59 && seconde <= 59 && hour >= 0 && minute >= 0 && seconde >= 0)
  {
    clock.fillByHMS(hour, minute, seconde);//15:28 30"
    clock.setTime();//write time to the RTC chip
    Serial.println(F("[DONE] Heure changé avec succès :"));
    Serial.println(getTime()); 
  }
  else
    Serial.println(F("[ERROR] Erreur du domaine de definition, valeur attendu: HEURE{0-23}:MINUTE{0-59}:SECONDE{0-59}"));
  return;
}

void ft_changedate(char *str)  // En vrai je peux compacte les deux en une 
{
  short mois; 
  short jour;
  short anne; // ex: 10:04:06

  mois = extractNum(str, 0);
  jour = extractNum(str, 3);
  anne = extractNum(str, 6);

  if(mois <= 1 && jour <= 31 && anne <= 2099 && mois >= 1 && jour >= 1 && anne >= 2000)
  {
    clock.fillByYMD(anne, mois, jour);
    clock.setTime();//write time to the RTC chip
    Serial.print(F("[DONE] Date changé avec succès :"));
    Serial.println(getTime());    
  }
  else
    Serial.println(F("[ERROR] Erreur du domaine de definition, valeur attendu: MOIS{1-12},JOUR{1-31},ANNEE{2000-2099}"));
  return;
}

void ft_clock(char *str)
{
  short sizeStrClock;

  sizeStrClock = 8;

  if(ft_checkStrHoraire(str, sizeStrClock) == 0)
    ft_changeClock(str);
  else 
    Serial.println(F("[ERROR] Erreur de Syntaxe, format attendu: HEURE{0-23}:MINUTE{0-59}:SECONDE{0-59}"));
  return;
}

void ft_date(char *str)
{
  short sizeStrdate;

  sizeStrdate = 10;

  if(ft_checkStrHoraire(str, sizeStrClock) == 0)
    ft_changedate(str);
  else 
    Serial.println(F("[ERROR] Erreur de Syntaxe, format attendu: MOIS{1-12},JOUR{1-31},ANNEE{2000-2099}"));
  return;
}



void ft_day(char *str)
{ 
  int i;
  char *DAY[] = "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN";

  i = 0;
  while(i < 7)
  {
    if(strcmp( DAY[i], str) == 0)
    {
      clock.fillDayOfWeek(DAY[i]);
      Serial.print(F("[DONE] Jour changé avec succès :"));
      Serial.println(getTime());    
      return;
    }
  }
  Serial.println(F("[ERROR] Erreur de Syntaxe, format attedu {MON,TUE,WED,THU,FRI,SAT,SUN}"));
  return;
}

void ft_modifCapteursHoraire(char *str)
{ 
  switch(dateInsert)
  {
    case 1 :
    ft_clock(str);
    break;
    case 2 :
    ft_date(str);
    break;
    case 3 :
    ft_day(str);
    break;
  }
  dateInsert = 0;
}



void ft_version()
{
  Serial.print(F("Version du programme :")); 
  Serial.print(VERSION);
  Serial.print(F("Numerot de lot :"));
  Serial.println(NUM_LOT);
}

void ft_enterClock()
{
  Serial.print(F("[INFO] Date/Heure actuelle :"));
  Serial.println(getTime());
  Serial.println(F("[WAITING] Veuillez configurer l’heure du jour au format HEURE{0-23}:MINUTE{0-59}:SECONDE{0-59}"));
  dateInsert = 1;
}

void ft_enterDay()
{
  Serial.print(F("[INFO] Date/Heure actuelle :"));
  Serial.println(getTime());
  Serial.println(F("[WAITING] Veuillez configurer du jour de la semaine{MON,TUE,WED,THU,FRI,SAT,SUN}"));
  dateInsert = 3;
}

void ft_enterDate()
{
  Serial.print(F("[INFO] Date/Heure actuelle :"));
  Serial.println(getTime());
  Serial.println(F("[WAITING] Veuillez configurer la date du jour au format MOIS{1-12},JOUR{1-31},ANNEE{2000-2099}"));
  dateInsert = 2;
}

short ft_imput_capteurs(char *strC, Parametre *pVar)
{
  int i;
  i = 0;
  
  while(i < 4 && pVar == NULL)  //permet d'arreter le while dès qu'il a trouvé correspondance
  {
    if(strcmp(capteurs[i].name, strC) == true)
    {
      pVar = &capteurs[i];
      return(1);
    }
    if(strcmp(capteurs[i].lowName, strC) == true)
    {
       pVar = &capteurs[i];
      return(2);
    }
    if(strcmp(capteurs[i].highName, strC) == true)
    {
       pVar = &capteurs[i];
      return(3);
    }
    i++;
  }
  return(NULL);
}


void ft_log_intervall(char *str) //same ft_filesize sauf pas de limite spécifique donnée
{
  short temp; 

  temp = LOG_INTERVALL;
  if ( ft_imput_LFT( "LOG_INTERVALL", &LOG_INTERVALL) == true)
  {
    Serial.print(F("[DONE] Changement de LOG_INTERVALL = "));
    Serial.print(temp); 
    Serial.print(F(" en LOG_INTERVALL = "));
    Serial.println(LOG_INTERVALL);
  }
  return;
}

void ft_timeout(char *str) //same ft_filesize sauf pas de limite spécifique donnée
{
  short temp; 
  temp = TIMEOUT;

  if (ft_imput_LFT( "TIMEOUT", &TIMEOUT) == true) 
  {
    Serial.print(F("[DONE] Changement de TIMEOUT = "));
    Serial.print(temp); 
    Serial.print(F(" en TIMEOUT = "));
    Serial.println(TIMEOUT);
   } 
  return;
}

void ft_filesize(char *str)   //doit y avoir moyen d'opti
{
  short temp = FILE_MAX_SIZE ;      //on fait une copie de la variable  
  if(ft_imput_LFT( "FILE_MAX_SIZE", &FILE_MAX_SIZE) == true ) //la variable est modif si true
  {
    if(FILE_MAX_SIZE <= 4096)
    {
      Serial.print(F("[DONE] Changement de FILE_MAX_SIZE = "));
       Serial.print(temp); 
       Serial.print(F(" en FILE_MAX_SIZE = "));
       Serial.println(FILE_MAX_SIZE);
      if(FILE_MAX_SIZE == 4096 && imputArchiFile == false)
      {
        ft_archivage();
        imputArchiFile = true;
      }
    }  
    if(FILE_MAX_SIZE > 4096) //check de la limite spécifique a file max size 
    {
      Serial.println(F("[WRONG] Domaine de de def de FILE_MAX_SIZE : {0, 4096}"));
      FILE_MAX_SIZE = temp; // on réatribue la valeur temp du début pcq var modifier dans ft_imput_lft
    }
  }
}


boolean ft_imput_LFT(char *str, short *var) //fct qui sert a check si l'argu est bon syntaxiquement et si oui il check ausis si c'est positif ) = true alors il renvoi true :)
{ 
  short temp;
  short newValue;

  temp = *var;

  if(ft_checkArguValid(str) == false)
  {
    Serial.print(F("[ERROR] Argument de "));
    Serial.print(str);
    Serial.println(F("non valide, veuillez entrer une valeur numérique (0-9)"));
    return(false);
  }

  newValue = ft_findNum(str);
  if(newValue < 0)
  {
    Serial.print(F("[ERROR] Valeur:")); 
    Serial.print(newValue);
    Serial.println(F(" incorrecte veuillez entrer une valeur positive ou nul"));
    return(false);
  }
  else
  {
    *var = temp;  // si valeur argu correcte alors on l'attribut a la var
    return(true);
  }
  return(false);
}


void modifPara(char* str) // peux peut etre etre simplifier avec des enum ? ou genre un tableau de de toutes les soluce // ouais nan galere // bon okay je l'ai fait
{
  Parametre *pVar;
  char *strC;
  short i;
  short idVar;
  //char *tabOpt[] = { "RESET", "VERSION", "CLOCK", "DATE", "DAY", "TIMEOUT", "LOG_INTERVALL", "FILE_MAX_SIZE" };
  idVar = 0;
  pVar = NULL;
  i = 0;

  
  fptrVoid ftabV[5] = { &ft_reset, &ft_version, &ft_enterClock, &ft_enterDate, &ft_enterDay};
  fptrChar ftabC[3] = { &ft_timeout, &ft_log_intervall, &ft_filesize};

  if(dateInsert > 0) 
  {
    ft_modifCapteursHoraire(str);
    return;
  }

  //switch str ne marche qu'en java :(
  strC = newStrWithoutNum(str); 
  while(i < 8)
  {
      if(i < 5)
      {
        if( strcmp(str, tabOpt[i]) == true)
        {
          (*( ftabV[i]))(); 
          return;
        } 
      }
      else
      {
        if( strcmp(strC, tabOpt[i]) == true)
        {
          (*( ftabC[i - 5]))(str); 
          return;
        }
      }
      i++;
  }
  //struct capteurs
    idVar = ft_imput_capteurs(strC, pVar); 
  if(pVar != NULL)
    ft_beforeModifCapt(*pVar, idVar, str);
  else
    Serial.println(F("[ERROR] Commande introuvable"));
  return;
}


void configuration()
{
  int timeActivite;
  char bufferSerial[SIZE_BUFFER];
  int i;
  boolean dataR;

  dataR = false;
  firstLoop = 1;
  timeActivite = getClockInSec(); 

  RGB_color(255, 255, 0); // YELLOW


  Serial.println(F("[BEGIN] Configuration mod :"));
  while( (getClockInSec() > (timeActivite + (30 * 60 ))) && modConfig == true) 
  {
  
    
    if(dateInsert == 0 && dataR == true) //dataR pour pas spam et dateInsert en cas d'insertion de date
    {
      Serial.println(F("[WAITING] Veuillez entrer une commande:"));
      dataR = false;
    } 
    
    i = 0;
    while (Serial.available() > 0)
    {
      if(i < SIZE_BUFFER)
      {
        bufferSerial[i] = Serial.read();
        i++;
      }
    }
    if( i != 0) //info recu
    {
      if( i > SIZE_BUFFER)
        Serial.print(F("[ERROR] Trop de caractere recu"));
      else
        modifPara(ft_newStrFromBuff(bufferSerial));
        
      clearBuff(bufferSerial); // useless en vrai mais par propreté :)
      timeActivite = getClockInSec(); //reset time acti
      dataR = true;
    } 
  }
  Serial.print(F("[END] Configuration mod by "));
  if(modConfig == false)
    Serial.println(F("BOUTON ROUGE 5 SEC"));
  else
  {
    Serial.println(F("TIME ACTIVITE > 30MIN"));
    modConfig = false;
  }
  RGB_color(0, 255, 0); // GREEN
  //faudrait peut etre free toute les merdes //il y a rien a free :)
}



char* ft_newStrFromBuff(char* string)
{
  short sizeStr;
  char* newStr;
  sizeStr = ft_strlength(string);

  newStr = (char*)malloc(sizeof(char) * sizeStr  ); //ptetre un + 1
  ft_cpStr(string, newStr);
  return(newStr);
}

void ft_cpStr(char* strS, char* strO)
{
  short i;
  short sizeS;

  i = 0;
  sizeS = ft_strlength(strS);
  while(i < sizeS)
    strO[i] = strS[i];
}


void clearBuff(char* str)
{
  short i;
  while(i < SIZE_BUFFER)
  {
    str[i] = '\0';
  }
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
