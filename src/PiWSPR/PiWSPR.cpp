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

//---  VFO initial setup
typedef unsigned char byte;
typedef bool boolean;



#include <unistd.h>
//---- Program specific includes
#include "./PiWSPR.h"		// wspr definitions and functions
#include "../lib/WSPR.h"
#include "../lib/DDS.h"

#define WSPR_RAND_OFFSET 80
#define VFO_START 14095600
#define GPIO04     4
#define GPIO20    20
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

//---- Generic memory allocations

int value=0;
int lastEncoded=0;
int counter=0;
int clkLastState=0; 
char hi[80];
byte memstatus=0;
byte ntimes=1;
int a;
int anyargs = 0;
float SetFrequency=VFO_START;
float ppm=1000.0;
struct sigaction sa;
bool running=true;
byte keepalive=0;
bool first=true;
char port[80];
byte gpio=GPIO04;
char callsign[10];
char locator[10];
int  power=10;

char wspr_message[20];          // user beacon message to encode
unsigned char wspr_symbols[WSPR_LENGTH] = {};
unsigned long tuning_words[WSPR_LENGTH];

void cbkDDS();
DDS    dds(cbkDDS);
WSPR   wspr(NULL);

//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//                              ROUTINE STRUCTURE
//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//--------------------------------------------------------------------------
// Callback for DDS pointers
//--------------------------------------------------------------------------
void cbkDDS() {

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
\t-n set number of times to emit (0 indefinite)\n\
\t-g set GPIO port (4 or 20)\n\
\t-h help (this help).\n\n\
\t e.g.: PiWSPR -c LU7DID -l GF05 -d 20 -f 14095600 -n 1 -g 20\n\
\n");
}
//---------------------------------------------------------------------------------------------
// Signal handlers, SIGALRM is used as a timer, all other signals means termination
//---------------------------------------------------------------------------------------------
static void terminate(int num)
{
    printf("\n received signal(%d %s)\n",num,strsignal(num));
    running=false;
    dds.clk->disableclk(gpio);
    dds.close();
    usleep(100000);
    printf("\n Program abnormally terminated\n");
    exit(4);
   
}
//---------------------------------------------------------------------------------------------
// MAIN Program
//---------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

//--- Initial presentation

  sprintf(hi,"%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);
  printf(hi);

//--- Parse arguments

   while(1)
        {
                a = getopt(argc, argv, "f:ed:hs:g:p:c:l:");
        
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
                       SetFrequency=7040100.0;
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
                case 'g': // GPIO
              
                        gpio = atoi(optarg);
                        if (gpio != GPIO04 && gpio != GPIO20) {
                           sprintf(port,optarg);
                           fprintf(stderr,"Invalid selection for GPIO(%s), must be 4 or 20\n",optarg);
                           break;
                        }
                        sprintf(port,optarg);
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
                case 's': //serial port
                        sprintf(port,optarg);
                        fprintf(stderr, "Serial port:%s\n", optarg);
                        break;
                case -1:
                break;
                case '?':
                        if (isprint(optopt) )
                        {
                           fprintf(stderr, "PiWSPR: unknown option `-%c'.\n", optopt);
                        }
                        else
                        {
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

//--- Define the rest of the signal handlers, basically as termination exceptions

    for (int i = 0; i < 64; i++) {

        if (i != SIGALRM && i != 17 && i != 28 ) {
           std::memset(&sa, 0, sizeof(sa));
           sa.sa_handler = terminate;
           sigaction(i, &sa, NULL);
        }
    }


//--- Generate DDS (code excerpt mainly from tune.cpp by Evariste Courjaud F5OEO

    dds.gpio=gpio;
    dds.setppm(ppm);

//--- Generate WSPR message

    sprintf(wspr_message, "%s %s %d", callsign,locator,power);
    wspr.code_wspr(wspr_message, wspr_symbols);
    printf("WSPR Message\n");
    printf("------------\n");
    for (int i = 0; i < WSPR_LENGTH; i++) {
      printf("%d", wspr_symbols[i]);
    }
    printf("\n");

    bool WSPRwindow=false;
    tm *gmtm;
    char* dt;
    time_t now;
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
    }

//--- Transmit WSPR message

    printf("Current time is %s\n",dt);
    printf("Starting TX\n");

    int j=0;
    srand(time(0));
    float wspr_offset=(2.0*rand()/((double)RAND_MAX+1.0)-1.0)*(WSPR_RAND_OFFSET);
    printf("Random frequency offset %10.2f\n",wspr_offset);

    while (j<WSPR_LENGTH && running==true){
        int t=wspr_symbols[j];
        if ((t >= 0) && (t <= 3) ) {
           float frequency=(SetFrequency + (t * 1.4648));
           frequency+=wspr_offset;
           dds.set(frequency);
        }
       j++;
       usleep(683000); 
    }
  dds.clk->disableclk(gpio);
  dds.close();
  usleep(100000);

  printf("\nEnding Tx\n");
  usleep(10000);

  exit(0);
}
