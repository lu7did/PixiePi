/**
 * pixie.c
 * Raspberry Pi based transceiver
 *
 * This program turns the Raspberry pi into a DDS software able
 * to operate as the LO for a double conversion rig, in this case
 * the popular Pixie setup, but can be extended to any other suitable
 * scheme
 *
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *    Adafruit's python code for CharLCDPlate 
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

//*---- Program specific includes

#include "pixie.h"
#include "ClassMenu.h"
#include "VFOSystem.h"
#include "LCDLib.h"

//*----------------------------------------------------------------------------
//* Special macro definitions to adapt for previous code on the Arduino board
//*----------------------------------------------------------------------------
typedef unsigned char byte;
typedef bool boolean;


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


#define TCPIP_INTERFACE_RESET_SECONDS_TIME		(5 * 60)		//If interface is not connected for # seconds cause a reset of the interface to ensure it will reconnect to new connections
#define TCPIP_INTERFACE_CHECK_SECONDS_TIME		15				//Check the conencterion every # seconds (so we can flag to our applicaiton if it is connected or not)

//*----------------------------------------------------------------------------
//*  Program parameter definitions
//*----------------------------------------------------------------------------

const char   *PROGRAMID="PixiePi";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019";


//*-------------------------------------------------------------------------------------------------
//* Main structures
//*-------------------------------------------------------------------------------------------------

//*--- VFO object

VFOSystem vx(showFreq,NULL,NULL,NULL);

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


//*--- LCD management object

LCDLib lcd(NULL);
int LCD_LIGHT=LCD_ON;  // On

//*--- timer setup

auto startEncoder=std::chrono::system_clock::now();
auto endEncoder=std::chrono::system_clock::now();

auto startPush=std::chrono::system_clock::now();
auto endPush=std::chrono::system_clock::now();
 
//*--- LCD custom character definitions

byte TX[8] = {
  0B11111,
  0B10001,
  0B11011,
  0B11011,
  0B11011,
  0B11011,
  0B11111,
};
byte A[8] = {
  0b01110,
  0b10001,
  0b11111,
  0b10001,
  0b10001,
  0b00000,
  0b11111,
};

byte B[8] = {
  0b11110,
  0b10001,
  0b11110,
  0b10001,
  0b11110,
  0b00000,
  0b11111,
};

byte K[8] = {31,17,27,27,27,17,31};
byte S[8] = {31,17,23,17,29,17,31};
byte B1[8]= {24,24,24,24,24,24,24};
byte B2[8]= {30,30,30,30,30,30,30};
byte B3[8]= {31,31,31,31,31,31,31};
byte B4[8]= {
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
bool running=true;
byte keepalive=0;
byte backlight=BACKLIGHT_DELAY;
bool fFirst=false;

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
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*                              ROUTINE STRUCTURE
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
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
//*-------------------------------------------------------------------------------------------------
//* print_usage
//* help message at program startup
//*-------------------------------------------------------------------------------------------------
void print_usage(void)
{

fprintf(stderr,"\ntune -%s\n\
Usage:\ntune  [-f Frequency] [-h] \n\
-f floatfrequency carrier Hz(50 kHz to 1500 MHz),\n\
-p set clock ppm instead of ntp adjust\n\
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
           printf("KEYUP!\n");
           endPush = std::chrono::system_clock::now();
           int lapPush=std::chrono::duration_cast<std::chrono::milliseconds>(endPush - startPush).count();
           std::cout << "Button Pushed " << lapPush << "ms.\n";
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
        printf("Switch KeyDown\n");
        lcd.backlight(true);
        TBCK=backlight;;
}
//*--------------------------[Rotary Encoder Interrupt Handler]--------------------------------------
//* Interrupt handler for Rotary Encoder CW and CCW control
//*--------------------------------------------------------------------------------------------------
void updateEncoders(int gpio, int level, uint32_t tick)
{
        if (level != 0) {
           return;
        }

       if (getWord(USW,BCW)==true || getWord(USW,BCCW) ==true) {
          return;
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
    printf("Received SIG Interrupt %d",num);
    running=false;
   
}
//*---------------------------------------------------
void sigalarm_handler(int sig)
{

   if (wtd.mItem == 0) {
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
  lcd.setCursor(14,1);

  if (wlan0 == true) {
     lcd.print("*");
  } else {
     lcd.print(" ");
  }
  alarm(1);

}
//--------------------------------------
//----- Execute a shell commandATE -----
//--------------------------------------

string do_console_command_get_result (char* command)
{
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
	return(result);
}
//--------------------------------------
//----- CHECK THE CONNECTION STATE -----
//--------------------------------------
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
#include "gui.h"
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

       while(1)
        {
                a = getopt(argc, argv, "f:ehp:");
        
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
                        break;
                case 'p': //ppm
                        ppm=atof(optarg);
                        break;  
                case 'h': // help
                        print_usage();
                        exit(1);
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


//*--- Setup LCD menues for MENU mode (CLI=true)

    menuRoot.add((char*)"Mode",&mod);
    menuRoot.add((char*)"VFO",&vfo);
    menuRoot.add((char*)"Split",&spl);
    menuRoot.add((char*)"Step",&stp);
    menuRoot.add((char*)"Shift",&shf);
    menuRoot.add((char*)"Keyer",&kyr);
    menuRoot.add((char*)"WatchDog",&wtd);
    menuRoot.add((char*)"BackLight",&bck);
    menuRoot.add((char*)"Lock",&lck);


//*--- Setup child LCD menues

    vfo.add((char*)" A",NULL);
    vfo.set(0);

    spl.add((char*)" Off",NULL);
    spl.set(0);

    stp.add((char*)" 100 Hz",NULL);
    stp.set(0);

    shf.add((char*)" 600 Hz",NULL);
    shf.set(0);

    kyr.add((char*)" IambicA",NULL);
    kyr.set(1);

    wtd.add((char*)"Off",NULL);  
    wtd.set(0);

    sprintf(gui," %d secs",backlight);
    bck.add((char*)gui,NULL);  
    bck.set(backlight);

    mod.add((char*)"CWU  ",NULL);
    mod.set(0);

    lck.add((char*)"Off",NULL);  
    lck.set(0);


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
    lcd.createChar(1,A);
    lcd.createChar(2,B);
    lcd.createChar(3,K);
    lcd.createChar(4,S);
    lcd.createChar(5,B1);
    lcd.createChar(6,B2);
    //lcd.createChar(7,B3);
    lcd.createChar(7,B4);
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

//*--- Define the rest of the signal handlers, basically as termination exceptions

    for (int i = 0; i < 64; i++) {

        if (i != SIGALRM && i != 17 ) {
           std::memset(&sa, 0, sizeof(sa));
           sa.sa_handler = terminate;
           sigaction(i, &sa, NULL);
        }
    }

//*--- Generate DDS (code excerpt mainly from tune.cpp by Evariste Courjaud F5OEO
    generalgpio gengpio;
    gengpio.setpulloff(4);
    padgpio     pad;
    pad.setlevel(7);
    clkgpio     *clk=new clkgpio;
    clk->SetAdvancedPllMode(true);

    if(ppm!=1000) {   //ppm is set else use ntp
      clk->Setppm(ppm);
    }

    clk->SetCenterFrequency(SetFrequency,10);
    clk->SetFrequency(000);
    clk->enableclk(4);

//*--- DDS is running at the SetFrequency value (initial)

    alarm(1);  // set an alarm for 1 seconds from now to clear all values

    showPanel();
    showFreq();
//*--- Firmware initialization completed
//*--- Execute an endless loop while runnint is true
                
    while(running)
      {
         usleep(1000000);

 	 if (getWord(USW,CONX) == true) {
            checkLAN();
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
               clk->SetCenterFrequency(SetFrequency,10);
               clk->SetFrequency(000);
               clk->enableclk(4);
            }

         } else {

         }
         CMD_FSM();

//*--- Clear the frequency moved marker from the display once the delay expires

         if (getWord(TSW,FBCK)==true && backlight != 0) {
            lcd.backlight(false);
            setWord(&TSW,FBCK,false);
         }
      }

//*--- running become false, the program is terminating

    lcd.backlight(false);
    clk->disableclk(4);
    clk->disableclk(20);
    delete(clk);

    usleep(100000);
    printf("\nProgram terminated....\n");
    exit(0);
}


