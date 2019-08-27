
/*=============================================================================================================================
  pirtty 
  This software takes a message as argument and send it using Baudot RTTY with a given shift and baudrate at a specified
  frequency. Uses the rpitx libraries to generate the RF signal thru GPIO4 on a Raspberry Pi
  ==============================================================================================================================
  Written by Dr. Pedro E. Colla LU7DID (2019)
  Based on extensive, massive and friendly recognized excerpts from existing code
        rpi_rtty package from Mike ADAMS G3ZLO (2017) at http://yeovil-arc.com/rpi_rtty
        pisstv   package from Evariste COURJAUD F5OEO (evaristec@gmail.com) and at GitHub rpitx
  =============================================================================================================================
    Transmitting using this software requires a suitable licence. 
    Usage of this software is not the responsability of the author.
  =============================================================================================================================
    * Fair and educated warning *

    Raspberry Pi is a marvel.
    Hamradio is the best thing ever invented.
    ¡So don't ruin either by connecting GPIO04 directly to an antenna!

    You'll make life of others in your neighboor unsormountable, and even
    could get your Raspberry Pi fried in the process.

    Google "raspberry pi rpitx low pass filter" to get some good advice on what to put between your Raspberry and your antenna
    Or go to https://www.dk0tu.de/blog/2016/05/28_Raspberry_Pi_Lowpass_Filters/ for very good and easy to implement ideas
  =============================================================================================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
  =============================================================================================================================
    Installation procedure

          git clone http://www.github.com/lu7did/pirtty
          cd ¨/pirtty/src
          make
          sudo make install
  ============================================================================================================================
    Usage
	  pirtty  freq[Hz] "Message to Send" shift[Hz] baudrate[baud]
          pirtty for help
  ============================================================================================================================= 
    RTTY 45         45.45 baud      0.0220022seconds = 22.0022 mS frequency shift 170Hz
    RTTY 50         50.0 baud       6.6 cps (66 wpm)        
    RTTY 75         75.0 baud       10.0 cps (100 wpm) 

    Amateur uses typically send RTTY using a 170 Hz shift and 45.5 bauds while commercial uses (if any) uses 850 Hz with 50/75 bauds
  =============================================================================================================================
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "/home/pi/librpitx/src/librpitx.h"
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <stdint.h>
#define GPIO04   4
#define GPIO20  20
using namespace std;
typedef unsigned char byte;
typedef bool boolean;

// Global variables

char letters=0; /* letters/figures shift monitor */

/* Base freq is 1350 Hz higher than dial freq in USB */

double mark_frequency = 1350L;
double frequency_shift = 170L; /* default value. Can be overwritten on command line */
double space_frequency;
double baud_rate;
uint32_t bit_period = 22002L; /* default value for 45.45baud. period of a start pulse in uS */

ngfmdmasync *fmmod;
static double GlobalTuningFrequency=00000.0;
int FifoSize=10000; //10ms
bool running=true;
byte gpio=GPIO04;

string PROGRAMID="pirtty";
string VERSION="1.0";
string BUILD="015";

/* playtone
   this method takes a frequency to send and for how long, it's the
   core generation routine.
   from pisstv program at the rpitx package

   playtone((double)FrequencyMartin1[1],TimingMartin1[1]);

*/
void playtone(double Frequency,uint32_t Timing)//Timing in 0.1us

{

		uint32_t SumTiming=0;
		SumTiming+=Timing%100;

 		if(SumTiming>=100) 
		{
			Timing+=100;
			SumTiming=SumTiming-100;
		}
		int NbSamples=(Timing/100);


		while(NbSamples>0)
		{

			usleep(10);
			int Available=fmmod->GetBufferAvailable();
			if(Available>FifoSize/2)
			{	
				int Index=fmmod->GetUserMemIndex();
				if(Available>NbSamples) Available=NbSamples;
				for(int j=0;j<Available;j++)
				{
					fmmod->SetFrequencySample(Index+j,Frequency);
					NbSamples--;
				}
			}
		}		
}

/* sendtones
   call with a pointer to the run-length array of chars
   computes frequencies as shifts from baseband carrier (USB)
   using a format of run lengths and frequencies to output 
   method originally from rpi_rtty
   modified to feed pairs internally instead of generating a file
*/
void sendtones(char* ita2_run) // call with pointer to run-length array of chars <RPI_RTTY>
{
char pulse;
uint32_t timing;
int i=0;
char total_pulses = 0;

        double space_frequency = mark_frequency + frequency_shift;
        while ((pulse = ita2_run[i]) != '\0')
                {
                 // union of frequency (Hz) and time period (nS)

                timing = bit_period * pulse;


                if (i & 1) {
                        playtone(space_frequency, timing );  } //WriteTone in the original
                else {
                        playtone(mark_frequency, timing );  }  //WriteTone in the original
                i++;
                total_pulses += pulse;
                };
}

