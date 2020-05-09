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
#include "../lib/ClassMenu.h"
#include "../lib/LCDLib.h"
#include "../lib/DDS.h"
#include "../minIni/minIni.h"
#include "../log.c/log.h"
#include "../lib/MMS.h"

#include "/home/pi/OrangeThunder/src/OT/OT.h"
#include "/home/pi/OrangeThunder/src/lib/CAT817.h"
#include "/home/pi/OrangeThunder/src/lib/gpioWrapper.h"
#include "/home/pi/OrangeThunder/src/lib/genVFO.h"
#include "/home/pi/OrangeThunder/src/lib/CallBackTimer.h"

#include <iostream>
#include <cstdlib>    // for std::rand() and std::srand()
#include <ctime>      // for std::time()

#include <chrono>
#include <future>


void gpiochangePin(int pin, int state);
void gpiochangeEncoder(int clk,int dt,int state);


byte  TRACE=0x01;
byte  MSW=0x00;
byte  GSW=0x00;
byte  SSW=0x00;

int   a;
int   anyargs;
int   lcd_light;
//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="PixiePi";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2018,2020";

LCDLib    *lcd;
char*     LCD_Buffer;
// *----------------------------------------------------------------*
// *                  GPIO support processing                       *
// *----------------------------------------------------------------*
// --- gpio object
gpioWrapper* gpio=nullptr;
char   *gpio_buffer;
void gpiochangePin();

// *----------------------------------------------------------------*
// *                  VFO Subsytem definitions                      *
// *----------------------------------------------------------------*
genVFO*    vfo=nullptr;

// *----------------------------------------------------------------*
// *                Manu Subsytem definitions                       *
// *----------------------------------------------------------------*
MMS* root;
MMS* keyer;
MMS* speed;
MMS* step;
MMS* shift;
MMS* drive;
MMS* backl;
MMS* cool;
MMS* straight;
MMS* iambicA;
MMS* iambicB;
MMS* spval;
MMS* stval;
MMS* shval;
MMS* drval;
MMS* backon;
MMS* backof;
MMS* coolon;
MMS* coolof;

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
byte    FT817;
char    port[80];
long    catbaud=4800;
int     SNR;
// *----------------------------------------------------------------*
// *                  CAT support processing                        *
// *----------------------------------------------------------------*
float f=7030000;


