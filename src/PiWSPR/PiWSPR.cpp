
/**
 * PiWSPR.cpp 
 * Raspberry Pi based WSPR beacon
 *

 * This program turns the Raspberry pi into a WSPR beacon software able
 * to operate at the indicated frequency as a direct RF generator
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

//---  VFO initial setup
typedef unsigned char byte;
typedef bool boolean;



#include <unistd.h>
//---- Program specific includes

#include "./PiWSPR.h"		// wspr definitions and functions
#include "../lib/WSPR.h"
#include "../lib/DDS.h"

//----------------------------------------------------------------------------
//  Program parameter definitions
//----------------------------------------------------------------------------

const char   *PROGRAMID="PiWSPR";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019";

//-------------------------------------------------------------------------------------------------
// Main structures
//-------------------------------------------------------------------------------------------------

struct sigaction sa;

//---- Generic memory allocations

int    value=0;
int    lastEncoded=0;
int    counter=0;
int    clkLastState=0; 
char   hi[80];
byte   memstatus=0;
byte   ntimes=1;
int    a;
int    anyargs = 0;
float  SetFrequency=VFO_START;
float  ppm=1000.0;
bool   running=true;
byte   keepalive=0;
bool   first=true;
char   port[80];
byte   gpio=GPIO04;
char   callsign[10];
char   locator[10];
int    power=10;
int    ntx=0;
int    nskip=0;
byte   ptt=GPIO12;

char   wspr_message[40];          // user beacon message to encode
unsigned char wspr_symbols[WSPR_LENGTH] = {};
unsigned long tuning_words[WSPR_LENGTH];

bool WSPRwindow=false;
void cbkDDS();

//*---- Define WSPR memory blocks

DDS    *dds=new DDS(cbkDDS);
WSPR   wspr(NULL);

//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//                              ROUTINE STRUCTURE
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//--------------------------------------------------------------------------
// Callback for DDS pointers
//--------------------------------------------------------------------------
void cbkDDS() {

    //NOP
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
\t-n set GPIO port for PTT (default: GPIO12)\n\
\t-i set number of windows to skip between transmission (0 none, default)\n\
\t-g set GPIO port (4 or 20)\n\
\t-x force WSPR Window now (test only)\n\
\t-h help (this help).\n\n\
\t e.g.: PiWSPR -c LU7DID -l GF05 -d 20 -f 14095600 -n 1 -g 20\n\
\n");
}
//---------------------------------------------------------------------------------------------
// Signal handlers, SIGALRM is used as a timer, all other signals means termination
//---------------------------------------------------------------------------------------------
static void terminate(int num)
{
    fprintf(stderr,"\n Received signal INT(%d %s)\n",num,strsignal(num));
    running=false;
}
//---------------------------------------------------------------------------------------------
// MAIN Program
//---------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

//--- Initial presentation

  fprintf(stderr,"%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);

//--- Parse arguments

   while(1)
        {
                a = getopt(argc, argv, "f:ed:hs:g:p:n:c:l:x");
        
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
                     fprintf(stderr,"Forcing WSPR window now!\n");
                     break; 
                case 'f': // Frequency
                     if (!strcasecmp(optarg,"LF")) {
                        SetFrequency=137500.0;
                     } else if (!strcasecmp(optarg,"LF-15")) {
                               SetFrequency=137612.5;
                            } else if (!strcasecmp(optarg,"MF")) {
                                      SetFrequency=475700.0;
                                   } else if (!strcasecmp(optarg,"MF-15")) {
                                             SetFrequency=475812.5;
                                          } else if (!strcasecmp(optarg,"160m")) {
                                                    SetFrequency=1838100.0;
                                                 } else if (!strcasecmp(optarg,"160m-15")) {
                                                           SetFrequency=1838212.5;
                                                        } else if (!strcasecmp(optarg,"80m")) {
                                                                  SetFrequency=3594100.0;
                                                               } else if (!strcasecmp(optarg,"60m")) {
                                                                         SetFrequency=5288700.0;
                                                                      } else if (!strcasecmp(optarg,"40m")) {
                                                                                SetFrequency=7038600.0;
                                                                             } else if (!strcasecmp(optarg,"30m")) {
                                                                                       SetFrequency=10140200.0;
                                                                                    } else if (!strcasecmp(optarg,"20m")) {
                                                                                              SetFrequency=14097100.0;
                                                                                           } else if (!strcasecmp(optarg,"17m")) {
                                                                                                     SetFrequency=18106100.0;
                                                                                                  } else if (!strcasecmp(optarg,"15m")) {
                                                                                                            SetFrequency=21096100.0;
                                                                                                         } else if (!strcasecmp(optarg,"12m")) {
                                                                                                                   SetFrequency=24926100.0;
                                                                                                                } else if (!strcasecmp(optarg,"10m")) {
                                                                                                                          SetFrequency=28126100.0;
                                                                                                                       } else if (!strcasecmp(optarg,"6m")) {
                                                                                                                                 SetFrequency=50294500.0;
                                                                                                                              } else if (!strcasecmp(optarg,"4m")) {
                                                                                                                                        SetFrequency=70092500.0;
                                                                                                                                     } else if (!strcasecmp(optarg,"2m")) {
                                                                                                                                               SetFrequency=144490500.0;
                                                                                                                                            } else {
                                                                                                                                               SetFrequency = atof(optarg);
                                                                                                                                            }
                     
		     fprintf(stderr,"Frequency: %10.0f Hz\n",SetFrequency);
                     break;
        	case 'd': //power
			power=atoi(optarg);
			fprintf(stderr,"Power: %d dBm\n",power);
			break;
		case 'c': //callsign
			sprintf(callsign,optarg);
			fprintf(stderr,"Callsign: %s\n",callsign);
			break;
		case 'l': //locator
			sprintf(locator,optarg);
			fprintf(stderr,"Locator: %s\n",locator);
			break;
                case 'n': // GPIO PTT
                        ptt=atoi(optarg);
                        if (gpio < 1 || gpio > 19) {
                           ptt=GPIO12;
                        }
                        fprintf(stderr,"PTT Out: GPIO%d\n",ptt);
                        break;
                case 'g': // GPIO
                        gpio = atoi(optarg);
                        if (gpio != GPIO04 && gpio != GPIO20) {
                           fprintf(stderr,"Invalid selection for GPIO(%s), must be 4 or 20\n",optarg);
                           break;
                        }
			fprintf(stderr,"Pin Out: GPIO%d\n",gpio);
                        break;
                case 'p': //ppm
                        ppm=atof(optarg);
			fprintf(stderr,"PPM: %10.2f",ppm);
                        break;  
                case 'h': // help
                        print_usage();
                        exit(1);
                        break;
                case -1:
                break;
                case '?':
                        if (isprint(optopt) ) {
                           fprintf(stderr, "PiWSPR: unknown option `-%c'.\n", optopt);
                        } else {
                           fprintf(stderr, "PiWSPR: unknown option character `\\x%x'.\n", optopt);
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

//*--- Initialize GPIO management (PTT)

    if(gpioInitialise()<0) {
       fprintf(stderr,"Cannot initialize GPIO\n");
       return -1;
    }


//--- Generate DDS (code excerpt mainly from tune.cpp by Evariste Courjaud F5OEO

    dds->gpio=gpio;
    dds->power=MAXLEVEL;
    dds->setppm(1000.0);

//*--- Seed random number generator

    srand(time(0));

//--- Generate WSPR message

    sprintf(wspr_message, "%s %s %d", callsign,locator,power);
    wspr.code_wspr(wspr_message, wspr_symbols);
    printf("WSPR Message\n");
    printf("------------\n");
    for (int i = 0; i < WSPR_LENGTH; i++) {
      printf("%d", wspr_symbols[i]);
    }
    printf("\n");

    tm *gmtm;
    char* dt;
    time_t now;

    now=time(0);
    dt=ctime(&now);

    float f=SetFrequency+WSPR_SHIFT+WSPR_BAND;
//*-------------------------------------------------------------------
// Wait for next WSPR window (even minutes)
//*-------------------------------------------------------------------
    printf("Waiting till next window\n");

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

    fprintf(stderr,"Current time is %s Starting TX\n",dt);

    int j=0;
    float wspr_offset=(2.0*rand()/((double)RAND_MAX+1.0)-1.0)*(WSPR_RAND_OFFSET);
    f+=wspr_offset;
    fprintf(stderr,"Random frequency offset %10.2f\n",wspr_offset);
    dds->open(f);

//*---- Turn on Transmitter

    gpioWrite(ptt,PTT_ON);
    usleep(1000);
    fprintf(stderr,"Frequency base(%10.0f) center(%10.0f) offset(%10.0f)\n",SetFrequency,f,wspr_offset);
    signal(SIGINT, terminate); 
    while (j<WSPR_LENGTH && running==true){
       int t=wspr_symbols[j];
       if ((t >= 0) && (t <= 3) ) {
          float frequency=(f + (t * 1.4648));
          dds->set(frequency);
          usleep(1000);
       }
       j++;
       usleep(683000); 
    } //* End of symbol sending while

//*--- Finalize beacon

    fprintf(stderr,"Turning off TX\n");
    gpioWrite(ptt,PTT_OFF);

    dds->close();
    usleep(100000);

    delete(dds);
    printf("\nEnding beacon\n");
    exit(0);
}
