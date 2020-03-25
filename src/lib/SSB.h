//--------------------------------------------------------------------------------------------------
// SSB   (HEADER CLASS)
//--------------------------------------------------------------------------------------------------
// Este es el firmware del diseÃ±o de SSB para PixiePi
// SSB class implementation of a simple USB transceiver
// Solo para uso de radioaficionados, prohibido su utilizacion comercial
// Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//--------------------------------------------------------------------------------------------------

#ifndef SSB_h
#define SSB_h

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

#include "/home/pi/PixiePi/src/lib/FIRFilter.h"
#include "/home/pi/PixiePi/src/lib/Decimator.h"
#include "/home/pi/PixiePi/src/lib/Interpolator.h"
#include "/home/pi/PixiePi/src/lib/AGControl.h"

#define MAXLEVEL 1

//----- System Variables

#define BUFFERSIZE 96000
#define IQBURST          4000

//---------------------------------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------------------------------

typedef unsigned char byte;
typedef bool boolean;
typedef void (*CALLBACK)();


void setWord(unsigned char* SysWord,unsigned char  v, bool val);
bool getWord (unsigned char SysWord, unsigned char v);

typedef struct ACBStruct {
     boolean active;
     float reference;
     float rate;
     float max_gain;
     AGControl* g;
     float* gain;
} ACB;

//---------------------------------------------------------------------------------------------------
// SSB CLASS
//---------------------------------------------------------------------------------------------------
class SSB
{
  public: 
  
      SSB();
      int generate(short *input,int len,float* I,float* Q);

      byte TRACE=0x00;
      ACB  agc;
      int  decimation_factor;




//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="SSB";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";


private:

char   msg[80]; 

float* a;
float* b;
float* c;

FIRFilter    *iFilter;
FIRFilter    *qFilter;
Decimator    *d;


};

#endif
//---------------------------------------------------------------------------------------------------
// SSB CLASS Implementation
//--------------------------------------------------------------------------------------------------
// Este es el firmware del diseÃ±o de SSB para PixiePi
// Solo para uso de radioaficionados, prohibido su utilizacion comercial
// Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//--------------------------------------------------------------------------------------------------

