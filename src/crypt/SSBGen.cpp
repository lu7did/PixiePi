/*
 * SSBGen.cpp
 * Raspberry Pi based USB experimental SSB Generator
 * Experimental version largely modelled after Generator.java by Takafumi INOUE (JI3GAB)
 *---------------------------------------------------------------------
 * This program operates as a controller for a Raspberry Pi to control
 * a Pixie transceiver hardware.
 * Project at http://www.github.com/lu7did/PixiePi
 *---------------------------------------------------------------------
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *    Adafruit's python code for CharLCDPlate
 *    tune.cpp from rpitx package by Evariste Courjaud F5OEO
 *    sendiq.cpp from rpitx package (also) by Evariste Coujaud (F5EOE)
 *    wiringPi library (git clone git://git.drogon.net/wiringPi)
 *    iambic-keyer (https://github.com/n1gp/iambic-keyer)
 *    log.c logging facility by  rxi https://github.com/rxi/log.c
 *    minIni configuration management package by Compuphase https://github.com/compuphase/minIni/tree/master/dev
 *    tinyalsa https://github.com/tinyalsa/tinyalsa
 * Also libraries
 *    librpitx by  Evariste Courjaud (F5EOE)
 *    libcsdr by Karol Simonyi (HA7ILM) https://github.com/compuphase/minIni/tree/master/dev
 *
 * ---------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */


#define PROGRAM_VERSION "1.0"
#define PTT 0B00000001
#define VOX 0B00000010
#define RUN 0B00000100
#define MAX_SAMPLERATE 200000
#define BUFFERSIZE      96000
#define IQBURST          4000



#include <unistd.h>
#include "stdio.h"
#include <cstring>
#include <signal.h>
#include <stdlib.h>
#include <cstdlib>      // for std::rand() and std::srand()
#include <ctime>        // for std::time()
#include <iostream>
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
#include <limits.h>
#include "/home/pi/PixiePi/src/lib/Decimator.h" 
#include "/home/pi/PixiePi/src/lib/Interpolator.h" 
#include "/home/pi/PixiePi/src/lib/FIRFilter.h" 
#include "/home/pi/PixiePi/src/lib/AGControl.h" 
#include "/home/pi/librpitx/src/librpitx.h"


//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="SSGGen";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";

typedef unsigned char byte;
typedef bool boolean;

bool running=true;
long int TVOX=0;
byte MSW=0;

float* a;
float* b;
float* c;
float* r;

float* i1;
float* i2;
float* i3;
float* iLow;
float* qLow;

float* iOut;
float* qOut;
AGControl*  agc;

float  agc_rate=0.25;
float  agc_reference=1.0;
float  agc_max_gain=5.0;
float  agc_current_gain=1.0;

int numSamples=0;
int numSamplesLow=0;
int exitrecurse=0;
int mode = 0;
int bufferLengthInBytes;
    
FIRFilter    *iFilter;
FIRFilter    *qFilter;

Decimator    *d1;

int decimation_factor = 8;
short *buffer_i16;

std::complex<float> CIQBuffer[IQBURST];	
//--------------------------[System Word Handler]---------------------------------------------------
// getSSW Return status according with the setting of the argument bit onto the SW
//--------------------------------------------------------------------------------------------------
bool getWord (unsigned char SysWord, unsigned char v) {

  return SysWord & v;

}
//--------------------------------------------------------------------------------------------------
// setSSW Sets a given bit of the system status Word (SSW)
//--------------------------------------------------------------------------------------------------
void setWord(unsigned char* SysWord,unsigned char v, bool val) {

  *SysWord = ~v & *SysWord;
  if (val == true) {
    *SysWord = *SysWord | v;
  }

}

//--------------------------------------------------------------------------------------------------
// timer_exec 
// timer management
//--------------------------------------------------------------------------------------------------
void timer_exec()
{
  if (TVOX!=0) {
     TVOX--;
     if(TVOX==0) {
       printf("VOX turned off\n");
     }
  }
}

