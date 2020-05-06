

/*
 * PixiePi
 * Raspberry Pi based transceiver controller
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
 *
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

//----------------------------------------------------------------------------
//  includes
//----------------------------------------------------------------------------

//*---- Generic includes

#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
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
#include <wiringPiI2C.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <wiringSerial.h>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include "/home/pi/librpitx/src/librpitx.h"

//*---- Program specific includes

#include "/home/pi/OrangeThunder/src/OT/OT.h"
#include "../lib/ClassMenu.h"
#include "../lib/LCDLib.h"
#include "../lib/DDS.h"
#include "../minIni/minIni.h"
#include "../log.c/log.h"
#include "../lib/MMS.h"

#include "/home/pi/OrangeThunder/src/lib/VFOSystem.h"
#include "/home/pi/OrangeThunder/src/OT/OT.h"
#include "/home/pi/OrangeThunder/src/lib/CAT817.h"
#include "/home/pi/OrangeThunder/src/lib/gpioWrapper.h"

#include <iostream>
#include <cstdlib>    // for std::rand() and std::srand()
#include <ctime>      // for std::time()

void gpiochangePin(int pin, int state);
void gpiochangeEncoder(int clk,int dt,int state);


byte  TRACE=0x00;
byte  MSW=0x00;
int   a;
int   anyargs;

//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="PixiePi";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2018,2020";


// *----------------------------------------------------------------*
// *                  GPIO support processing                       *
// *----------------------------------------------------------------*
// --- gpio object
gpioWrapper* gpio=nullptr;
char   *gpio_buffer;
void gpiochangePin();

#include "../lib/GUI.h"


//*--------------------------[System Word Handler]---------------------------------------------------
//* getWord Return status according with the setting of the argument bit onto the SW
//*--------------------------------------------------------------------------------------------------
bool getWord (unsigned char SysWord, unsigned char v) {

  return SysWord & v;

}
//*--------------------------------------------------------------------------------------------------
//* setWord Sets a given bit of the system status Word (SSW)
//*--------------------------------------------------------------------------------------------------
void setWord(unsigned char* SysWord,unsigned char v, bool val) {

  *SysWord = ~v & *SysWord;
  if (val == true) {
    *SysWord = *SysWord | v;
  }

}

//*--------------------------------------------------------------------------------------------------
//* main execution of the program
//*--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    
     fprintf(stderr,"%s Version %s Build(%s) %s\n",PROGRAMID,PROG_VERSION,PROG_BUILD,COPYRIGHT);

while(true)
        {
                a = getopt(argc, argv, "f:eds:hg:i:p:A:C:D:E:F:G:M:S:");

                if(a == -1) 
                {
                        if(anyargs) {
                           break; 
                        } else {
                           break;
                        }
                }
                anyargs = 1;    

                switch(a)
                {

                case 'A': 
                        break;
                case 'i':
                        break;
                case 'C':
                        break;
                case 'D':
                        break;
                case 'E': 
                        break;

                case 'F':
                        break;
                case 'G':     // gain in dB 
                        break;
                case 'M':
                        break;
                case 'S':
                        break;
                case 'W':
                        break;
                case 'f': // Frequency
                        break;
		case 'g': // GPIO
			break;
                case 'p': //ppm
                        break;
                case 'd': //debug
                        break;
                case 'h': // help
                        break;
                case 's': //serial port
                        break;
                case -1:
                break;
                case '?':
                        exit(1);
                        break;
                default:
                        exit(1);
                        break;
                }
        }



//*--- Create infrastructure for the execution of the GUI

     createMenu();

//*---  Define and initialize GPIO interface

     setupGPIO();


  exit(0);
}


