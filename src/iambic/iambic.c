// gcc iambic.c -o iambic -lwiringPi -l pigpio -lpthread
// or make, to run sudo ./iambic [options]

/*



    10/12/2016, Rick Koch / N1GP, I adapted Phil's verilog code from
                the openHPSDR Hermes iambic.v implementation to build
                and run on a raspberry PI 3.

    1/7/2017,   N1GP, adapted to work with Jack Audio, much better timing.
    4/20/2019,  LU7DID, modified switches, added dynamic control of speed

--------------------------------------------------------------------------------
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
Boston, MA  02110-1301, USA.
--------------------------------------------------------------------------------


---------------------------------------------------------------------------------
        Copywrite (C) Phil Harman VK6PH May 2014
        Copyright (C) Pedro Colla LU7DID 2019
---------------------------------------------------------------------------------

        The code implements an Iambic CW keyer.  The following features are supported:

                * Variable speed control from 1 to 60 WPM
                * Dot and Dash memory
                * Straight, Bug, Iambic Mode A or B Modes
                * Variable character weighting
                * Automatic Letter spacing
                * Paddle swap

        Dot and Dash memory works by registering an alternative paddle closure whilst a paddle is pressed.
        The alternate paddle closure can occur at any time during a paddle closure and is not limited to being
        half way through the current dot or dash. This feature could be added if required.

        In Straight mode, closing the DASH paddle will result in the output following the input state.  This enables a
        straight morse key or external Iambic keyer to be connected.

        In Bug mode closing the dot paddle will send repeated dots.

        The difference between Iambic Mode A and B lies in what the keyer does when both paddles are released. In Mode A the
        keyer completes the element being sent when both paddles are released. In Mode B the keyer sends an additional
        element opposite to the one being sent when the paddles are released.

        This only effects letters and characters like C, period or AR.

        Automatic Letter Space works as follows: When enabled, if you pause for more than one dot time between a dot or dash
        the keyer will interpret this as a letter-space and will not send the next dot or dash until the letter-space time has been met.
        The normal letter-space is 3 dot periods. The keyer has a paddle event memory so that you can enter dots or dashes during the
        inter-letter space and the keyer will send them as they were entered.

        Speed calculation -  Using standard PARIS timing, dot_period(mS) = 1200/WPM
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <poll.h>
#include <sched.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

#include <wiringPi.h>
#include <softTone.h>
#include <pigpio.h>
//#include "../lib/keyed_tone.h"


static void* keyer_thread(void *arg);
static pthread_t keyer_thread_id;


static int dot_memory = 0;
static int dash_memory = 0;
static int key_state = 0;
static int kdelay = 0;
static int dot_delay = 0;
static int dash_delay = 0;
static int kcwl = 0;
static int kcwr = 0;
static int *kdot;
static int *kdash;
static int cw_keyer_speed = 20;
static int cw_keyer_weight = 55;
static int cw_keys_reversed = 0;
static int cw_keyer_mode = KEYER_MODE_A;
static int cw_keyer_sidetone_frequency = 600;
static int cw_keyer_sidetone_gain = 10;
static int cw_keyer_sidetone_envelope = 5;
static int cw_keyer_spacing = 0;
static int cw_active_state = 0;
static sem_t cw_event;
static int keyer_out = 0;
       CALLBACK keyChange=NULL; 
       byte keyState=KEY_UP; 

//*-------------------------------------------------------------------------------------------------------
//* keyer_update
//*------------------------------------------------------------------------------------------------------
void keyer_update() {
    dot_delay = 1200 / cw_keyer_speed;

    // will be 3 * dot length at standard weight

    dash_delay = (dot_delay * 3 * cw_keyer_weight) / 50;

    if (cw_keys_reversed) {
        kdot = &kcwr;
        kdash = &kcwl;
    } else {
        kdot = &kcwl;
        kdash = &kcwr;
    }
}
//*-------------------------------------------------------------------------------------------
//* keyer_event
//* Detects whether the left or right paddle has been activated
//*-------------------------------------------------------------------------------------------
void keyer_event(int gpio, int level, uint32_t tick) {
    int state = (cw_active_state == 0) ? (level == 0) : (level != 0);

    if (gpio == LEFT_PADDLE_GPIO) {
        kcwl = state;
    }
    else  // RIGHT_PADDLE_GPIO
     {   kcwr = state;
     }

    if (state || cw_keyer_mode == KEYER_STRAIGHT)
        sem_post(&cw_event);
}
//*---------------------------------------------------------------------------------------------
//* clear_memory
//*---------------------------------------------------------------------------------------------
void clear_memory() {

    dot_memory  = 0;
    dash_memory = 0;

}
//*---------------------------------------------------------------------------------------------
//* set_keyer_out
//* set the PTT or KEYER output depending on the Keyer mode
//*---------------------------------------------------------------------------------------------
void set_keyer_out(int state) {

    if (keyer_out != state) {
        keyer_out = state;

        if (state) {
            keyState=KEY_DOWN;                         //* The keyer has been pressed or the PTT activated
            if (keyChange!=NULL) { keyChange(); }      //* First execute callback
            setPTT(true);
        }
        else {
            keyState=KEY_UP;                           //* The keyer has been released or the PTT deactivated
            if (keyChange!=NULL) { keyChange(); }      //* First execute callback
            setPTT(false);
        }
    }
}
//*------------------------------------------------------------------------------------------------------------
//* keyer_thread
//* This thread is waiting for keyer events, when they occur they are just handled,
//* this is an infinite loop which runs until the main program "running" condition is set to false
//* However it doesn't alter the program timing as it waist on the semaphore cw_event until there is something
//* to process
//*------------------------------------------------------------------------------------------------------------
static void* keyer_thread(void *arg) {

    int pos;
    struct timespec loop_delay;
    int interval = 1000000; // 1 ms

    while(running==true) {
        sem_wait(&cw_event);
        key_state = CHECK;

        while (key_state != EXITLOOP) {
            switch(key_state) {
            case CHECK: // check for key press
                if (cw_keyer_mode == KEYER_STRAIGHT) {       // Straight/External key or bug

                    if (*kdash || *kdot) {                  // send manual dashes
                        set_keyer_out(1);
                        key_state = EXITLOOP;
                    } else {
                      if (*kdot) {                // and automatic dots
                        key_state = PREDOT;
                      } else {
                        set_keyer_out(0);
                        key_state = EXITLOOP;
                      }
                    }
                } else {
                    if (*kdot) {
                        key_state = PREDOT;
                    }  else { if (*kdash) {
                                key_state = PREDASH;
                               } else {
                                set_keyer_out(0);
                                key_state = EXITLOOP;
                               }
                    }
                }
                break;
            case PREDOT:                         // need to clear any pending dots or dashes
                clear_memory();
                key_state = SENDDOT;
                break;
            case PREDASH:
                clear_memory();
                key_state = SENDDASH;
                break;

            // dot paddle  pressed so set keyer_out high for time dependant on speed
            // also check if dash paddle is pressed during this time
            case SENDDOT:
                set_keyer_out(1);
                if (kdelay == dot_delay) {
                    kdelay = 0;
                    set_keyer_out(0);
                    key_state = DOTDELAY;        // add inter-character spacing of one dot length
                } else  
                    kdelay++;
                
                // if ode A and both paddels are relesed then clear dash memory
                if (cw_keyer_mode == KEYER_MODE_A) {
                    if (!*kdot & !*kdash) 
                        dash_memory = 0;
                    } else {
                        if (*kdash) {                   // set dash memory
                           dash_memory = 1;
                         }
                }
                break;

            // dash paddle pressed so set keyer_out high for time dependant on 3 x dot delay and weight
            // also check if dot paddle is pressed during this time
            case SENDDASH:
                set_keyer_out(1);
                if (kdelay == dash_delay) {
                    kdelay = 0;
                    set_keyer_out(0);
                    key_state = DASHDELAY;       // add inter-character spacing of one dot length
                }
                else
                     kdelay++;

                // if Mode A and both padles are relesed then clear dot memory
                if (cw_keyer_mode == KEYER_MODE_A) {
                    if (!*kdot & !*kdash) {
                        dot_memory = 0;
                    } else {
                        if (*kdot) {                    // set dot memory
                            dot_memory = 1;
                        }
                    }
                }
                break;

            // add dot delay at end of the dot and check for dash memory, then check if paddle still held
            case DOTDELAY:
                if (kdelay == dot_delay) {
                    kdelay = 0;
                    if(!*kdot && cw_keyer_mode == KEYER_STRAIGHT)   // just return if in bug mode
                        key_state = EXITLOOP;
                    else
                        if (dash_memory)                 // dash has been set during the dot so service
                           key_state = PREDASH;
                        else
                            key_state = DOTHELD;             // dot is still active so service
                }
                else
                  kdelay++;

                if (*kdash)                                 // set dash memory
                    dash_memory = 1;
                break;

            // add dot delay at end of the dash and check for dot memory, then check if paddle still held
            case DASHDELAY:
                if (kdelay == dot_delay) {
                    kdelay = 0;

                    if (dot_memory)                       // dot has been set during the dash so service
                        key_state = PREDOT;
                    else
                        key_state = DASHHELD;            // dash is still active so service
                } else
                     kdelay++;

                if (*kdot)                                  // set dot memory
                    dot_memory = 1;
                break;

            // check if dot paddle is still held, if so repeat the dot. Else check if Letter space is required
            case DOTHELD:
                if (*kdot)                                  // dot has been set during the dash so service
                    key_state = PREDOT;
                else
                    if (*kdash)                            // has dash paddle been pressed
                       key_state = PREDASH;
                    else
                       if (cw_keyer_spacing) {    // Letter space enabled so clear any pending dots or dashes
                          clear_memory();
                          key_state = LETTERSPACE;
                       }
                else
                    key_state = EXITLOOP;
                break;

            // check if dash paddle is still held, if so repeat the dash. Else check if Letter space is required
            case DASHHELD:
                if (*kdash)                   // dash has been set during the dot so service
                    key_state = PREDASH;
                else 
                    if (*kdot)               // has dot paddle been pressed
                        key_state = PREDOT;
                    else
                       if (cw_keyer_spacing) {    // Letter space enabled so clear any pending dots or dashes
                          clear_memory();
                          key_state = LETTERSPACE;
                       }
                       else
                          key_state = EXITLOOP;
                break;

            // Add letter space (3 x dot delay) to end of character and check if a paddle is pressed during this time.
            // Actually add 2 x dot_delay since we already have a dot delay at the end of the character.
            case LETTERSPACE:
                if (kdelay == 2 * dot_delay) {
                    kdelay = 0;
                    if (dot_memory)         // check if a dot or dash paddle was pressed during the delay.
                        key_state = PREDOT;
                    else
                        if (dash_memory)
                           key_state = PREDASH;
                        else
                           key_state = EXITLOOP;   // no memories set so restart
                }
                else
                    kdelay++;

                // save any key presses during the letter space delay
                if (*kdot)
                   dot_memory = 1;
                if (*kdash)
                   dash_memory = 1;
                break;

            default:
                key_state = EXITLOOP;

            }

            clock_gettime(CLOCK_MONOTONIC, &loop_delay);
            loop_delay.tv_nsec += interval;
            while (loop_delay.tv_nsec >= NSEC_PER_SEC) {
                loop_delay.tv_nsec -= NSEC_PER_SEC;
                loop_delay.tv_sec++;
            }
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &loop_delay, NULL);
        }
    }
    fprintf(stderr,"\n\nSIGINT received, terminating\n");
    return 0 ;
}
//*------------------------------------------------------------------------------------------------
//* iambic_close
//* stop the thread and destroy the semaphore, then exit the iambic, called upon termination
//*------------------------------------------------------------------------------------------------
void iambic_close() {

    fprintf(stderr,"Stopping keyer thread and killing semaphore\n");
    pthread_join(keyer_thread_id, 0);
    sem_destroy(&cw_event);
    return;

}
//*------------------------------------------------------------------------------------------------
//* iambic_init
//* this is the initialization for the iambic algorithms, the ports are set and the thread launched
//* it has to be called prior to any usage of the keyer
//*------------------------------------------------------------------------------------------------
int iambic_init()  {

    fprintf(stderr,"Initializing Keyer thread\n");
    if(gpioInitialise()<0) {
        fprintf(stderr,"Cannot initialize GPIO\n");
        return -1;
    }

    gpioSetMode(RIGHT_PADDLE_GPIO, PI_INPUT);
    gpioSetPullUpDown(RIGHT_PADDLE_GPIO,PI_PUD_UP);
    usleep(100000);

    gpioSetAlertFunc(RIGHT_PADDLE_GPIO, keyer_event);

    gpioSetMode(LEFT_PADDLE_GPIO, PI_INPUT);
    gpioSetPullUpDown(LEFT_PADDLE_GPIO,PI_PUD_UP);
    usleep(100000);

    gpioSetAlertFunc(LEFT_PADDLE_GPIO, keyer_event);

    //gpioSetMode(SPEED_GPIO, PI_INPUT);
    //gpioSetPullUpDown(SPEED_GPIO,PI_PUD_UP);
    //usleep(100000);

    gpioSetMode(KEYER_OUT_GPIO, PI_OUTPUT);
    gpioWrite(KEYER_OUT_GPIO, 0);

    keyer_update();

    if (wiringPiSetup () < 0) {
        printf ("Unable to setup wiringPi: %s\n", strerror (errno));
        return 1;
    }
    int i=0;
    fprintf(stderr,"Ready to operate\n");
    if (SIDETONE_GPIO) {
        softToneCreate(SIDETONE_GPIO);
    }

    i = sem_init(&cw_event, 0, 0);
    i |= pthread_create(&keyer_thread_id, NULL, keyer_thread, NULL);
    if(i < 0) {
        fprintf(stderr,"pthread_create for keyer_thread failed %d\n", i);
        exit(-1);
    }

    fprintf(stderr,"Keyer thread initialized\n");
    return 0;
}
