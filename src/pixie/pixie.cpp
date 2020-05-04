
/*
 * pixie.c
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
#include "/home/pi/librpitx/src/librpitx.h"
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <wiringSerial.h>
#include <sstream>
#include <iomanip>
#include <assert.h>

//*---- Program specific includes

#include "pixie.h"
#include "../lib/ClassMenu.h"
#include "../lib/LCDLib.h"
#include "../lib/DDS.h"
#include "../minIni/minIni.h"
#include "../log.c/log.h"

#include "/home/pi/OrangeThunder/src/lib/VFOSystem.h"
#include "/home/pi/OrangeThunder/src/OT/OT.h"
#include "/home/pi/OrangeThunder/src/lib/CAT817.h"

#include <iostream>
#include <cstdlib>    // for std::rand() and std::srand()
#include <ctime>      // for std::time()

//*--- Define Initialization Values for CAT

byte FT817;
byte mode=MCW;
int  shift=VFO_SHIFT;
int  ritofs=0;
int  step=0;
byte ddspower=DDS_MAXLEVEL;
byte trace=0x00;
byte ptt=KEYER_OUT_GPIO;
byte txonly=ALWAYS;
int  keyer_brk=KEYER_BRK;
//*---- Keyer specific definitions
byte sidetone_gpio=SIDETONE_GPIO;

int i=0;
byte auxcnt=128;
extern "C" {
bool running=true;
bool ready=false;

#include "../iambic/iambic.c"

}

void changeFreq(float f);
void CATchangeMode();
void CATchangeFreq();
void CATchangeStatus();
void CATgetRX();
void CATgetTX();

//*----------------------------------------------------------------------------
//*  Program parameter definitions
//*----------------------------------------------------------------------------

const char   *PROGRAMID="PixiePi";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="45";
const char   *COPYRIGHT="(c) LU7DID 2020";

char *strsignal(int sig);
extern const char * const sys_siglist[];
//*-------------------------------------------------------------------------------------------------
//* Main structures
//*-------------------------------------------------------------------------------------------------
//*--- VFO object
VFOSystem vx(showFreq,NULL,NULL,NULL);

//*--- DDS object
DDS *dds=new DDS(changeFreq);

//*--- CAT object
CAT817 cat(CATchangeFreq,CATchangeStatus,CATchangeMode,CATgetRX,CATgetTX);

//*--- Strutctures to hold menu definitions

MenuClass menuRoot(NULL);

MenuClass mod(ModeUpdate);
MenuClass vfo(VfoUpdate);
MenuClass rit(RitUpdate);
MenuClass stp(StepUpdate);
MenuClass shf(ShiftUpdate);
MenuClass spl(SplitUpdate);
MenuClass kyr(KeyerUpdate);
MenuClass wtd(WatchDogUpdate);
MenuClass lck(LockUpdate);
MenuClass bck(BackLightUpdate);
MenuClass drv(DriverUpdate);
MenuClass spd(SpeedUpdate);

//*--- LCD management object

LCDLib lcd(NULL);
int lcd_light=LCD_ON;  // On

//*--- debouncing logic setup
auto startEncoder=std::chrono::system_clock::now();
auto endEncoder=std::chrono::system_clock::now();

auto startPush=std::chrono::system_clock::now();
auto endPush=std::chrono::system_clock::now();

auto startAux=std::chrono::system_clock::now();
auto endAux=std::chrono::system_clock::now();
 
//*--- LCD custom character definitions

byte top_line[8]    = {0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};
byte bottom_line[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};
byte both_lines[8]  = {0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};
byte dot[8]         = {0b00000, 0b00000, 0b00011, 0b00011, 0b00011, 0b00011, 0b00000, 0b00000};

byte C00[8] = {0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B00011} ;
byte C01[8] = {0B11111,
               0B00001,
               0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B11111} ;

byte C02[8] = {0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11111} ;
byte C03[8] = {0B11111,
               0B11000,
               0B11000,
               0B11000,
               0B11000,
               0B11000,
               0B11000,
               0B11111} ;
byte C04[8] = {0B11000,
               0B11000,
               0B11000,
               0B11000,
               0B11000,
               0B11000,
               0B11000,
               0B11111} ;
byte C05[8] = {0B11111,
               0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B00011,
               0B00011} ;
byte C06[8] = {0B11111,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11111} ;
byte C07[8] = {0B11111,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11011,
               0B11011} ;

byte TX[8] = {0B11111,0B10001,0B11011,0B11011,0B11011,0B11011,0B11111}; // Inverted T (Transmission Mode)
byte S1[8] = {0B10000,0B10000,0B10000,0B10000,0B10000,0B10000,0B10000}; // S1 Signal
byte S2[8] = {0B11000,0B11000,0B11000,0B11000,0B11000,0B11000,0B11000}; // S2 Signal
byte S3[8] = {0B11100,0B11100,0B11100,0B11100,0B11100,0B11100,0B11100}; // S3 Signal
byte S4[8] = {0B11110,0B11110,0B11110,0B11110,0B11110,0B11110,0B11110}; // S4 Signal
byte S5[8] = {0B11111,0B11111,0B11111,0B11111,0B11111,0B11111,0B11111}; // S5 Signal
byte B[8]  = {0b11110,0b10001,0b11110,0b10001,0b11110,0b00000,0b11111};
byte K[8]  = {31,17,27,27,27,17,31};    //Inverted K (Keyer)
byte S[8]  = {31,17,23,17,29,17,31};    //Inverted S (Split)
byte B1[8] = {24,24,24,24,24,24,24};    // -
byte B2[8] = {30,30,30,30,30,30,30};    // /
byte B3[8] = {31,31,31,31,31,31,31};    // |
byte B4[8] = {0b00000,0b10000,0b01000,0b00100,0b00010,0b00001,0b00000};
byte XLOCK[8] = {0b11111,0b10001,0b11111,0b00100,0b11100,0b00100,0b11100}; //VFOA

//*---- Generic memory allocations

int value=0;
int lastEncoded=0;
int counter=0;
int clkLastState=0; 
char hi[80];
byte memstatus=0;
int a;
int anyargs = 0;
float f=VFO_START;
float ppm=1000.0;
struct sigaction sa;
byte keepalive=0;
byte backlight=BACKLIGHT_DELAY;
char port[80];
long catbaud=CATBAUD;
byte gpio=GPIO04;
bool fSwap=false;

int maxrit=MAXRIT;
int minrit=MINRIT;
int ritstep=RITSTEP;
int ritstepd=RITSTEPD;

//*--- Keyer related memory definitions


char snd_dev[64]="hw:0";

//* --- Define minIni related parameters

char inifile[80];
char iniStr[100];
long nIni;
int  sIni,kIni;
char iniSection[50];

//*--- System Status Word initial definitions

byte MSW  = 0;
byte TSW  = 0;
byte USW  = 0;
byte JSW  = 0;
byte MCB =0;

byte LUSW = 0;
int  TVFO = 0;
int  TBCK = backlight;
int  TBRK = 0;
int  TWIFI = 10;

bool wlan0 = false;
void showPTT();

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                              ROUTINE STRUCTURE
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*------------------------------------------------------------------------------------------------------------
//* updatestep
//* update step for a given VFO
//*------------------------------------------------------------------------------------------------------------
void updatestep(byte v, int step) {

       switch(step) {
          case 0 : {vx.vfostep[v]=VFO_STEP_100Hz; break;}
          case 1 : {vx.vfostep[v]=VFO_STEP_500Hz; break;}
          case 2 : {vx.vfostep[v]=VFO_STEP_1KHz; break;}
          case 3 : {vx.vfostep[v]=VFO_STEP_5KHz; break;}
          case 4 : {vx.vfostep[v]=VFO_STEP_10KHz; break;}
          case 5 : {vx.vfostep[v]=VFO_STEP_50KHz; break;}
          case 6 : {vx.vfostep[v]=VFO_STEP_100KHz; break;}
       }
       return;
}

#include "../lib/gui.h"


//*--------------------------[minIni callback]---------------------------------------------------
//* call back to manage configuration data from .cfg file
//*--------------------------------------------------------------------------------------------------

int Callback(const char *section, const char *key, const char *value, void *userdata)
{
  (void)userdata;
  log_trace("    [%s]\t%s=%s", section, key, value);
  return 1;
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
//*--------------------------------------------------------------------------------------------
//* set_PTT
//* Manage the PTT of the transceiver (can be used from the keyer or elsewhere
//*--------------------------------------------------------------------------------------------
void setPTT(bool statePTT) {

    float df=0;
    float f=0;

    int i=0;

//*---------------------------------*
//*          PTT Activated          *
//*---------------------------------*
    if (statePTT==true) {

//*--- if SPLIT swap VFO AND if also CW shift the carrier by vfoshift[current VFO]

        

       if (getWord(FT817,SPLIT)==true  && fSwap==false) {
          fSwap=true;
          vx.vfoAB=(vx.vfoAB==VFOA ? VFOB : VFOA);
          showGUI();
       }

       if (mode==MCW) {
          df=(float)vx.vfoshift[vx.vfoAB];
       }

       if (mode==MCWR) {
          df=-(float)vx.vfoshift[vx.vfoAB];

       }

       f=(float)(vx.vfo[vx.vfoAB]+(mode==MCW || mode==MCWR ? df : 0));
       log_trace("setPTT(): PTT On setting DDS to f=%d DDS(%d) df=%d MODE=%d shift(%d) vfo(%d)",(int)f,(int)dds->f,(int)df,mode,vx.vfoshift[vx.vfoAB],(int)vx.vfo[vx.vfoAB]);

//*--- Now turn the transmitter at the frequency f based on the mode

       log_info("PTT(On)");
       setWord(&FT817,PTT,true);

       switch(mode) {
           case MCW:
           case MCWR: 
                   {
                     (txonly==1 ? dds->start(f) : dds->set(f)) ;
                     //softToneWrite (sidetone_gpio, cw_keyer_sidetone_frequency);

                     break;
                   }
           case MUSB:
           case MLSB:
                   {
                     log_info("PTT: Invalid mode");
                     break;
                   }
       }

       gpioWrite(ptt, PTT_ON);
       setWord(&FT817,PTT,true);
       return;
    } 

//*---------------------------------*
//*          PTT Inactivated        *
//*---------------------------------*


    log_trace("PTT <OFF>, Receiver mode");
    log_info("PTT(Off)");
    //softToneWrite (sidetone_gpio, 0);
    gpioWrite(ptt, PTT_OFF);

    if (getWord(FT817,SPLIT)==true && fSwap==true){
       (vx.vfoAB==VFOA ? vx.vfoAB=VFOB : vx.vfoAB = VFOA);
       showGUI();
    }

    fSwap=false;  
    f=vx.vfo[vx.vfoAB]+(rit.mItem!=0 ? ritofs : 0);
    log_trace("setPTT(): PTT Off setting DDS f=%d df=%d MODE=%d RIT(%d)",(int)f,(int)df,mode,(rit.mItem!=0 ? ritofs : 0));

    switch(mode) {

       case MCW:
       case MCWR:
            {
            if (keyer_brk==0) {
               (txonly==1 ? dds->stop() : dds->set((float)(vx.get(vx.vfoAB))));
               log_trace("Break-In Timer not set frequency");
            } else {
               TBRK=keyer_brk;
               log_trace("Break-In Timer set");
            }
            break;
            }
       case MUSB:
       case MLSB:
            {
            log_trace("PTT: Invalid mode");
            break;
            }
    }

    setWord(&FT817,PTT,false);
    return;

}
//*---------------------------------------------------------------------------------------------------
//* keyChangeEvent
//* callback from iambic to signal the key has been set up or down
//*---------------------------------------------------------------------------------------------------
void keyChangeEvent() {

     if (getWord(FT817,PTT) == false && getWord(MSW,CMD)==true) {
        if (key_state == SENDDASH) {
           setWord(&USW,BCW,true);
        }
        if (key_state == SENDDOT) {
           setWord(&USW,BCCW,true);
        }
        return;
     }
     if (keyState==KEY_DOWN) {
        setWord(&FT817,PTT,true);
        showSMeter((int)dds->power*2);
     } else {
        setWord(&FT817,PTT,false);
        showSMeter(0);
     }

     cat.FT817=FT817;
     log_trace("Keyer state(%d)",getWord(FT817,PTT));
     showPTT();

}
//*---------------------------------------------------------------------------
//*
//*---------------------------------------------------------------------------

void changeFreq(float f) {


}
//*---------------------------------------------------------------------------
//* CATchangeFreq()
//* CAT Callback when frequency changes
//*---------------------------------------------------------------------------
void CATchangeFreq() {

  //
  log_trace("CATchangeFreq(): cat.SetFrequency(%d) SetFrequency(%d)",(int)cat.SetFrequency,(int)f);

  f=cat.SetFrequency;
  log_info("CAT Frequency set to %10.0f",cat.SetFrequency);
  long int fx=(long int)f;
  log_trace("changeFreq: Frequency set to f(%d)",fx);
  dds->power=ddspower;
  cat.POWER=dds->power;
  if (vx.vfoAB==VFOA) {
     setWord(&FT817,VFO,false);
     setWord(&cat.FT817,VFO,false);
  } else {
     setWord(&FT817,VFO,true);
     setWord(&cat.FT817,VFO,true);
  }
  dds->set(f);
  vx.set(vx.vfoAB,fx);
  

}
//*-----------------------------------------------------------------------------------------------------------
//* CATchangeMode
//* Validate the new mode is a supported one
//* At this point only CW,CWR,USB and LSB are supported
//*-----------------------------------------------------------------------------------------------------------
void CATchangeMode() {

       log_trace("CATchangeMode(): cat.MODE(%d) MODE(%d)",cat.MODE,mode);
       log_info("CAT Change Mode()");
       if (cat.MODE != MUSB && cat.MODE != MLSB && cat.MODE != MCW && cat.MODE != MCWR) {
          mode=cat.MODE;
          log_info("CATchangeMode(): INVALID MODE");
          showMode();
          return;
       }

       cat.POWER=dds->power;

       mode=cat.MODE;
       mod.mItem=cat.MODE;
       ModeUpdate();
       return;

}

//*------------------------------------------------------------------------------------------------------------
//* CATchangeStatus
//* Detect which change has been produced and operate accordingly
//*------------------------------------------------------------------------------------------------------------
void CATchangeStatus() {
       //fprintf(stderr,"CATchangeStatus()\n");
       log_trace("CATchangeStatus():PTT(%d)",getWord(FT817,PTT));
//*---------------------
       cat.POWER=dds->power;

       if (getWord(cat.FT817,PTT) != getWord(FT817,PTT)) {        //* PTT Changed
          setWord(&FT817,PTT,getWord(cat.FT817,PTT));
          if (getWord(FT817,PTT)==true) {
             keyState=KEY_DOWN;
             showSMeter((int)2*dds->power & 0x0f);
             //fprintf(stderr,"CATchangeStatus() PTT(On)\n");
          } else {
             keyState=KEY_UP;
             showSMeter(0);
             //fprintf(stderr,"CATchangeStatus() PTT(Off)\n");
          }
          setPTT(getWord(FT817,PTT));
          showPTT();
       }

//*---------------------

       if (getWord(cat.FT817,RIT) != getWord(FT817,RIT)) {        //* RIT Changed
          log_info("CATchangeStatus():RIT");
          setWord(&FT817,RIT,getWord(cat.FT817,RIT));
          (getWord(FT817,RIT)==true ? rit.mItem=1 : rit.mItem=0);
          RitUpdate();
       }

       if (getWord(cat.FT817,LOCK) != getWord(FT817,LOCK)) {      //* LOCK Changed
          log_info("CATchangeStatus():LOCK");
          setWord(&FT817,LOCK,getWord(cat.FT817,LOCK));
       }

       if (getWord(cat.FT817,SPLIT) != getWord(FT817,SPLIT)) {    //* SPLIT mode Changed
          log_trace("CATchangeStatus():SPLIT SPLIT(%d) cat.SPLIT(%d)",getWord(FT817,SPLIT),getWord(cat.FT817,SPLIT));
          setWord(&FT817,SPLIT,getWord(cat.FT817,SPLIT));
          (getWord(FT817,SPLIT)==true ? spl.mItem = 1 : spl.mItem=0); 
          SplitUpdate();
       }

       if (getWord(cat.FT817,VFO) != getWord(FT817,VFO)) {        //* VFO Changed
         log_trace("CATchangeStatus():VFO Change VFO(%d) cat.VFO(%d)",getWord(FT817,VFO),getWord(cat.FT817,VFO));
          setWord(&FT817,VFO,getWord(cat.FT817,VFO));
          if(getWord(FT817,VFO)==false){
            vfo.mItem=1;
            vx.vfoAB=VFOB;
          } else {
            vfo.mItem=0;
            vx.vfoAB=VFOA;
          }
          VfoUpdate();
          log_info("VFO Changed now VFOAB(%d) f(%li)",vx.vfoAB,vx.get(vx.vfoAB)); 

       }

       vx.setVFOShift(VFOA,shift);
       vx.setVFOShift(VFOB,shift);
       return;

}
void CATgetRX() {
}
void CATgetTX() {
}

//*-------------------------------------------------------------------------------------------------
//* print_usage
//* help message at program startup
//*-------------------------------------------------------------------------------------------------
void print_usage(void)
{

fprintf(stderr,
"\npixie -%s\n"
"Usage:\npixie  [-f  float frequency carrier in Hz (50 KHz to 1500 MHz\n"
"               [-h] this help\n"
"               [-g  GPIO port to use for RF output(4 or 20)\n"
"               [-p] set clock ppm instead of ntp adjust\n"
"               [-a  GPIO active_state (0=LOW, 1=HIGH) default is 0]\n"
"               [-C  strict_char_spacing (0=off, 1=on)]\n"
"               [-D  sound device string (default is hw:0)]\n"
"               [-E  sidetone start/end ramp envelope in ms (default is 5)]\n"
"               [-F  sidetone_freq_hz]\n"
"               [-G  sidetone gain in dB]\n"
"               [-M  mode (0=straight or bug, 1=iambic_a, 2=iambic_b)]\n"
"               [-S  speed_wpm] [-w weight (33-66)]\n",PROGRAMID);

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
void timer_SMeter() {

    if (getWord(MSW,CMD)==true) {
       return;
    }


}
//*---------------------------------------------------------------------------
//* Timer callback function
//*---------------------------------------------------------------------------
void timer_exec()
{
  //std::cout << "I am doing something" << std::endl;

  if (TVFO>0){
     TVFO--;
     if (TVFO==0){
        setWord(&TSW,FTU,false);
        setWord(&TSW,FTD,false);
        setWord(&TSW,FVFO,true);
     }
  }

  TWIFI--;
  if (TWIFI==0){
     setWord(&USW,CONX,true);
  }

  if (TBCK>0){
     TBCK--;
     if (TBCK==0){
        if (backlight !=0) {
           setWord(&TSW,FBCK,true);
        }
     }
  }

  if (TBRK>0){
     TBRK--;
     if (TBRK==0){
       (txonly==1 ? dds->stop() : dds->set((float)(vx.get(vx.vfoAB))));
     }
   }
    
}
//*-------------------------------------------------------------------------------------------
//* aux_event
//* Detects whether the aux button has been activated
//*-------------------------------------------------------------------------------------------
void aux_event(int gpio, int level, uint32_t tick) {

        log_trace("Event AUX level(%d)",level);

        if (level != 0) {
           endAux = std::chrono::system_clock::now();
           int lapAux=std::chrono::duration_cast<std::chrono::milliseconds>(endAux - startAux).count();
           if (getWord(USW,BAUX)==true) {
              log_trace("Last aux pending processing, ignore!");
              return;
           }

           if (lapAux < MINSWPUSH) {
              log_trace("<AUX> pulse too short! ignored!");
           } else {
 	     setWord(&USW,BAUX,true);
             log_trace("AUX Event detected");   
             if (lapAux > MAXSWPUSH) {
                log_trace("Aux pulse really long, %d ms. ",lapAux);
             }
             return;
           }
           return;
        }
        startAux = std::chrono::system_clock::now();
        lcd.backlight(true);
        lcd.setCursor(0,0);
        TBCK=backlight;;

}

//*--------------------------[Rotary Encoder Interrupt Handler]--------------------------------------
//* Interrupt handler routine for Rotary Encoder Push button
//*--------------------------------------------------------------------------------------------------
void updateSW(int gpio, int level, uint32_t tick)
{

        if (level != 0) {
           endPush = std::chrono::system_clock::now();
           int lapPush=std::chrono::duration_cast<std::chrono::milliseconds>(endPush - startPush).count();
           if (getWord(USW,BMULTI)==true) {
              log_trace("Last push pending processsing, ignore!");
              return;
           }
           if (lapPush < MINSWPUSH) {
              log_trace("Push pulse too short! ignored!");
           } else {
             setWord(&USW,BMULTI,true);
             if (lapPush > MAXSWPUSH) {
                log_trace("Push pulse really long, %d ms. KDOWN signal!",lapPush);
                setWord(&USW,KDOWN,true);
             } else {
                log_trace("Push pulse brief, %d ms. KDOWN false!",lapPush);
                setWord(&USW,KDOWN,false);
             }
           return;
           }
        }
        startPush = std::chrono::system_clock::now();
        int pushSW=gpioRead(ENCODER_SW);
        lcd.backlight(true);
        TBCK=backlight;;
}
//*--------------------------[Rotary Encoder Interrupt Handler]--------------------------------------
//* Interrupt handler for Rotary Encoder CW and CCW control
//*--------------------------------------------------------------------------------------------------
void updateEncoders(int gpio, int level, uint32_t tick)
{
        if (level != 0) {  //ignore non falling part of the interruption
           return;
        }

        if (getWord(USW,BCW)==true || getWord(USW,BCCW) ==true) { //exit if pending to service a previous one

           return;
        }

        if (getWord(FT817,PTT)==true) {
           return;    // If transmitting disable encoder
        }  

        int clkState=gpioRead(ENCODER_CLK);
        int dtState= gpioRead(ENCODER_DT);

        TBCK=backlight;
        lcd.backlight(true);

        endEncoder = std::chrono::system_clock::now();
        int lapEncoder=std::chrono::duration_cast<std::chrono::milliseconds>(endEncoder - startEncoder).count();

        if ( lapEncoder  < MINENCLAP )  {
             log_trace("Encoder: ignore pulse too close from last");
             return;
        }

        if (dtState != clkState) {
          counter++;
          setWord(&USW,BCCW,true);
        } else {
          counter--;
          setWord(&USW,BCW,true);
        }

        clkLastState=clkState;        
        startEncoder = std::chrono::system_clock::now();

}

//*---------------------------------------------------------------------------------------------
//* Signal handlers, SIGALRM is used as a timer, all other signals means termination
//*---------------------------------------------------------------------------------------------
static void terminate(int num)
{
    log_warn("\n Received signal (%d %s)",num,strsignal(num));
    sem_post(&cw_event);
    running=false;
   
}
//*---------------------------------------------------
void sigalarm_handler(int sig)
{

   return;


}
//*-----------------------------------------------------------------------------
//* Execute a shell command
//*-----------------------------------------------------------------------------
string do_console_command_get_result (char* command)
{
        log_trace("Executing external command: %s",command);
	FILE* pipe = popen(command, "r");
    
	if (!pipe)
		return "ERROR";
	
	char buffer[128];
	string result = "";
	while(!feof(pipe))
	{
		if(fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
        log_trace("External command result: %s",result);
	return(result);
}
//*-----------------------------------------------------------------------------
//* Convert number to large font
//**-----------------------------------------------------------------------------
void display_LargeNumberA(byte number,int position) {

     switch(number) {
     case 1 : {
               lcd.setCursor(position,0);
               lcd.write(0);
               lcd.write(255);
               lcd.write(254);
               lcd.setCursor(position,1);
               lcd.write(1);
               lcd.write(255);
               lcd.write(1);
               break;
               }
     case 2 : {
               lcd.setCursor(position,0);
               lcd.write(2);
               lcd.write(2);
               lcd.write(255);
               lcd.setCursor(position,1);
               lcd.write(255);
               lcd.write(1);
               lcd.write(1);
               break;
               }
     case 3 : {
               lcd.setCursor(position,0);
               lcd.write(0);
               lcd.write(2);
               lcd.write(255);
               lcd.setCursor(position,1);
               lcd.write(1);
               lcd.write(1);
               lcd.write(255);
               break;
               }
     case 4 : {
               lcd.setCursor(position,0);
               lcd.write(255);
               lcd.write(1);
               lcd.write(255);
               lcd.setCursor(position,1);
               lcd.write(254);
               lcd.write(254);
               lcd.write(255);
               break;
               }
     case 5 : {
               lcd.setCursor(position,0);
               lcd.write(255);
               lcd.write(2);
               lcd.write(2);
               lcd.setCursor(position,1);
               lcd.write(1);
               lcd.write(1);
               lcd.write(255);
               break;
               }
     case 6 : {
               lcd.setCursor(position,0);
               lcd.write(255);
               lcd.write(2);
               lcd.write(2);
               lcd.setCursor(position,1);
               lcd.write(255);
               lcd.write(1);
               lcd.write(255);
               break;
               }
     case 7 : {
               lcd.setCursor(position,0);
               lcd.write(0);
               lcd.write(0);
               lcd.write(255);
               lcd.setCursor(position,1);
               lcd.write(254);
               lcd.write(254);
               lcd.write(255);
               break;
               }
     case 8 : {
               lcd.setCursor(position,0);
               lcd.write(255);
               lcd.write(2);
               lcd.write(255);
               lcd.setCursor(position,1);
               lcd.write(255);
               lcd.write(1);
               lcd.write(255);
               break;
               }
     case 9 : {
               lcd.setCursor(position,0);
               lcd.write(255);
               lcd.write(2);
               lcd.write(255);
               lcd.setCursor(position,1);
               lcd.write(1);
               lcd.write(1);
               lcd.write(255);
               break;
               }
     case 0 : {
               lcd.setCursor(position,0);
               lcd.write(255);
               lcd.write(0);
               lcd.write(255);
               lcd.setCursor(position,1);
               lcd.write(255);
               lcd.write(1);
               lcd.write(255);
               break;
               }
     default : {
               lcd.setCursor(position,0);
               lcd.write(0);
               lcd.write(255);
               lcd.write(254);
               lcd.setCursor(position,1);
               lcd.write(1);
               lcd.write(255);
               lcd.write(1);
               break;
               }
        

     }
}
void display_LargeNumber(byte number,int position) {

        switch(number)  {
           case 1 : {
                     lcd.setCursor(position,0);
                     lcd.write(0);
                     lcd.setCursor(position,1);
                     lcd.write(0);
		     break;
                    }
           case 2 : {
                     lcd.setCursor(position,0);
                     lcd.write(1);
                     lcd.setCursor(position,1);
                     lcd.write(3);
		     break;

                    }
           case 3 : {
                     lcd.setCursor(position,0);
                     lcd.write(1);
                     lcd.setCursor(position,1);
                     lcd.write(1);
		     break;

                    }
           case 4 : {
                     lcd.setCursor(position,0);
                     lcd.write(2);
                     lcd.setCursor(position,1);
                     lcd.write(5);
		     break;

                    }
           case 5 : {
                     lcd.setCursor(position,0);
                     lcd.write(3);
                     lcd.setCursor(position,1);
                     lcd.write(1);
		     break;

                    }
           case 6 : {
                     lcd.setCursor(position,0);
                     lcd.write(4);
                     lcd.setCursor(position,1);
                     lcd.write(6);
		     break;

                    }
           case 7 : {
                     lcd.setCursor(position,0);
                     lcd.write(5);
                     lcd.setCursor(position,1);
                     lcd.write(0);
		     break;

                    }
           case 8 : {
                     lcd.setCursor(position,0);
                     lcd.write(6);
                     lcd.setCursor(position,1);
                     lcd.write(6);
		     break;

                    }
           case 9 : {
                     lcd.setCursor(position,0);
                     lcd.write(6);
                     lcd.setCursor(position,1);
                     lcd.write(5);
		     break;

                    }
           case 0 : {
                     lcd.setCursor(position,0);
                     lcd.write(7);
                     lcd.setCursor(position,1);
                     lcd.write(2);
		     break;

                    }
           default :     {
                     lcd.setCursor(position,7);
                     lcd.write(7);
                     lcd.setCursor(position,2);
                     lcd.write(7);
		     break;

                    }
  

        }

}
//*--------------------------------------------------------------------------------------------
//* showFreq
//* manage the presentation of frequency to the LCD display
//*--------------------------------------------------------------------------------------------
void showFreq() {

  FSTR v;  
  long int f=vx.get(vx.vfoAB); 
  vx.computeVFO(f,&v);

  if (v.millions >= 10) {
     byte m=v.millions/10;
     display_LargeNumber(m,0);

  } else {
     lcd.setCursor(0,0);
     lcd.print(" ");
     lcd.setCursor(0,1);
     lcd.print(" ");
  }

  display_LargeNumber(v.millions,1);
  display_LargeNumber(v.hundredthousands,2);
  display_LargeNumber(v.tenthousands,3);
  display_LargeNumber(v.thousands,4);

  sprintf(hi,"%d%d",v.hundreds,v.tens);
  lcd.setCursor(5,0);
  lcd.print(string(hi));

  if (getWord(TSW,FTU)==true) {
     lcd.setCursor(5,1);
     lcd.typeChar((char)126);
  } 
  if (getWord(TSW,FTD)==true) {
     lcd.setCursor(5,1);
     lcd.typeChar((char)127);
  }

  if (getWord(TSW,FVFO)==true) {
     lcd.setCursor(5,1);
     lcd.typeChar((char)0x20);
     setWord(&TSW,FVFO,false);
  }

  if (ready==true) {
     signal(SIGALRM, &sigalarm_handler);  // set a signal handler
     alarm(TWOSECS);  // set an alarm for 2 seconds from now
  }
  memstatus = 0; // Trigger memory write

}

//*----------------------------------------------------------------------------------------------------
//* processVFO
//* identify CW or CCW movements of the rotary encoder and changes the frequency accordingly
//*----------------------------------------------------------------------------------------------------
void processVFO() {

//*--- CW (frequency increase)

   if (getWord(FT817,PTT)==true) {
      setWord(&USW,BCW,false);
      setWord(&USW,BCCW,false);
      return;
   }


   if (getWord(USW,BCW)==true) {

      if (rit.mItem != 0) {
         ritofs=ritofs+(ritofs<maxrit ? ritstep : 0);
         showRit();
         setWord(&JSW,XVFO,true);
      } else { 
         (vx.isVFOLocked()==false ? vx.updateVFO(vx.vfoAB,vx.vfostep[vx.vfoAB]):vx.updateVFO(vx.vfoAB,0));
         setWord(&JSW,XVFO,true);
         setWord(&TSW,FTU,true);
         TVFO=ONESECS;
         showFreq();
      }
      setWord(&USW,BCW,false);
      setWord(&TSW,FTD,false);
      setWord(&TSW,FVFO,false);
   }

//*--- CCW frequency decrease

   if (getWord(USW,BCCW)==true) {

      if (rit.mItem != 0) {
         ritofs=ritofs+(ritofs>minrit ? ritstepd : 0);
         setWord(&JSW,XVFO,true);
         showRit();
      } else { 
         (vx.isVFOLocked()==false ? vx.updateVFO(vx.vfoAB,-vx.vfostep[vx.vfoAB]):vx.updateVFO(vx.vfoAB,vx.vfostep[vx.vfoAB]));
         TVFO=ONESECS;
         setWord(&TSW,FTD,true);
         setWord(&JSW,XVFO,true);
         showFreq();
      }
      setWord(&USW,BCCW,false);
      setWord(&TSW,FTU,false);
      setWord(&TSW,FVFO,false);
   }

}
//*--------------------------------------------------------------------------------------------------
//* main execution of the program
//*--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{

//*--- Initial presentation

    fprintf(stderr,"%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);
    dbg_setlevel(1);
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // set initial seed value to system clock

    log_set_quiet(0);
    log_set_level(LOG_INFO);


//*--- Process arguments (mostly an excerpt from tune.cpp)

       while(true)
        {
                a = getopt(argc, argv, "f:eds:hg:i:p:A:C:D:E:F:G:M:S:");

                if(a == -1) 
                {
                        if(anyargs) {
                           break; 
                        } else {
                           a='h'; 
                        }
                }
                anyargs = 1;    

                switch(a)
                {

                case 'A': 
                        cw_active_state = atoi(optarg);
                        log_info(" CW Active State A: %d",cw_active_state);
                        break;
                case 'i':
                        strcpy(inifile,optarg);
                        log_info("INI file: %s",inifile);
                        break;
                case 'C':
                        cw_keyer_spacing = atoi(optarg);
                        log_info(" CW Keyer Space  C: %d",cw_keyer_spacing);
                        break;
                case 'D':
                        strcpy(snd_dev, optarg);
                        log_info(" SOUNDCARD HW    D:%s",snd_dev);
                        break;
                case 'E': 
                        cw_keyer_sidetone_envelope = atoi(optarg);
                        log_info(" CW Tone Envelop E: %d", cw_keyer_sidetone_envelope);
                        break;

                case 'F':
                        cw_keyer_sidetone_frequency = atoi(optarg);
                        log_info(" CW Tone Freq    F: %d",cw_keyer_sidetone_frequency);
                        break;
                case 'G':     // gain in dB 
                        cw_keyer_sidetone_gain = atoi(optarg);
                        log_info(" CW Tone Gain    G: %d",cw_keyer_sidetone_gain);
                        break;
                case 'M':
                        cw_keyer_mode = atoi(optarg);
                        log_info(" CW Keyer Mode   M: %d",cw_keyer_mode);
                        break;
                case 'S':
                        cw_keyer_speed = atoi(optarg);
                        log_info(" CW Keyer Speed  S: %d",cw_keyer_speed);
                        break;
                case 'W':
                        cw_keyer_weight = atoi(optarg);
                        log_info(" CW Keyer Weight W: %d",cw_keyer_weight);
                        break;
                case 'f': // Frequency
                        f = atof(optarg);
                        break;
		case 'g': // GPIO
		        gpio=atoi(optarg);
			if (gpio!=GPIO04 && gpio!=GPIO20) {
		  	   log_info(" Invalid GPIO pin used (%s), default to GPIO04",optarg);
			   gpio=GPIO04;
			}
			break;
                case 'p': //ppm
                        ppm=atof(optarg);
                        log_info(" PPM correction    p:%d",(int)ppm);
                        break;
                case 'd': //debug
                        trace=0x02;
                        //cat.TRACE=LOG_TRACE;
			cat.TRACE=0x02;
                        log_set_quiet(0);
                        log_set_level(LOG_TRACE);
                        log_info(" Trace debug mode active");
                        break;
                case 'h': // help
                        print_usage();
                        exit(1);
                        break;
                case 's': //serial port
                        sprintf(port,optarg);
                        log_info(" serial port: %s", port);
                        break;
                case -1:
                break;
                case '?':
                        if (isprint(optopt) ) {
                           log_info("pixie: unknown option `-%c'.", optopt);
                        } else                         {
                           log_info("pixie: unknown option character `\\x%x'.", optopt);
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


//*--- Get Basic configuration data (INIMARKER)

   trace=ini_getl("MISC","TRACE",2,inifile);
   log_info("pixie: Trace level set to %d",trace);   
   switch(trace) {

     case 0x00  : {log_set_level(LOG_FATAL); cat.TRACE=0x00; break; }
     case 0x01  : {log_set_level(LOG_ERROR); cat.TRACE=0x01; break; }
     case 0x02  : {log_set_level(LOG_WARN);  cat.TRACE=0x02; break; }
     case 0x03  : {log_set_level(LOG_INFO);  cat.TRACE=0x02; break; }
     case 0x04  : {log_set_level(LOG_DEBUG); cat.TRACE=0x02; break; }
     case 0x05  : {log_set_level(LOG_TRACE); cat.TRACE=0x02; break; }
     default    : {log_set_level(LOG_INFO);  trace=2; cat.TRACE=trace; break; }
  }

  cat.TRACE=0x02; // REMOVER


//*--- Configuration: VFO

   (ini_getl("VFO","AB",VFOA,inifile)==VFOA ? f=ini_getl("VFO","VFOA",VFO_START,inifile)*1.0 : f=ini_getl("VFO","VFOB",VFO_START,inifile)*1.0);  

   mode=ini_getl("VFO","MODE",MCW,inifile);

   step=ini_getl("VFO","VFO_STEP",VFO_STEP_100Hz,inifile);
   updatestep(VFOA,step);
   updatestep(VFOB,step);

   shift=ini_getl("VFO","VFO_SHIFT",VFO_SHIFT,inifile);

   setWord(&FT817,SPLIT,ini_getl("VFO","SPLIT",0,inifile));
   setWord(&FT817,RIT,ini_getl("VFO","RIT",0,inifile));
   setWord(&FT817,LOCK,ini_getl("VFO","LOCK",0,inifile));
   //setWord(&FT817,TXONLY,ini_getl("VFO","TXONLY",0,inifile));
   log_fatal("Transceiver configuration");
   log_fatal("VFO A/B(%d) Mode(%d) Shift(%d) Step(%d) f(%10.0f) Split(%d) RIT(%d) Lock(%d) TxOnly(%d)",ini_getl("VFO","AB",VFOA,inifile),mode,shift,step,f,ini_getl("VFO","SPLIT",0,inifile),ini_getl("VFO","RIT",0,inifile),ini_getl("VFO","LOCK",0,inifile),ini_getl("VFO","TXONLY",0,inifile));

   maxrit=ini_getl("VFO","MAXRIT",MAXRIT,inifile);
   minrit=ini_getl("VFO","MINRIT",MINRIT,inifile);
   ritstep=ini_getl("VFO","RITSTEP",RITSTEP,inifile),
   ritstepd=ini_getl("VFO","RITSTEPD",RITSTEPD,inifile);
   log_fatal("Transceiver status configuration");
   log_fatal("VFO MaxRIT(%d) MinRIT(%d) RIT+(%d) RIT-(%d)",maxrit,minrit,ritstep,ritstepd);

//*--- Configuration: MISC

   backlight=ini_getl("MISC","BACKLIGHT",BACKLIGHT_DELAY,inifile);
   log_fatal("Transceiver misc. configuration");
   log_fatal("Misc Trace(%d) Backlight Timeout(%d)",trace,backlight);
   TBCK=backlight;

//*---- Configuration: DDS 
   gpio=ini_getl("DDS","GPIO",GPIO04,inifile);
   ptt=ini_getl("DDS","PTT",KEYER_OUT_GPIO,inifile);
   //txonly=ini_getl("DDS","TXONLY",0,inifile);
   txonly=0;
   ddspower=ini_getl("DDS","MAXLEVEL",DDS_MAXLEVEL,inifile);
   log_fatal("DDS configuration");
   log_fatal("DDS f(%10.0f) GPIO(%d) POWER(%d) PTT(%d) TxOnly(%d)",f,gpio,ddspower,ptt,txonly);


//*--- Configuration: CAT
   nIni=ini_gets("CAT", "PORT", "/tmp/ttyv1", port, sizearray(port), inifile);
   catbaud=ini_getl("CAT","BAUD",CATBAUD,inifile);
   //cat.TRACE=trace;

   log_fatal("CAT sub-system configuration");
   log_fatal("CAT Port(%s) at baud(%d) trace(%d)",port,catbaud,cat.TRACE);


//*--- Configuration: KEYER

   cw_keyer_mode=ini_getl("KEYER","KEYER_MODE",KEYER_STRAIGHT,inifile);
   cw_keyer_speed=ini_getl("KEYER","KEYER_SPEED",KEYER_SPEED,inifile);
   cw_keyer_sidetone_frequency=ini_getl("KEYER","KEYER_SIDETONE_FREQUENCY",KEYER_SIDETONE_FREQUENCY,inifile);
   cw_keyer_sidetone_gain=ini_getl("KEYER","KEYER_SIDETONE_GAIN",KEYER_SIDETONE_GAIN,inifile);
   cw_keyer_sidetone_envelope=ini_getl("KEYER","KEYER_SIDETONE_ENVELOPE",KEYER_SIDETONE_ENVELOPE,inifile);
   cw_keyer_spacing=ini_getl("KEYER","KEYER_SPACING",KEYER_SPACING,inifile);
   cw_active_state=ini_getl("KEYER","KEYER_ACTIVE",KEYER_LOW,inifile);
   sidetone_gpio=ini_getl("KEYER","KEYER_SIDETONE_GPIO",SIDETONE_GPIO,inifile);
   nIni=ini_gets("KEYER","SND_DEV","hw:0",snd_dev,sizearray(snd_dev),inifile);
   keyer_brk=ini_getl("KEYER","KEYER_BRK",KEYER_BRK,inifile);
   log_fatal("Keyer configuration");
   log_fatal("Keyer mode(%d) speed(%d) freq(%d) gain(%d) envelope(%d) spacing(%d) GPIO(%d) SoundHW(%s) Break(%d)",cw_keyer_mode,cw_keyer_speed,cw_keyer_sidetone_frequency,cw_keyer_sidetone_gain,cw_keyer_sidetone_envelope,cw_keyer_spacing,sidetone_gpio,snd_dev,keyer_brk);


   //cat.TRACE=trace;
   //cat.TRACE=0x02;  //REMOVER

//*---- Establish initial values of system variables

    setWord(&MSW,CMD,false);
    setWord(&MSW,GUI,false);
    setWord(&MSW,PTT,true);
    setWord(&MSW,DRF,false);
    setWord(&MSW,DOG,false);
    setWord(&MSW,LCLK,false);
    setWord(&MSW,SQL,false);

    setWord(&USW,BBOOT,true);
    setWord(&USW,BMULTI,false);
    setWord(&USW,BCW,false);
    setWord(&USW,BCCW,false);
    setWord(&USW,SQ,false);
    setWord(&USW,MIC,false);
    setWord(&USW,KDOWN,false);
  
    setWord(&JSW,JLEFT,false);
    setWord(&JSW,JRIGHT,false);
    setWord(&JSW,JUP,false);
    setWord(&JSW,JDOWN,false);
    setWord(&JSW,XVFO,false);

    showSMeter(0);

//*--- Setup LCD menues for MENU mode (to be shown when CMD=true), a MenuClass object needs to be created first for each

    menuRoot.add((char*)"Mode",&mod);
    menuRoot.add((char*)"VFO",&vfo);
    menuRoot.add((char*)"RIT",&rit);
    menuRoot.add((char*)"Split",&spl);
    menuRoot.add((char*)"Step",&stp);
    menuRoot.add((char*)"Shift",&shf);
    menuRoot.add((char*)"Keyer",&kyr);
    menuRoot.add((char*)"WatchDog",&wtd);
    menuRoot.add((char*)"BackLight",&bck);
    menuRoot.add((char*)"Lock",&lck);
    menuRoot.add((char*)"Speed",&spd);
    menuRoot.add((char*)"Power",&drv);


//*--- Setup child LCD menues

    vfo.add((char*)" A",NULL);
    vfo.add((char*)" B",NULL);
    vfo.set(0);

    spl.add((char*)" Off",NULL);
    spl.add((char*)" On ",NULL);
    spl.set(0);

    rit.add((char*)" Off",NULL);
    rit.add((char*)" On ",NULL);
    rit.set(0);

    stp.add((char*)" 100 Hz",NULL);
    stp.set(0);

    shf.add((char*)" ",NULL);
    shf.set(2);
    shf.refresh();

    kyr.add((char*)" Straight",NULL);
    kyr.add((char*)" Iambic A",NULL);
    kyr.add((char*)" Iambic B",NULL);
    kyr.set(1);

    wtd.add((char*)"Off",NULL);  
    wtd.add((char*)"On ",NULL);
    wtd.set(0);

    sprintf(gui," %d secs",backlight);
    bck.add((char*)gui,NULL);  
    bck.set(backlight);


    mod.add((char*)"CW ",NULL);
    mod.set(mode);
    mod.refresh();

    lck.add((char*)"Off",NULL);  
    lck.set(0);

    drv.add((char*)"  Max",NULL);
    drv.set(7);
    drv.refresh();

    sprintf(gui," %d wpm",cw_keyer_speed);
    spd.add((char*)gui,NULL);
    spd.set(cw_keyer_speed);
    spd.refresh();


//*---- Initialize GPIO

    if(gpioInitialise()<0) {
        log_fatal("Cannot initialize GPIO");
        return -1;
    }


//*---- Turn cooler on

    gpioSetMode(GPIO_COOLER, PI_OUTPUT);
    gpioWrite(GPIO_COOLER, 1);
    usleep(100000);
 
//*---- Manage AUX key

    gpioSetMode(AUX_GPIO, PI_INPUT);
    gpioSetPullUpDown(AUX_GPIO,PI_PUD_UP);
    gpioSetAlertFunc(AUX_GPIO,aux_event);
    usleep(100000);

//*---- Configure Encoder
    gpioSetMode(ENCODER_SW, PI_INPUT);
    gpioSetPullUpDown(ENCODER_SW,PI_PUD_UP);
    gpioSetAlertFunc(ENCODER_SW,updateSW);
    usleep(100000);

    gpioSetMode(ENCODER_CLK, PI_INPUT);
    gpioSetPullUpDown(ENCODER_CLK,PI_PUD_UP);
    usleep(100000);

    gpioSetISRFunc(ENCODER_CLK, FALLING_EDGE,0,updateEncoders);
    gpioSetMode(ENCODER_DT, PI_INPUT);
    gpioSetPullUpDown(ENCODER_DT,PI_PUD_UP);
    usleep(100000);


    counter = 0;

//*--- Initialize LCD display

    lcd.begin(16,2);
    lcd.clear();


    lcd.createChar(0,TX);
    lcd.createChar(1,S1);
    lcd.createChar(2,S2);
    lcd.createChar(3,S3);
    lcd.createChar(4,S4);
    lcd.createChar(5,S5);

    lcd.createChar(6,B2);   // Potentially
    lcd.createChar(7,B4);   // Erasable

//*---- TEST bit letters


    lcd.createChar(0,C00);
    lcd.createChar(1,C01);
    lcd.createChar(2,C02);
    lcd.createChar(3,C03);
    lcd.createChar(4,C04);
    lcd.createChar(5,C05);
    lcd.createChar(6,C06);
    lcd.createChar(7,C07);


    lcd.backlight(true);

//*--- Show banner briefly (1 sec)

    lcd.lcdLoc(LINE1);
    sprintf(hi,"%s %s [%s]",PROGRAMID,PROG_VERSION,PROG_BUILD);
    lcd.print(string(hi));
    lcd.lcdLoc(LINE2);
    lcd.print(string(COPYRIGHT));
    delay(ONESECS);

    if (wiringPiSetup () < 0) {
        log_fatal("Unable to setup wiringPi: %s", strerror (errno));
        return 1;
    }

//*---- Define the VFO System parameters (Initial Firmware conditions)

  vx.setVFOdds(setDDSFreq);
  vx.setVFOBand(VFOA,ini_getl("VFO","VFO_BAND_START",VFO_BAND_START,inifile));
  vx.set(VFOA,ini_getl("VFO","VFOA",VFO_START,inifile));
  vx.setVFOLimit(VFOA,ini_getl("VFO","VFO_START",VFO_START,inifile),ini_getl("VFO","VFO_END",VFO_END,inifile));
  vx.setVFOShift(VFOA,ini_getl("VFO","VFO_SHIFT",VFO_SHIFT,inifile));
  vx.setVFOBand(VFOB,ini_getl("VFO","VFO_BAND_START",VFO_BAND_START,inifile));
  vx.set(VFOB,ini_getl("VFO","VFOB",VFO_START,inifile));
  vx.setVFOLimit(VFOB,ini_getl("VFO","VFO_START",VFO_START,inifile),ini_getl("VFO","VFO_END",VFO_END,inifile));
  vx.setVFOShift(VFOB,ini_getl("VFO","VFO_SHIFT",VFO_SHIFT,inifile));

  vx.setVFO(ini_getl("VFO","AB",VFOA,inifile));


//*--- After the initializacion clear the LCD and show panel in VFOMode

    lcd.clear();

//*---  Define the handler for the SIGALRM signal (timer)

    signal(SIGALRM, &sigalarm_handler);  // set a signal handler
    timer_start(timer_exec,MSEC100);

//*--- Define the rest of the signal handlers, basically as termination exceptions

    for (int i = 0; i < 64; i++) {

        if (i != SIGALRM && i != 17 && i != 28) {
           signal(i,terminate);
        }
    }

//*--- Initialize CAT


    //cat.TRACE=trace;
    cat.open(port,catbaud);
    cat.SetFrequency=f;

    setWord(&FT817,PTT,false);

    cat.FT817=FT817;
    cat.MODE=mode;

//*--- Generate DDS (code excerpt mainly from tune.cpp by Evariste Courjaud F5OEO

    dds->gpio=byte(gpio);
    dds->power=byte(ddspower);
    (txonly==0 ? dds->start(ini_getl("VFO","VFOA",VFO_START,inifile)) : void(_NOP));
    cat.POWER=dds->power;

//*---

    keyChange=keyChangeEvent;
    iambic_init();

    alarm(ONESECS);  // set an alarm for 1 seconds from now to clear all values
    showPanel();
    showFreq();


    vfo.refresh();
    spl.refresh();
    rit.refresh();
    stp.refresh();
    kyr.refresh();
    wtd.refresh();
    bck.refresh();
    mod.refresh();
    lck.refresh();
    drv.refresh();
    spd.refresh();


//*----------------------------------------------------------------------------
//*--- Firmware initialization completed
//*----------------------------------------------------------------------------

//*--- Execute an endless loop while running is true

    ready=true;     //Initialization completed now enable loop runnint
    while(running)
      {
         usleep(100000);
         cat.get();

//*--- if in COMMAND MODE (VFO) any change in frequency is detected and transferred to the PLL

         if (getWord(MSW,CMD)==false && getWord(FT817,PTT)==false) {
             float freq=f;
             processVFO();
             if (getWord(JSW,XVFO)==true) {
                setWord(&JSW,XVFO,false);
                (rit.mItem!=0 ? f = (float) (vx.get(vx.vfoAB)+ritofs) : freq=(float) vx.get(vx.vfoAB));
                if (freq != dds->f) {
                   f=freq;
                   cat.SetFrequency=f;
                   dds->set(f);
                }
             }
         } else {
         }
         CMD_FSM();
         if (TBCK!=0 && backlight !=0 && getWord(MSW,CMD)==false) {
            lcd.backlight(true);
         }
//*--- Clear the frequency moved marker from the display once the delay expires

         if (getWord(TSW,FBCK)==true && backlight != 0 && getWord(MSW,CMD)==false) {
            setWord(&TSW,FBCK,false);
	    lcd.backlight(false);
            showSMeter(0);
         }
      }
//*-------------------------------------------------------------------------------------------
//*--- running become false, the program is terminating
//*-------------------------------------------------------------------------------------------
//*--- Stop keyer thread

    log_info("Closing keyer");
    iambic_close();

//*--- Stop the DDS function

    fprintf(stderr,"Closing DDS txonly(%d)\n",txonly);
    log_info("Closing DDS");

    (txonly==0 ? dds->stop() : void(_NOP));
    delete(dds);

//*---- Saving configuration
    fprintf(stderr,"Saving configuration\n");

    log_info("Saving configuration");

    sprintf(iniStr,"%d",mode);
    nIni=ini_puts("VFO","MODE",iniStr,inifile);

    sprintf(iniStr,"%li",vx.vfostep[vx.vfoAB]);
    nIni=ini_puts("VFO","VFO_STEP",iniStr,inifile);

    sprintf(iniStr,"%d",shift);
    nIni=ini_puts("VFO","VFO_SHIFT",iniStr,inifile);

    sprintf(iniStr,"%ld",vx.get(VFOA));
    nIni = ini_puts("VFO","VFOA",iniStr, inifile);

    sprintf(iniStr,"%ld",vx.get(VFOB));
    nIni = ini_puts("VFO", "VFOB",iniStr, inifile);

    sprintf(iniStr,"%d",vx.vfoAB);
    nIni = ini_puts("VFO", "AB",iniStr,inifile);

    sprintf(iniStr,"%d",mode);
    nIni = ini_puts("VFO", "MODE",iniStr,inifile);

    sprintf(iniStr,"%d",getWord(FT817,RIT));
    nIni = ini_puts("VFO", "RIT",iniStr,inifile);

    sprintf(iniStr,"%d",getWord(FT817,LOCK));
    nIni = ini_puts("VFO", "LOCK",iniStr,inifile);

    sprintf(iniStr,"%d",getWord(FT817,SPLIT));
    nIni = ini_puts("VFO", "SPLIT",iniStr,inifile);

    sprintf(iniStr,"%d",cw_keyer_mode);
    nIni = ini_puts("KEYER","KEYER_MODE",iniStr,inifile);

    sprintf(iniStr,"%d",backlight);
    nIni = ini_puts("MISC","BACKLIGHT",iniStr,inifile);

//*---- Configuration: DDS 
    sprintf(iniStr,"%d",gpio);
    nIni = ini_puts("DDS","GPIO",iniStr,inifile);
    sprintf(iniStr,"%d",ptt);
    nIni = ini_puts("DDS","PTT",iniStr,inifile);
    sprintf(iniStr,"%d",txonly);
    nIni = ini_puts("DDS","TXONLY",iniStr,inifile);
    sprintf(iniStr,"%d",ddspower);
    nIni = ini_puts("DDS","MAXLEVEL",iniStr,inifile);
    sprintf(iniStr,"%s",port);
    nIni = ini_puts("CAT","PORT",iniStr,inifile);
    sprintf(iniStr,"%li",catbaud);
    nIni = ini_puts("CAT","BAUD",iniStr,inifile);

    sprintf(iniStr,"%d",cw_keyer_speed);
    nIni = ini_puts("KEYER","KEYER_SPEED",iniStr,inifile);

   cw_keyer_mode=ini_getl("KEYER","KEYER_MODE",KEYER_STRAIGHT,inifile);

    sprintf(iniStr,"%d",cw_keyer_mode);
    nIni = ini_puts("KEYER","KEYER_MODE",iniStr,inifile);
    sprintf(iniStr,"%d",cw_keyer_spacing);
    nIni = ini_puts("KEYER","KEYER_SPACING",iniStr,inifile);
    sprintf(iniStr,"%d",cw_active_state);
    nIni = ini_puts("KEYER","KEYER_ACTIVE",iniStr,inifile);
    sprintf(iniStr,"%s",snd_dev);
    nIni = ini_puts("KEYER","SND_DEV",iniStr,inifile);
    sprintf(iniStr,"%d",keyer_brk);
    nIni = ini_puts("KEYER","KEYER_BRK",iniStr,inifile);

    sprintf(iniStr,"%d",backlight);
    nIni = ini_puts("MISC","BACKLIGHT",iniStr,inifile);

//*---- Turn cooler off 

    gpioWrite(GPIO_COOLER, 0);


//*--- turn LCD off

    fprintf(stderr,"main(): Turn off LCD light\n");
    lcd.backlight(false);
    lcd.clear();

    fprintf(stderr,"\nProgram terminated....\n");
    exit(0);
}