SSB::SSB()
{

  fprintf(stderr,"%s:: AGC Object Initialization\n",PROGRAMID);

  agc.g=new AGControl();
  agc.active=false;
  agc.rate=0.25;
  agc.reference=1.0;
  agc.max_gain=5.0;
  agc.gain=NULL;

  decimation_factor=8;

  fprintf(stderr,"%s: Filter coefficiente buffer creation\n",PROGRAMID); 
  a=(float*) malloc(96*sizeof(float));
  b=(float*) malloc(96*sizeof(float));
  c=(float*) malloc(96*sizeof(float));

  fprintf(stderr,"%s:init(): Filter initialization\n",PROGRAMID);
  
/*
     Kaiser Window FIR Filter
     Passband: 0.0 - 3000.0 Hz
     Order: 83
     Transition band: 3000.0 Hz
     Stopband attenuation: 80.0 dB
*/     


    a[0] =	-1.7250879E-5f;
    a[1] =	-4.0276995E-5f;
    a[2] =	-5.6314686E-5f;
    a[3] =	-4.0164417E-5f;
    a[4] =	3.0053454E-5f;
    a[5] =	1.5370155E-4f;
    a[6] =	2.9180944E-4f;
    a[7] =	3.6717512E-4f;
    a[8] =	2.8903902E-4f;
    a[9] =	3.1934875E-11f;
    a[10] =	-4.716546E-4f;
    a[11] =	-9.818495E-4f;
    a[12] =	-0.001290066f;
    a[13] =	-0.0011395542f;
    a[14] =	-3.8172887E-4f;
    a[15] =	9.0173044E-4f;
    a[16] =	0.0023420234f;
    a[17] =	0.003344623f;
    a[18] =	0.003282209f;
    a[19] =	0.0017731993f;
    a[20] =	-0.0010558856f;
    a[21] =	-0.004450674f;
    a[22] =	-0.0071515352f;
    a[23] =	-0.007778209f;
    a[24] =	-0.0053855875f;
    a[25] =	-2.6561373E-10f;
    a[26] =	0.0070972904f;
    a[27] =	0.013526209f;
    a[28] =	0.016455514f;
    a[29] =	0.013607533f;
    a[30] =	0.0043148645f;
    a[31] =	-0.009761283f;
    a[32] =	-0.02458954f;
    a[33] =	-0.03455451f;
    a[34] =	-0.033946108f;
    a[35] =	-0.018758629f;
    a[36] =	0.011756961f;
    a[37] =	0.054329403f;
    a[38] =	0.10202855f;
    a[39] =	0.14574805f;
    a[40] =	0.17644218f;
    a[41] =	0.18748334f;
    a[42] =	0.17644218f;
    a[43] =	0.14574805f;
    a[44] =	0.10202855f;
    a[45] =	0.054329403f;
    a[46] =	0.011756961f;
    a[47] =	-0.018758629f;
    a[48] =	-0.033946108f;
    a[49] =	-0.03455451f;
    a[50] =	-0.02458954f;
    a[51] =	-0.009761283f;
    a[52] =	0.0043148645f;
    a[53] =	0.013607533f;
    a[54] =	0.016455514f;
    a[55] =	0.013526209f;
    a[56] =	0.0070972904f;
    a[57] =	-2.6561373E-10f;
    a[58] =	-0.0053855875f;
    a[59] =	-0.007778209f;
    a[60] =	-0.0071515352f;
    a[61] =	-0.004450674f;
    a[62] =	-0.0010558856f;
    a[63] =	0.0017731993f;
    a[64] =	0.003282209f;
    a[65] =	0.003344623f;
    a[66] =	0.0023420234f;
    a[67] =	9.0173044E-4f;
    a[68] =	-3.8172887E-4f;
    a[69] =	-0.0011395542f;
    a[70] =	-0.001290066f;
    a[71] =	-9.818495E-4f;
    a[72] =	-4.716546E-4f;
    a[73] =	3.1934875E-11f;
    a[74] =	2.8903902E-4f;
    a[75] =	3.6717512E-4f;
    a[76] =	2.9180944E-4f;
    a[77] =	1.5370155E-4f;
    a[78] =	3.0053454E-5f;
    a[79] =	-4.0164417E-5f;
    a[80] =	-5.6314686E-5f;
    a[81] =	-4.0276995E-5f;
    a[82] =	-1.7250879E-5f;

    /*
     * Kaiser Window FIR Filter
     * Passband: 0.0 - 1350.0 Hz
     * modulation freq: 1650Hz
     * Order: 88
     * Transition band: 500.0 Hz
     * Stopband attenuation: 60.0 dB
     */

    b[0] =	-2.081541E-4f;
    b[1] =	-3.5587244E-4f;
    b[2] =	-5.237722E-5f;
    b[3] =	-1.00883444E-4f;
    b[4] =	-8.27162E-4f;
    b[5] =	-7.391658E-4f;
    b[6] =	9.386093E-5f;
    b[7] =	-6.221307E-4f;
    b[8] =	-0.0019506976f;
    b[9] =	-8.508009E-4f;
    b[10] =	2.8596455E-4f;
    b[11] =	-0.002028003f;
    b[12] =	-0.003321186f;
    b[13] =	-2.7830937E-4f;
    b[14] =	2.7148606E-9f;
    b[15] =	-0.004654892f;
    b[16] =	-0.0041854046f;
    b[17] =	0.001115112f;
    b[18] =	-0.0017027275f;
    b[19] =	-0.008291345f;
    b[20] =	-0.0034240147f;
    b[21] =	0.0027767413f;
    b[22] =	-0.005873899f;
    b[23] =	-0.011811939f;
    b[24] =	-2.075215E-8f;
    b[25] =	0.003209243f;
    b[26] =	-0.0131212445f;
    b[27] =	-0.013072912f;
    b[28] =	0.0064319638f;
    b[29] =	1.0081245E-8f;
    b[30] =	-0.023050211f;
    b[31] =	-0.009034872f;
    b[32] =	0.015074444f;
    b[33] =	-0.010180626f;
    b[34] =	-0.034043692f;
    b[35] =	0.004729156f;
    b[36] =	0.024004854f;
    b[37] =	-0.033643555f;
    b[38] =	-0.043601833f;
    b[39] =	0.04075407f;
    b[40] =	0.03076061f;
    b[41] =	-0.10492244f;
    b[42] =	-0.049181364f;
    b[43] =	0.30635652f;
    b[44] =	0.5324795f;
    b[45] =	0.30635652f;
    b[46] =	-0.049181364f;
    b[47] =	-0.10492244f;
    b[48] =	0.03076061f;
    b[49] =	0.04075407f;
    b[50] =	-0.043601833f;
    b[51] =	-0.033643555f;
    b[52] =	0.024004854f;
    b[53] =	0.004729156f;
    b[54] =	-0.034043692f;
    b[55] =	-0.010180626f;
    b[56] =	0.015074444f;
    b[57] =	-0.009034872f;
    b[58] =	-0.023050211f;
    b[59] =	1.0081245E-8f;
    b[60] =	0.0064319638f;
    b[61] =	-0.013072912f;
    b[62] =	-0.0131212445f;
    b[63] =	0.003209243f;
    b[64] =	-2.075215E-8f;
    b[65] =	-0.011811939f;
    b[66] =	-0.005873899f;
    b[67] =	0.0027767413f;
    b[68] =	-0.0034240147f;
    b[69] =	-0.008291345f;
    b[70] =	-0.0017027275f;
    b[71] =	0.001115112f;
    b[72] =	-0.0041854046f;
    b[73] =	-0.004654892f;
    b[74] =	2.7148606E-9f;
    b[75] =	-2.7830937E-4f;
    b[76] =	-0.003321186f;
    b[77] =	-0.002028003f;
    b[78] =	2.8596455E-4f;
    b[79] =	-8.508009E-4f;
    b[80] =	-0.0019506976f;
    b[81] =	-6.221307E-4f;
    b[82] =	9.386093E-5f;
    b[83] =	-7.391658E-4f;
    b[84] =	-8.27162E-4f;
    b[85] =	-1.00883444E-4f;
    b[86] =	-5.237722E-5f;
    b[87] =	-3.5587244E-4f;
    b[88] =	-2.081541E-4f;

    /*
     * Kaiser Window FIR Filter
     *
     * Filter type: Q-filter
     * Passband: 0.0 - 1350.0 Hz
     * modulation freq: 1650Hz
     *  with +90 degree pahse shift
     * Order: 88
     * Transition band: 500.0 Hz
     * Stopband attenuation: 60.0 dB
     */

    c[0] =	6.767926E-5f;
    c[1] =	-2.1822347E-4f;
    c[2] =	-3.3091355E-4f;
    c[3] =	1.1819744E-4f;
    c[4] =	2.1773627E-9f;
    c[5] =	-8.6602167E-4f;
    c[6] =	-5.9300865E-4f;
    c[7] =	3.814961E-4f;
    c[8] =	-6.342388E-4f;
    c[9] =	-0.00205537f;
    c[10] =	-5.616135E-4f;
    c[11] =	4.8721067E-4f;
    c[12] =	-0.002414588f;
    c[13] =	-0.003538588f;
    c[14] =	-2.7166707E-9f;
    c[15] =	-3.665928E-4f;
    c[16] =	-0.0057645175f;
    c[17] =	-0.004647882f;
    c[18] =	8.681589E-4f;
    c[19] =	-0.0034366683f;
    c[20] =	-0.010545009f;
    c[21] =	-0.0045342376f;
    c[22] =	9.309649E-4f;
    c[23] =	-0.01009504f;
    c[24] =	-0.015788108f;
    c[25] =	-0.0027427748f;
    c[26] =	-0.0020795742f;
    c[27] =	-0.021347176f;
    c[28] =	-0.019808702f;
    c[29] =	-4.1785704E-9f;
    c[30] =	-0.011752444f;
    c[31] =	-0.037658f;
    c[32] =	-0.020762002f;
    c[33] =	8.017756E-4f;
    c[34] =	-0.03406628f;
    c[35] =	-0.060129803f;
    c[36] =	-0.01745214f;
    c[37] =	-0.008082453f;
    c[38] =	-0.08563026f;
    c[39] =	-0.09845453f;
    c[40] =	-0.010001372f;
    c[41] =	-0.06433928f;
    c[42] =	-0.31072536f;
    c[43] =	-0.35893586f;
    c[44] =	0.0f;
    c[45] =	0.35893586f;
    c[46] =	0.31072536f;
    c[47] =	0.06433928f;
    c[48] =	0.010001372f;
    c[49] =	0.09845453f;
    c[50] =	0.08563026f;
    c[51] =	0.008082453f;
    c[52] =	0.01745214f;
    c[53] =	0.060129803f;
    c[54] =	0.03406628f;
    c[55] =	-8.017756E-4f;
    c[56] =	0.020762002f;
    c[57] =	0.037658f;
    c[58] =	0.011752444f;
    c[59] =	4.1785704E-9f;
    c[60] =	0.019808702f;
    c[61] =	0.021347176f;
    c[62] =	0.0020795742f;
    c[63] =	0.0027427748f;
    c[64] =	0.015788108f;
    c[65] =	0.01009504f;
    c[66] =	-9.309649E-4f;
    c[67] =	0.0045342376f;
    c[68] =	0.010545009f;
    c[69] =	0.0034366683f;
    c[70] =	-8.681589E-4f;
    c[71] =	0.004647882f;
    c[72] =	0.0057645175f;
    c[73] =	3.665928E-4f;
    c[74] =	2.7166707E-9f;
    c[75] =	0.003538588f;
    c[76] =	0.002414588f;
    c[77] =	-4.8721067E-4f;
    c[78] =	5.616135E-4f;
    c[79] =	0.00205537f;
    c[80] =	6.342388E-4f;
    c[81] =	-3.814961E-4f;
    c[82] =	5.9300865E-4f;
    c[83] =	8.6602167E-4f;
    c[84] =	-2.1773627E-9f;
    c[85] =	-1.1819744E-4f;
    c[86] =	3.3091355E-4f;
    c[87] =	2.1822347E-4f;
    c[88] =	-6.767926E-5f;

    fprintf(stderr,"%s: Decimator creation\n",PROGRAMID);
    d = new Decimator(a, 83, decimation_factor);

    fprintf(stderr,"%s: I/Q FIR Hilbert Filter\n",PROGRAMID);

    iFilter = new FIRFilter(b,89);
    qFilter = new FIRFilter(c,89);

    fprintf(stderr,"%s::SSB() Object creation completed\n",PROGRAMID);
  
}
//*--------------------------------------------------------------------------------------------------
// decimate: take one sample and compute one filterd output
//*--------------------------------------------------------------------------------------------------
int SSB::generate(short *input,int len,float* I,float* Q) {

      d->decimate(input,len,I,Q);                      //Input is fs=48KHz decimate by 8
  int numSamplesLow = len / decimation_factor;              //Number of samples is much lower now
      iFilter->do_filter(I,numSamplesLow);                  //I Filter just unity gain to delay no phase change
      qFilter->do_filter(Q,numSamplesLow);                  //Q Filter also unity gain but with a 90 degree shift
      if (agc.active==true) {				    //If AGC is active then perform AGC adjustment
         agc.g->computeagc(I,Q,I,Q,numSamplesLow,agc.rate,agc.reference,agc.max_gain,agc.gain);
      }
      return numSamplesLow;  
}
