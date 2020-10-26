/**********************************************************************/
/***  fichier: ft_time.c                    		                ***/
/***  fonction de temps 	  					                    ***/
/**********************************************************************/

int getClockInSec(); //ou short 
{
	return((clock.hour * 3600) + (clock.minute * 60) + clock.second);
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
   return time;
}