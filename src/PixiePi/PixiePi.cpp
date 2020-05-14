/*
 * PixiePi
 * Raspberry Pi based transceiver controller
 *---------------------------------------------------------------------
 * This program operates as a controller for a Raspberry Pi to control
 * a Pixie transceiver hardware.
 * Project at http://www.github.com/lu7did/PixiePi
 *---------------------------------------------------------------------
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *    Adafruit's python code for CharLCDPlate 
 *    tune.cpp from rpitx package by Evariste Courjaud F5OEO
 *    sendiq.cpp from rpitx package (also) by Evariste Coujaud (F5EOE)
 *    wiringPi library (git clone git://git.drogon.net/wiringPi)
 *    iambic-keyer (https://github.com/n1gp/iambic-keyer)
 *    log.c logging facility by  rxi https://github.com/rxi/log.c
 *    minIni configuration management package by Compuphase https://github.com/compuphase/minIni/tree/master/dev
 *
 * Also libraries
 *    librpitx by  Evariste Courjaud (F5EOE)
 *    libcsdr by Karol Simonyi (HA7ILM) https://github.com/compuphase/minIni/tree/master/dev
 * 
 * ---------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

//----------------------------------------------------------------------------
//  includes
//----------------------------------------------------------------------------

//*---- Generic includes

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
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <wiringSerial.h>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include "/home/pi/librpitx/src/librpitx.h"

//*---- Program specific includes
#include "/home/pi/OrangeThunder/src/OT/OT.h"
#include "../lib/LCDLib.h"
#include "../lib/MMS.h"
#include "/home/pi/OrangeThunder/src/lib/CAT817.h"
#include "/home/pi/OrangeThunder/src/lib/genVFO.h"
#include "/home/pi/OrangeThunder/src/lib/CallBackTimer.h"


#include <iostream>
#include <cstdlib>    // for std::rand() and std::srand()
#include <ctime>      // for std::time()

#include <chrono>
#include <future>
#define SIGTERM_MSG "SIGTERM received.\n"

byte  TRACE=0x02;
byte  MSW=0x00;
byte  GSW=0x00;
byte  SSW=0x00;

void setPTT(bool f);
static void sighandler(int signum);
//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="PixiePi";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2018,2020";


extern "C" {
bool running=true;
bool ready=false;

#include "../lib/iambic.c"

}

int   a;
int   anyargs;
int   lcd_light;

LCDLib    *lcd;
char*     LCD_Buffer;
// *----------------------------------------------------------------*
// *                  GPIO support processing                       *
// *----------------------------------------------------------------*
//*--- debouncing logic setup
auto startEncoder=std::chrono::system_clock::now();
auto endEncoder=std::chrono::system_clock::now();

auto startPush=std::chrono::system_clock::now();
auto endPush=std::chrono::system_clock::now();

auto startAux=std::chrono::system_clock::now();
auto endAux=std::chrono::system_clock::now();

auto startLeft=std::chrono::system_clock::now();
auto endLeft=std::chrono::system_clock::now();

auto startRight=std::chrono::system_clock::now();
auto endRight=std::chrono::system_clock::now();

int value=0;
int lastEncoded=0;
int counter=0;
int clkLastState=0; 

// *----------------------------------------------------------------*
// *                  VFO Subsytem definitions                      *
// *----------------------------------------------------------------*
genVFO*    vfo=nullptr;

// *----------------------------------------------------------------*
// *                Manu Subsytem definitions                       *
// *----------------------------------------------------------------*
MMS* root;
MMS* keyer;
MMS* mode;
MMS* speed;
MMS* step;
MMS* shift;
MMS* drive;
MMS* backl;
MMS* cool;
MMS* beacon;
MMS* straight;
MMS* iambicA;
MMS* iambicB;
MMS* spval;
MMS* stval;
MMS* shval;
MMS* drval;
MMS* modval;
MMS* backval;
MMS* coolon;
MMS* coolof;
MMS* bcnon;
MMS* bcnoff;

int  TVFO=0;
int  TBCK=0;
int  TSAVE=0;
// *----------------------------------------------------------------*
// *                  CAT Subsytem definitions                      *
// *----------------------------------------------------------------*
void    CATchangeMode();      // Callback when CAT receives a mode change
void    CATchangeFreq();      // Callback when CAT receives a freq change
void    CATchangeStatus();    // Callback when CAT receives a status change

CAT817* cat=nullptr;
char    port[80];
long    catbaud=4800;

int     SNR;
// *----------------------------------------------------------------*
// *                  CAT support processing                        *
// *----------------------------------------------------------------*
float f=7030000;
byte  s=20;
byte  m=MCW;
char  callsign[16];
char  grid[16];
byte  l=7;
int   b=0;
float x=600.0;
byte  k=0;
int   backlight=0;

#include "../lib/GUI.h"
#include "../lib/VFO.h"


struct sigaction sigact;
CallBackTimer* masterTimer;

//*--------------------------[System Word Handler]---------------------------------------------------
//* getWord Return status according with the setting of the argument bit onto the SW
//*--------------------------------------------------------------------------------------------------
bool getWord (unsigned char SysWord, unsigned char v) {

  return SysWord & v;

}
//*--------------------------------------------------------------------------------------------------
//* setWord Sets a given bit of the system status Word (SSW)
//*--------------------------------------------------------------------------------------------------
void setWord(unsigned char* SysWord,unsigned char v, bool val) {

  *SysWord = ~v & *SysWord;
  if (val == true) {
    *SysWord = *SysWord | v;
  }

}
// ======================================================================================================================
// sighandler
// ======================================================================================================================
static void sighandler(int signum)
{

   (TRACE >= 0x00 ? fprintf(stderr, "\n%s:sighandler() Signal caught(%d), exiting!\n",PROGRAMID,signum) : _NOP);
   setWord(&MSW,RUN,false);
   if (getWord(MSW,RETRY)==true) {
      (TRACE >= 0x00 ? fprintf(stderr, "\n%s:sighandler() Re-entering SIG(%d), force!\n",PROGRAMID,signum) : _NOP);
      exit(16);
   }
   setWord(&MSW,RETRY,true);

}
//--------------------------------------------------------------------------------------------------
// FT8ISR - interrupt service routine, keep track of the FT8 sequence windows
//--------------------------------------------------------------------------------------------------
void ISRHandler() {

   if (TVFO!=0) { //VFO change marker
      TVFO--;
      if (TVFO==0) {
         setWord(&SSW,FVFO,true);
      }
   }
   if (TBCK!=0) { //Backlight timer
      (getWord(MSW,BCK)==true ? TBCK-- : _NOP);
      if (TBCK==0) {
         setWord(&SSW,FBCK,true);
      }
   }

   if (TSAVE!=0) { //Backlight timer
      TSAVE--;
      if (TSAVE==0) {
         setWord(&SSW,FSAVE,true);
      }
   }
}

//*-------------------------------------------------------------------------------------------------
//* print_usage
//* help message at program startup
//*-------------------------------------------------------------------------------------------------
void print_usage(void)
{

fprintf(stderr,"\n%s version %s build (%s)\n"
"Usage:\nPixiaPi [-f frequency {Hz}]\n"
"                [-s keyer speed {5..50 wpm default=20}]\n"
"                [-m mode {0=LSB,1=USB,2=CW,3=CWR,4=AM,5=FM,6=WFM,7=PKT,8=DIG default=CW}]\n"
"                [-c {callsign}]\n"
"                [-g {grid locator}]\n"
"                [-l {power level (0..7 default=7}]\n"
"                [-p {CAT port}]\n"
"                [-b Beacon enabled {0=inactive 1..60 minutes}]\n"
"                [-v Verbose {0,1,2,3 default=0}]\n"
"                [-x Shift {600..800Hz default=600}]\n"
"                [-B Backlight timeout {0..60 minutes default=0 (not activated)}]\n"
"                [-k Keyer {0=Straight,1=Iambic A,2=Iambic B default=0}]\n",PROGRAMID,PROG_VERSION,PROG_BUILD);

}

//*--------------------------------------------------------------------------------------------------
//* main execution of the program
//*--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
     std::srand(static_cast<unsigned int>(std::time(nullptr))); // set initial seed value to system clock    
     fprintf(stderr,"%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);

     for (int i = 0; i < 64; i++) {
        if (i != SIGALRM && i != 17 && i != 28) {
           signal(i,sighandler);
        }
     }


while(true)
        {
                a = getopt(argc, argv, "f:s:m:c:g:l:p:b:B:v:x:k:h?");

                if(a == -1) 
                {
                        if(anyargs) {
                           break; 
                        } else {
                           break;
                        }
                }
                anyargs = 1;    

                switch(a)
                {

                case 'f': 
	                f=atof(optarg);
                        fprintf(stderr,"%s:main() args(f)=%5.0f\n",PROGRAMID,f);
                        break;
                case 's':
                        s=atoi(optarg);
                        fprintf(stderr,"%s:main() args(speed)=%d\n",PROGRAMID,s);
                        break;
                case 'm':
                        m=atoi(optarg);
                        fprintf(stderr,"%s:main() args(mode)=%d\n",PROGRAMID,m);
                        break;
                case 'c':
                        strcpy(callsign,optarg);
                        fprintf(stderr,"%s:main() args(callsign)=%s\n",PROGRAMID,callsign);
                        break;
                case 'g': 
                        strcpy(grid,optarg);
                        fprintf(stderr,"%s:main() args(grid)=%s\n",PROGRAMID,grid);
                        break;
                case 'l':
                        l=atoi(optarg);
                        fprintf(stderr,"%s:main() args(level)=%d\n",PROGRAMID,l);
                        break;
                case 'p':     // gain in dB 
                        strcpy(port,optarg);
                        fprintf(stderr,"%s:main() args(port)=%s\n",PROGRAMID,port);
                        break;
                case 'b':
                        b=atoi(optarg);
                        fprintf(stderr,"%s:main() args(beacon)=%d secs\n",PROGRAMID,b);
                        break;
                case 'B':
                        backlight=atoi(optarg);
                        fprintf(stderr,"%s:main() args(backlight)=%d secs\n",PROGRAMID,backlight);
                        break;
                case 'v':
                        TRACE=atoi(optarg);
                        fprintf(stderr,"%s:main() args(verbose)=%d\n",PROGRAMID,TRACE);
                        break;
                case 'x':
			x=atof(optarg);
                        fprintf(stderr,"%s:main() args(shift)=%5.0f\n",PROGRAMID,x);
                        break;
                case 'k': // keyer mode
                        k=atoi(optarg);
                        fprintf(stderr,"%s:main() args(keyer)=%d\n",PROGRAMID,k);
                        break;
                case -1:
                        break;
                case '?':
                        print_usage();
                        exit(1);
                        break;
                default:
                        print_usage();
                        exit(1);
                        break;
                }
        }

//*--- Create memory resources

    (TRACE>=0x01 ? fprintf(stderr,"%s:main() Memory resources acquired\n",PROGRAMID) : _NOP);
     LCD_Buffer=(char*) malloc(32);

//*--- Define and initialize LCD interface

     setupLCD();
     sprintf(LCD_Buffer,"%s %s(%s)",PROGRAMID,PROG_VERSION,PROG_BUILD);
     lcd->println(0,0,LCD_Buffer);
     sprintf(LCD_Buffer,"%s","Booting..");
     lcd->println(0,1,LCD_Buffer);

//*--- Establish master clock

    (TRACE>=0x01 ? fprintf(stderr,"%s:main() Master timer enabled\n",PROGRAMID) : _NOP);
     masterTimer=new CallBackTimer();
     masterTimer->start(1,ISRHandler);


//*--- Setup VFO System

    (TRACE>=0x01 ? fprintf(stderr,"%s:main() VFO sub-system initialization\n",PROGRAMID) : _NOP);
     vfo=new genVFO(&freqVfoHandler,&ritVfoHandler,&modeVfoHandler,&changeVfoHandler);
     vfo->TRACE=TRACE;
     vfo->FT817=0x00;
     vfo->setMode(MCW);
     vfo->setBand(VFOA,vfo->getBand(f));
     vfo->setBand(VFOB,vfo->getBand(f));
     vfo->set(VFOA,f);
     vfo->set(VFOB,f);
     vfo->setSplit(false);
     vfo->setRIT(VFOA,false);
     vfo->setRIT(VFOB,false);
     vfo->POWER=(l & 0x0f);

     vfo->vfo=VFOA;
     setWord(&vfo->FT817,VFO,VFOA);

//*--- Setup the CAT system

    (TRACE>=0x01 ? fprintf(stderr,"%s:main() CAT system initialization\n",PROGRAMID) : _NOP);

     cat=new CAT817(CATchangeFreq,CATchangeStatus,CATchangeMode,CATgetRX,CATgetTX);
     cat->FT817=vfo->FT817;
     cat->POWER=vfo->POWER;
     cat->f=f;
     cat->MODE=m;
     cat->TRACE=TRACE;
     cat->open(port,catbaud);


//*--- Create infrastructure for the execution of the GUI

     createMenu();

//*-------------------------------------------------------------------------------------------------------------------
//*---  Define and initialize GPIO interface
//*
//*     clk (GPIO17)----(rotary encoder)------
//*     dt  (GPIO18)
//*
//*     aux (GPIO10)----(encoder push)--------
//*     sw  (GPIO27)----(aux button)---------
//*
//*
//*--------------------------------------------------------------------------------------------------------------------

      setupGPIO();


//*--- Initialize a startup

    (TRACE>=0x01 ? fprintf(stderr,"%s:main() Starting operations\n",PROGRAMID) : _NOP);
     setWord(&GSW,FGUI,true);  //force an initial update of the LCD panel
     setWord(&MSW,RUN,true);   //mark the program to start running


//*--- start the keyer

     cw_keyer_mode=k;
     iambic_init();

     setBacklight(true);
     lcd->clear();
     showGui();

     setCooler(true);
     setPTT(false);

     //gpio->writePin(GPIO_COOLER,1);
//--------------------------------------------------------------------------------------------------
// Main program loop
//--------------------------------------------------------------------------------------------------

     while(getWord(MSW,RUN)==true) {

//*--- Read and process events coming from the CAT subsystem
          (cat->active==true ? cat->get() : (void) _NOP);

//*--- Read and process  events coming from the GUI interface

          processGui();
     }
 
//*--- Stop the keyer

  iambic_close();

//  gpio->writePin(GPIO_COOLER,1);

  lcd->backlight(false);
  lcd->setCursor(0,0);
  lcd->clear();

  setCooler(false);
  setPTT(false);
  gpioTerminate();
  exit(0);
}


