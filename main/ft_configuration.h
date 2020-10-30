/**********************************************************************/
/***  fichier: ft_configuration.h                                   ***/
/***  en tête de config.c  						                    ***/
/**********************************************************************/

#ifndef FT_CONFIGURATION_H
#define FT_CONFIGURATION_H

#define SIZE_BUFFER 20


const PROGMEM char *tabOpt[] = { "RESET", "VERSION", "CLOCK", "DATE", "DAY", "TIMEOUT", "LOG_INTERVALL", "FILE_MAX_SIZE" };

typedef void (*fptrVoid)();
typedef void (*fptrChar)(char*);

short dateInsert = 0;
boolean imputArchiFile = false;

extern short ft_findNum(char *msg);
extern char* newStrWithoutNum(char *str);
extern void ft_reset();
extern boolean ft_checkArguValid(char* str);
extern void ft_beforeModifCapt(Parametre variable, short idVar, char *str);
extern void ft_modifCapteurs(String name, short *valuePara, short valueInsere, short max, short min);
extern short ft_strlength(char *str);
extern short ft_isCaraNum(char c);
extern short ft_isCaraColon(char c);
extern short ft_checkStrDate(char* str, short size);
extern short extractNum(char *str, int s);
extern void ft_changeClock(char *str);
extern void ft_changedate(char *str);
extern void ft_clock(char *str);
extern void ft_date(char *str);
extern void ft_day(char *str);
extern void ft_modifCapteursHoraire(char *str);
extern void ft_version();
extern void ft_enterClock();
extern void ft_enterDay();
extern void ft_enterDate();
extern short ft_imput_capteurs(char *strC, Parametre *pVar);
extern void ft_log_intervall(char *str);
extern void ft_timeout(char *str);
extern void ft_filesize(char *str);
extern boolean ft_imput_LFT(char *str, short *var, String nameVa);
extern void modifPara(char *str);
extern void configuration();
extern void clearBuff(char* str);
extern void ft_cpStr(char* strS, char* strO);
extern char* ft_newStrFromBuff(char* string);

#endif
