
//*--------------------------------------------------------------------------------------------------
//* Decimator   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//  Decimator class implementation Modelled after FIRFilter.java  from JI3GAB
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef Decimator_h
#define Decimator_h

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
#include <limits.h>

#define MAXLEVEL 1


//----- System Variables

#define RUNNING 0B00000001
#define BUFFERSIZE 96000
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
class Decimator
{
  public: 
  
      Decimator(float* a,int n_tap,int factor);
      void decimate(short* x, int len, float* I,float* Q);
      void decimate(short* x, int len, short* buffer);

      byte TRACE=0x00;
      float* fbuffer;

//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="Decimator";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";

  private:
      char   msg[80]; 
      int    factor;		// decimation factor
      float* coeff;	// filter coefficients
      int    n_tap;		// number of taps(coefficients)
      int    in_idx;		// index at which new data will be inserted
      float* buf;	// used as a circular buffer
};

#endif
//*---------------------------------------------------------------------------------------------------
//* Decimator CLASS Implementation
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
Decimator::Decimator(float* a,int n_tap,int factor)
{
  this->in_idx=0;
  this->n_tap = n_tap;
  this->coeff = a;
  this->buf = (float*) malloc(BUFFERSIZE*sizeof(float) * 2);
  this->fbuffer=(float*) malloc(BUFFERSIZE*sizeof(float) * 2);
  this->factor=factor;
  (TRACE>=0x02 ? fprintf(stderr,"%s:: Object creation completed tap(%d) factor(%d)\n",PROGRAMID,n_tap,factor) : _NOP);
  
}
// --------------------------------------------------------------------------------------------------
// decimate: another interface for decimator creating I/Q pairs with gain adjustment
// --------------------------------------------------------------------------------------------------
void Decimator::decimate(short* x, int len, short* buffer) {

    float f=(float)(SHRT_MAX/2.0);

    decimate(x,len,fbuffer,NULL);
    for (int i=0;i<factor;i++) {
        buffer[i]=(short)(fbuffer[i]*f);
    }
    return;
    
}
// --------------------------------------------------------------------------------------------------
// decimate: another interface for decimator creating I/Q pairs with gain adjustment
// --------------------------------------------------------------------------------------------------
void Decimator::decimate(short* x, int len, float* I,float* Q) {

  int m = 0;			// output index
  in_idx=0;
  float f=(float)(SHRT_MAX/2.0);
 
  for (int k = 0; k < len; ) {
    for (int n = 0; n < factor; n++) {
	buf[in_idx++] = (float)x[k++]/f;
	if (in_idx >= n_tap) {
	    in_idx -= n_tap;
	}
    }
    int j = in_idx - 1;
    if (j < 0) {
	j = n_tap - 1;
    }
    float y = 0.0;    
    for (int i = 0; i < n_tap; ++i) {
       if (j < 0) {
	  j += n_tap;
       }
       y = y + coeff[i] * buf[j--];
    }

    I[m] = (float)y;
    if (Q!=NULL) {
       Q[m] = I[m];
    }
    m++;
  }
}

