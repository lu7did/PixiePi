/*
 * genCW.cpp
 * Raspberry Pi based CW  experimental Generator
 * Expedimental version largely modelled after tune.cpp from Evariste
 *---------------------------------------------------------------------
 * This program operates as a controller for a Raspberry Pi to control
 * a raspberry pi based  transceiver using TAPR hardware.
 * Project at http://www.github.com/lu7did/OrangeThunder
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
#include <functional>

#include <sys/types.h>
#include <sys/stat.h>

//#include "/home/pi/PixiePi/src/lib/RPI.h" 
//#include "/home/pi/PixiePi/src/lib/SSB.h"
#include "/home/pi/librpitx/src/librpitx.h"
#include "/home/pi/OrangeThunder/src/OT/OT.h"
#include "/home/pi/OrangeThunder/src/OT4D/transceiver.h"
#include "/home/pi/PixiePi/src/lib/DDS.h"

//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="genCW";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2019,2020";


byte     TRACE=0x02;
typedef unsigned char byte;
typedef bool boolean;

float    f=14074000;

char*    cmd_buffer;
int      cmd_length;
int      cmd_FD = UNDEFINED;
int      cmd_result;

int      ax;

byte     MSW=0;
byte     fVOX=0;
long int TVOX=0;

int      gpio_ptt=GPIO_PTT;

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
     (TRACE>=0x03 ? fprintf(stderr,"%s:timer_exec() Timer TVOX countdown(%ld)\n",PROGRAMID,TVOX) : _NOP);
     if(TVOX==0) {
       fVOX=1;
       (TRACE>=0x02 ? fprintf(stderr,"%s:timer_exec Timer TVOX expired\n",PROGRAMID) : _NOP);
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
// *---------------------------------------------------
// * alarm handler
// *---------------------------------------------------
void sigalarm_handler(int sig)
{

   timer_exec();
   alarm(ONESEC);
   return;


}
//---------------------------------------------------------------------------------
// Print usage
//---------------------------------------------------------------------------------
void print_usage(void)
{
fprintf(stderr,"%s %s [%s]\n\
Usage:  \n\
-p            PTT GPIO port (default none)\n\
-f            frequency in HZ (default 14074000)\n\
-d            auto PTT with VOX\n\
-v            VOX activated timeout in secs (default 0 )\n\
-t            DEbug level (0=no debug, 2=max debug)\n\
-?            help (this help).\n\
\n",PROGRAMID,PROG_VERSION,PROG_BUILD);


} /* end function print_usage */

