//*--------------------------------------------------------------------------------------------------
//* FIRFilter   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//  FIR Filter implementation Modelled after FIRFilter.java  from JI3GAB
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------

#ifndef FIRFilter_h
#define FIRFilter_h

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
class FIRFilter
{
  public: 
  
      FIRFilter(float* a,int n_tap);

      float do_filter(float x);
      void  do_filter(float* x, int len);

      byte 	  TRACE=0x00;

//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="FIRFilter";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";



  private:
      char   msg[80]; 
      float* coeff;
      int    n_tap;
      int    in_idx;
      float* buf;

};

#endif
//*---------------------------------------------------------------------------------------------------
//* FIRFilter CLASS Implementation
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
FIRFilter::FIRFilter(float* a,int n_tap)
{
  this->in_idx=0;
  this->n_tap = n_tap;
  this->coeff = a;
  this->buf = (float*) malloc(BUFFERSIZE*sizeof(float) * 2);
  fprintf(stderr,"%s Object creation completed taps(%d)\n",PROGRAMID,n_tap);

  
}
//*--------------------------------------------------------------------------------------------------
// do_filter: take one sample and compute one filterd output
//*--------------------------------------------------------------------------------------------------
float FIRFilter::do_filter(float x) {
float y = 0.0;


    buf[in_idx] = x;
    int j = in_idx;
	
    for (int i = 0; i < n_tap; ++i) {
      if (j < 0) {
	j += n_tap;
      }
      y = y + coeff[i] * buf[j--];
    }
  
    in_idx++;
    if (in_idx >= n_tap) {
      in_idx -= n_tap;
    }
  
    return (float)y;
  }
//*--------------------------------------------------------------------------------------------------
// do_filter: take one sample and compute one filterd output
//*--------------------------------------------------------------------------------------------------
// do_filter: take an array of samples and compute
  // the same number of filterd output.
  // The contents of the array will be overriden
  // by filtered output.
void FIRFilter::do_filter(float* x, int len) {
	
    for (int k = 0; k < len; ++k) {
      float y = 0.0;
      int j = in_idx;
      buf[in_idx] = x[k];
	    
      for (int i = 0; i < n_tap; ++i) {
	if (j < 0) {
	   j += n_tap;
        }
	y = y + coeff[i] * buf[j--];
      }
      in_idx++;
      if (in_idx >= n_tap) {
	in_idx -= n_tap;
      }
      x[k] = (float)y;
    }
}

