/**********************************************************************
* Filename    : testRotary.c
  code excerpt from  support@sunfounder.com
* 		     website     : www.sunfounder.com
**********************************************************************/

/*
 * PixiePi.c
 * Raspberry Pi based transceiver
 *
 *
 * Created by Pedro E. Colla (lu7did@gmail.com)
 * Code excerpts from several packages:
 *    Adafruit's python code for CharLCDPlate
 * ---------------------------------------------------------------------
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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>

#define  RoAPin     0    
#define  RoBPin     1
#define  RoSPin     2

static volatile int globalCounter = 0 ;

unsigned char flag;
unsigned char Last_RoB_Status;
unsigned char Current_RoB_Status;

void rotaryDeal(void)
{
	Last_RoB_Status = digitalRead(RoBPin);

	while(!digitalRead(RoAPin)){
		Current_RoB_Status = digitalRead(RoBPin);
		flag = 1;
	}

	if(flag == 1){
		flag = 0;
		if((Last_RoB_Status == 0)&&(Current_RoB_Status == 1)){
			globalCounter ++;
			printf("globalCounter : %d\n",globalCounter);
		}
		if((Last_RoB_Status == 1)&&(Current_RoB_Status == 0)){
			globalCounter --;
			printf("globalCounter : %d\n",globalCounter);
		}

	}
}

void rotaryClear(void)
{
	if(digitalRead(RoSPin) == 0)
	{
		globalCounter = 0;
		printf("globalCounter : %d\n",globalCounter);
		delay(1000);
	}
}

int main(void)
{
	if(wiringPiSetup() < 0){
		fprintf(stderr, "Unable to setup wiringPi:%s\n",strerror(errno));
		return 1;
	}

        printf("testRotary sample code n");
        //fprintf("testRotary Error channel\n");

	pinMode(RoAPin, INPUT);
	pinMode(RoBPin, INPUT);
	pinMode(RoSPin, INPUT);

	pullUpDnControl(RoSPin, PUD_UP);

	while(1){
		rotaryDeal();
		rotaryClear();
	}

	return 0;
}
