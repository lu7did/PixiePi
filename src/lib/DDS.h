
//*--------------------------------------------------------------------------------------------------
//* DDS Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef DDS_h
#define DDS_h

#define _NOP        (byte)0
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
#include "/home/pi/librpitx/src/librpitx.h"
//#include "/home/pi/PixiePi/src/lib/DDS.h"
#include "/home/pi/OrangeThunder/src/OT/OT.h"

typedef unsigned char byte;
typedef bool boolean;
typedef void (*CALLBACKDDS)(float f);

//--------------------------[System Word Handler]---------------------------------------------------
// getSSW Return status according with the setting of the argument bit onto the SW
//--------------------------------------------------------------------------------------------------
bool getWord (unsigned char SysWord, unsigned char v);
void setWord(unsigned char* SysWord,unsigned char v, bool val);

//*---------------------------------------------------------------------------------------------------
//* DDS CLASS
//*---------------------------------------------------------------------------------------------------
class DDS
{
  public: 
  
      DDS(CALLBACKDDS c);
      CALLBACKDDS changeFreq=NULL;

      float get();
      void set(float f);
      void start(float f);
      void stop();
      void setppm(float ppm);

      float       f;
      //clkgpio     *clk=new clkgpio;
      clkgpio     *clk;
      //clkgpio     *clk=new clkgpio;
      generalgpio gengpio;
      padgpio     pad;
      byte        gpio=GPIO_DDS;
      float       ppm=1000.0;
      byte        power=DDS_MAXLEVEL;
      byte 	  TRACE=0x00;
      byte        MSW=0x00;

// --- Program initialization
const char   *PROGRAMID="DDS";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";


  private:
      char msg[80]; 
};

#endif
//*---------------------------------------------------------------------------------------------------
//* DDS CLASS Implementation
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de CAT para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
DDS::DDS(CALLBACKDDS c)
{


 if (c!=NULL) {changeFreq=c;}   //* Callback of change VFO frequency

 gengpio.setpulloff(GPIO_DDS);
 gpio=GPIO_DDS;

 (TRACE>=0x01 ? fprintf(stderr,"%s::DDS object successfully created\n",PROGRAMID) : _NOP);
}
//*------------------------------------------------------------------------
//* open
//* start the dds
//*------------------------------------------------------------------------
void DDS::start(float freq) {

  clk=new clkgpio;
  clk->SetAdvancedPllMode(true);

  gengpio.setpulloff(gpio);
  pad.setlevel(power);

  setppm(ppm);
  set(freq);

  setWord(&MSW,RUN,true);
  usleep(10000);
 (TRACE>=0x01 ? fprintf(stderr,"%s::start() GPIO(%d) power(%d) freq(%g) started!\n",PROGRAMID,this->gpio,this->power,this->f) : _NOP);

}
//*-----------------------------------------------------------------------
//* close
//* finalizes and close the DDS
//*-----------------------------------------------------------------------
void DDS::stop() {

    clk->disableclk(gpio);
    delete(clk);
    usleep(100000);
    setWord(&MSW,RUN,false);
    (TRACE>=0x01 ? fprintf(stderr,"%s::stop() stopped!\n",PROGRAMID) : _NOP);

}

//*-------------------------------------------------------------------------
//* get()
//* get the current frequency
//*-------------------------------------------------------------------------
float DDS::get() {

   return f;

}
//*-------------------------------------------------------------------------
//* set()
//* set the current frequency
//*-------------------------------------------------------------------------
void DDS::set(float freq) {

   
   int fx=(int)freq;
   this->f=freq;
   if (changeFreq!=NULL && getWord(MSW,RUN)==true) {
      changeFreq(f);
   }

   gengpio.setpulloff(gpio);
   pad.setlevel(power);

   clk->SetAdvancedPllMode(true);
   clk->SetCenterFrequency(this->f,10);
   clk->SetFrequency(000);
   clk->enableclk(gpio);

   (TRACE==0x01 ? fprintf(stderr,"DDS::set(): Frequency set (%d)\n",fx) : _NOP);

}
//*------------------------------------------------------------------------
//* setppm
//*------------------------------------------------------------------------
void DDS::setppm(float ppm) {
  if(ppm!=1000) {   //ppm is set else use ntp
    clk->Setppm(ppm);
  }

}

