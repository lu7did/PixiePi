
//*--------------------------------------------------------------------------------------------------
//* SSB Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//  Modelled after iqsend from the rpitx package
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef SSB_h
#define SSB_h

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
#define MAXLEVEL 1

//----- System Variables

#define RUNNING 0B00000001
#define PTT     0B00000010
#define VOX     0B00000100

void setWord(unsigned char* SysWord,unsigned char  v, bool val);
bool getWord (unsigned char SysWord, unsigned char v);

typedef unsigned char byte;
typedef bool boolean;
typedef void (*CALLBACK)();



//*---------------------------------------------------------------------------------------------------
//* DDS CLASS
//*---------------------------------------------------------------------------------------------------
class SSB
{
  public: 
  
      SSB(CALLBACK c);
      CALLBACK changeFreq=NULL;

      float get();
      void set(float f);
      void open(float f);
      void close();
      //void setppm(float ppm);

      float       SetFrequency;
      //clkgpio     *clk=new clkgpio;
      //clkgpio     *clk;
      //clkgpio     *clk=new clkgpio;
      //generalgpio gengpio;
      //padgpio     pad;
      //byte        gpio=GPIO04;
      //float       ppm=1000.0;
      //byte        power=MAXLEVEL;


      byte 	  TRACE=0x00;
      byte        MSW=0;

  private:
      char msg[80]; 
//-----
      bool running=true;
      long Tbreak=0;

};

#endif
//*---------------------------------------------------------------------------------------------------
//* SSB CLASS Implementation
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
SSB::SSB(CALLBACK c)
{


 if (c!=NULL) {changeFreq=c;}   //* Callback of change VFO frequency
 //gengpio.setpulloff(GPIO04);
 //gpio=GPIO04;


}
//*------------------------------------------------------------------------
//* open
//* start the dds
//*------------------------------------------------------------------------
void SSB::open(float f) {

  //clk=new clkgpio;
  //clk->SetAdvancedPllMode(true);

  //gengpio.setpulloff(gpio);
  //pad.setlevel(power);

  //setppm(ppm);
  //set(f);
}
//*-----------------------------------------------------------------------
//* close
//* finalizes and close the DDS
//*-----------------------------------------------------------------------
void SSB::close() {

    //clk->disableclk(gpio);
    //delete(clk);
    //usleep(100000);

}

//*-------------------------------------------------------------------------
//* get()
//* get the current frequency
//*-------------------------------------------------------------------------
float SSB::get() {

   return SetFrequency;

}
//*-------------------------------------------------------------------------
//* set()
//* set the current frequency
//*-------------------------------------------------------------------------
void SSB::set(float f) {

   
   //int fx=(int)f;
   //(TRACE==0x01 ? fprintf(stderr,"SSB::set(): Frequency set (%d)\n",fx) : _NOP);

   SetFrequency=f;
   //if (changeFreq!=NULL) {
   //   changeFreq();
   //}
   //gengpio.setpulloff(gpio);
   //pad.setlevel(power);

   //clk->SetAdvancedPllMode(true);
   //clk->SetCenterFrequency(SetFrequency,10);
   //clk->SetFrequency(000);
   //clk->enableclk(gpio);

}
//*------------------------------------------------------------------------
//* setppm
//*------------------------------------------------------------------------
//void SSB::setppm(float ppm) {
//  if(ppm!=1000) {   //ppm is set else use ntp
//    clk->Setppm(ppm);
//  }
//}

