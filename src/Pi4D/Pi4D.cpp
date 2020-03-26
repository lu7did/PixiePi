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
#define MAX_SAMPLERATE 200000
#define BUFFERSIZE      96000
#define IQBURST          4000
#define VOX_TIMER        20



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
//#include <pigpio.h>
#include <wiringPi.h>
//#include <wiringPiI2C.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <limits.h>
#include "/home/pi/PixiePi/src/lib/SSB.h" 
#include "/home/pi/PixiePi/src/lib/CAT817.h" 
#include "/home/pi/PixiePi/src/pixie/pixie.h" 
#include "/home/pi/librpitx/src/librpitx.h"


//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="Pi4D";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";
enum  {typeiq_i16,typeiq_u8,typeiq_float,typeiq_double,typeiq_carrier};

typedef unsigned char byte;
typedef bool boolean;


// --- Transceiver control structure
long int TVOX=0;

byte MSW=0;
byte trace=0x00;


// --- CAT object
void CATchangeMode();
void CATchangeFreq();
void CATchangeStatus();

CAT817* cat;
byte FT817;

float SampleRate=6000;
float ppm=1000.0;
char  port[80];
long  catbaud=CATBAUD;


byte gpio=GPIO04;

iqdmasync* iqtest=nullptr;

int   ax;
int   anyargs = 1;
float SetFrequency=7080000;

//bool loop_mode_flag=false;

char* FileName=NULL;
int   Harmonic=1;
int   InputType=typeiq_i16;
int   Decimation=1;

int   m=1;
int   SR=48000;
int   FifoSize=IQBURST*4;


// --- SSB generation objects

float  agc_rate=0.25;
float  agc_reference=1.0;
float  agc_max_gain=5.0;
float  agc_current_gain=1.0;

SSB*   usb;
float* Ibuffer;
float* Qbuffer;

float  gain=1.0;

int    numSamples=0;
int    numSamplesLow=0;
int    exitrecurse=0;
int    bufferLengthInBytes;
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

//--------------------------------------------------------------------------------------------
// set_PTT
// Manage the PTT of the transceiver (can be used from the keyer or elsewhere
//--------------------------------------------------------------------------------------------
void setPTT(bool statePTT) {

//---------------------------------*
//          PTT Activated          *
//---------------------------------*
    if (statePTT==true) {

//--- if SPLIT swap VFO AND if also CW shift the carrier by vfoshift[current VFO]

       fprintf(stderr,"%s:setPTT() PTT On PTT(%s)\n",PROGRAMID,(getWord(MSW,PTT) ? "true" : "false"));
       setWord(&cat->FT817,PTT,true);
       setWord(&MSW,PTT,true);

       //if (iqtest != nullptr) {
       //   iqtest->stop();
       //   delete(iqtest);
       //   iqtest=nullptr;
       //   usleep(100000);
       //}

       fprintf(stderr,"%s turning GPIO PTT on\n",PROGRAMID);
       //if (wiringPiSetup()<0) {
       //    fprintf(stderr,"%s: Unable to setup gpio when PTT On\n",PROGRAMID);
       //    exit(16);
       //} 
       //pinMode (KEYER_OUT_GPIO, OUTPUT) ;
       digitalWrite(5,HIGH);

       //system("gpio mode \"12\" out");
       //system("gpio -g write \"12\" 1");

       //iqtest=new iqdmasync(SetFrequency,SampleRate,14,FifoSize,MODE_IQ);
       //iqtest->SetPLLMasterLoop(3,4,0);
       usleep(10000);
       return;
    } 

//---------------------------------*
//          PTT Inactivated        *
//---------------------------------*
    fprintf(stderr,"%s:setPTT() PTT Off PTT(%s)\n",PROGRAMID,(getWord(MSW,PTT) ? "true" : "false"));
    setWord(&cat->FT817,PTT,false);
    setWord(&MSW,PTT,false);
    //fprintf(stderr,"%s flags for PTT has been set\n",PROGRAMID);

    //if (iqtest != nullptr) {
       //fprintf(stderr,"%s deallocating objects\n",PROGRAMID);
    //   iqtest->stop();
    //   delete(iqtest);
    //   iqtest=nullptr;
       //fprintf(stderr,"%s deallocatED objects\n",PROGRAMID);
    //   usleep(10000);
    //}

    //fprintf(stderr,"%s About to gpio out 19 off\n",PROGRAMID);
    //if (wiringPiSetup()<0) {
    //   fprintf(stderr,"%s: Unable to setup gpio when PTT On\n",PROGRAMID);
    //   exit(16);
    //} 
    //fprintf(stderr,"%s turning GPIO PTT Off\n",PROGRAMID);
    //pinMode (KEYER_OUT_GPIO, OUTPUT) ;
    digitalWrite(5,LOW);

    //system("gpio mode \"12\" out");
    //system("gpio -g write \"12\" 0");
    //fprintf(stderr,"%s Have set gpio out 12 off\n",PROGRAMID);


}
//---------------------------------------------------------------------------
// CATchangeFreq()
// CAT Callback when frequency changes
//---------------------------------------------------------------------------
void CATchangeFreq() {

  fprintf(stderr,"%s::CATchangeFreq() cat.SetFrequency(%d) SetFrequency(%d)\n",PROGRAMID,(int)cat->SetFrequency,(int)SetFrequency);
  if ((cat->SetFrequency<VFO_START) || (cat->SetFrequency>VFO_END)) {
     fprintf(stderr,"%s::CATchangeFreq() cat.SetFrequency(%d) out of band is rejected\n",PROGRAMID,(int)cat->SetFrequency);
     cat->SetFrequency=SetFrequency;
     return;
  }

  SetFrequency=cat->SetFrequency;

  if (iqtest != nullptr) {
     iqtest->clkgpio::disableclk(GPIO04);
     iqtest->clkgpio::SetAdvancedPllMode(true);
     iqtest->clkgpio::SetCenterFrequency(SetFrequency,SampleRate);
     iqtest->clkgpio::SetFrequency(0);
     iqtest->clkgpio::enableclk(GPIO04);
  }
  fprintf(stderr,"%s::CATchangeFreq() Frequency set to SetFrequency(%d)\n",PROGRAMID,(int)SetFrequency);
}
//-----------------------------------------------------------------------------------------------------------
// CATchangeMode
// Validate the new mode is a supported one
// At this point only CW,CWR,USB and LSB are supported
//-----------------------------------------------------------------------------------------------------------
void CATchangeMode() {

  fprintf(stderr,"%s::CATchangeMode() cat.MODE(%d)\n",PROGRAMID,cat->MODE);

  if (cat->MODE == MUSB) {
     fprintf(stderr,"%s::CATchangeMode() cat.MODE(%d) accepted\n",PROGRAMID,cat->MODE);
     return;
  }

  fprintf(stderr,"%s::CATchangeMode() cat.MODE(%d) invalid only USB supported\n",PROGRAMID,cat->MODE);
  cat->MODE=MUSB;
  return;

}

