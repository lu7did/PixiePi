
//*--------------------------------------------------------------------------------------------------
//* Interpolator   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//  Decimator class implementation Modelled after FIRFilter.java  from JI3GAB
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef Interpolator_h
#define Interpolator_h

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
class Interpolator
{
  public: 
  
     
      Interpolator(float* a,int n_tap,int factor);
      void interpolate(float* x, int len,float* out);

      byte 	  TRACE=0x00;

//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="Interpolator";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";

  private:
      char   msg[80]; 
      int factor;		// decimation factor
      float* coeff;	// filter coefficients
      int n_tap;		// number of taps(coefficients)
      int in_idx;		// index at which new data will be inserted
      float* buf;	// used as a circular buffer

};

#endif
//*---------------------------------------------------------------------------------------------------
//* Interpolator CLASS Implementation
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
Interpolator::Interpolator(float* a,int n_tap,int factor)
{

  this->in_idx=0;
  this->n_tap = n_tap;
  this->coeff = a;
  this->buf = (float*) malloc(BUFFERSIZE*sizeof(float) * 2);
  this->factor=factor;
  fprintf(stderr,"Interpolator::Interpolator() Object creation Completed\n");

}
//*--------------------------------------------------------------------------------------------------
// interpolate: take one sample and compute one filterd output
// takes an array of samples and compute
// interpolated output. Filtering is performed after
// interpolation to avoid aliasing.
// It omits unnecessary multiply with inserted zeros.
//*--------------------------------------------------------------------------------------------------
void Interpolator::interpolate(float* x,int len, float* out) {

   int m = 0;			// output index
   for (int k = 0; k < len; k++) {
       buf[in_idx] = x[k];
       for (int n = 0; n < factor; n++) {
	float y = 0.0;
	int j = in_idx;
	for (int i = n; i < n_tap; i+=factor) {
	  if (j < 0)
	    //j += buf.length;
            j += BUFFERSIZE;
	  y = y + coeff[i] * buf[j--];
	}
	out[m++] = (float)(factor * y);
      }
      in_idx++;
      //if (in_idx >= buf.length) {  // REvisar si buf.length es BUFFER_SIZE o *2
      if (in_idx >= BUFFERSIZE) {
	in_idx -= BUFFERSIZE;
      }
   }
}
