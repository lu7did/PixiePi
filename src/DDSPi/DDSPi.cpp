/**
 * DDSPi.cpp 
 * Raspberry Pi based DDS
 *
 * This program turns the Raspberry pi into a DDS software able
 * to operate as the LO for a double conversion rig or as a direct RF generator
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *    tune.cpp from rpitx package by Evariste Courjaud F5OEO
 *     wiringPi library (git clone git://git.drogon.net/wiringPi)
 *    
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

//*----------------------------------------------------------------------------
//*  includes
//*----------------------------------------------------------------------------

//---- Generic includes

#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sched.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <pigpio.h>
#include <wiringPiI2C.h>
#include <unistd.h>
#include "/home/pi/librpitx/src/librpitx.h"
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <wiringSerial.h>
#include <sstream>
#include <iomanip>

//*---- Program specific includes

#include "DDSPi.h"
#include "../lib/CAT817.h"
#include "../lib/DDS.h"

//*---  VFO initial setup
typedef unsigned char byte;
typedef bool boolean;

#define VFO_START 	  7000000
#define VFO_END           7299000
#define VFO_BAND_START          3
#define ONESEC               1000
#define GPIO04                  4
#define GPIO20                 20

#define VFO_DELAY               1

//*----------------------------------------------------------------------------
//*  Program parameter definitions
//*----------------------------------------------------------------------------

const char   *PROGRAMID="DDSPi";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019";
//*-------------------------------------------------------------------------------------------------
//* Main structures
//*-------------------------------------------------------------------------------------------------


//*---- Generic memory allocations

int value=0;
int lastEncoded=0;
int counter=0;
int clkLastState=0; 
char hi[80];
byte memstatus=0;
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

//*---- Callback prototypes

void cbkFreq();
void cbkStatus();
void cbkMode();
void cbkRxStatus();
void cbkTxStatus();
void cbkDDS();

//*------- CAT and DDS objects

CAT817 cat(cbkFreq,cbkStatus,cbkMode,cbkRxStatus,cbkTxStatus);
DDS    dds(cbkDDS);

//*--------------------------------------------------------------------------
//* Callback for DDS pointers
//*--------------------------------------------------------------------------
void cbkDDS() {

}
//*--------------------------------------------------------------------------------------------------
//* Callback for CAT pointers
//*------------------------------------------------------------------------------------------
void cbkFreq() {

   SetFrequency = cat.SetFrequency;
   int f=(int)SetFrequency;
   printf("Frequency set to f(%d)\n",f);
   dds.set(SetFrequency);

}
//*------------
void cbkStatus() {

}
//*------------
void cbkMode() {

}
//*------------
void cbkRxStatus() {
}
//*------------
void cbkTxStatus() {

}
//*--------------------------[System Word Handler]---------------------------------------------------
//* getSSW Return status according with the setting of the argument bit onto the SW
//*--------------------------------------------------------------------------------------------------
bool getWord (unsigned char SysWord, unsigned char v) {

  return SysWord & v;

}
//*--------------------------------------------------------------------------------------------------
//* setSSW Sets a given bit of the system status Word (SSW)
//*--------------------------------------------------------------------------------------------------
void setWord(unsigned char* SysWord,unsigned char v, bool val) {

  *SysWord = ~v & *SysWord;
  if (val == true) {
    *SysWord = *SysWord | v;
  }

}


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                              ROUTINE STRUCTURE
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*-------------------------------------------------------------------------------------------------
//* print_usage
//* help message at program startup
//*-------------------------------------------------------------------------------------------------
void print_usage(void)
{

fprintf(stderr,"\nDDSPi -%s\n\
Usage:\ntune  [-f Frequency] [-h] \n\
-f floatfrequency carrier Hz(50 kHz to 1500 MHz),\n\
-p set clock ppm instead of ntp adjust\n\
-d set DEBUG mode\n\
-g set GPIO port (4 or 20)\n\
-s set serial CAT port\n\
-h            help (this help).\n\
\n",\
PROGRAMID);

}
//*---------------------------------------------------------------------------
//* Timer handler function
//*---------------------------------------------------------------------------
void timer_start(std::function<void(void)> func, unsigned int interval)
{
  std::thread([func, interval]()
  { 
    while (true)
    { 
      auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
      func();
      std::this_thread::sleep_until(x);
    }
  }).detach();
}
//*---------------------------------------------------------------------------
//* Timer callback function
//*---------------------------------------------------------------------------
void timer_exec()
{


}
//*---------------------------------------------------------------------------------------------
//* Signal handlers, SIGALRM is used as a timer, all other signals means termination
//*---------------------------------------------------------------------------------------------
static void terminate(int num)
{
    printf("\n received signal(%d %s)\n",num,strsignal(num));
    running=false;
   
}
//*---------------------------------------------------
void sigalarm_handler(int sig)
{
}
//*--------------------------------------------------------------------------------------------------
//* main execution of the program
//*--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{

//*--- Initial presentation

    sprintf(hi,"%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);
    printf(hi);
    dbg_setlevel(1);

//*--- Process arguments (mostly an excerpt from tune.cpp
    sprintf(port,"/tmp/ttyv1");

    while(1)
        {
                a = getopt(argc, argv, "f:edhs:g:p:");
        
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
                        SetFrequency = atof(optarg);
			fprintf(stderr,"DDSPi: Frequency (%d)\n",(int)SetFrequency);
                        break;
                case 'g': // Frequency
              
                        gpio = atoi(optarg);
    			if (gpio != GPIO04 && gpio != GPIO20) {
      			   sprintf(port,optarg);
			   fprintf(stderr,"DDSPi: Invalid selection for GPIO(%s), must be 4 or 20\n",optarg);
			   break;
			}
                        sprintf(port,optarg);
                        fprintf(stderr, "DDSPi: GPIO port set to:%s\n", optarg);
                        break;
                case 'p': //ppm
                        ppm=atof(optarg);
                        break;  
                case 'h': // help
                        print_usage();
                        exit(1);
                        break;
                case 'd': //
                        cat.TRACE=0x01;
			fprintf(stderr,"DDSPi: DEBUG mode enabled\n");
                        break;
                case 's': //serial port
                        sprintf(port,optarg);
                        fprintf(stderr, "DDSPi: serial port:%s\n", optarg);
                        break;
                case -1:
                break;
                case '?':
                        if (isprint(optopt) )
                        {
                           fprintf(stderr, "DDSPi: unknown option `-%c'.\n", optopt);
                        }
                        else
                        {
                           fprintf(stderr, "DDSPi: unknown option character `\\x%x'.\n", optopt);
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


//---- Establish initial values of system variables

    if (wiringPiSetup () < 0) {
        printf ("Unable to setup wiringPi: %s\n", strerror (errno));
        return 1;
    }

    signal(SIGALRM, &sigalarm_handler);  // set a signal handler

//*--- timer start interrupt every 1 sec
    
    timer_start(timer_exec,1000);

//*--- Define the rest of the signal handlers, basically as termination exceptions

    for (int i = 0; i < 64; i++) {

        if (i != SIGALRM && i != 17 && i != 28 ) {
           std::memset(&sa, 0, sizeof(sa));
           sa.sa_handler = terminate;
           sigaction(i, &sa, NULL);
        }
    }

//*--- Generate DDS (code excerpt mainly from tune.cpp by Evariste Courjaud F5OEO

    dds.gpio=gpio;
    dds.ppm=ppm;
    dds.open(SetFrequency);

//*--- Establish parameters for CAT connection

    cat.open(port,4800);
    cat.SetFrequency=SetFrequency;

//*--- Execute an endless loop while runnint is true

    while(running)
      {
       cat.get();
       usleep(10000);
      }
    dds.close();
    usleep(100000);
    printf("\nProgram terminated....\n");
    exit(0);
}


