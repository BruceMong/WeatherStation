/**********************************************************************/
/***  fichier: configuration.c                                      ***/
/***  partie config                							        ***/
/**********************************************************************/

//#include "ft_configuration.h"  //normalement useless
//#include "main.h"

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
	newStr[i] = '\0'
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
 	if(NB_CAPTORS >= 5) capteurs[4] = {"TEMP_EAU"}; //à def
 	if(NB_CAPTORS >= 6) capteurs[5] = {"FORCE_COURANT"}; //à def
 	if(NB_CAPTORS >= 7) capteurs[6] = {"FORCE_VENT"}; //à def
 	if(NB_CAPTORS >= 8) capteurs[7] = {"TAUX_PARTICULE"}; //à def


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

void ft_beforeModifCapt(Parametre variable, short idVar, char *str)
{
	String *name;
	short *ValuePara

	short lastValue;
	short valueInsere;
	short *pVar;
	
	if(ft_checkArguValid(str) == false)
	{
		Serial.print(F("[ERROR] Argument de " + nameVa));
		Serial.println(F(" non valide, veuillez entrer une valeur numérique (0-9)"));
	}
	else
	{
		if(idVar == 1)
			ft_modifCapteurs( &variable.name, &variable.working, ft_findNum(str), 0, 1);
		if(idVar == 2)
			ft_modifCapteurs( &variable.lowName, &variable.lowValue, ft_findNum(str), &variable.maxDom, &variable.minDom);
		if(idVar == 3)
			ft_modifCapteurs( &variable.highName, &variable.highValue, ft_findNum(str), &variable.maxDom, &variable.minDom);
	}
	return();
}

		
		
//copy paste en haut 

