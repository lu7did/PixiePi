
//*--------------------------------------------------------------------------------------------------
//* FastAGC   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//  FastAGC class implementation Modelled after fastagc_ff  from csdr
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef FastAGC_h
#define FastAGC_h

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


typedef struct fastagc_ff_s
{
    float* buffer_1;
    float* buffer_2;
    float* buffer_input; //it is the actual input buffer to fill
    float peak_1;
    float peak_2;
    int input_size;
    float reference;
    float last_gain;
} fastagc_ff_t;

//typedef void (*CALLBACK)();

void setWord(unsigned char* SysWord,unsigned char  v, bool val);
bool getWord (unsigned char SysWord, unsigned char v);


//*---------------------------------------------------------------------------------------------------
//* FIRFilter CLASS
//*---------------------------------------------------------------------------------------------------
class FastAGC
{
  public: 
  
      FastAGC();
      void agc(fastagc_ff_t* input, float* output);

      byte TRACE=0x00;

//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="FastAGC";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";

  private:
      char   msg[80]; 

};

#endif
//*---------------------------------------------------------------------------------------------------
//* Decimator CLASS Implementation
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de SSB para PixiePi
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
FastAGC::FastAGC()
{


   fprintf(stderr,"FastAGC::FastAGC() Object creation completed\n");

  
}
//*--------------------------------------------------------------------------------------------------
// decimate: take one sample and compute one filterd output
//*--------------------------------------------------------------------------------------------------
void FastAGC::agc(fastagc_ff_t* input, float* output) {

    //Gain is processed on blocks of samples.
    //You have to supply three blocks of samples before the first block comes out.
    //AGC reaction speed equals input_size*samp_rate*2

    //The algorithm calculates target gain at the end of the first block out of the peak value of all the three blocks.
    //This way the gain change can easily react if there is any peak in the third block.
    //Pros: can be easily speeded up with loop vectorization, easy to implement
    //Cons: needs 3 buffers, dos not behave similarly to real AGC circuits

    //Get the peak value of new input buffer
    float peak_input=0;
    for(int i=0;i<input->input_size;i++) //@fastagc_ff: peak search
    {
        float val=fabs(input->buffer_input[i]);
        if(val>peak_input) peak_input=val;
    }

    //Determine the maximal peak out of the three blocks
    float target_peak=peak_input;
    if(target_peak<input->peak_2) target_peak=input->peak_2;
    if(target_peak<input->peak_1) target_peak=input->peak_1;

    //we change the gain linearly on the apply_block from the last_gain to target_gain.
    float target_gain=input->reference/target_peak;
    if(target_gain>FASTAGC_MAX_GAIN) target_gain=FASTAGC_MAX_GAIN;
    //fprintf(stderr, "target_gain: %g\n",target_gain);

    for(int i=0;i<input->input_size;i++) //@fastagc_ff: apply gain
    {
        float rate=(float)i/input->input_size;
        float gain=input->last_gain*(1.0-rate)+target_gain*rate;
        output[i]=input->buffer_1[i]*gain;
    }

    //Shift the three buffers
    float* temp_pointer=input->buffer_1;
    input->buffer_1=input->buffer_2;
    input->peak_1=input->peak_2;
    input->buffer_2=input->buffer_input;
    input->peak_2=peak_input;
    input->buffer_input=temp_pointer;
    input->last_gain=target_gain;
    //fprintf(stderr,"target_gain=%g\n", target_gain);
}