//------------------------------------------------------------------------------------------------------------
// CATchangeStatus
// Detect which change has been produced and operate accordingly
//------------------------------------------------------------------------------------------------------------
void CATchangeStatus() {

  fprintf(stderr,"%s::CATchangeStatus() cat.FT817(%d)\n",PROGRAMID,cat->FT817);

  if (getWord(cat->FT817,PTT) != getWord(FT817,PTT)) {        // PTT Changed
     fprintf(stderr,"%s::CATchangeStatus() cat.FT817(%d) PTT changed to %s\n",PROGRAMID,cat->FT817,getWord(cat->FT817,PTT) ? "true" : "false");
     setPTT(getWord(cat->FT817,PTT));
  }


  if (getWord(cat->FT817,RIT) != getWord(FT817,RIT)) {        // RIT Changed
     fprintf(stderr,"%s::CATchangeStatus() cat.FT817(%d) RIT changed to %s ignored\n",PROGRAMID,cat->FT817,getWord(cat->FT817,RIT) ? "true" : "false");
  }

  if (getWord(cat->FT817,LOCK) != getWord(FT817,LOCK)) {      // LOCK Changed
     fprintf(stderr,"%s::CATchangeStatus() cat.FT817(%d) LOCK changed to %s ignored\n",PROGRAMID,cat->FT817,getWord(cat->FT817,LOCK) ? "true" : "false");
  }

  if (getWord(cat->FT817,SPLIT) != getWord(FT817,SPLIT)) {    // SPLIT mode Changed
     fprintf(stderr,"%s::CATchangeStatus() cat.FT817(%d) SPLIT changed to %s ignored\n",PROGRAMID,cat->FT817,getWord(cat->FT817,SPLIT) ? "true" : "false");
  }

  if (getWord(cat->FT817,VFO) != getWord(FT817,VFO)) {        // VFO Changed
     fprintf(stderr,"%s::CATchangeStatus() cat.FT817(%d) VFO changed to %s ignored\n",PROGRAMID,cat->FT817,getWord(cat->FT817,VFO) ? "VFO A" : "VFO B");
  }

  FT817=cat->FT817;
  return;

}
//--------------------------------------------------------------------------------------------------
// Stubs for callback not implemented yed
//--------------------------------------------------------------------------------------------------