void ft_modifCapteurs(String name, short *valuePara, short valueInsere, short max, short min)
{

	if(valueInsere >= min && valueInsere <= max)
	{
		lastValue = *valuePara;
		*valuePara = valueInsere;
		Serial.print(F("[DONE] Changement de " + name + "="));
		Serial.println(F(lastValue + "a" + name + "=" + *valuePara + "."));
	}				
	else
	{
		Serial.println(F("[ERROR] Valeur entree:"+ valueInsere +" est hors du domaine de definition {"));
		Serial.println(F(min + "," + max + "} de " + name));	
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

short ft_isColon(char c)
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
	short seconde;	// ex: 10:04:06

	hour = extractNum(*str, 0);
	minute = extractNum(*str, 3);
	seconde = extractNum(*str, 6);

	if(hour <= 23 && minute <= 59 && seconde <= 59 && hour >= 0 && minute >= 0 && seconde >= 0)
 	{
 		clock.fillByHMS(hour, minute, seconde);//15:28 30"
 		clock.setTime();//write time to the RTC chip
 		Serial.println(F("[DONE] Heure changé avec succès :" + getTime())); 
 	}
 	else
 		Serial.println(F("[ERROR] Erreur du domaine de definition, valeur attendu: HEURE{0-23}:MINUTE{0-59}:SECONDE{0-59}"));
 	return();
}

void ft_changedate(char *str)  // En vrai je peux compacte les deux en une 
{
	short mois; 
	short jour;
	short anne;	// ex: 10:04:06

	mois = extractNum(*str, 0);
	jour = extractNum(*str, 3);
	anne = extractNum(*str, 6);

	if(mois <= 1 && jour <= 31 && anne <= 2099 && mois >= 1 && jour >= 1 && anne >= 2000)
 	{
 		clock.fillByYMD(anne, mois, jour);
 		clock.setTime();//write time to the RTC chip
 		Serial.println(F("[DONE] Date changé avec succès :" + getTime()));
 	}
 	else
 		Serial.println(F("[ERROR] Erreur du domaine de definition, valeur attendu: MOIS{1-12},JOUR{1-31},ANNEE{2000-2099}"));
 	return();
}

void ft_clock(char *str)
{
	short sizeStrClock;

	sizeStrClock = 8;

	if(ft_checkStrHoraire(str, sizeStrClock) == 0)
		ft_changeClock(str);
	else 
		Serial.println(F("[ERROR] Erreur de Syntaxe, format attendu: HEURE{0-23}:MINUTE{0-59}:SECONDE{0-59}"));
	return();
}

void ft_date(char *str)
{
	short sizeStrdate;

	sizeStrdate = 10;

	if(ft_checkStrHoraire(str, sizeStrClock) == 0)
		ft_changedate(str);
	else 
		Serial.println(F("[ERROR] Erreur de Syntaxe, format attendu: MOIS{1-12},JOUR{1-31},ANNEE{2000-2099}"));
	return();
}



void ft_day(char *str)
{	
	int i;
	char DAY[7][3] = "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN";

	i = 0;
	while(i < 7)
	{
		if(strcmp( *DAY[i], str) == 0)
		{
			clock.fillDayOfWeek(*DAY[i]);
			Serial.println(F("[DONE] Jour changé avec succès :" + getTime()));
			return;
		}
	}
	Serial.println(F("[ERROR] Erreur de Syntaxe, format attedu {MON,TUE,WED,THU,FRI,SAT,SUN}"));
	return;
}

void ft_modifCapteursyHoraire(char *str, short *codeDate)
{	
	switch(codeDate)
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
	*codeDate = 0;
}



void ft_version();
{
	Serial.println(F("Version du programme :" + VERSION));
	Serial.println(F("Numerot de lot :" + NUMLOT));
}

void ft_enterClock(short *dateInsert)
{
	Serial.println(F("[INFO] Date/Heure actuelle :" + getTime()));
	Serial.println(F("[WAITING] Veuillez configurer l’heure du jour au format HEURE{0-23}:MINUTE{0-59}:SECONDE{0-59}"));
	*dateInsert = 1;
}

void ft_enterDay(short *dateInsert)
{
	Serial.println(F("[INFO] Date/Heure actuelle :" + getTime()));
	Serial.println(F("[WAITING] Veuillez configurer du jour de la semaine{MON,TUE,WED,THU,FRI,SAT,SUN}"));
	*dateInsert = 3;
}

void ft_enterDate(short *dateInsert)
{
	Serial.println(F("[INFO] Date/Heure actuelle :" + getTime()));
	Serial.println(F("[WAITING] Veuillez configurer la date du jour au format MOIS{1-12},JOUR{1-31},ANNEE{2000-2099}"));
	*dateInsert = 2;
}

short ft_imput_capteurs(char *strC, Parametre *pVar)
{
	while(i < 4 && pVar == NULL) 	//permet d'arreter le while dès qu'il a trouvé correspondance
	{
		switch (strC)
		{		
		case capteurs[i].name:	 
		pVar = capteurs[i];
		  return(1);
		case capteurs[i].lowName:
		pVar = capteurs[i];
		  return(2);
		case capteurs[i].highName:
		pVar = capteurs[i];
		  return(3);
		}
		i++;
	}
	return(0);
}


void ft_log_intervall(char *str)
{

	short temp; 

	temp = LOG_INTERVALL;
	if (ft_imput_LFT( "LOG_INTERVALL", &LOG_INTERVALL, ft_findNum(str), 0, 0) == true)
		Serial.println(F("[DONE] Changement de LOG_INTERVALL = " + temp + " en LOG_INTERVALL = " + newValue));

	return();
}

void ft_timeout(char *str)
{

	short temp; 

	temp = TIMEOUT;
	if (ft_imput_LFT( "TIMEOUT", &TIMEOUT, ft_findNum(str), 0, 0) == true)
		Serial.println(F("[DONE] Changement de TIMEOUT = " + temp + " en TIMEOUT = " + newValue));

	return();
}

void ft_filesize(char *str, boolean imputArchiFile) 
{
	short temp = FILE_MAX_SIZE
	if(ft_imput_LFT( "FILE_MAX_SIZE", &FILE_MAX_SIZE, ft_findNum(str), 0, 0) == true)
	{
		if(FILE_MAX_SIZE <= 4096)
			Serial.println(F("[DONE] Changement de FILE_MAX_SIZE = " + temp + " en FILE_MAX_SIZE = " + newValue));
		if(FILE_MAX_SIZE == 4096 && imputArchiFile == false)
		{
			ft_archivage();
			imputArchiFile = true;
		}
		if(FILE_MAX_SIZE > 4096)
		{
			Serial.println(F("[WRONG] Domaine de de def de FILE_MAX_SIZE : {0, 4096}"));
			FILE_MAX_SIZE = temp;
		}
	}
}


boolean ft_imput_LFT(char *str, short *var, String nameVa)
{	
	short temp;
	short newValue;

	temp = *var;

	if(ft_checkArguValid(str) == false)
	{
		Serial.print(F("[ERROR] Argument de " + nameVa));
		Serial.println(F("non valide, veuillez entrer une valeur numérique (0-9)"));
		return(false);
	}

	newValue = ft_findNum(str)
	if (newValue < 0)
		Serial.println(F("[ERROR] Valeur:" + newValue + " incorrecte veuillez entrer une valeur positive ou nul"));
	else
	{
		*var = temp;
		return(true);
	}
	return(false);
}


void modifPara(char *str, short *dateInsert, boolean imputArchiFile)
{
	Parametre *pVar;
	char *strC;
	short i;
	short idVar;

	varId = 0;
	pVar = NULL;
	i = 0;

	//*************************   Si HORAIRE (1part) true alors HORAIRE (2part)
	if(*dateInsert > 0) 
	{
		ft_modifCapteursyHoraire(str, *dateInsert);
		return();
	}
	//*************************  RESET, VERSION + HORAIRE (1part)
	switch(str)
	{
	case "RESET":
		ft_reset();
	  	return();
	case "VERSION":
		ft_version();
	  	return();
	case "CLOCK":
		ft_enterClock(*dateInsert);
		return();
	case "DATE":
		ft_enterDate(*dateInsert);
		return();
	case "DAY":
		ft_enterDay(*dateInsert);
		return();
	}
																						 //if (str == "RESET") // je suis pas sur de la syntaxe pcq c'est un *char voir si il faut pas le cast en String ou quoi

//******************************* 	 struct CAPTEURS + TIMEOUT et LOG_INTERVALL		(DATA with arg )
	strC = newStrWithoutNum(str);

	switch(strc)
	{
	case "TIMEOUT":
		ft_timeout(str);
		return();
	case "LOG_INTERVALL":
		ft_log_intervall(str);
		return();
	case "FILE_MAX_SIZE":
		ft_filesize(str, imputArchiFile);
		return();
	}
	idVar = ft_imput_capteurs(strC, *pVar);	
	if(pVar != NULL)
		ft_beforeModifCapt(pVar, idVar, str);
	else
		Serial.println(F("Commande introuvable"));
	return();
}



void configuration()
{
	int timeActivite;
	short dateInsert;
	const boolean imputArchiFile;

	firstLoop = 1;
	timeActivite = getClockInSec(); 
	dateInsert = 0;
	imputArchiFile = false;

	RGB_color(255, 255, 0); // YELLOW


	Serial.println(F("[BEGIN] Configuration mod :"));
	while( (getClockInSec() > (timeActivite + (30 * 60 ))) && modConfig == true) 
	{
		if(dateInsert == 0)
			Serial.println(F("[WAITING] Veuillez entrer une commande:"));

		if(Serial.available()) 
		{
    		char msg[] = Serial.read();
   			modifPara(msg, &dateInsert, imputArchiFile);

   			timeActivite = getClockInSec(); //reset time acti
   		}
	}
	Serial.print(F("[END] Configuration mod by "))
	if(modConfig == false)
		Serial.println(F("BOUTON ROUGE 5 SEC"));
	else
	{
		Serial.println(F("TIME ACTIVITE > 30MIN"));
		modConfig = false;
	}
	RGB_color(0, 255, 0); // GREEN
	//faudrait peut etre free toute les merdes
}