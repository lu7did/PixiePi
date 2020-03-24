/*
 * Pi4D.cpp
 * Raspberry Pi based USB experimental SSB Generator for digital modes (mainly WSPR and FT8)
 * Experimental version largely modelled after Generator.java by Takafumi INOUE (JI3GAB) and librpitx by Evariste  (F5OEO)
 * This program tries to mimic the behaviour of simple DSB transceivers used to operate low signal digital modes such as
 * WSPR or FT8, however, when paired with the PixiePi hardware it does receive with a double conversion receiver put it does
 * transmit using SSB (USB).
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
#include "/home/pi/PixiePi/src/lib/SSB.h" 
#include "/home/pi/librpitx/src/librpitx.h"


//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="Pi4D";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";
enum  {typeiq_i16,typeiq_u8,typeiq_float,typeiq_double,typeiq_carrier};

typedef unsigned char byte;
typedef bool boolean;

bool running=true;
long int TVOX=0;
byte MSW=0;

float  agc_rate=0.25;
float  agc_reference=1.0;
float  agc_max_gain=5.0;
float  agc_current_gain=1.0;

SSB*   usb;
float* Ibuffer;
float* Qbuffer;

float  gain=1.0;

int numSamples=0;
int numSamplesLow=0;
int exitrecurse=0;
int mode = 0;
int bufferLengthInBytes;
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
void timer_start(std::function<void(void)> func, unsigned int interval)
{
  std::thread([func, interval]()
  {
    while (getWord(MSW,RUN)==true)
    {
      auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
      func();
      std::this_thread::sleep_until(x);
    }
  }).detach();
}

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
-a            activate AGC\n\
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
	int   InputType=typeiq_i16;
	int   Decimation=1;
        timer_start(timer_exec,100);

        int   m=1;
	int   SR=48000;
	int   FifoSize=IQBURST*4;
        InputType=typeiq_float;



        fprintf(stderr,"%s %s [%s]\n",PROGRAMID,PROG_VERSION,PROG_BUILD);

        fprintf(stderr,"%s:main(): SSB controller generation\n",PROGRAMID); 

        usb=new SSB(&gain);

        usb->agc.reference=1.0;
	usb->agc.max_gain=5.0;
	usb->agc.rate=0.25;
        usb->agc.active=false;

	while(1)
	{
		ax = getopt(argc, argv, "i:f:s:h:cat:");
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
		case 'a': // loop mode
			usb->agc.active=true;
			fprintf(stderr,"%s: AGC activated\n",PROGRAMID);
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

        // Standard input definition

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
        Ibuffer =(float*)malloc(IQBURST*sizeof(short)*2);
        Qbuffer =(float*)malloc(IQBURST*sizeof(short)*2);

        fprintf(stderr,"%s:main(): FIFO buffer creation\n",PROGRAMID); 
	std::complex<float> CIQBuffer[IQBURST];	
        int numBytesRead=0;

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
					  int numSamplesLow=usb->generate(buffer_i16,nbread,Ibuffer,Qbuffer);
					  for(int i=0;i<numSamplesLow;i++)
					  {
 				            CIQBuffer[CplxSampleNumber++]=std::complex<float>(Ibuffer[i],Qbuffer[i]);
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