/* sendletter
   supply a bit pattern for the baudot frame as a string ita2,
   send start bit, variable bits LSB first and 2 stop bits
   also cares for the change between LETS/FIGS requied by baudot
   method originally from rpi_rtty modified to allow the structure
   to be sent as C++ likes
*/
void sendletter(char* ita2)
{

    char* maskChar;
        if (!letters)
                {
                 char locChar[]={1, 7, 0};maskChar=locChar;sendtones(maskChar);
                //sendtones(LETTERS);
                letters = 1;
                }
        sendtones(ita2);
}
/* sendfigure (mirror image of sendletter but for figues)
   supply a bit pattern for the baudot frame as a string ita2,
   send start bit, variable bits LSB first and 2 stop bits
   also cares for the change between LETS/FIGS requied by baudot
   method originally from rpi_rtty modified to allow the structure
   to be sent as C++ likes
*/
void sendfigure(char* ita2)
{
    char* maskChar;
        if (letters)
                {
		char locChar[]={1, 2, 1, 4, 0};maskChar=locChar;sendtones(maskChar);
                //sendtones(FIGURES);
                letters = 0;
                }
        sendtones(ita2);
}
/* sendachar
   get an ASCII char and decode it into Baudot and send it
   method originally from rpi_rtty modified to allow the structure of bits
   to be send as C++ likes
*/
void sendachar(char ascii)
{

//--- convert to upper case 
char c;
        c = (char) toupper(ascii);


/* Baudot run length coding example

        example 0b00100 
        idle MARK,
        start pulse, SPACE
        0 - SPACE
        0 - SPACE
        1 - MARK
        0 - SPACE
        0 - SPACE
        stop pulse - MARK
        stop pulse - MARK

        run length      3 (char starts with a SPACE), 1, 2, 2, 0 ; 0 indicates end of character.
                        must add up to 8        
*/
    char* maskChar;

//--- this is not a particularly efficient way to represent the conversion ASCII-BAUDOT
//--- but it's the shortest coding path to adapt the original implementation to the
//--- pointer requirements of C++. If you want clean code don't use C in the first place!

    switch (c){
        case ' ':
                {
                 char locChar[]={3, 1, 2, 2, 0};maskChar=locChar;sendtones(maskChar);
                 break;
                }
        case '\r':
                {
                 char locChar[]={4,1,1,2,0};maskChar=locChar;sendtones(maskChar);
                 break;
                }

        case '\n':
                {
                 char locChar[]={2,1,3,2,0};maskChar=locChar;sendtones(maskChar);
                 break;
                 }
        case 'Q':
                 {
                 char locChar[]={1,3,1,3,0};maskChar=locChar;sendletter(maskChar);
                 break;
                 }
        case 'W':
                 {
                 char locChar[]={1,2,2,3,0};maskChar=locChar;sendletter(maskChar);
                 break;
                 }
        case 'E':{
                  char locChar[]={1,1,4,2,0};maskChar=locChar;sendletter(maskChar);
                  break;
                  }
        case 'R':{
                  char locChar[]={2,1,1,1,1,2,0};maskChar=locChar;sendletter(maskChar);
                  break;
                 }
        case 'T':{
                 char locChar[]={5,3,0};maskChar=locChar;sendletter(maskChar);
                 break;
                 }
        case 'Y':{
                  char locChar[]={1,1,1,1,1,3,0};maskChar=locChar;sendletter(maskChar);
                  break;
                  }
        case 'U':{
                  char locChar[]={1,3,2,2,0};maskChar=locChar;sendletter(maskChar);
                  break;
 		 }
        case 'I':{
                  char locChar[]={2,2,2,2,0};maskChar=locChar;sendletter(maskChar);
                  break;
	  	 }
        case 'O':{
                 char locChar[]={4,4,0};maskChar=locChar;sendletter(maskChar);
                 break;
	 	 }
        case 'P':{
                 char locChar[]={2,2,1,3,0};maskChar=locChar;sendletter(maskChar);
                 break;
	 	 }
        case 'A':{
                 char locChar[]={1,2,3,2,0};maskChar=locChar;sendletter(maskChar);
                 break;
                 }
        case 'S':{
                 char locChar[]={1,1,1,1,2,2,0};maskChar=locChar;sendletter(maskChar);
                 break;
		 }
        case 'D':{
                 char locChar[]={1,1,2,1,1,2,0};maskChar=locChar;sendletter(maskChar);
                 break;
	 	 }

        case 'F':{
                 char locChar[]={1,1,1,2,1,2,0};maskChar=locChar;sendletter(maskChar);
                 break;
		}
        case 'G':{
                 char locChar[]={2,1,1,4,0};maskChar=locChar;sendletter(maskChar);
                 break;
		}
        case 'H':{
                 char locChar[]={3,1,1,3,0};maskChar=locChar;sendletter(maskChar);
                 break;
		}
        case 'J':{
                 char locChar[]={1,2,1,1,1,2,0};maskChar=locChar;sendletter(maskChar);
                break;
		}
        case 'K':{
                 char locChar[]={1,4,1,2,0};maskChar=locChar;sendletter(maskChar);
                break;}
        case 'L':{
                 char locChar[]={2,1,2,3,0};maskChar=locChar;sendletter(maskChar);
                break;}
        case 'Z':{
                 char locChar[]={1,1,3,3,0};maskChar=locChar;sendletter(maskChar);
                break;}
        case 'X':{
                 char locChar[]={1,1,1,5,0};maskChar=locChar;sendletter(maskChar);
                break;}
        case 'C':{
                 char locChar[]={2,3,1,2,0};maskChar=locChar;sendletter(maskChar);
                break;}
        case 'V':{
                 char locChar[]={2,6,0};maskChar=locChar;sendletter(maskChar);
                break;}
        case 'B':{
                 char locChar[]={1,1,2,4,0};maskChar=locChar;sendletter(maskChar);
                break;}
        case 'N':{
                 char locChar[]={3,2,1,2,0};maskChar=locChar;sendletter(maskChar);
                break;}
        case 'M':{
                 char locChar[]={3,5,0};maskChar=locChar;sendletter(maskChar);
                break;}
       case '1':{
                 char locChar[]={1,3,1,3,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '2':{
                 char locChar[]={1,2,2,3,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '3':{
                 char locChar[]={1,1,4,2,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '4':{
                 char locChar[]={2,1,1,1,1,2,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '5':{
                 char locChar[]={5,3,0};maskChar=locChar;sendfigure(maskChar);
                break;
		}
        case '6':{
                 char locChar[]={1,1,1,1,1,3,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '7':{
                 char locChar[]={1,3,2,2,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '8':{
                 char locChar[]={2,2,2,2,0};maskChar=locChar;sendfigure(maskChar);
                 break;}
        case '9':{
                 char locChar[]={4,4,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '0':{
                 char locChar[]={2,2,1,3,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '(':{
                 char locChar[]={1,4,1,2,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case ')':{
                 char locChar[]={2,1,2,3,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '"':{
                 char locChar[]={1,1,3,3,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '?':{
                 char locChar[]={1,1,2,4,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        case '-':{
                 char locChar[]={1,2,3,2,0};maskChar=locChar;sendfigure(maskChar);
                break;}
        default: {
                 char locChar[]={6,2,0};maskChar=locChar;sendtones(maskChar);
                 break; 
		 }
   }

}

/* terminate
   exception and sigs processing handler
   original from pisstv at the rpitx package
*/
static void
terminate(int num)
{
    running=false;
	fprintf(stderr,"Caught signal - Terminating %x\n",num);
   
}

/*==============================================================================================================
  main
  process arguments
  generate memory structures
  creates a modulator object
  send text
  terminate
================================================================================================================*/
int main(int argc, char **argv)
{
	float frequency=14.35e6;
        char *sText;
        int i=0;
	if (argc > 2) 
	{
		frequency=atof(argv[1]);
 		sText=(char *)argv[2];
                frequency_shift=atof(argv[3]);
                baud_rate=atof(argv[4]);
                gpio=atoi(argv[5]);
                if (gpio!=GPIO04 && gpio!=GPIO20) {
 		   gpio=GPIO04;
		}

                sscanf((char *)argv[3],"%lf",&frequency_shift);  /* in Hz, negative value for LSB */
                sscanf((char *)argv[4],"%lf",&baud_rate);

                if ((baud_rate != 45.5) && (baud_rate != 50.0) && (baud_rate != 75)) baud_rate=45.5;
                if ((frequency_shift != 170) && (frequency_shift != 850)) frequency_shift=170.0;

                bit_period = 10000000L / baud_rate;  //fix to accomodate timing requirements in 0.1 uSecs


	}
	else
	{
                printf("%s version %s build(%s)\r\n",PROGRAMID.c_str(),VERSION.c_str(),BUILD.c_str());
		printf("usage : %s frequency(Hz) \"text_to_send\" shift[Hz] rate[baud] GPIO[4 or 20]\n",PROGRAMID.c_str());

		exit(0);
	}
	for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

	fmmod=new ngfmdmasync(frequency,100000,14,FifoSize);
        fmmod->disableclk(4);
        fmmod->enableclk(gpio);
        while (sText[i] > '\n')
                {
                sendachar(sText[i++]);
                }

        sendachar(char(0x00));
        fmmod->disableclk(gpio);
	delete fmmod;
	return 0;
}



