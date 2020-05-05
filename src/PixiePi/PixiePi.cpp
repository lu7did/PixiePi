

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

#include <iostream>
#include <cstdlib>    // for std::rand() and std::srand()
#include <ctime>      // for std::time()


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

void procKeyerUpdate(MMS* p) {

     fprintf(stderr,"Executing async update on %s\n",p->mText);

}
void procSpeedChange(MMS* p) {

    fprintf(stderr,"=====> Speed update current value(%d-%s)\n",p->mVal,p->mText);
    sprintf(p->mText," %02d wpm",p->mVal*5); 
    fprintf(stderr,"=====> Speed update new     value(%d-%s)\n",p->mVal,p->mText);
}
//*--------------------------------------------------------------------------------------------------
//* main execution of the program
//*--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{


MMS* root=new MMS(0,(char*)"Raiz",NULL,NULL);

MMS* keyer=new MMS(0,(char*)"Keyer",NULL,procKeyerUpdate);

MMS* straight=new MMS(0,(char*)"Straight",NULL,NULL);
MMS* iambicA =new MMS(1,(char*)"Iambic A",NULL,NULL);
MMS* iambicB =new MMS(2,(char*)"Iambic B",NULL,NULL);


//MMS* speed=new MMS(1,(char*)"Speed",procSpeedChange,NULL,1,10);
MMS* speed=new MMS(1,(char*)"Speed",procSpeedChange,NULL);
MMS* dpval=new MMS(5,(char*)"*",NULL,NULL);
speed->lowerLimit(1);
speed->upperLimit(10);


MMS* shift=new MMS(2,(char*)"Shift",NULL,NULL);
MMS* shval=new MMS(1,(char*)"-----",NULL,NULL);

MMS* drive=new MMS(3,(char*)"Drive",NULL,NULL);


     root->add(root,keyer);
     root->add(root,speed);
     root->add(root,shift);
     root->add(root,drive);


     keyer->add(keyer,straight);
     keyer->add(keyer,iambicA);
     keyer->add(keyer,iambicB);

     speed->add(speed,dpval);
     shift->add(shift,shval);

     fprintf(stderr,"test list() at level (%s)\n",root->mText);
     root->list();

     fprintf(stderr,"Testing limits of speed (%s)\n",root->mText);

     speed->backup();
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->move(+1);
     fprintf(stderr,"+1 value %s\n",speed->get()->mText);
     speed->list();

     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);
     speed->move(-1);
     fprintf(stderr,"-1 value %s\n",speed->get()->mText);


     keyer->backup();

     fprintf(stderr,"Changing individual values of speed (%d-%s)\n",speed->curr->mVal,speed->curr->mText);
     speed->set(10);
     shift->set((char*)"500 Hz");
     fprintf(stderr,"Changing individual values of speed (%d-%s)\n",shift->curr->mVal,shift->curr->mText);

     exit(0);
}


