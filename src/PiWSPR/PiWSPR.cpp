
/**
 * PiWSPR.cpp 
 * Raspberry Pi based WSPR beacon
 *
 *
 * This program turns the Raspberry pi into a WSPR beacon software able
 * to operate at the indicated frequency as a direct RF generator
 * This program is part of the PixiePi platform
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *     tune.cpp from rpitx package by Evariste Courjaud F5OEO
 *     wiringPi library (git clone git://git.drogon.net/wiringPi)
 *     WSPR-beacon by Alexander Fasching OE5TKM
 * ---------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
//----------------------------------------------------------------------------
//  includes
//----------------------------------------------------------------------------

//---- Generic includes

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <ctime>
#include <wiringPi.h>
#include <iostream>
#include <csignal>

//--- type initial setup
typedef unsigned char byte;
typedef bool boolean;


#include <unistd.h>
//---- Program specific includes

#include "./PiWSPR.h"		// wspr definitions and functions
#include "../lib/WSPR.h"
#include "/home/pi/OrangeThunder/src/OT/OT.h"
#include "../lib/LCDLib.h"
#include "/home/pi/librpitx/src/librpitx.h"
//----------------------------------------------------------------------------
//  Program parameter definitions
//----------------------------------------------------------------------------

const char   *PROGRAMID="PiWSPR";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";

//-------------------------------------------------------------------------------------------------
// Main structures
//-------------------------------------------------------------------------------------------------

struct sigaction sa;

//---- Generic memory allocations

byte   TRACE=0x00;
byte   MSW=0x00;

int    value=0;
int    lastEncoded=0;
int    counter=0;
int    clkLastState=0; 
char   hi[80];
byte   memstatus=0;
byte   ntimes=1;
int    a;
int    anyargs = 0;
int    lcd_light;
float    f=7038600;
float    ppm=1000.0;
byte     gpio=GPIO_DDS;;
char     callsign[10];
char     locator[10];
int      POWER=7;
int      ntx=0;
int      nskip=0;
char     timestr[16];
int      Upsample=100;
int      FifoSize=WSPR_LENGTH;
float    Deviation=WSPR_RATE;
float    offset=WSPR_SHIFT;
float    RampRatio=0;
char     command[256];

fskburst *fsk=nullptr;


char   wspr_message[40];          // user beacon message to encode
unsigned char wspr_symbols[WSPR_LENGTH] = {};
unsigned long tuning_words[WSPR_LENGTH];

bool WSPRwindow=false;

//*---- Define WSPR memory blocks

//DDS    *dds=new DDS(NULL);
WSPR   wspr(NULL);
LCDLib *lcd;
char*  LCD_Buffer;

//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//                              ROUTINE STRUCTURE
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//--------------------------[System Word Handler]---------------------------------------------------
// getSSW Return status according with the setting of the argument bit onto the SW
//--------------------------------------------------------------------------------------------------
bool getWord (unsigned char SysWord, unsigned char v) {

  return SysWord & v;

}
//--------------------------------------------------------------------------------------------------
// setSSW Sets a given bit of the system status Word (SSW)
//--------------------------------------------------------------------------------------------------
void setWord(unsigned char* SysWord,unsigned char v, bool val) {

  *SysWord = ~v & *SysWord;
  if (val == true) {
    *SysWord = *SysWord | v;
  }

}
//--------------------------------------------------------------------------------------------------
// returns the time in a string format
//--------------------------------------------------------------------------------------------------
char* getTime() {

       time_t theTime = time(NULL);
       struct tm *aTime = localtime(&theTime);
       int hour=aTime->tm_hour;
       int min=aTime->tm_min;
       int sec=aTime->tm_sec;
       sprintf(timestr,"%02d:%02d:%02d",hour,min,sec);
       return (char*) &timestr;

}

//-------------------------------------------------------------------------------------------------
// print_usage
// help message at program startup
//-------------------------------------------------------------------------------------------------
void print_usage(void)
{
fprintf(stderr,"\n\
Usage:\n\
\t-f frequency Hz(50000 Hz to 1500000000 Hz) or band 10m,20m..\n\
\t-p set clock ppm instead of ntp adjust\n\
\t-c set callsign (ej LU7DID)\n\
\t-l set locator (ej GF05)\n\
\t-d set power in dBm\n\
\t-i set number of windows to skip between transmission (0 none, default)\n\
\t-g set GPIO port (4 or 20)\n\
\t-x force WSPR Window now (test only)\n\
\t-h help (this help).\n\n\
\t e.g.: PiWSPR -c LU7DID -l GF05 -d 20 -f 7038600 -n 1 -g 20\n\
\n");
}
//---------------------------------------------------------------------------------------------
// Signal handlers, SIGALRM is used as a timer, all other signals means termination
//---------------------------------------------------------------------------------------------
static void terminate(int num)
{
    fprintf(stderr,"\n%s %s:terminate() Received signal INT(%d %s)\n",getTime(),PROGRAMID,num,strsignal(num));
    if (getWord(MSW,RETRY)==true) {
       fprintf(stderr,"\n%s %s:terminate() Re-entry of termination signal force exit INT(%d %s)\n",getTime(),PROGRAMID,num,strsignal(num));
       exit(16);
    }
    setWord(&MSW,RUN,false);
    setWord(&MSW,RETRY,true);
}
//---------------------------------------------------------------------------------------------
// gpioWrite (manager gpio write externally)
//---------------------------------------------------------------------------------------------
void gpioWrite(int pin,int s) {
    sprintf(command,"gpio write %d %d",pin,s);
    system(command);
    fprintf(stderr,"%s %s:gpioWrite: set gpio pin(%d) as (%s)\n",getTime(),PROGRAMID,pin,(s==0 ? "Off" : "On"));
    return;
}

//---------------------------------------------------------------------------------------------
// MAIN Program
//---------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

//--- Initial presentation

  fprintf(stderr,"%s %s Version %s Build(%s) %s\n",getTime(),PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);

//--- Parse arguments

   while(1)
        {
                a = getopt(argc, argv, "f:ed:hs:g:p:v:c:l:x");
        
                if(a == -1) 
                {
                        if(anyargs) {
                           break; 
                        } else {
                           a='h'; //print usage and exit
                        }
                }
                anyargs = 1;    

                switch(a)
                {
                case 'x': // Force WSPR window now (test only!)
                     WSPRwindow=true;
                     fprintf(stderr,"%s %s:main() Forcing WSPR window now!\n",getTime(),PROGRAMID);
                     break; 
                case 'f': // Frequency
                     if (!strcasecmp(optarg,"LF")) {
                        f=137500.0;
                     } else if (!strcasecmp(optarg,"LF-15")) {
                               f=137612.5;
                            } else if (!strcasecmp(optarg,"MF")) {
                                      f=475700.0;
                                   } else if (!strcasecmp(optarg,"MF-15")) {
                                             f=475812.5;
                                          } else if (!strcasecmp(optarg,"160m")) {
                                                    f=1838100.0;
                                                 } else if (!strcasecmp(optarg,"160m-15")) {
                                                           f=1838212.5;
                                                        } else if (!strcasecmp(optarg,"80m")) {
                                                                  f=3594100.0;
                                                               } else if (!strcasecmp(optarg,"60m")) {
                                                                         f=5288700.0;
                                                                      } else if (!strcasecmp(optarg,"40m")) {
                                                                                f=7038600.0;
                                                                             } else if (!strcasecmp(optarg,"30m")) {
                                                                                       f=10140200.0;
                                                                                    } else if (!strcasecmp(optarg,"20m")) {
                                                                                              f=14097100.0;
                                                                                           } else if (!strcasecmp(optarg,"17m")) {
                                                                                                     f=18106100.0;
                                                                                                  } else if (!strcasecmp(optarg,"15m")) {
                                                                                                            f=21096100.0;
                                                                                                         } else if (!strcasecmp(optarg,"12m")) {
                                                                                                                   f=24926100.0;
                                                                                                                } else if (!strcasecmp(optarg,"10m")) {
                                                                                                                          f=28126100.0;
                                                                                                                       } else if (!strcasecmp(optarg,"6m")) {
                                                                                                                                 f=50294500.0;
                                                                                                                              } else if (!strcasecmp(optarg,"4m")) {
                                                                                                                                        f=70092500.0;
                                                                                                                                     } else if (!strcasecmp(optarg,"2m")) {
                                                                                                                                               f=144490500.0;
                                                                                                                                            } else {
                                                                                                                                               f = atof(optarg);
                                                                                                                                            }
                     
		     fprintf(stderr,"%s %s:main() Frequency: %10.0f Hz\n",getTime(),PROGRAMID,f);
                     break;
        	case 'd': //power
			POWER=atoi(optarg);
			fprintf(stderr,"%s %s:main() Power: %d dBm\n",getTime(),PROGRAMID,POWER);
			break;
        	case 'v': //verbose
			TRACE=atoi(optarg);
			fprintf(stderr,"%s %s:main() TRACE: %d\n",getTime(),PROGRAMID,TRACE);
			break;
		case 'c': //callsign
			sprintf(callsign,optarg);
			fprintf(stderr,"%s %s:main() Callsign: %s\n",getTime(),PROGRAMID,callsign);
			break;
		case 'l': //locator
			sprintf(locator,optarg);
			fprintf(stderr,"%s %s:main() Locator: %s\n",getTime(),PROGRAMID,locator);
			break;
                case 'g': // GPIO
                        gpio = atoi(optarg);
                        if (gpio != GPIO_DDS) {
                           fprintf(stderr,"%s %s:main() Invalid selection for GPIO(%s), must be 4\n",getTime(),PROGRAMID,optarg);
                           break;
                        }
			fprintf(stderr,"%s %s:main() Pin Out: GPIO%d\n",getTime(),PROGRAMID,gpio);
                        break;
                case 'p': //ppm
                        ppm=atof(optarg);
			fprintf(stderr,"%s %s:main() PPM: %10.2f",getTime(),PROGRAMID,ppm);
                        break;  
                case 'h': // help
                        print_usage();
                        exit(1);
                        break;
                case -1:
                break;
                case '?':
                        if (isprint(optopt) ) {
                           fprintf(stderr, "%s %s:main() unknown option `-%c'.\n",getTime(),PROGRAMID, optopt);
                        } else {
                           fprintf(stderr, "%s %s:main() unknown option character `\\x%x'.\n",getTime(),PROGRAMID, optopt);
                        }
                        print_usage();

                        exit(1);
                        break;
                default:
                        print_usage();
                        exit(1);
                        break;
                }
        }

//--- Define the INT signal handlers, basically as termination exceptions

    for (int i = 0; i < 64; i++) {

        if (i != SIGALRM && i != 17 && i != 28 ) {
           signal(i,terminate);
        }
    }

//--- Convert librpitx power level to WSPR power level
//  0   10mW 10 dBm
//  1
//  2
//  3  100mW 20 dBm 
//  4
//  5
//  6    
//  7    1W  30 dBm
//--- Generate WSPR message
int WSPRPower=20;

    if (POWER>=0 && POWER <3) {WSPRPower=10;}
    if (POWER>=3 && POWER <7) {WSPRPower=20;}
    if (POWER>=7) {WSPRPower=30;}

    fprintf(stderr,"%s %s:main() Power GPIO level(%d) Power to be informed is %d dBm\n",getTime(),PROGRAMID,POWER,WSPRPower);

//*-----------------------------------------------------------------------------------------
//* Setup LCD Display 
//*-----------------------------------------------------------------------------------------
    LCD_Buffer=(char*) malloc(32);
    lcd=new LCDLib(NULL);

    lcd->begin(16,2);
    lcd->clear();
    lcd->createChar(0,TX);
    lcd_light=LCD_ON;
    lcd->backlight(true);
    lcd->setCursor(0,0);
  
    fprintf(stderr,"%s %s:main() LCD display turned on\n",getTime(),PROGRAMID);
    sprintf(LCD_Buffer,"%s %s(%s)",PROGRAMID,PROG_VERSION,PROG_BUILD);
    lcd->println(0,0,LCD_Buffer);

    sprintf(LCD_Buffer,"%s %s %d",callsign,locator,WSPRPower);
    lcd->println(0,1,LCD_Buffer);

//--- Generate librpitx fskburst object (ideas taken from pift8.cpp by Courjaud F5OEO)

    float wspr_offset=(2.0*rand()/((double)RAND_MAX+1.0)-1.0)*(WSPR_RAND_OFFSET);
    fprintf(stderr,"%s %s:main() Frequency f(%10.2f) WSPR offset(%10.2f) random offset(%10.2f)\n",getTime(),PROGRAMID,f,offset,wspr_offset);

    FifoSize=162;
    Deviation=1.46;

    fsk= new fskburst(f+offset+wspr_offset, 1.46, Deviation, 14, FifoSize,Upsample,RampRatio);
    if(ppm!=1000) {   //ppm is set else use ntp
       fsk->Setppm(ppm);
       fsk->SetCenterFrequency(f+wspr_offset,50);            
    }
    usleep(1000);

//*--- Seed random number generator

    srand(time(0));

//*--- transform message to FSK codes to be used during the WSPR frame
unsigned char Symbols[FifoSize];

    sprintf(wspr_message, "%s %s %d", callsign,locator,WSPRPower);
    wspr.code_wspr(wspr_message, wspr_symbols);


    fprintf(stderr,"\n%s %s:main() %s:",getTime(),PROGRAMID,"WSPR Message");
    for (int i = 0; i < 162; i++) {
      fprintf(stderr,"%d", wspr_symbols[i]);
      Symbols[i]=wspr_symbols[i];
    }
    fprintf(stderr,"%s","\n");

//*---- wait for WSPR window to start

    tm *gmtm;
    char* dt;
    time_t now;

    now=time(0);
    dt=ctime(&now);

    float freq=f;

    gpioWrite(GPIO_COOLER,1);

//*-------------------------------------------------------------------
// Wait for next WSPR window (even minutes)
//*-------------------------------------------------------------------
    fprintf(stderr,"%s %s:main() %s",getTime(),PROGRAMID,"Waiting till next window\n");
    while (WSPRwindow==false) {

 // current date/time based on current system
       now = time(0);

 // convert now to string form
       dt = ctime(&now);

 // convert now to tm struct for UTC
       gmtm = gmtime(&now);
       dt = asctime(gmtm);
       byte m=( time( 0 ) % 3600 ) / 60;
       if ((m%2 ==0) && gmtm->tm_sec == 0) {
          WSPRwindow = true;
       }
    }   //* End of window waiting while

//*-------------------------------------------------------------------------------------------
//* Start transmission of WSPR message
//*-------------------------------------------------------------------------------------------
    fprintf(stderr,"%s %s:main() Current time is %s Starting TX\n",getTime(),PROGRAMID,dt);
    fprintf(stderr,"%s %s:main() Frequency base(%10.0f) center(%10.0f) offset(%10.0f)\n",getTime(),PROGRAMID,f,f+offset+wspr_offset,wspr_offset);

//*---- Turn on Transmitter

    lcd->setCursor(15,1);
    lcd->write(0);

    gpioWrite(GPIO_PTT,1);

    signal(SIGINT, terminate); 
    setWord(&MSW,RUN,true);


    fprintf(stderr,"%s %s:main() %s\n",getTime(),PROGRAMID,"Starting to TX!");

//*--- the object will send the entire symbol table and then return

    fsk->enableclk(GPIO_DDS);
    fsk->SetSymbols(Symbols, 162);
    fsk->stop();
    fsk->disableclk(GPIO_DDS);

    delete(fsk);
    fprintf(stderr,"%s %s:main() End of Tx\n",getTime(),PROGRAMID);

//*--- Finalize beacon

    gpioWrite(GPIO_PTT,0);
    gpioWrite(GPIO_COOLER,0);

//*--- Turn off LCD

    lcd->backlight(false);
    lcd->setCursor(0,0);
    lcd->clear();

    fprintf(stderr,"%s %s:main() Program execution terminated\n",getTime(),PROGRAMID);
    exit(0);
}
