/* testEncoder.c :
 * Permits to test the "rotaryencoder" library.
 * No more interest, except curiosity or learning.
 * V.1.2.0
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
 
 /*
  * At starting, the LEDs will be enlighted (ON) few seconds for check,
  * then a full list of rotary encoders parameters/caracteristics will
  * be displayed, and axial push buttons included if any.
  * Then, two lines will be displayed reporting all the values status.
  * Each modification of any will display two new lines of status.
  * Not beautifull, but efficient to check the library, the components
  * and your wiring. ;-)  Each switch must be at ground during the "ON",
  * pullup resistors are programmed inside the GPIO, don't add them.
  * Enjoy !
  */

#include <stdio.h>
#include <wiringPi.h>

#include "rotaryencoder.c"

#define	ON    1
#define	OFF   0
#define UP    1
#define DOWN  0
#define YES   1
#define NO    0
#define OUI   1
#define	NON   0

// used GPIO for 2 UP/DOWN monitor flashing LEDs (arbitrary, you can change as desired)
#define	LED_DOWN  25
#define	LED_UP    29

// used PWM GPIO for LED dim demo with the first rotary encoder
#define PWM_LED   1
// the first encoder is used to play with PWM_LED dim, "1" is mandatory: only one PWM output !

extern int speed ;

void pwmWrite (int pin, int value) ;

