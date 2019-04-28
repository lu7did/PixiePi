/* rotaryencoder.h :
 * Header for "rotaryencoder.c" LGPLv3 library for Raspberry Pi.
 * Needs its rotaryencoder.c file companion and "wiringPi" LPGLv3 lib.
 * V1.1.0
 */

/*=======================================================================\
|      - Copyright (c) - August 2015 - F6HQZ - Francois BERGERET -       |
|                                                                        |
| rotaryencoder.c and rotaryencoder.h files can run only with the        |
| necessary and excellent wiringPi tools suite for Raspberry Pi from the |
| "Gordons Projects" web sites from Gordon Henderson :                   |
| https://projects.drogon.net/raspberry-pi/wiringpi/                     |
| http://wiringpi.com/                                                   |
|                                                                        |
| My library permits an easy use of few rotary encoders with push switch |
| in ther axe and use them as "objects" stored in structures. Like this, |
| they are easy to read or modify values and specs from anywhere in your |
| own software which must use them.                                      |
|                                                                        |
| A big effort has been done to supress any bounce or false action, for  |
| all rotary encoders and switches pulses. This was my main focus.       |
|                                                                        |
| Thanks to friends who have supported me for this project and all guys  |
| who have shared their own codes with the community.                    |
|                                                                        |
| Always exploring new technologies, curious about any new idea or       |
| solution, while respecting and thanking the work of alumni who have    |
| gone before us.                                                        |
|                                                                        |
| Enjoy !                                                                |
|                                                                        |
| Feedback for bugs or ameliorations to f6hqz-m@hamwlan.net, thank you ! |
\=======================================================================*/
 
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation; either version 3 of the 
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program; 
 * if not, see <http://www.gnu.org/licenses/>.
 */

/* Raspi 2 or B+ : 
 * 17 pins / 2 pins per encoder = 8 rotary encoders maximum
 *  or 17 buttons maximum
 *  or a mix of them until max pins
 * 
 * Don't change any of these defined or declared values
 * you must pass them from your own code by a classical fonction call 
 * with the correct variables as described bellow and seen in the 
 * "test.c" code exemple.
 */

#define max_encoders 8

#define max_buttons 17

#define	ON	1
#define	OFF	0
#define UP 1
#define DOWN 0
#define YES 1
#define NO 0
#define OUI	1
#define	NON	0
#define HIGH 1
#define LOW 0

struct encoder
{
	char *label ;                   // name or label as "Volume" or "Balance" or "Treble", etc...
	int pin_a ;                     // which GPIO received the A pin from the rotary encoder
	int pin_b ;                     // which GPIO received the B pin from the rotary encoder
	unsigned char sequence ;        // rotary encoder sends a complete 4 steps sequence (full cycle) or 1/4 cycle only per detent
	unsigned char reverse ;         // encoder much count or rotate in reverse 
	unsigned char looping ;         // looping from one end to other when value limits are reached, from high_Limit to low_Limit and reverse
	long int low_Limit ;            // max lowerst value, could be negative
	long int high_Limit ;           // max higherst value, could be negative
	volatile long int value ;       // used to drive your solution, can be the starting default value or something in memory somewhere
	volatile long int lastEncoded ; // memo to compare 2 consecutive steps binary values, don't modify
	unsigned long int pause ;       // pause time between gaps to reset rotary encoder speed level, in microsecondes
	int speed_Level_Threshold_2 ;   // second speed shift level threshold value
	int speed_Level_Threshold_3 ;   // third speed level threshold value
	int speed_Level_Threshold_4 ;   // fourth speed level threshold value
	int speed_Level_Multiplier_2 ;  // second speed level multiplier value
	int speed_Level_Multiplier_3 ;  // third speed level multiplier value
	int speed_Level_Multiplier_4 ;  // fourth speed level multiplier value
	unsigned long int last_IRQ_a ;  // last time IRQ on pin_A
	unsigned long int last_IRQ_b ;  // last time IRQ on pin_B
	int last_Pin ;                  // last pin which has been active
	unsigned char active_flag ;     // already working on its status
};

struct encoder encoders[max_encoders] ;

struct encoder *setupencoder(char *label, int pin_a, int pin_b, unsigned char sequence, 
	unsigned char reverse, unsigned char looping, long int low_Limit, long int high_Limit, 
	long int value, unsigned long int pause, 
	int speed_Level_Threshold_2, int speed_Level_Threshold_3, int speed_Level_Threshold_4,
	int speed_Level_Multiplier_2, int speed_Level_Multiplier_3, int speed_Level_Multiplier_4) ; 

//---------------------------------------------------------------------
// buttons

struct button
{
	char *label ;                          // name or label as "Effect" or "Mute" or "+10dB", etc...
	int pin ;                              // which GPIO received the button wire
	volatile long int value ;              // used to drive your solution, can be the starting default value or something in memory somewhere
	unsigned long int timestamp ;          // time when last state change occured (ON or OFF) to detect a loooonger push ON
	unsigned long int previous_timestamp ; // time when previous state change occured (ON or OFF) to detect a loooonger push ON	
	unsigned char active_flag ;            // already working on its status (to avoid loop reentrance for each bounce)
};

struct button buttons[max_buttons] ;

struct button *setupbutton(char *label, int pin, long int value) ;
