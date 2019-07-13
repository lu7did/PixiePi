/*
 * pixie.c
 * Raspberry Pi based transceiver controller
 *---------------------------------------------------------------------
 * This program turns the Raspberry pi into a DDS software able
 * to operate as the LO for a double conversion rig, in this case
 * the popular Pixie setup, but can be extended to any other suitable
 * scheme
 *---------------------------------------------------------------------
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *    Adafruit's python code for CharLCDPlate 
 *    tune.cpp from rpitx package by Evariste Courjaud F5OEO
 *    wiringPi library (git clone git://git.drogon.net/wiringPi)
 *    iambic-keyer (https://github.com/n1gp/iambic-keyer)
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

//*---- Program specific includes

#include "pixie.h"
#include "../lib/ClassMenu.h"
#include "../lib/VFOSystem.h"
#include "../lib/LCDLib.h"
#include "../lib/CAT817.h"
#include "../lib/DDS.h"

#include <iostream>
#include <cstdlib> // for std::rand() and std::srand()
#include <ctime> // for std::time()

//*--- Define Initialization Values for CAT

byte FT817;
byte MODE=MCW;
int  SHIFT=600;
int  RITOFS=0;
int  STEP=0;
byte DDSPOWER=7;
//*---- Keyer specific definitions

int i=0;
extern "C" {
bool running=true;


#include "../iambic/iambic.c"

}

//*--- Encoder pin definition

#define ENCODER_CLK 17
#define ENCODER_DT  18
#define ENCODER_SW  27

//*---  VFO initial setup

#define VFO_START 	  7000000
#define VFO_END           7299000
#define VFO_BAND_START          3
#define ONESEC               1000


#define VFO_DELAY               1
#define BACKLIGHT_DELAY        60

#define GPIO04 			4
#define GPIO20	   	       20
//*----------------------------------------------------------------------------------
//*  System Status Word
//*----------------------------------------------------------------------------------
//*--- Master System Word (MSW)

#define CMD       0B00000001
#define GUI       0B00000010
#define PTT       0B00000100
#define DRF       0B00001000
#define DOG       0B00010000
#define LCLK      0B00100000
#define SQL       0B01000000
#define BCK       0B10000000

//*----- Master Timer and Event flagging (TSW)

#define FT1       0B00000001
#define FT2       0B00000010
#define FT3       0B00000100
#define FTU       0B00001000
#define FTD       0B00010000
#define FVFO      0B00100000
#define FDOG      0B01000000
#define FBCK      0B10000000
//*----- UI Control Word (USW)

#define BBOOT     0B00000001
#define BMULTI    0B00000010
#define BCW       0B00000100
#define BCCW      0B00001000
#define SQ        0B00010000
#define MIC       0B00100000
#define KDOWN     0B01000000
#define BUSY      0B10000000       //Used for Squelch Open in picoFM and for connected to DDS on sinpleA
#define CONX      0B10000000

//*----- Joystick Control Word (JSW)

#define JLEFT     0B00000001
#define JRIGHT    0B00000010
#define JUP       0B00000100
#define JDOWN     0B00001000


//#define TCPIP_INTERFACE_RESET_SECONDS_TIME		(5 * 60)		//If interface is not connected for # seconds cause a reset of the interface to ensure it will reconnect to new connections
//#define TCPIP_INTERFACE_CHECK_SECONDS_TIME		15				//Check the conencterion every # seconds (so we can flag to our applicaiton if it is connected or not)

void changeFreq();
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
const char   *PROG_BUILD="01";
const char   *COPYRIGHT="(c) LU7DID 2019";

char *strsignal(int sig);
extern const char * const sys_siglist[];
//*-------------------------------------------------------------------------------------------------
//* Main structures
//*-------------------------------------------------------------------------------------------------

//*--- VFO object
VFOSystem vx(showFreq,NULL,NULL,NULL);

//*--- DDS object
DDS dds(changeFreq);

//*--- CAT object
CAT817 cat(CATchangeFreq,CATchangeStatus,CATchangeMode,CATgetRX,CATgetTX);


//*--- Strutctures to hold menu definitions

MenuClass menuRoot(NULL);

MenuClass mod(ModeUpdate);
MenuClass vfo(VfoUpdate);
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
int LCD_LIGHT=LCD_ON;  // On

//*--- debouncing logic setup
auto startEncoder=std::chrono::system_clock::now();
auto endEncoder=std::chrono::system_clock::now();

auto startPush=std::chrono::system_clock::now();
auto endPush=std::chrono::system_clock::now();
 
//*--- LCD custom character definitions
//* TX -- char(0)     <T>
//* S0 -- char(1)     < >
//* S1 -- char(2)     |
//* S2 -- char(3)     ||
//* S3 -- char(4)     |||
//* S4 -- char(5)     ||||
//* S5 -- char(6)     |||||
//* SL -- char(7)     

byte TX[8] = {  //Inverted T (Transmission mode)
  0B11111,
  0B10001,
  0B11011,
  0B11011,
  0B11011,
  0B11011,
  0B11111,
};

byte S1[8] = { //S1 signal
  0B10000,
  0B10000,
  0B10000,
  0B10000,
  0B10000,
  0B10000,
  0B10000,
};
byte S2[8] = { //S2 signal
  0B11000,
  0B11000,
  0B11000,
  0B11000,
  0B11000,
  0B11000,
  0B11000,
};
byte S3[8] = { //S3 signal
  0B11100,
  0B11100,
  0B11100,
  0B11100,
  0B11100,
  0B11100,
  0B11100,
};
byte S4[8] = { //S4 signal
  0B11110,
  0B11110,
  0B11110,
  0B11110,
  0B11110,
  0B11110,
  0B11110,
};
byte S5[8] = { //S5 signal
  0B11111,
  0B11111,
  0B11111,
  0B11111,
  0B11111,
  0B11111,
  0B11111,
};


byte A[8] = {   //VFO A
  0b01110,
  0b10001,
  0b11111,
  0b10001,
  0b10001,
  0b00000,
  0b11111,
};

byte B[8] = {   //VFO B
  0b11110,
  0b10001,
  0b11110,
  0b10001,
  0b11110,
  0b00000,
  0b11111,
};

byte K[8] = {31,17,27,27,27,17,31};    //Inverted K (Keyer)
byte S[8] = {31,17,23,17,29,17,31};    //Inverted S (Split)
byte B1[8]= {24,24,24,24,24,24,24};    // -
byte B2[8]= {30,30,30,30,30,30,30};    // /
byte B3[8]= {31,31,31,31,31,31,31};    // |
byte B4[8]= {                          // \
  0b00000,
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00001,
  0b00000,
};

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
byte keepalive=0;
byte backlight=BACKLIGHT_DELAY;
char port[80];
byte gpio=GPIO04;


//*--- Keyer related memory definitions


char snd_dev[64]="hw:0";


//*--- System Status Word initial definitions

byte MSW  = 0;
byte TSW  = 0;
byte USW  = 0;
byte JSW  = 0;

byte LUSW = 0;
int  TVFO = 0;
int  TBCK = backlight;

int  TWIFI = 10;

bool wlan0 = false;
void showPTT();


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                              ROUTINE STRUCTURE
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
#include "../lib/gui.h"

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

    float f=SetFrequency;
    if (statePTT==true) {
       fprintf(stderr,"PTT <ON>, PA Powered\n");

//*--- if SPLIT swap VFO AND if also CW shift the carrier by vfoshift[current VFO]

       if (getWord(FT817,SPLIT)==true) {
          vx.vfoAB=(vx.vfoAB==VFOA ? VFOB : VFOA);
          f=(float)vx.get(vx.vfoAB);
       }
          
       if (MODE==MCW) {
          f=f+(float)vx.vfoshift[vx.vfoAB];
       }

       if (MODE==MCWR) {
          f=f-(float)vx.vfoshift[vx.vfoAB];
       }
       
       if (f != SetFrequency) {
          dds.set(f);
       }
       softToneWrite (SIDETONE_GPIO, cw_keyer_sidetone_frequency);
       gpioWrite(KEYER_OUT_GPIO, 1);
       return;
    } 

    fprintf(stderr,"PTT <OFF>, Receiver mode\n");
    softToneWrite (SIDETONE_GPIO, 0);
    gpioWrite(KEYER_OUT_GPIO, 0);

    if (getWord(FT817,SPLIT)==true){
       (vx.vfoAB==VFOA ? vx.vfoAB=VFOB : vx.vfoAB = VFOA);
    }


    if (MODE==MCW || MODE == MCWR || getWord(FT817,SPLIT)==true) {
       dds.set((float)(vx.get(vx.vfoAB)));
    }

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
        showSMeter((int)dds.power*2);
     } else {
        setWord(&FT817,PTT,false);
        showSMeter(0);
     }

     cat.FT817=FT817;
     fprintf(stderr,"Keyer state(%d)\n",getWord(FT817,PTT));
     showPTT();

}

void changeFreq() {


}

//*---------------------------------------------------------------------------
//* CATchangeFreq()
//* CAT Callback when frequency changes
//*---------------------------------------------------------------------------
void CATchangeFreq() {

  SetFrequency=cat.SetFrequency;
  long int f=(long int)SetFrequency;
  printf("changeFreq: Frequency set to f(%d)\n",f);
  dds.power=DDSPOWER;
  dds.set(SetFrequency);
  vx.set(vx.vfoAB,f);
  //showVFO();
  

}
//*-----------------------------------------------------------------------------------------------------------
//* CATchangeMode
//* Validate the new mode is a supported one
//* At this point only CW,CWR,USB and LSB are supported
//*-----------------------------------------------------------------------------------------------------------
void CATchangeMode() {

       if (cat.MODE != MUSB && cat.MODE != MLSB && cat.MODE != MCW && cat.MODE != MCWR) {
          cat.MODE = MODE;
          return;
       }

       MODE=cat.MODE;
       showMode();
       return;

}
//*------------------------------------------------------------------------------------------------------------
//* CATchangeStatus
//* Detect which change has been produced and operate accordingly
//*------------------------------------------------------------------------------------------------------------
void CATchangeStatus() {

       fprintf(stderr,"CATchangeStatus():PTT\n");
//*---------------------
       if (getWord(cat.FT817,PTT) != getWord(FT817,PTT)) {        //* PTT Changed
          setWord(&FT817,PTT,getWord(cat.FT817,PTT));
          if (getWord(FT817,PTT)==true) {
             keyState=KEY_DOWN;
          } else {
             keyState=KEY_UP;
          }
          showPTT();
          setPTT(getWord(FT817,PTT));
          //return;
       }
       fprintf(stderr,"CATchangeStatus():RIT\n");

//*---------------------
       if (getWord(cat.FT817,RIT) != getWord(FT817,RIT)) {        //* RIT Changed
          setWord(&FT817,RIT,getWord(cat.FT817,RIT));
          //return;
       }
       fprintf(stderr,"CATchangeStatus():LOCK\n");

       if (getWord(cat.FT817,LOCK) != getWord(FT817,LOCK)) {      //* LOCK Changed
          setWord(&FT817,LOCK,getWord(cat.FT817,LOCK));
          //return;
       }
       fprintf(stderr,"CATchangeStatus():SPLIT\n");

       if (getWord(cat.FT817,SPLIT) != getWord(FT817,SPLIT)) {    //* SPLIT mode Changed
          setWord(&FT817,SPLIT,getWord(cat.FT817,SPLIT));
          showSplit();
          //return;
       }
       fprintf(stderr,"CATchangeStatus():VFO\n");

       if (getWord(cat.FT817,VFO) != getWord(FT817,VFO)) {        //* VFO Changed
          setWord(&FT817,VFO,getWord(cat.FT817,VFO));
          showVFO(); 
          //return;
       }

       fprintf(stderr,"CATchangeStatus():STEP=%d\n",STEP);
       switch(STEP) {
          case 0 : {vx.vfostep[vx.vfoAB]=100; break;}
          case 1 : {vx.vfostep[vx.vfoAB]=500; break;}
          case 2 : {vx.vfostep[vx.vfoAB]=1000; break;}
          case 3 : {vx.vfostep[vx.vfoAB]=5000; break;}
          case 4 : {vx.vfostep[vx.vfoAB]=10000; break;}
          case 5 : {vx.vfostep[vx.vfoAB]=50000; break;}
          case 6 : {vx.vfostep[vx.vfoAB]=100000; break;}
       }
       fprintf(stderr,"STEP set to (%d) var(%d Hz)\n",STEP,vx.vfostep[vx.vfoAB]);

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

    if (getWord(FT817,PTT)==true) {
       return;
    } else {
      float prng= (float)std::rand();
      float pmax= (float)RAND_MAX;
      float v   = abs(15*(prng/pmax));
      fprintf(stderr,"Random number generated is %d MAX(%d)\n",prng,RAND_MAX);
      showSMeter((int)v);
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
              printf("Last push pending processsing, ignore!\n");
              return;
           }
           if (lapPush < 10) {
              printf("Push pulse too short! ignored!\n");
           } else {
             setWord(&USW,BMULTI,true);
             if (lapPush > 2000) {
                printf("Push pulse really long, %d ms. KDOWN signal!\n",lapPush);
                setWord(&USW,KDOWN,true);
             } else {
                printf("Push pulse brief, %d ms. KDOWN false!\n",lapPush);
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
        std::cout << "Rotary Encoder lap " << lapEncoder << "ms.\n";

        if ( lapEncoder  < 2 )  {
             printf("Encoder: ignore pulse too close from last\n");
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
    fprintf(stderr,"\n received signal(%d %s)\n",num,strsignal(num));
    sem_post(&cw_event);
    running=false;
    fprintf(stderr,"\n terminating program by user interruption\n");
   
}
//*---------------------------------------------------
void sigalarm_handler(int sig)
{

   return;

   if (wtd.mItem == 0) { // Exit if watchdog not enabled
      lcd.setCursor(15,1);
      lcd.print("-");
      return;
   }

   keepalive++;
   keepalive=keepalive & 0x03;
   lcd.setCursor(15,1);
   switch(keepalive) {
    case 0:                          {lcd.print("|");break;}
    case 1:                          {lcd.print("/");break;}
    case 2:                          {lcd.print("-");break;}
    case 3:                          {lcd.write(7);break;}
    default:                         {lcd.print("|");break;}
  }

  lcd.setCursor(14,1);  //Process check of wlan0 interface to verify Wifi availability

  if (wlan0 == true) {
     lcd.print("*");
  } else {
     lcd.print(" ");
  }
  alarm(1);

}
//*-----------------------------------------------------------------------------
//* Execute a shell command
//*-----------------------------------------------------------------------------
string do_console_command_get_result (char* command)
{
        fprintf(stderr,"Executing external command: %s\n",command);
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
        fprintf(stderr,"External command result: %s\n",result);
	return(result);
}
//*---------------------------------------------------------------------
//* start Keyer services
//*---------------------------------------------------------------------
void startKeyer() {

	string CommandResult = do_console_command_get_result((char*)"sudo ./keyer start &");
        return;

}
//*---------------------------------------------------------------------
//* stop Keyer services
//*---------------------------------------------------------------------
void stopKeyer() {

	string CommandResult = do_console_command_get_result((char*)"sudo pkill iambic");
        return;

}

//*---------------------------------------------------------------------
//* Check Wifi connection status
//*---------------------------------------------------------------------
void checkLAN() {

        if (wtd.mItem == 0) {
           return;
        }

	string CommandResult = do_console_command_get_result((char*)"cat /sys/class/net/wlan0/operstate");
	if (CommandResult.find("up") == 0)		//If first character is '1' then interface is connected (command returns: '1', '0' or a 'not found' error message)
	{
           wlan0=true;
	} else 	{
           wlan0=false;
	}
        return;
}
//*--------------------------------------------------------------------------------------------
//* showFreq
//* manage the presentation of frequency to the LCD display
//*--------------------------------------------------------------------------------------------
void showFreq() {

  FSTR v;  
  char hi[80];  
  long int f=vx.get(vx.vfoAB); 
  vx.computeVFO(f,&v);

  if (v.millions < 10) {
     sprintf(hi," %d.%d%d%d %d%d",v.millions,v.hundredthousands,v.tenthousands,v.thousands,v.hundreds,v.tens);
  } else {
     sprintf(hi,"%d.%d%d%d %d%d",v.millions,v.hundredthousands,v.tenthousands,v.thousands,v.hundreds,v.tens);
  }
  lcd.setCursor(0,1);
  lcd.print(string(hi));

  if (getWord(TSW,FTU)==true) {
     lcd.setCursor(9,1);
     lcd.typeChar((char)126);
  } 
  if (getWord(TSW,FTD)==true) {
     lcd.setCursor(9,1);
     lcd.typeChar((char)127);

  }
  if (getWord(TSW,FVFO)==true) {
     lcd.setCursor(9,1);
     lcd.typeChar((char)0x20);
     setWord(&TSW,FVFO,false);
  }

  
  signal(SIGALRM, &sigalarm_handler);  // set a signal handler
  alarm(2);  // set an alarm for 10 seconds from now

  memstatus = 0; // Trigger memory write

}

//*----------------------------------------------------------------------------------------------------
//* processVFO
//* identify CW or CCW movements of the rotary encoder and changes the frequency accordingly
//*----------------------------------------------------------------------------------------------------
void processVFO() {

//*--- CW (frequency increase)

   if (getWord(USW,BCW)==true) {
       if (vx.isVFOLocked()==false){
          vx.updateVFO(vx.vfoAB,vx.vfostep[vx.vfoAB]);      
       } else {
          vx.updateVFO(vx.vfoAB,0);      
       }
      setWord(&USW,BCW,false);
      setWord(&TSW,FTU,true);
      setWord(&TSW,FTD,false);
      setWord(&TSW,FVFO,false);
      TVFO=1;
      showFreq();
   }

//*--- CCW frequency decrease

   if (getWord(USW,BCCW)==true) {
       if (vx.isVFOLocked()==false){
          vx.updateVFO(vx.vfoAB,-vx.vfostep[vx.vfoAB]); 
       } else {
          vx.updateVFO(vx.vfoAB,0);
       }
       setWord(&USW,BCCW,false);
       setWord(&TSW,FTD,true);
       setWord(&TSW,FTU,false);
       setWord(&TSW,FVFO,false);
       TVFO=1;
       showFreq();

   }
 
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
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // set initial seed value to system clock

    sprintf(port,"/tmp/ttyv1");



//*--- Process arguments (mostly an excerpt from tune.cpp

       while(1)
        {
                a = getopt(argc, argv, "f:eds:hg:p:A:C:D:E:F:G:M:S:");

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
                        fprintf(stderr,"A: %d\n",cw_active_state);
                        break;
                case 'C':
                        cw_keyer_spacing = atoi(optarg);
                        fprintf(stderr,"C: %d\n",cw_keyer_spacing);
                        break;
                case 'D':
                        strcpy(snd_dev, optarg);
                        fprintf(stderr,"D:%s\n",snd_dev);
                        break;
                case 'E': 
                        cw_keyer_sidetone_envelope = atoi(optarg);
                        fprintf(stderr,"E: %d\n", cw_keyer_sidetone_envelope);
                        break;

                case 'F':
                        cw_keyer_sidetone_frequency = atoi(optarg);
                        fprintf(stderr,"F: %d\n",cw_keyer_sidetone_frequency);
                        break;
                case 'G':     // gain in dB 
                        cw_keyer_sidetone_gain = atoi(optarg);
                        fprintf(stderr,"G: %d\n",cw_keyer_sidetone_gain);
                        break;
                case 'M':
                        cw_keyer_mode = atoi(optarg);
                        fprintf(stderr,"M: %d\n",cw_keyer_mode);
                        break;
                case 'S':
                        cw_keyer_speed = atoi(optarg);
                        fprintf(stderr,"S: %d\n",cw_keyer_speed);
                        break;
                case 'W':
                        cw_keyer_weight = atoi(optarg);
                        fprintf(stderr,"W: %d\n",cw_keyer_weight);
                        break;
                case 'f': // Frequency
                        SetFrequency = atof(optarg);
                        break;
		case 'g': // GPIO
		        gpio=atoi(optarg);
			if (gpio!=GPIO04 && gpio!=GPIO20) {
		  	   fprintf(stderr,"Invalid GPIO pin used (%s), default to GPIO04\n",optarg);
			   gpio=GPIO04;
			}
			break;
                case 'p': //ppm
                        ppm=atof(optarg);
                        break;
                case 'd': //debug
                        cat.TRACE=0x01;
                        fprintf(stderr,"d: DEBUG mode activated\n");
                        break;
                case 'h': // help
                        print_usage();
                        exit(1);
                        break;
                case 's': //serial port
                        sprintf(port,optarg);
                        fprintf(stderr, "serial port:%s\n", optarg);
                        break;
                case -1:
                break;
                case '?':
                        if (isprint(optopt) )
                        {
                                fprintf(stderr, "pixie: unknown option `-%c'.\n", optopt);
                        }
                        else
                        {
                                fprintf(stderr, "pixie: unknown option character `\\x%x'.\n", optopt);
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


//*--- Setup LCD menues for MENU mode (to be shown when CMD=true), a MenuClass object needs to be created first for each

    menuRoot.add((char*)"Mode",&mod);
    menuRoot.add((char*)"VFO",&vfo);
    menuRoot.add((char*)"Split",&spl);
    menuRoot.add((char*)"Step",&stp);
    menuRoot.add((char*)"Shift",&shf);
    menuRoot.add((char*)"Keyer",&kyr);
    menuRoot.add((char*)"WatchDog",&wtd);
    menuRoot.add((char*)"BackLight",&bck);
    menuRoot.add((char*)"Lock",&lck);
    menuRoot.add((char*)"Speed",&spd);
    menuRoot.add((char*)"DDS Drive",&drv);


//*--- Setup child LCD menues

    vfo.add((char*)" A",NULL);
    vfo.add((char*)" B",NULL);
    vfo.set(0);

    spl.add((char*)" Off",NULL);
    spl.add((char*)" On ",NULL);
    spl.set(0);

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
    mod.set(2);
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


//*---- Initialize Iambic Keyer

//*---- Initialize Rotary Encoder

    if(gpioInitialise()<0) {
        fprintf(stderr,"Cannot initialize GPIO\n");
        return -1;
    }

    gpioSetMode(ENCODER_CLK, PI_INPUT);
    gpioSetPullUpDown(ENCODER_CLK,PI_PUD_UP);
    usleep(100000);

    gpioSetISRFunc(ENCODER_CLK, FALLING_EDGE,0,updateEncoders);
    gpioSetMode(ENCODER_DT, PI_INPUT);
    gpioSetPullUpDown(ENCODER_DT,PI_PUD_UP);
    usleep(100000);

    gpioSetMode(ENCODER_SW, PI_INPUT);
    gpioSetPullUpDown(ENCODER_SW,PI_PUD_UP);
    gpioSetAlertFunc(ENCODER_SW,updateSW);
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

    lcd.backlight(true);

//*--- Show banner briefly (1 sec)

    lcd.lcdLoc(LINE1);
    char hi[80];
    sprintf(hi,"%s %s [%s]",PROGRAMID,PROG_VERSION,PROG_BUILD);
    lcd.print(string(hi));
    lcd.lcdLoc(LINE2);
    lcd.print(string(COPYRIGHT));
    delay(ONESEC);

    if (wiringPiSetup () < 0) {
        printf ("Unable to setup wiringPi: %s\n", strerror (errno));
        return 1;
    }

//*---- Define the VFO System parameters (Initial Firmware conditions)

  vx.setVFOdds(setDDSFreq);

  vx.setVFOBand(VFOA,VFO_BAND_START);
  vx.set(VFOA,VFO_START);
  vx.setVFOStep(VFOA,VFO_STEP_100Hz);
  vx.setVFOLimit(VFOA,VFO_START,VFO_END);

  vx.setVFOBand(VFOB,VFO_BAND_START);
  vx.set(VFOB,VFO_START);
  vx.setVFOStep(VFOB,VFO_STEP_100Hz);
  vx.setVFOLimit(VFOB,VFO_START,VFO_END);

  vx.setVFO(VFOA);

  vx.set(vx.vfoAB,SetFrequency);
  vx.set(vx.vfoAB,SetFrequency);

//*--- After the initializacion clear the LCD and show panel in VFOMode

    lcd.clear();

//*---  Define the handler for the SIGALRM signal (timer)

    signal(SIGALRM, &sigalarm_handler);  // set a signal handler
    timer_start(timer_exec,1000);
    //timer_start(timer_SMeter,1000);

//*--- Define the rest of the signal handlers, basically as termination exceptions

    for (int i = 0; i < 64; i++) {

        if (i != SIGALRM && i != 17 && i != 28) {
           std::memset(&sa, 0, sizeof(sa));
           sa.sa_handler = terminate;
           sigaction(i, &sa, NULL);
        }
    }

//*--- Initialize CAT


    cat.open(port,4800);
    cat.SetFrequency=SetFrequency;

    setWord(&FT817,PTT,false);
    setWord(&FT817,RIT,false);
    setWord(&FT817,LOCK,false);
    setWord(&FT817,SPLIT,false);
    setWord(&FT817,VFO,false);

    cat.FT817=FT817;
    cat.MODE=MODE;

//*--- Generate DDS (code excerpt mainly from tune.cpp by Evariste Courjaud F5OEO
    dds.ppm=ppm;
    dds.gpio=gpio;
    dds.power=DDSPOWER;
    dds.open(SetFrequency);


//*---
    //startKeyer();
    keyChange=keyChangeEvent;
    iambic_init();

//*--- DDS is running at the SetFrequency value (initial)

    alarm(1);  // set an alarm for 1 seconds from now to clear all values
    showPanel();
    showFreq();

//*----------------------------------------------------------------------------
//*--- Firmware initialization completed
//*----------------------------------------------------------------------------

//*--- Execute an endless loop while running is true

    while(running)
      {
         usleep(100000);
         cat.get();

 	 if (getWord(USW,CONX) == true) {
            //checkLAN();
            setWord(&USW,CONX,false);
	    TWIFI=30;
         } 
//*--- if in COMMAND MODE (VFO) any change in frequency is detected and transferred to the PLL
         if (getWord(MSW,CMD)==false) {

            processVFO();
            if (SetFrequency != vx.get(vx.vfoAB)) {

               int fVFO=vx.get(vx.vfoAB)/1000;
	       int fPLL=int(SetFrequency/1000);
               printf("PLL=%d VFO=%d\n",fPLL,fVFO);

//*--- A frequency change has been detected, alter the PLL with it

               SetFrequency = vx.get(vx.vfoAB) * 1.0;
               dds.set(SetFrequency);

            }

         } else {

         }
         CMD_FSM();
         lcd.backlight(true);

//*--- Clear the frequency moved marker from the display once the delay expires

         if (getWord(TSW,FBCK)==true && backlight != 0) {
            //* -- Temporary lcd.backlight(false);
            setWord(&TSW,FBCK,false);
         }
      }
//*-------------------------------------------------------------------------------------------
//*--- running become false, the program is terminating
//*-------------------------------------------------------------------------------------------
//*--- Stop keyer thread

    iambic_close();
//*--- Stop the DDS function

    dds.close();

//*--- turn LCD off

    lcd.backlight(false);
    lcd.clear();
    printf("\nProgram terminated....\n");
    exit(0);
}