int main(void)
{
	printf("\n * ROTARY ENCODER DEMO SOFTWARE FOR RASPBERRY PI * \n\n");

	wiringPiSetup () ;

/*	LEDs (outputs)
	enlighted for 2 sec at starting to check them, 
	then when moving values up or down 
*/
//	pinMode (25,OUTPUT) ;           // output to drive LED
//	pinMode (29,OUTPUT) ;           // output to drive LED
//	digitalWrite (25,ON) ;          // ON
//	digitalWrite (29,ON) ;          // ON
//	pinMode (PWM_LED, PWM_OUTPUT) ; // pin 1 is the only one PWM capable pin on RapsberryPi pcb
//	pinMode (PWM_LED,1024) ;        // Max bright value at starting
//	delay (2000) ;                  // mS
//	digitalWrite (25,OFF) ;         // OFF
//	digitalWrite (29,OFF) ;         // OFF
 
/*
 *  Please, see variables meaning in the rotaryencoder.c and rotaryencoder.h files
 *  and adapt them to your case, suppress or add the neccessary encoders and witches
 *  depending your project, in this two following structure types (one "object" each line) :
 */
	struct encoder *encoder = 
	setupencoder ("LUMIERE",0,2,YES,NO,NO,0,1024,500,500000,40000,25000,10000,10,25,50) ;
	setupencoder ("GRAVE",3,4,YES,NO,NO,-5000,5000,0,500000,30000,15000,6000,10,100,1000) ;
	setupencoder ("VOLUME",5,6,YES,YES,YES,0,5000,0,500000,30000,15000,6000,10,100,1000) ;
	
	// axial buttons (or any button) are there :
	struct button *button = 
	setupbutton("LUMIERE",7,1) ; // pin  7 and ON  at starting
	setupbutton("GRAVE",21,0) ;  // pin 21 and OFF at starting
	setupbutton("VOLUME",22,0) ; // pin 22 and OFF at starting
	
	extern numberofencoders ;
	extern numberofbuttons ;
	
	long int memo_rotary[numberofencoders] ; // record the rotary encoder value for modification detection later
	long int memo_button[numberofbuttons] ; // record the button value for modification detection later

	printf("\nROTARY ENCODERS list :\n\n-----------------\n") ;
	for (; encoder < encoders + numberofencoders ; encoder++)
	{
		printf("Label:\"%s\" \n pin A: %d \n pin B: %d \n mem address: %d \n full sequence each step: %d \n reverse rotation: %d \n looping if limit reached: %d \n low_Limit value: %d \n high_Limit value: %d \n operator rotation pause duration detection (time between two steps in microsec): %d \n speed_Level_Threshold_2 (time between two steps in microsec): %d \n speed_Level_Multiplier_2: %d \n speed_Level_Threshold_3 (time between two steps in microsec): %d \n speed_Level_Multiplier_3: %d \n speed_Level_Threshold_4 (time between two steps in microsec): %d \n speed_Level_Multiplier_4: %d \n-----------------\n",
		encoder->label, encoder->pin_a, encoder->pin_b, encoder, encoder->sequence, encoder->reverse, encoder->looping, encoder->low_Limit, encoder->high_Limit, encoder->pause, encoder->speed_Level_Threshold_2, encoder->speed_Level_Multiplier_2, encoder->speed_Level_Threshold_3, encoder->speed_Level_Multiplier_3, encoder->speed_Level_Threshold_4, encoder->speed_Level_Multiplier_4) ;
	}
	printf("\nBUTTONS list :\n\n-----------------\n") ;
	for (; button < buttons + numberofbuttons ; button++)
		{ printf("Label:\"%s\" \n pin: %d \n mem address: %d \n-----------------\n", button->label, button->pin, button) ; }

	printf("\n Two LEDs must be connected at #25 and #29 pins if you want to observe the rotation direction (normal or reverse). \n") ;
	printf(" The positive pin of the LED is to connect to the Raspi output pin, the negative pin of the LED to the minus (the \"ground\" or \"0V\"),\n but a serial resistor of about 1kOhms must be inserted to limit the current.") ;
	printf("\n The first rotary encoder is used to modify the PWM-LED dim which must be connected on pin #1, not another, which s the only one PWM capable. \n") ;
	printf("\n Some rotary encoders own a push button in their axis to trigger some extra feature, \n as to load or save something in memory, or change the feature to modify, etc... \n\n") ;
	
	while (1)
	{
		delay (10) ; // 10 ms default, decreases the loop speed (and the CPU load from about 25% to minus than 0.3%)
		digitalWrite (LED_DOWN, OFF) ;	// OFF
		digitalWrite (LED_UP, OFF) ; 	// OFF

		int step = 0 ;
		unsigned char print = 0 ;

		// check if any rotary encoder modified value		
		struct encoder *encoder = encoders ;
		for (; encoder < encoders + numberofencoders ; encoder++)
		{
			if (encoder->value != memo_rotary[step])
			{	
				print = 1 ;
				memo_rotary[step] = encoder->value ;
			}	
			++step ;	
		} 
		
		step = 0 ;
	
		// check if any button modified value	
		struct button *button = buttons ;
		for (; button < buttons + numberofbuttons ; button++)
		{
			if (button->value != memo_button[step])
			{	
				print = 1 ;
				memo_button[step] = button->value ;
			}	
			++step ;	
		}
		
		// and if any value modified, then display the new value (and all others too)
		if (print) 
		{
			struct encoder *encoder = encoders ;
			for (; encoder < encoders + numberofencoders; encoder++)
			{
				// encoder pins, name, address in memory, current value 
				printf("A:%d B:%d \"%s\"[%d]:%-5d ", encoder->pin_a, encoder->pin_b, encoder->label, encoder, encoder->value) ;
				if (encoder == encoders)
				{ // first encoder is "reserved" for PWM_LED dim demo
					//pwmWrite (PWM_LED, encoder->value) ;
     					printf("encoder detected\n");
				}
			}
			// current rotation speed, last pause duration, number of eliminated bounces from starting
			printf("- speed: %-4d - gap: %-10d \nBUTTONS:  ", speed, gap) ;
			
			struct button *button = buttons ;
			for (; button < buttons + numberofbuttons; button++)
			{
				// button pin, name, address in memory, current value
				printf("\"%s\"[%d](pin:%d):%d  ", button->label, button, button->pin, button->value) ; 
			}
			printf("- for all, cancelled bounces: %-5d \n\n", bounces) ;
			print = 0 ;
		}
	}
	return(0) ;
}

