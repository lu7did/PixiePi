
//*--------------------------------------------------------------------------------------------------
//* SSB Processor class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de procesamiento SSB
//* This class  process a stream of DSP processed values
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
#include <unistd.h>
#include "stdio.h"
#include <cstring>
#include <signal.h>
#include <stdlib.h>

#define MAX_SAMPLERATE 200000
#define IQBURST 4000

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
      CALLBACK callSSB=NULL;

      void open(float f);
      void close();
      void process();
      float       SetFrequency;
      float       SampleRate=48000;
      int         InputType=typeiq_float;
      int         Decimation=1;
      FILE        *iqfile=NULL;
      iqdmasync   *iqtest;
      bool        TX;
      char FileName[80];
      byte        TRACE;
      bool        enabled=false;

  private:

      char msg[80]; 
      bool loop_mode_flag=false;
      int Harmonic=1;

      enum {typeiq_i16,typeiq_u8,typeiq_float,typeiq_double};
      //int a;
      //int anyargs = 1;

      int SR;
      int FifoSize;
      int nbread;
};

#endif
//*---------------------------------------------------------------------------------------------------
//* SSB CLASS Implementation
//*--------------------------------------------------------------------------------------------------
SSB::SSB(CALLBACK c)
{

   if (c!=NULL) {callSSB=c;}   //* Callback 

   sprintf(FileName,"%s","/tmp/ssbpipe");

   iqfile=fopen(FileName,"rb");
   if (iqfile==NULL) {
      (TRACE>=0x00 ? fprintf(stderr,"SSB: Pipeline error %s not opened\n",FileName) : _NOP);
      enabled=false;
   } else {
      enabled=true;
   }
   SR=48000;
   FifoSize=IQBURST*4;
   TX=false;
   TRACE=0x00;
   (TRACE>=0x00 ? fprintf(stderr,"SSB: Object created, pipe served %s FIFO(%d) SampleRate(%d)\n",FileName,FifoSize,SR) : _NOP);

}
//*------------------------------------------------------------------------
//* open
//* start the processor
//*------------------------------------------------------------------------
void SSB::open(float f) {


   (TRACE>=0x00 ? fprintf(stderr,"SSB: SSB transmitter opened\n") : _NOP);
   iqtest=new iqdmasync(SetFrequency,SampleRate,14,FifoSize,MODE_IQ);
   iqtest->SetPLLMasterLoop(3,4,0);
   TX=true;
   


}
//*-----------------------------------------------------------------------
//* close
//* finalizes and close the SSB Processor
//*-----------------------------------------------------------------------
void SSB::close() {

   (TRACE>=0x00 ? fprintf(stderr,"SSB: SSB transmitter closed\n") : _NOP);
   iqtest->stop();
   TX=false;
   delete(iqtest);

}
//*-----------------------------------------------------------------------
//* process
//* actual stream processing
//*-----------------------------------------------------------------------
void SSB::process() {

   std::complex<float> CIQBuffer[IQBURST]; 
   int CplxSampleNumber=0;
   static float IQBuffer[IQBURST*2];

   nbread=fread(IQBuffer,sizeof(float),IQBURST*2,iqfile);

   if (TX==false || enabled==false) {
      return;
   }

   if(nbread>0) {
     
     for(int i=0;i<nbread/2;i++) {
        if(i%Decimation==0) {       
          CIQBuffer[CplxSampleNumber++]=std::complex<float>(IQBuffer[i*2],IQBuffer[i*2+1]);
        }
     }
   } else {
     return;
   }

   iqtest->SetIQSamples(CIQBuffer,CplxSampleNumber,Harmonic);
}



