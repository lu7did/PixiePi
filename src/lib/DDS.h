
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

#define GPIO04   4
#define GPIO20  20 
#define MAXLEVEL 7

typedef unsigned char byte;
typedef bool boolean;
typedef void (*CALLBACK)();



//*---------------------------------------------------------------------------------------------------
//* DDS CLASS
//*---------------------------------------------------------------------------------------------------
class DDS
{
  public: 
  
      DDS(CALLBACK c);
      CALLBACK changeFreq=NULL;

      float get();
      void set(float f);
      void open(float f);
      void close();
      void setppm(float ppm);
      void disable();
      void enable();

      float       SetFrequency;
      clkgpio     *clk=new clkgpio;
      generalgpio gengpio;
      padgpio     pad;
      byte        gpio=GPIO04;
      float       ppm=1000.0;
      byte        power=MAXLEVEL;
      byte 	  TRACE=0x00;

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
DDS::DDS(CALLBACK c)
{

  if (c!=NULL) {changeFreq=c;}   //* Callback of change VFO frequency

//  gengpio.setpulloff(4);
  gpio=GPIO04;

  gengpio.setpulloff(gpio);
  pad.setlevel(power);
  clk->SetAdvancedPllMode(true);


}
//*-------------------------------------------------------------------------
//* get()
//* get the current frequency
//*-------------------------------------------------------------------------
float DDS::get() {

   return SetFrequency;

}
//*-------------------------------------------------------------------------
//* set()
//* set the current frequency
//*-------------------------------------------------------------------------
void DDS::set(float f) {

   int fx=(int)f;
   (TRACE==0x01 ? fprintf(stderr,"DDS::set(): Frequency set (%d)\n",fx) : _NOP);

   SetFrequency=f;
   if (changeFreq!=NULL) {
      changeFreq();
   }
   gengpio.setpulloff(gpio);
   pad.setlevel(power);

   clk->SetAdvancedPllMode(true);
   clk->SetCenterFrequency(SetFrequency,10);
   clk->SetFrequency(000);
   clk->enableclk(gpio);

}
//*------------------------------------------------------------------------
//* enable DDS
//*------------------------------------------------------------------------

void DDS::enable() {
   clk->enableclk(gpio);
}
//*------------------------------------------------------------------------
//* disable DDS
//*------------------------------------------------------------------------
void DDS::disable() {

   //set(0);
   clk->disableclk(gpio);

}
//*------------------------------------------------------------------------
//* setppm
//*------------------------------------------------------------------------
void DDS::setppm(float ppm) {
  if(ppm!=1000) {   //ppm is set else use ntp
    clk->Setppm(ppm);
  }
}
//*------------------------------------------------------------------------
//* open
//* start the dds
//*------------------------------------------------------------------------
void DDS::open(float f) {

  setppm(ppm);
  set(f);
}
//*-----------------------------------------------------------------------
//* close
//* finalizes and close the DDS
//*-----------------------------------------------------------------------
void DDS::close() {

    clk->disableclk(gpio);
    delete(clk);

    usleep(100000);

}

