//*--------------------------------------------------------------------------------------------------
//* DDS Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef DDS_h
#define DDS_h

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

      float       SetFrequency;
      clkgpio     *clk=new clkgpio;
      generalgpio gengpio;
      padgpio     pad;
      float       ppm=1000.0;


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

  gengpio.setpulloff(4);
  pad.setlevel(7);

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

   SetFrequency=f;
   if (changeFreq!=NULL) {
      changeFreq();
   }

   clk->SetCenterFrequency(SetFrequency,10);
   clk->SetFrequency(000);
   clk->enableclk(4);

}
//*------------------------------------------------------------------------
//* open
//* start the dds
//*------------------------------------------------------------------------
void DDS::open(float f) {

  if(ppm!=1000) {   //ppm is set else use ntp
    clk->Setppm(ppm);
  }

  set(f);
}
//*-----------------------------------------------------------------------
//* close
//* finalizes and close the DDS
//*-----------------------------------------------------------------------
void DDS::close() {

    clk->disableclk(4);
    clk->disableclk(20);
    delete(clk);

    usleep(100000);

}

