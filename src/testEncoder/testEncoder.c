/*
 * testEncoder.c
 * 
 * Created by Tobias Gall <toga@tu-chemnitz.eu>
 * Based on Adafruit's python code for CharLCDPlate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "testEncoder.h"

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


int value=0;
int lastEncoded=0;
int counter=0;
int clkLastState=0; 

#define ENCODER_CLK 17
#define ENCODER_DT  18
#define ENCODER_SW  27

void updateSW(int gpio, int level, uint32_t tick)
{

        if (level != 0) {
           return;
        }
        int pushSW=gpioRead(ENCODER_SW);
        printf("Switch Pressed\n");

}
void updateEncoders(int gpio, int level, uint32_t tick)
{

        if (level != 0) {
           return;
        }

        int clkState=gpioRead(ENCODER_CLK);
        int dtState= gpioRead(ENCODER_DT);

        if (dtState != clkState) {
          counter++;
        } else {
          counter--;
        }
        printf("Rotary encoder activated counter=%d\n",counter);
        clkLastState=clkState;
//        }
    
}


int main(void)
{
    printf("testEncoder Version 1.0 test drive PixiePi\n");

    if(gpioInitialise()<0) {
        fprintf(stderr,"Cannot initialize GPIO\n");
        return -1;
    }

    gpioSetMode(ENCODER_CLK, PI_INPUT);
    gpioSetPullUpDown(ENCODER_CLK,PI_PUD_UP);
    usleep(100000);

    gpioSetISRFunc(ENCODER_CLK, FALLING_EDGE,0,updateEncoders);
    gpioSetMode(ENCODER_DT, PI_INPUT);
    gpioSetPullUpDown(ENCODER_DT,PI_PUD_UP);
    usleep(100000);

    gpioSetMode(ENCODER_SW, PI_INPUT);
    gpioSetPullUpDown(ENCODER_SW,PI_PUD_UP);
    gpioSetAlertFunc(ENCODER_SW,updateSW);
    usleep(100000);

    counter = 0;

    if (wiringPiSetup () < 0) {
        printf ("Unable to setup wiringPi: %s\n", strerror (errno));
        return 1;
    }
    printf("Press SW button or turn clock/counterclock wise the encoder");

    while(1) {



    } 

    return 0;
} 