#include "../lib/GUI.h"



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
//---------------------------------------------------------------------------
// CATchangeFreq()
// CAT Callback when frequency changes
//---------------------------------------------------------------------------
void CATchangeFreq() {

   if (getWord(MSW,PTT)==false) {
     (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeFreq() cat.SetFrequency(%d) request while transmitting, ignored!\n",PROGRAMID,(int)cat->SetFrequency) : _NOP);
     f=cat->SetFrequency;
     return;
   }
   
  cat->SetFrequency=f;
  (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeFreq() Frequency change is not allowed(%d)\n",PROGRAMID,(int)f) : _NOP);

}
//-----------------------------------------------------------------------------------------------------------
// CATchangeMode
// Validate the new mode is a supported one
// At this point only CW,CWR,USB and LSB are supported
//-----------------------------------------------------------------------------------------------------------
void CATchangeMode() {

  (TRACE>=0x02 ? fprintf(stderr,"%s:CATchangeMode() requested MODE(%d) not supported\n",PROGRAMID,cat->MODE) : _NOP);
  cat->MODE=MUSB;
  return;

}
//------------------------------------------------------------------------------------------------------------
// CATchangeStatus
// Detect which change has been produced and operate accordingly
//------------------------------------------------------------------------------------------------------------
void CATchangeStatus() {

  (TRACE >= 0x03 ? fprintf(stderr,"%s:CATchangeStatus() FT817(%d) cat.FT817(%d)\n",PROGRAMID,FT817,cat->FT817) : _NOP);

  if (getWord(cat->FT817,PTT) != getWord(MSW,PTT)) {
     (TRACE>=0x02 ? fprintf(stderr,"%s:CATchangeStatus() PTT change request cat.FT817(%d) now is PTT(%s)\n",PROGRAMID,cat->FT817,getWord(cat->FT817,PTT) ? "true" : "false") : _NOP);
     //setPTT(getWord(cat->FT817,PTT));
     setWord(&MSW,PTT,getWord(cat->FT817,PTT));
  }

  if (getWord(cat->FT817,RIT) != getWord(FT817,RIT)) {        // RIT Changed
     (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeStatus() RIT change request cat.FT817(%d) RIT changed to %s ignored\n",PROGRAMID,cat->FT817,getWord(cat->FT817,RIT) ? "true" : "false") : _NOP);
  }

  if (getWord(cat->FT817,LOCK) != getWord(FT817,LOCK)) {      // LOCK Changed
     (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeStatus() LOCK change request cat.FT817(%d) LOCK changed to %s ignored\n",PROGRAMID,cat->FT817,getWord(cat->FT817,LOCK) ? "true" : "false") : _NOP);
  }

  if (getWord(cat->FT817,SPLIT) != getWord(FT817,SPLIT)) {    // SPLIT mode Changed
     (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeStatus() SPLIT change request cat.FT817(%d) SPLIT changed to %s ignored\n",PROGRAMID,cat->FT817,getWord(cat->FT817,SPLIT) ? "true" : "false") : _NOP);
  }

  if (getWord(cat->FT817,VFO) != getWord(FT817,VFO)) {        // VFO Changed
     (TRACE >=0x01 ? fprintf(stderr,"%s:CATchangeStatus() VFO change request not supported\n",PROGRAMID) : _NOP);
  }
  FT817=cat->FT817;
  return;

}
//--------------------------------------------------------------------------------------------------
// Callback to process SNR signal (if available)
//--------------------------------------------------------------------------------------------------
void CATgetRX() {

    //cat->RX=cat->snr2code(SNR);

}
//--------------------------------------------------------------------------------------------------
// Callback to process Power/SWR signal (if available)
//--------------------------------------------------------------------------------------------------
void CATgetTX() {

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
      TBCK--;
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
//*--------------------------------------------------------------------------------------------------
//* main execution of the program
//*--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    
     fprintf(stderr,"%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);

//*--- Establish exception handling structure

     sigact.sa_handler = sighandler;
     sigemptyset(&sigact.sa_mask);
     sigact.sa_flags = 0;


     sigaction(SIGINT, &sigact, NULL);
     sigaction(SIGTERM, &sigact, NULL);
     sigaction(SIGQUIT, &sigact, NULL);
     sigaction(SIGPIPE, &sigact, NULL);
     signal(SIGPIPE, SIG_IGN);


while(true)
        {
                a = getopt(argc, argv, "f:eds:hg:i:p:A:C:D:E:F:G:M:S:");

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

                case 'A': 
                        break;
                case 'i':
                        break;
                case 'C':
                        break;
                case 'D':
                        break;
                case 'E': 
                        break;

                case 'F':
                        break;
                case 'G':     // gain in dB 
                        break;
                case 'M':
                        break;
                case 'S':
                        break;
                case 'W':
                        break;
                case 'f': // Frequency
                        break;
		case 'g': // GPIO
			break;
                case 'p': //ppm
                        break;
                case 'd': //debug
                        break;
                case 'h': // help
                        break;
                case 's': //serial port
                        break;
                case -1:
                break;
                case '?':
                        exit(1);
                        break;
                default:
                        exit(1);
                        break;
                }
        }

//*--- Create memory resources

    (TRACE>=0x01 ? fprintf(stderr,"%s:main() Memory resources acquired\n",PROGRAMID) : _NOP);
     gpio_buffer=(char*) malloc(BUFSIZE);
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

//*--- Setup the CAT system
    (TRACE>=0x01 ? fprintf(stderr,"%s:main() CAT system initialization\n",PROGRAMID) : _NOP);

     cat=new CAT817(CATchangeFreq,CATchangeStatus,CATchangeMode,CATgetRX,CATgetTX);
     cat->FT817=FT817;
     cat->POWER=DDS_MAXLEVEL;
     cat->SetFrequency=f;
     cat->MODE=MUSB;
     cat->TRACE=TRACE;
     cat->open(port,catbaud);
     cat->getRX=CATgetRX;

//*--- Setup VFO System

    (TRACE>=0x01 ? fprintf(stderr,"%s:main() VFO sub-system initialization\n",PROGRAMID) : _NOP);
     vfo=new genVFO(NULL,NULL,NULL);
     vfo->TRACE=TRACE;
     vfo->FT817=FT817;
     vfo->MODE=cat->MODE;
     vfo->setBand(VFOA,vfo->getBand(f));
     vfo->setBand(VFOB,vfo->getBand(f));
     vfo->set(VFOA,f);
     vfo->set(VFOB,f);
     vfo->setSplit(false);
     vfo->setRIT(VFOA,false);
     vfo->setRIT(VFOB,false);

     vfo->vfo=VFOA;
     setWord(&cat->FT817,VFO,VFOA);

//*--- Initialize a startup

    (TRACE>=0x01 ? fprintf(stderr,"%s:main() Starting operations\n",PROGRAMID) : _NOP);
     setWord(&GSW,FGUI,true);  //force an initial update of the LCD panel
     setWord(&MSW,RUN,true);   //mark the program to start running

     //setWord(&MSW,BCK,true);

     setBacklight(true);
     lcd->clear();
     showGui();
//--------------------------------------------------------------------------------------------------
// Main program loop
//--------------------------------------------------------------------------------------------------

     while(getWord(MSW,RUN)==true){

//*--- Read and process events coming from the CAT subsystem
      (cat->active==true ? cat->get() : (void) _NOP);

//*--- Read and process events coming from the GPIO subsystem

      int gpio_read=gpio->readpipe(gpio_buffer,BUFSIZE);
          if (gpio_read>0) {
              gpio_buffer[gpio_read]=0x00;
             (TRACE>=0x02 ? fprintf(stderr,"%s",(char*)gpio_buffer) : _NOP);
          }

//*--- Read and process  events coming from the GUI interface

          processGui();
     }


  exit(0);
}


