int led1 = 13;
int led2 = 12;
int led3 = 11;
int led4 = 10;
int ledv = 9;
int bouton1 = 2;
int bouton2 = 3;
int bouton3 = 4;
int bouton4 = 5;
int etat_bouton1 = 0;
int etat_bouton2 = 0;
int etat_bouton3 = 0;
int etat_bouton4 = 0;
int etat_led1 = 0;
int etat_led2 = 0;
int etat_led3 = 0;
int etat_led4 = 0;
int etat_ledv = 0;
boolean last_etat1 = 0;
boolean last_etat2 = 0;
boolean last_etat3 = 0;
boolean last_etat4 = 0;


 int compteur = 0;

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(ledv, OUTPUT);
  pinMode(bouton1, INPUT_PULLUP);
  pinMode(bouton2, INPUT_PULLUP);
  pinMode(bouton3, INPUT_PULLUP);
  pinMode(bouton4, INPUT_PULLUP);
  digitalWrite(bouton1, 1);
  digitalWrite(bouton2, 1);
  digitalWrite(bouton3, 1);
  digitalWrite(bouton4, 1);
  digitalWrite(led1, 1);
  digitalWrite(led2, 1);
  digitalWrite(led3, 1);
  digitalWrite(led4, 1);
  digitalWrite(ledv, 1);
  etat_bouton1 = 1;
  etat_bouton2 = 1;
  etat_bouton3 = 1;
  etat_bouton4 = 1;
}

int ft_bouton(int b1, int b2, int b3, int b4)
{
  int a;
  a = 0;

  etat_bouton1 = digitalRead(bouton1);
  etat_bouton2 = digitalRead(bouton2);
  etat_bouton3 = digitalRead(bouton3);
  etat_bouton4 = digitalRead(bouton4);

  if(etat_bouton1 == digitalRead(bouton1) )
    a++;
  if(etat_bouton2 == digitalRead(bouton2) )
    a++;
  if(etat_bouton3 == digitalRead(bouton3) )
    a++;
  if(etat_bouton4 == digitalRead(bouton4) )
    a++;

  if(a == (b1 + b2 + b3 + b4))
    return(1);
  else 
    return(0);
}

void release()
{
  while( digitalRead(bouton1) == 0 &&  digitalRead(bouton2) == 0 && digitalRead(bouton3) == 0  && digitalRead(bouton4) == 0)
  {
    ;
  }
}

void loop() 
{

  if(compteur == 0)
  {
    compteur = compteur + ft_bouton(1 , 1 , 0, 0);
  }

  release();
  if(compteur == 1)
  {
    compteur = compteur + ft_bouton(1 , 0 ,0 ,0);
    if(compteur == 1)
    {
      compteur = 0;
    }
  }
  release();
  if(compteur == 2)
  {
    compteur = compteur + ft_bouton(1 , 0 ,0 ,0);
    if(compteur == 2)
      compteur = 0;
  }
  release();
  if(compteur == 3)
  {
    compteur = compteur + ft_bouton(1 , 0 ,0 ,1);
    if(compteur == 3)
      compteur = 0;
  }

  switch(compteur)
  {
    case 0 :
      //tout éteindre 
    break;
    case 1 : 
      // allumé 1 
    break;
    case 2 :
      // allumé 2
    break;
    case 3 : 
      //  allumé 3
    break;
    case 4 :
      //  allumé 4
     // + coffre ouvert
    break;
  }






}

void ini()
{
  digitalWrite(led1, 1);
  digitalWrite(led2, 1);
  digitalWrite(led3, 1);
  digitalWrite(led4, 1);
  digitalWrite(ledv, 1);
}

void error()
{
  digitalWrite(led1, 0);
  digitalWrite(led2, 0);
  digitalWrite(led3, 0);
  digitalWrite(led4, 0);
  digitalWrite(ledv, 0);
  delay(100);
  digitalWrite(led1, 1);
  digitalWrite(led2, 1);
  digitalWrite(led3, 1);
  digitalWrite(led4, 1);
  digitalWrite(ledv, 1);
  delay(100);
  digitalWrite(led1, 0);
  digitalWrite(led2, 0);
  digitalWrite(led3, 0);
  digitalWrite(led4, 0);
  digitalWrite(ledv, 0);
  delay(100);
  digitalWrite(led1, 1);
  digitalWrite(led2, 1);
  digitalWrite(led3, 1);
  digitalWrite(led4, 1);
  digitalWrite(ledv, 1);
  delay(100);
  digitalWrite(led1, 0);
  digitalWrite(led2, 0);
  digitalWrite(led3, 0);
  digitalWrite(led4, 0);
  digitalWrite(ledv, 0);
  delay(100);
  digitalWrite(led1, 1);
  digitalWrite(led2, 1);
  digitalWrite(led3, 1);
  digitalWrite(led4, 1);
  digitalWrite(ledv, 1);
  delay(100);


  
}