void CATgetRX() {
}
void CATgetTX() {
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
       fprintf(stderr,"%s::timer_exec() VOX timer expiration event\n",PROGRAMID);
       setWord(&MSW,VOX,true);
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
    setWord(&MSW,RUN,false);
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

        fprintf(stderr,"%s %s [%s]\n",PROGRAMID,PROG_VERSION,PROG_BUILD);

        InputType=typeiq_float;
        sprintf(port,"/tmp/ttyv1");
        timer_start(timer_exec,100);

        setWord(&MSW,RUN,true);
        setWord(&MSW,VOX,false);

// ---- Initialize GPIO

        //if(gpioInitialise()<0) {
        //   fprintf(stderr,"%s gpio initialization failure\n",PROGRAMID);
        //   return -1;
        //}


// ---- Turn cooler on

        //gpioSetMode(COOLER_GPIO, PI_OUTPUT);
        //gpioWrite(COOLER_GPIO, 1);
        //usleep(100000);


        //system("gpio mode \"12\" out");
        //system("gpio -g write \"12\" 0");

        //system("gpio mode \"12\" out");
        //system("gpio -g write \"12\" 0");

        //gpioSetMode(KEYER_OUT_GPIO, PI_OUTPUT);
        //usleep(100000);


        if (wiringPiSetup () < 0) {
           fprintf(stderr,"%s: Unable to setup wiringPi error(%s)\n",PROGRAMID,strerror(errno));
           exit(16);
        }
        fprintf(stderr,"%s:main(): wiringPi controller setup completed\n",PROGRAMID); 

        pinMode(5,OUTPUT);
        digitalWrite(5,LOW);
        //if (gpioInitialise()<0) {
        //   fprintf(stderr,"%s: Unable to setup gpio\n",PROGRAMID);
        //   return 1;
        //}

        //gpioSetMode(COOLER_GPIO,KEYER_OUT_GPIO);
        //gpioWrite(KEYER_OUT_GPIO, 0);
        //usleep(100000);

        //gpioSetMode(COOLER_GPIO, PI_OUTPUT);
        //gpioWrite(COOLER_GPIO, 1);
        //usleep(100000);

//--------------------------------------------------------------------------------------------------
// SSB (USB) controller generation
//--------------------------------------------------------------------------------------------------

float   gain=1.0;

        usb=new SSB();
        usb->agc.reference=1.0;
	usb->agc.max_gain=2.0;
	usb->agc.rate=0.25;
        usb->agc.active=false;
	usb->agc.gain=&gain;

        fprintf(stderr,"%s:main(): SSB controller generation\n",PROGRAMID); 

//--------------------------------------------------------------------------------------------------
//      Argument parsting
//--------------------------------------------------------------------------------------------------
	while(1)
	{
		ax = getopt(argc, argv, "i:f:s:p:h:cat:");
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
                case 'p': //serial port
                        sprintf(port,optarg);
                        fprintf(stderr,"%s Serial Port(%s)",PROGRAMID, port);
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

//--------------------------------------------------------------------------------------------------
// Parse input file (normally /dev/stdin)
//--------------------------------------------------------------------------------------------------
	if(FileName==NULL) {fprintf(stderr,"%s: Need an input\n",PROGRAMID);exit(1);}

//--------------------------------------------------------------------------------------------------
// Setup trap handling
//--------------------------------------------------------------------------------------------------
        fprintf(stderr,"%s:main(): Trap handler initialization\n",PROGRAMID);

	for (int i = 0; i < 64; i++) {
           struct sigaction sa;
           std::memset(&sa, 0, sizeof(sa));
           sa.sa_handler = terminate;
           sigaction(i, &sa, NULL);
        }

//--------------------------------------------------------------------------------------------------
// Setup CAT object
//
//--------------------------------------------------------------------------------------------------

        cat=new CAT817(CATchangeFreq,CATchangeStatus,CATchangeMode,CATgetRX,CATgetTX);

        FT817=0x00;
        cat->FT817=FT817;
        cat->POWER=7;
        cat->SetFrequency=SetFrequency;
        cat->MODE=MUSB;
        cat->TRACE=0x00;
        cat->open(port,CATBAUD);
        setWord(&cat->FT817,AGC,false);
        setWord(&cat->FT817,PTT,false);


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


        fprintf(stderr,"%s: I/O RF Object created\n",PROGRAMID);
        iqtest=new iqdmasync(SetFrequency,SampleRate,14,FifoSize,MODE_IQ);
        iqtest->SetPLLMasterLoop(3,4,0);

//generate buffer areas

        fprintf(stderr,"%s:main(): Memory buffer creation\n",PROGRAMID);
        buffer_i16 =(short*)malloc(SR*sizeof(short)*2);
        Ibuffer =(float*)malloc(IQBURST*sizeof(short)*2);
        Qbuffer =(float*)malloc(IQBURST*sizeof(short)*2);

        fprintf(stderr,"%s:main(): FIFO buffer creation\n",PROGRAMID); 
	std::complex<float> CIQBuffer[IQBURST];	
        int numBytesRead=0;

        fprintf(stderr,"%s: Starting operations\n",PROGRAMID);
        setWord(&MSW,RUN,true);

        float voxmin=usb->agc.max_gain;
        float voxmax=0.0;
        float voxlvl=voxmin;

        setPTT(false);

	while(getWord(MSW,RUN)==true)
	{
			int CplxSampleNumber=0;
  			cat->get();
			switch(InputType)
			{
				case typeiq_float:
				{
					int nbread=fread(buffer_i16,sizeof(short),1024,iqfile);
					if(nbread>0)
					{
					  int numSamplesLow=usb->generate(buffer_i16,nbread,Ibuffer,Qbuffer);
					  if (getWord(MSW,PTT)==true) {
					  for(int i=0;i<numSamplesLow;i++)
					  {
 				             CIQBuffer[CplxSampleNumber++]=std::complex<float>(Ibuffer[i],Qbuffer[i]);
					  }
					  } else {
					  numSamplesLow=1024/usb->decimation_factor;
					  for(int i=0;i<numSamplesLow;i++)
					  {
 				             CIQBuffer[CplxSampleNumber++]=std::complex<float>(0.0,0.0);
					  }
 					  }
					} else {
					  printf("%s: End of file\n",PROGRAMID);
                                          setWord(&MSW,RUN,false);
					}
				}
				break;	
		}
		//if (getWord(MSW,PTT)==true) {
		   iqtest->SetIQSamples(CIQBuffer,CplxSampleNumber,Harmonic);
                //}

// VOX analysis
          if (usb->agc.active==true) {
                if (gain>voxmax) {
		   voxmax=gain;
		   voxlvl=voxmax*0.90;
		   fprintf(stderr,"%s::main() VOX MAX level max(%8f) min(%8f) trig(%8f) gain(%8f) VOX(%s) PTT(%s)\n",PROGRAMID,voxmax,voxmin,voxlvl,gain,(getWord(MSW,VOX) ? "true" : "false"),(getWord(MSW,PTT) ? "true" : "false"));
                }

		if (gain<voxmin) {
		   voxmin=gain;
		   fprintf(stderr,"%s::main() VOX MIN level max(%8f) min(%8f) trig(%8f) gain(%8f) VOX(%s) PTT(%s)\n",PROGRAMID,voxmax,voxmin,voxlvl,gain,(getWord(MSW,VOX) ? "true" : "false"),(getWord(MSW,PTT) ? "true" : "false"));

                }
 
		if (gain<=voxlvl) {
  		   TVOX=VOX_TIMER;
		   setWord(&MSW,VOX,false);
		   fprintf(stderr,"%s::main() VOX trigger activated max(%8f) min(%8f) trig(%8f) gain(%8f) VOX(%s) PTT(%s)\n",PROGRAMID,voxmax,voxmin,voxlvl,gain,(getWord(MSW,VOX) ? "true" : "false"),(getWord(MSW,PTT) ? "true" : "false"));

		   if (getWord(MSW,PTT)==false) {
		      setPTT(true);
		      setWord(&MSW,PTT,true);
		      fprintf(stderr,"%s::main() VOX PTT activated max(%8f) min(%8f) trig(%8f) gain(%8f) VOX(%s) PTT(%s)\n",PROGRAMID,voxmax,voxmin,voxlvl,gain,(getWord(MSW,VOX) ? "true" : "false"),(getWord(MSW,PTT) ? "true" : "false"));

		   }
		}

		if (getWord(MSW,VOX)==true) {
		   setWord(&MSW,VOX,false);
		   setPTT(false);
		   setWord(&MSW,PTT,false);
		   fprintf(stderr,"%s::main() VOX TIMEOUT event max(%8f) min(%8f) trig(%8f) gain(%8f) VOX(%s) PTT(%s)\n",PROGRAMID,voxmax,voxmin,voxlvl,gain,(getWord(MSW,VOX) ? "true" : "false"),(getWord(MSW,PTT) ? "true" : "false"));

		}
           }
	}


        fprintf(stderr,"%s: Turning infrastructure off\n",PROGRAMID);

	iqtest->stop();
        delete(iqtest);

        fprintf(stderr,"%s: Turning off virtual rig and cooler\n",PROGRAMID);

        //system("gpio -g write \"19\" 0");
        setPTT(false);

        //system("gpio mode \"12\" out");
        //system("gpio -g write \"12\" 0");

}