//---------------------------------------------------------------------------
// Timer handler function
//---------------------------------------------------------------------------
//void timer_start(std::function<void(void)> func, unsigned int interval)
//{
//  std::thread([func, interval]()
//  {
//    while (getWord(MSW,RUN)==true)
//    {
//      auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
//      func();
//      std::this_thread::sleep_until(x);
//    }
//  }).detach();
//}

//---------------------------------------------------------------------------------
// Print usage
//---------------------------------------------------------------------------------
void print_usage(void)
{
fprintf(stderr,"%s %s [%s]\n\
Usage: [-i File Input][-s Samplerate][-l] [-f Frequency] [-h Harmonic number] \n\
-i            path to File Input \n\
-s            SampleRate 10000-250000 \n\
-f float      central frequency Hz(50 kHz to 1500 MHz),\n\
-c            carrier mode only\n\
-h            Use harmonic number n\n\
-?            help (this help).\n\
\n",PROGRAMID,PROG_VERSION,PROG_BUILD);


} /* end function print_usage */
//---------------------------------------------------------------------------------
// terminate
//---------------------------------------------------------------------------------

static void terminate(int num)
{
    running=false;
    fprintf(stderr,"%s: Caught TERM signal(%x) - Terminating \n",PROGRAMID,num);
    if (exitrecurse > 0) {
       fprintf(stderr,"%s: Recursive trap - Force termination \n",PROGRAMID);
       exit(16);
    }
    exitrecurse++;

}
//---------------------------------------------------------------------------------
// init()
//---------------------------------------------------------------------------------
void init() {

  fprintf(stderr,"%s:init(): Filter initialization\n",PROGRAMID);
  /*
     * Kaiser Window FIR Filter
     * Passband: 0.0 - 3000.0 Hz
     * Order: 83
     * Transition band: 3000.0 Hz
     * Stopband attenuation: 80.0 dB
     */
/*
    a[0]= -0.015700f;
    a[1]=  0.000000f;
    a[2]=  0.270941f;
    a[3]=  0.500000f;
    a[4]=  0.270941f;
    a[5]=  0.000000f;
    a[6]= -0.015700f;
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

    fprintf(stderr,"%s: init() Decimator coefficient array creation\n",PROGRAMID);

    //r[0]  = 0.020062601640232477f;
    //r[1]  = 0.05020111275666616f;
    //r[2]  = 0.09412637661715571f;
    //r[3]  = 0.14118664858725588f;
    //r[4]  = 0.17765552108476357f;
    //r[5]  = 0.19141793392484863f;
    //r[6]  = 0.17765552108476357f;
    //r[7]  = 0.14118664858725588f;
    //r[8]  = 0.094126376617571f;
    //r[9]  = 0.05020111275666616f;
    //r[10] = 0.020062601640232477f;
    //r[11] = 0.0;
    //r[12] = 0.0;
    //r[13] = 0.0;

    fprintf(stderr,"%s: init() Decimator creation\n",PROGRAMID);

    //d1 = new Decimator(a, 7, decimation_factor);
    d1 = new Decimator(a, 83, decimation_factor);

    fprintf(stderr,"%s: init() I/Q FIR Filter (Hilbert)\n",PROGRAMID);

    iFilter = new FIRFilter(b,89);
    qFilter = new FIRFilter(c,89);
}
//---------------------------------------------------------------------------------
// main 
//---------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	int   ax;
	int   anyargs = 1;
	float SetFrequency=7080000;
	float SampleRate=48000;



	//bool loop_mode_flag=false;

	char* FileName=NULL;
	int   Harmonic=1;
	enum  {typeiq_i16,typeiq_u8,typeiq_float,typeiq_double,typeiq_carrier};
	int   InputType=typeiq_i16;
	int   Decimation=1;
        //timer_start(timer_exec,100);

        int   m=1;
	int   SR=48000;
	int   FifoSize=IQBURST*4;
        InputType=typeiq_float;


        fprintf(stderr,"%s %s [%s]\n",PROGRAMID,PROG_VERSION,PROG_BUILD);

	while(1)
	{
		ax = getopt(argc, argv, "i:f:s:h:ct:");
	
		if(ax == -1) 
		{
			if(anyargs) break;
			else ax='h'; //print usage and exit
		}
		anyargs = 1;	

		switch(ax)
		{
		case 'i': // File name
			FileName = optarg;
			fprintf(stderr,"%s: Filename(%s)\n",PROGRAMID,FileName);
			break;
		case 'c': // loop mode
			InputType=typeiq_carrier;
			fprintf(stderr,"%s: Carrier mode only\n",PROGRAMID);
			break;
		case 'f': // Frequency
			SetFrequency = atof(optarg);
			fprintf(stderr,"%s: Frequency(%10f)\n",PROGRAMID,SetFrequency);
			break;
		case 's': // SampleRate (Only needeed in IQ mode)
			SampleRate = atoi(optarg);
			fprintf(stderr,"%s: SampleRate(%10f)\n",PROGRAMID,SampleRate);
			if(SampleRate>MAX_SAMPLERATE) 
			{
				for(int i=2;i<12;i++) //Max 10 times samplerate
				{
					if(SampleRate/i<MAX_SAMPLERATE) 
					{
						SampleRate=SampleRate/i;
						Decimation=i;
						break;
					}
				}
				if(Decimation==1)
				{
					 fprintf(stderr,"%s: SampleRate too high : >%d sample/s",PROGRAMID,10*MAX_SAMPLERATE);
					 exit(1);
				} 
				else
				{
					fprintf(stderr,"%s: Warning samplerate too high, decimation by %d will be performed",PROGRAMID,Decimation);	 
				}
			};
			break;
		case 'h': // help
			Harmonic=atoi(optarg);
			break;
		case -1:
        	break;
		case '?':
			if (isprint(optopt) )
 			{
 			   fprintf(stderr, "%s: unknown option `-%c'.\n",PROGRAMID,optopt);
 			}
			else
			{
				fprintf(stderr, "%s: unknown option character `\\x%x'.\n",PROGRAMID,optopt);
			}
			print_usage();
			exit(1);
			break;
		default:
			print_usage();
			exit(1);
			break;
		}/* end switch a */
	}/* end while getopt() */

	if(FileName==NULL) {fprintf(stderr,"%s: Need an input\n",PROGRAMID);exit(1);}

        fprintf(stderr,"%s:main(): Trap handler initialization\n",PROGRAMID);

	for (int i = 0; i < 64; i++) {
           struct sigaction sa;
           std::memset(&sa, 0, sizeof(sa));
           sa.sa_handler = terminate;
           sigaction(i, &sa, NULL);
        }

        setWord(&MSW,RUN,true);
        setWord(&MSW,VOX,false);
        float gain=SHRT_MAX;

        fprintf(stderr,"%s:main(): Standard input definition\n",PROGRAMID);

	FILE *iqfile=NULL;
	if(strcmp(FileName,"-")==0) {
   	  iqfile=fopen("/dev/stdin","rb");
	} else {
	  iqfile=fopen(FileName	,"rb");
        }

	if (iqfile==NULL) 
	{
   	   printf("%s: input file issue\n",PROGRAMID);
	   exit(0);
	}

        fprintf(stderr,"%s: Input from %s\n",PROGRAMID,FileName);

