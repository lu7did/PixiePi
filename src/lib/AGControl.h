
//*--------------------------------------------------------------------------------------------------
//* AGC   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//  AGC class implementation Modelled after simple_agc_cc  from csdr
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef AGC_h
#define AGC_h

#define _NOP        (byte)0

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#include <unistd.h>
#include <math.h>

#define MAXLEVEL 1

//----- System Variables

#define RUNNING 0B00000001
#define BUFFERSIZE 96000
#define FASTAGC_MAX_GAIN 50
//*---------------------------------------------------------------------------------------------------
//* Definitions
//*---------------------------------------------------------------------------------------------------
typedef unsigned char byte;
typedef bool boolean;

//typedef void (*CALLBACK)();

void setWord(unsigned char* SysWord,unsigned char  v, bool val);
bool getWord (unsigned char SysWord, unsigned char v);


//*---------------------------------------------------------------------------------------------------
//* FIRFilter CLASS
//*---------------------------------------------------------------------------------------------------
class AGControl
{
  public: 
  
      AGControl();
      void computeagc(float* Iin, float* Qin, float* Iout, float* Qout,int input_size, float rate, float reference, float max_gain, float* current_gain) ;

      byte TRACE=0x00;

//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="AGControl";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";

  private:
      char   msg[80]; 

};

#endif
//*---------------------------------------------------------------------------------------------------
//* AGC CLASS Implementation
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
AGControl::AGControl()
{

   fprintf(stderr,"%s::AGC() Object creation completed\n",PROGRAMID);

  
}
//*--------------------------------------------------------------------------------------------------
// decimate: take one sample and compute one filterd output
//*--------------------------------------------------------------------------------------------------
void AGControl::computeagc(float* Iin, float* Qin, float* Iout, float* Qout,int input_size, float rate, float reference, float max_gain, float* current_gain) {

float rate_1minus=1-rate;
int   debugn = 0;
float f=1.0;

    if (current_gain != NULL) {
       f=*current_gain;
    }

    for(int i=0;i<input_size;i++)
    {
        float amplitude = sqrt(Iin[i]*Iin[i]+Qin[i]*Qin[i]);
        float ideal_gain = (reference/amplitude);
        if(ideal_gain>max_gain) ideal_gain = max_gain;
        if(ideal_gain<=0) ideal_gain = 0;
        f = ((ideal_gain-f)*rate) + (f*rate_1minus);
        Iout[i]=f*Iin[i];
        Qout[i]=f*Qin[i];
    }
    if (current_gain != NULL) {
       *current_gain=f;
    }
       
}

