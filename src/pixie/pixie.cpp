/**
 * pixie.c
 * Raspberry Pi based transceiver
 *
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *    Adafruit's python code for CharLCDPlate
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

#define PROGRAMID  "Pixie"
#define PROG_VERSION "1.0"
#define PROG_BUILD  "080"
#define COPYRIGHT "(c) LU7DID 2019"


#include <stdio.h>
#include <wiringPi.h>
#include <stdio.h>
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


#include "pixie.h"
#include "ClassMenu.h"
#include "VFOSystem.h"


typedef unsigned char byte;
typedef bool boolean;


//*--- Encoder pin definition

#define ENCODER_CLK 17
#define ENCODER_DT  18
#define ENCODER_SW  27


//*--- Define some device parameters

#define I2C_ADDR    0x27 // I2C device address

//*--- LCD constants (16x2 I2C controlled LCD display assumed)

#define LCD_CHR     1    // Mode - Sending data
#define LCD_CMD     0    // Mode - Sending command
#define LINE1       0x80 // 1st line
#define LINE2       0xC0 // 2nd line

#define LCD_ON      0x08
#define LCD_OFF     0x00
#define LCD_LIGHT   LCD_ON  // On

#define ENABLE  0b00000100 // Enable bit


#define VFO_START 	 14000000
#define VFO_END          14399999
#define VFO_BAND_START          4

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
#define FTC       0B00100000
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

//*-------------------------------------------------------------------------------------------------
//* Define class to manage VFO
//*-------------------------------------------------------------------------------------------------
VFOSystem vx(showFreq,NULL,NULL,NULL);

//*--- Strutctures to hold menu definitions

MenuClass menuRoot(NULL);

MenuClass band(BandUpdate);
MenuClass vfo(vfoUpdate);
MenuClass stp(StepUpdate);
MenuClass shf(ShiftUpdate);
MenuClass lck(LckUpdate);
MenuClass mod(ModUpdate);


//*---- Default values for Encoder + Push button

int value=0;
int lastEncoded=0;
int counter=0;
int clkLastState=0; 


int fd;                   // seen by all subroutines


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


//*-----------------------------------------------------------------------------------
//*--- Define System Status Words
//*-----------------------------------------------------------------------------------

byte MSW = 0;
byte TSW = 0;
byte USW = 0;
byte JSW = 0;
byte val = 0;
byte col = 0;
void sigalarm_handler(int sig)
{
    // This gets called when the timer runs out.  Try not to do too much here;
    // the recommended practice is to set a flag (of type sig_atomic_t), and have
    // code elsewhere check that flag (e.g. in the main loop of your program)
    setWord(&TSW,FTU,false);
    setWord(&TSW,FTD,false);
    setWord(&TSW,FTC,true);

}
//*=======================================================================================================================================================
//* Handlers for LCD display
//*=======================================================================================================================================================
//*--- float to string
void typeFloat(float myFloat)   {
  char buffer[20];
  sprintf(buffer, "%4.2f",  myFloat);
  typeln(buffer);
}

//*--- int to string
void typeInt(int i)   {
  char array1[20];
  sprintf(array1, "%d",  i);
  typeln(array1);
}

//*--- clr lcd go home loc 0x80
void ClrLcd(void)   {
  lcd_byte(0x01, LCD_CMD);
  lcd_byte(0x02, LCD_CMD);
}
//*--- go to location on LCD
void lcdLoc(int line)   {
  lcd_byte(line, LCD_CMD);
}

// out char to LCD at current position
void typeChar(char val)   {

  lcd_byte(val, LCD_CHR);
}


// this allows use of any size string
void typeln(const char *s)   {

  while ( *s ) lcd_byte(*(s++), LCD_CHR);

}

void lcd_createCustom(byte charnum, byte* charset) {
  lcd_byte (0b00010000 & charnum, LCD_CMD);      //# Start definine char charnum, line 0
  for (int i=0;i<8;i++) {
      lcd_byte (charset[i], LCD_CHR); //# first line solid line (11111b)
  }

//#continue sending data until all your chars are defined

  lcd_byte (1, LCD_CMD);       //# Clear screen. Ends char define
//  lcd_byte (LINE2, LCD_CMD);   //# start writing at line 1
//  lcd_byte (0, LCD_CHR);       //# print first user defined character

}
void lcd_pos(byte line, byte col) {


  lcd_byte(0b00000010,LCD_CMD); //Cursor home
  if (line==0) {
     lcd_byte (LINE1, LCD_CMD);   //# start writing at line 1
  } else {
     lcd_byte (LINE2, LCD_CMD);   //# start writing at line 1
  }
  for (int i=0; i<col;i++) {
      lcd_byte(0b00010100,LCD_CMD); //Cursor shift right
  }
}
void lcd_custom(byte charnum) {
//
//  lcd_byte (64, LCD_CMD);      //# Start definine char 0, line 0
//  lcd_byte (0b11111, LCD_CHR); //# first line solid line (11111b)
//  lcd_byte (0b10001, LCD_CHR); //# second line (10001b)
//  lcd_byte (0b11011, LCD_CHR); //# second line (10001b)
//  lcd_byte (0b11011, LCD_CHR); //# second line (10001b)
//  lcd_byte (0b11011, LCD_CHR); //# second line (10001b)
//  lcd_byte (0b11011, LCD_CHR); //# second line (10001b)
//  lcd_byte (0b11111, LCD_CHR); //# first line solid line (11111b)
 

//#continue sending data until all your chars are defined
//  lcd_byte (1, LCD_CMD);       //# Clear screen. Ends char define
//  lcd_byte (LINE2, LCD_CMD);   //# start writing at line 1
  lcd_byte (charnum, LCD_CHR);       //# print first user defined character

}


void lcd_byte(int bits, int mode)   {

  //Send byte to data pins
  // bits = the data
  // mode = 1 for data, 0 for command
  int bits_high;
  int bits_low;

  // uses the two half byte writes to LCD
  bits_high = mode | (bits & 0xF0) | LCD_LIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_LIGHT ;

  // High bits
  wiringPiI2CReadReg8(fd, bits_high);
  lcd_toggle_enable(bits_high);

  // Low bits
  wiringPiI2CReadReg8(fd, bits_low);
  lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)   {
  // Toggle enable pin on LCD display
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}
void lcd_init()   {
  // Initialise display
  lcd_byte(0x33, LCD_CMD); // Initialise
  lcd_byte(0x32, LCD_CMD); // Initialise
  lcd_byte(0x06, LCD_CMD); // Cursor move direction
  lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
  lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
  lcd_byte(0x01, LCD_CMD); // Clear display
  delayMicroseconds(500);
}

//*=======================================================================================================================================================
//* Implementarion of Menu Handlers
//*=======================================================================================================================================================
void setDDSFreq(){

  //* dummy hook
}

void BandUpdate() {
}
void vfoUpdate() {
}
void StepUpdate() {
}
void ShiftUpdate() {
}
void LckUpdate() {
}
void ModUpdate() {
}
//*--------------------------[Rotary Encoder Interrupt Handler]--------------------------------------
//* Interrupt handler routine for Rotary Encoder Push button
//*--------------------------------------------------------------------------------------------------
void updateSW(int gpio, int level, uint32_t tick)
{

        if (level != 0) {
           return;
        }
        int pushSW=gpioRead(ENCODER_SW);
        printf("Switch Pressed\n");
        //char hi[80];
        //val++;
        //sprintf(hi,"value %d ",val);
        ClrLcd();
        lcd_createCustom(0,TX);
        lcdLoc(LINE1);
        typeln("0123456789012345");

        lcd_pos(1,col);
        //lcdLoc(LINE2);
        lcd_custom(0);
	col++;
        col=col & 0x0f;

        //typeln(hi);
        //typeChar((char)val);


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

        if (dtState != clkState) {
          counter++;
          setWord(&USW,BCW,true);
        } else {
          counter--;
          setWord(&USW,BCCW,true);
        }
        //printf("Rotary encoder activated counter=%d\n",counter);
        //ClrLcd();
        //lcdLoc(LINE1);
        //sprintf(buftext,"%s","Rotary encoder turned..."));
        //typeln("Rotary encoder turned");

        clkLastState=clkState;
//        }
    
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

void sig_handler(int sig) {
   printf("\nProgram terminated....\n");
   exit(0);

}
//*--------------------------------------------------------------------------------------------
//* showFreq
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void showFreq() {

  char hi[80];
  FSTR v;  
    
  long int f=vx.get(vx.vfoAB); 
  vx.computeVFO(f,&v);

  ClrLcd();
  if (v.millions < 10) {
     sprintf(hi," %d.%d%d%d",v.millions,v.hundredthousands,v.tenthousands,v.thousands);
  } else {
     sprintf(hi,"%d.%d%d%d",v.millions,v.hundredthousands,v.tenthousands,v.thousands);
  }
  lcdLoc(LINE1);
  typeln(hi);
  if (getWord(TSW,FTU)==true) {
     typeChar((char)127);
  } 
  if (getWord(TSW,FTD)==true) {
     typeChar((char)126);
  }
  if (getWord(TSW,FTC)==true) {
     typeChar((char)0x20);
  }
  //signal(SIGALRM, &sigalrm_handler);  // set a signal handler
  //alarm(2);  // set an alarm for 10 seconds from now

//*---- Prepare to display
  //lcd.setCursor(2, 1);

  //if (v.millions <10) {
  //  typeln(" ");
  //}
  
  //typeln(v.millions);
  //typeln(".");
  //typeln(v.hundredthousands);
  //typeln(v.tenthousands);
  //typeln(v.thousands);

//**************************************
//* Setup device specific frequency    *
//**************************************
  //setFrequencyHook(f,&v); 
  //timepassed = millis();
  //memstatus = 0; // Trigger memory write

}

//*----------------------------------------------------------------------------------------------------
//* processVFO
//* come here to update the VFO
//*----------------------------------------------------------------------------------------------------
void processVFO() {

   if (getWord(USW,BCW)==true) {
       if (vx.isVFOLocked()==false){
          vx.updateVFO(vx.vfoAB,vx.vfostep[vx.vfoAB]);      
       } else {
          vx.updateVFO(vx.vfoAB,0);      
       }
      setWord(&USW,BCW,false);
      setWord(&TSW,FTU,true);
      setWord(&TSW,FTC,false);
      signal(SIGALRM, &sigalarm_handler);  // set a signal handler
      alarm(1);  // set an alarm for 10 seconds from now
      showFreq();
   }
   if (getWord(USW,BCCW)==true) {
       if (vx.isVFOLocked()==false){
          vx.updateVFO(vx.vfoAB,-vx.vfostep[vx.vfoAB]); 
       } else {
          vx.updateVFO(vx.vfoAB,0);
       }
       setWord(&USW,BCCW,false);
       setWord(&TSW,FTD,true);
       setWord(&TSW,FTC,false);
       showFreq();
       signal(SIGALRM, &sigalarm_handler);  // set a signal handler
       alarm(1);  // set an alarm for 10 seconds from now

   }
 
}


//*--------------------------------------------------------------------------------------------------
//* main execution of the program
//*--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{

    printf("%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);


//**********************************************************************************************
//*--- Initial value for system operating modes
//**********************************************************************************************

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



//*--- Setup LCD menues

    menuRoot.add((char*)"Band",&band);
    menuRoot.add((char*)"VFO",&vfo);
    menuRoot.add((char*)"Step",&stp);
    menuRoot.add((char*)"IF Shift",&shf);
    menuRoot.add((char*)"Lock",&lck);
    menuRoot.add((char*)"Mode",&mod);

//*--- Setup child LCD menues

    band.add((char*)"Off      ",NULL);
    band.set(0);

    shf.add((char*)" 0 Hz",NULL);
    shf.set(0);

    vfo.add((char*)" A",NULL);
    vfo.set(0);

//*---- Establish different and default step

    stp.add((char*)"  1 KHz",NULL);
    stp.set(3);

    lck.add((char*)"Off",NULL);  
    lck.set(0);
  
    mod.add((char*)"DDS",NULL);
    mod.set(0);

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

    fd = wiringPiI2CSetup(I2C_ADDR);
  
//printf("fd = %d ", fd);

    lcd_init(); // setup LCD
    char array1[30];
    sprintf(array1,"%s",PROGRAMID);

    ClrLcd();
    lcdLoc(LINE1);
    typeln(PROGRAMID);
    delay(1000);

    if (wiringPiSetup () < 0) {
        printf ("Unable to setup wiringPi: %s\n", strerror (errno));
        return 1;
    }

//*---- Define the VFO System parameters (Initial Firmware conditions)

  vx.setVFOdds(setDDSFreq);

  vx.setVFOBand(VFOA,VFO_BAND_START);
  vx.set(VFOA,VFO_START);
  vx.setVFOStep(VFOA,VFO_STEP_1KHz);
  vx.setVFOLimit(VFOA,VFO_START,VFO_END);

  vx.setVFOBand(VFOB,VFO_BAND_START);
  vx.set(VFOB,VFO_START);
  vx.setVFOStep(VFOB,VFO_STEP_1KHz);
  vx.setVFOLimit(VFOB,VFO_START,VFO_END);

  vx.setVFO(VFOA);




//*---- Signal handler

    signal(SIGINT, sig_handler);
    signal(SIGKILL, sig_handler);
    showFreq();

//*===============================================================================================================VVVVV
//*----- this is a transient test code, to be removed later


signal(SIGALRM, &sigalarm_handler);  // set a signal handler
alarm(1);  // set an alarm for 10 seconds from now

//for (int j = 0; j < 10 ; j++ ) {
//
//   unsigned char i=menuRoot.get();
//   MenuClass* z=menuRoot.getChild(i);
//   printf("<%d> %s\n",i,menuRoot.getCurrentText());
//   menuRoot.move(false,true);
//}

while (true) {


    processVFO();
    if(getWord(TSW,FTC)==true){
      showFreq();
      setWord(&TSW,FTC,false);
    }

}

}