// define I/Q object

        fprintf(stderr,"%s:main(): RF I/Q generator object creation\n",PROGRAMID);

	iqdmasync iqtest(SetFrequency,SampleRate,14,FifoSize,MODE_IQ);
	iqtest.SetPLLMasterLoop(3,4,0);

//*********************************************************************************************
//*--- This is the experimental template on how to change frequency without stop
//        iqtest.clkgpio::disableclk(4);
//        usleep(1000);
//        iqtest.clkgpio::SetAdvancedPllMode(true);
//        iqtest.clkgpio::SetCenterFrequency(SetFrequency+600,SampleRate);
//        iqtest.clkgpio::SetFrequency(0);
// 	  iqtest.clkgpio::enableclk(4);
//        usleep(1000);  
//**********************************************************************************************

//generate buffer areas

        fprintf(stderr,"%s:main(): Memory buffer creation\n",PROGRAMID);
        buffer_i16 =(short*)malloc(SR*sizeof(short)*2);

        fprintf(stderr,"%s:main(): Filter coefficiente buffer creation\n",PROGRAMID); 
        a=(float*) malloc(96*sizeof(float));
        b=(float*) malloc(96*sizeof(float));
        c=(float*) malloc(96*sizeof(float));

        fprintf(stderr,"%s:main(): Decimation buffer creation\n",PROGRAMID); 
        i1=(float*) malloc(SR*sizeof(float)*2);


        fprintf(stderr,"%s:main(): I/Q buffer creation\n",PROGRAMID); 
        iLow=(float*) malloc(IQBURST*sizeof(float));
        qLow=(float*) malloc(IQBURST*sizeof(float));

        fprintf(stderr,"%s:main(): FIFO buffer creation\n",PROGRAMID); 
	std::complex<float> CIQBuffer[IQBURST];	
        int numBytesRead=0;

        fprintf(stderr,"%s:main(): AGControl object creation\n",PROGRAMID); 
        iOut=(float*) malloc(IQBURST*sizeof(float));
        qOut=(float*) malloc(IQBURST*sizeof(float));


        agc=new AGControl();
        fprintf(stderr,"%s:main(): Filter initialization\n",PROGRAMID); 
        init();

        fprintf(stderr,"%s: Starting operations SHRT_MAX(%d)\n",PROGRAMID,(int)SHRT_MAX);
	while(running)
	{
			int CplxSampleNumber=0;
			switch(InputType)
			{
				case typeiq_float:
				{
					int nbread=fread(buffer_i16,sizeof(short),1024,iqfile);
					if(nbread>0)
					{
//*---- Adaptación de Generator SSB
                                        //Decimation to 12 KHz

                                          //d1->decimate(i1, nbread, iLow);   //nbread/4=1024
                                          d1->decimate(buffer_i16, nbread, gain, iLow, qLow);   //nbread/4=1024
			                  int numSamplesLow = nbread / decimation_factor;

					//I/Q Filter
					// I signal is passed thru a band pass filter
                                          iFilter->do_filter(iLow,numSamplesLow);

					// Q signal is passed through a BPF which as the same amplitude response 
					// than the I signal but also has a 90 degree phase shifter built in
					  qFilter->do_filter(qLow,numSamplesLow);

//*---- Fin de Adaptación Generador SSB las muestras deben quedar en CIQBuffer
                                          agc->computeagc(iLow,qLow,iOut,qOut,numSamplesLow,agc_rate,agc_reference,agc_max_gain,&agc_current_gain);
					  for(int i=0;i<numSamplesLow;i++)
					  {
 				            //CIQBuffer[CplxSampleNumber++]=std::complex<float>(iLow[i*2],qLow[i*2]);
 				            //CIQBuffer[CplxSampleNumber++]=std::complex<float>(iLow[i],qLow[i]);
 				            CIQBuffer[CplxSampleNumber++]=std::complex<float>(iOut[i],qOut[i]);
					  }


					} else {
					  printf("%s: End of file\n",PROGRAMID);
   					  running=false;
					}
				}
				break;	
		}
		iqtest.SetIQSamples(CIQBuffer,CplxSampleNumber,Harmonic);

	}
	iqtest.stop();
}