//---------------------------------------------------------------------------------
// setPTT
//---------------------------------------------------------------------------------
void setPTT(bool ptt) {

  
     setWord(&MSW,PTT,ptt);
    (TRACE>=0x02 ? fprintf(stderr,"%s:setPTT() set PTT(%s)\n",PROGRAMID,BOOL2CHAR(ptt)) : _NOP);

}
//---------------------------------------------------------------------------------
// terminate
//---------------------------------------------------------------------------------
static void terminate(int num)
{
    setWord(&MSW,RUN,false);
    (TRACE>=0x00 ? fprintf(stderr,"%s: Caught TERM signal(%x) - Terminating \n",PROGRAMID,num) : _NOP);
}
//---------------------------------------------------------------------------------
// main 
//---------------------------------------------------------------------------------
int main(int argc, char* argv[])
{

        timer_start(timer_exec,100);
        (TRACE>=0x00 ? fprintf(stderr,"%s %s [%s] tracelevel(%d)\n",PROGRAMID,PROG_VERSION,PROG_BUILD,TRACE) : _NOP);;
        cmd_result = mkfifo ( "/tmp/ptt_fifo", 0666 );
        (TRACE >= 0x02 ? fprintf(stderr,"%s:main() Command FIFO(%s) created\n",PROGRAMID,"/tmp/ptt_fifo") : _NOP);

        cmd_FD = open ( "/tmp/ptt_fifo", ( O_RDONLY | O_NONBLOCK ) );
        if (cmd_FD != -1) {
           (TRACE >= 0x02 ? fprintf(stderr,"%s:main() Command FIFO opened\n",PROGRAMID) : _NOP); 
           setWord(&MSW,RUN,true);
        } else {
           (TRACE >= 0x02 ? fprintf(stderr,"%s:main() Command FIFO creation failure . Program execution aborted tracelevel(%d)\n",PROGRAMID,TRACE) : _NOP); 
           exit(16);
        }
        (TRACE >= 0x02 ? fprintf(stderr,"%s:main() About to enter argument parsing\n",PROGRAMID) : _NOP); 
	while(1)
	{
		ax = getopt(argc, argv, "f:t:h");
                if(ax == -1) 
		{
			if(ax) break;
			else ax='h'; //print usage and exit
		}

	
		switch(ax)
		{
		case 'f': // AGC Maxlevel
			f  = atof(optarg);
  		        (TRACE >= 0x01 ? fprintf(stderr,"%s: Frequency(%2f)\n",PROGRAMID,f) : _NOP);
			break;


                case 't': // Debug level
                        TRACE = atoi(optarg);
                        if (TRACE >=0x00 && TRACE <= 0x03) {
                           (TRACE>=0x01 ? fprintf(stderr,"%s: Debug level established TRACE(%d)\n",PROGRAMID,TRACE) : _NOP);
                        } else {
                           (TRACE>=0x01 ? fprintf(stderr,"%s: Invalid debug level informed, ignored TRACE(%d)\n",PROGRAMID,TRACE) : _NOP);
                           TRACE=0x00;
                        }
                        break;
		case -1:
        	break;
		case '?':
			if (isprint(optopt) ) {
 			   fprintf(stderr, "%s: unknown option `-%c'.\n",PROGRAMID,optopt);
 			} else 	{
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

        (TRACE>=0x00 ? fprintf(stderr,"%s:main(): Trap handler initialization\n",PROGRAMID) : _NOP);
	for (int i = 0; i < 64; i++) {
           struct sigaction sa;
           std::memset(&sa, 0, sizeof(sa));
           sa.sa_handler = terminate;
           sigaction(i, &sa, NULL);
        }



DDS*   dds=nullptr;
//FILE *iqfile=NULL;


        dds=new DDS(NULL);
        dds->TRACE=TRACE;
        dds->gpio=4;
        dds->power=7;
        dds->f=f;
        dds->ppm=1000;
        dds->start(f);

 	//iqfile=fopen("/dev/stdin","rb");
        signal(SIGALRM, &sigalarm_handler);  // set a signal handler
        alarm(1);

//*---------------- Main execution loop

        setWord(&MSW,PTT,false);
        setWord(&MSW,RUN,true);
   int  j=0;
   int  k=0;
     
        (TRACE >= 0x00 ? fprintf(stderr,"%s:main(): Starting main loop\n",PROGRAMID) : _NOP); 




	while(getWord(MSW,RUN)==true)
	{
         usleep(100000);
// --- process commands thru pipe

//           cmd_length = read ( cmd_FD, ( void* ) cmd_buffer, 255 ); //Filestream, buffer to store in, number of bytes to read (max)
//           j++;
//           (TRACE >= 0x03 ? fprintf(stderr,"%s:main() read command buffer len(%d)\n",PROGRAMID,cmd_length) : _NOP);
//           if ( cmd_length > 0 ) {
//              cmd_buffer[cmd_length] = 0x00;
//              (TRACE >= 0x02 ? fprintf (stderr,"%s:main() Received data from command pipe (%s) len(%d)\n",PROGRAMID,cmd_buffer,cmd_length) : _NOP);
//              if (strcmp(cmd_buffer, "PTT=1\n") == 0) {
//                 (TRACE >= 0x02 ? fprintf (stderr,"%s:main() Received command(PTT=1) thru command pipe\n",PROGRAMID) : _NOP);
//                 setPTT(true);
//              }
//              if (strcmp(cmd_buffer, "PTT=0\n") == 0) {
//                 (TRACE>=0x02 ? fprintf (stderr,"%s:main() Received command(PTT=0) thru command pipe\n",PROGRAMID) : _NOP);
//                 setPTT(false);
//               }
//           }else;

// --- end of command processing

//           (TRACE>=0x03 ? fprintf(stderr,"%s:main() About to read sound buffer (%d)\n",PROGRAMID,k) : _NOP);
//           nbread=fread(buffer_i16,sizeof(short),1024,iqfile);
//           k=k+nbread;;
//           if(TRACE>=0x03) { if (k%100000) {fprintf(stderr,"%s:main() Keep alive tick (%d) len(%d)\n",PROGRAMID,k,nbread);}}

// --- Processing AGC results on  incoming signal

//           if (gain<mingain) {
//              mingain=gain;
//           }

//           if (gain>maxgain) {
//              maxgain=gain;
//              thrgain=maxgain*0.50;
//           }

//           if (vox_timeout > 0) {
//              if (gain<thrgain) {
//                 if (TVOX==0) {
//                    fprintf(stderr,"VOX=1\n\n");
//                 }
//                 TVOX=vox_timeout;
//                 if (autoPTT == true) {
//                    ((TRACE>=0x02 && getWord(MSW,PTT)==false) ? fprintf(stderr,"%s:main() autoPTT activated\n",PROGRAMID) : _NOP);
//                    setWord(&MSW,PTT,true);
//                 }
//              }
//
//              if (fVOX==1) {
//                 fVOX=0;
//                 fprintf(stderr,"VOX=0\n");
//                 if (autoPTT==true) {
//                    ((TRACE>=0x02 && getWord(MSW,PTT)==true) ? fprintf(stderr,"%s:main() autoPTT deactivated\n",PROGRAMID) : _NOP);
//                    setWord(&MSW,PTT,false);
//                 }////
//              }
//           }


//           (TRACE>=0x03 ? fprintf(stderr,"%s:main() Processed SSB signal gain(%g) nbread(%d)\n",PROGRAMID,gain,nbread) : _NOP);
           
//	   if(nbread>0) {
//              (TRACE>=0x03 ? fprintf(stderr,"%s:main() generate I/Q signal\n",PROGRAMID) : _NOP);
//	      numSamplesLow=usb->generate(buffer_i16,nbread,Ibuffer,Qbuffer);
//              if (getWord(MSW,PTT)==true) {
//                 for (int i=0;i<numSamplesLow;i++) {
//                     Fout[2*i]=Ibuffer[i];
//                     Fout[(2*i)+1]=Qbuffer[i];
//                 }
//                 if(k%20000 && TRACE>=0x03) {fprintf(stderr,"%s:main() PTT is active so data to sendiq\n",PROGRAMID);} 
//              } else {
//                 for (int i=0;i<numSamplesLow;i++) {
//                     Fout[2*i]=0.0;
//                     Fout[(2*i)+1]=0.0;
//                 }
//                 if(k%20000 && TRACE>=0x03) {fprintf(stderr,"%s:main() PTT is inactive so <NUL> is sent to sendiq\n",PROGRAMID);} 
//
//              }
//              fwrite(Fout, sizeof(float), numSamplesLow*2, outfile) ;
//              usleep(1000);
//           } else {
//   	      fprintf(stderr,"EOF=1\n");
//              setWord(&MSW,RUN,false);
// 	   }
//*---------------- program finalization cleanup
        }
        dds->stop();
        delete(dds);
 	fprintf(stderr,"CLOSE=1\n");

        (TRACE>=0x01 ? fprintf(stderr,"%s:main() program terminated normally\n",PROGRAMID) : _NOP);
        exit(0);

}

