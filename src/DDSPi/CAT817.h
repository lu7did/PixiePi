//*--------------------------------------------------------------------------------------------------
//* CAT817 CAT Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef CAT817_h
#define CAT817_h

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


typedef unsigned char byte;
typedef bool boolean;
typedef void (*CALLBACK)();


#define RIT   0B00010000
#define LOCK  0B00001000
#define PTT   0B00000100
#define SPLIT 0B00000010
#define VFO   0B00000001

#define LSB   0x00
#define USB   0x01
#define CW    0x02
#define CWR   0x03
#define AM    0x04
#define WFM   0x06
#define FM    0x08
#define DIG   0x0A
#define PKT   0x0C

//*---------------------------------------------------------------------------------------------------
//* VFOSystem CLASS
//*---------------------------------------------------------------------------------------------------
class CAT817
{
  public: 
  
      CAT817(CALLBACK f,CALLBACK s,CALLBACK m, CALLBACK r, CALLBACK t);
      
      CALLBACK changeFreq=NULL;
      CALLBACK changeMode=NULL;
      CALLBACK changeStatus=NULL;
      CALLBACK getRX=NULL;
      CALLBACK getTX=NULL;

      void get();
      void hex2str(char* r, byte* b,int l);
      int  bcdToDec(int v);
      int  decToBcd(int v);
      int  BCD2Dec(byte* BCDBuf);
      void dec2BCD(byte* BCDBuf, int f);
      void sendSerial(byte* BCDBuf,int len);
      void processCAT(byte* rxBuffer);
      void open(char* port, int speed);
      void sendStatus();
      void sendMode();

      int  RITOFS=0x00;
      byte MODEWORD=0xff;
      byte FT817;
      byte MODE=USB;
      byte TRACE=0x00;
      byte RX=0x00;
      byte TX=0x00;
      float SetFrequency;
    
 
  private:

      int n=0;
      byte rxBuffer[10];
      char buffer[10];
      int serial_port;
      char msg[80]; 
};

#endif
//*---------------------------------------------------------------------------------------------------
//* CAT817 CLASS Implementation
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de CAT para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
CAT817::CAT817(CALLBACK f,CALLBACK s,CALLBACK m, CALLBACK r, CALLBACK t)
{
  
  if (f!=NULL) {changeFreq=f;}   //* Callback of change VFO frequency
  if (s!=NULL) {changeStatus=s;} //* Callback of TX mode change
  if (m!=NULL) {changeMode=m;}   //* Callback of VFO Reset 
  if (r!=NULL) {getRX=r;}        //* Callback for RX Status
  if (t!=NULL) {getTX=t;}        //* Callback for TX Status
 
  FT817=0;
  MODE=USB;
}
//#*---------------------------------------------------------------------------
//#* bcdToDec
//#* Convert nibble
//#*---------------------------------------------------------------------------
int CAT817::bcdToDec(int v) {
  return  (v/16*10) + (v%16);
} 
//#*--------------------------------------------------------------------------
//#* decToBcd
//#* Convert nibble
//#*-------------------------------------------------------------------------
int CAT817::decToBcd(int v){
  return  (v/10*16) + (v%10);
}
//#*---------------------------------------------------------------------------
//#* hex2str
//#* Convert byte to Hex string
//#*---------------------------------------------------------------------------
void CAT817::hex2str(char* r, byte* b,int l) {

      for(int j = 0; j < l; j++) {
         sprintf(&r[2*j], "%02X",b[j]);
      }
}
//#*---------------------------------------------------------------------------
//#* BCD2Dec
//#* Convert 4 BCD bytes to integer
//#*---------------------------------------------------------------------------
int CAT817::BCD2Dec(byte* BCDBuf) {

    int f=0;
    f=f+bcdToDec(BCDBuf[0])*1000000;
    f=f+bcdToDec(BCDBuf[1])*10000;
    f=f+bcdToDec(BCDBuf[2])*100;
    f=f+bcdToDec(BCDBuf[3])*1;
    f=f*10;
    return f;
}
//#*---------------------------------------------------------------------------
//#* dec2BCD
//#* Convert convert frequency integer into BCD
//#*---------------------------------------------------------------------------
void CAT817::dec2BCD(byte* BCDBuf, int f){

    int fz=int(f/10);
    int f0=int(fz/1000000);
  
    int x1=fz-f0*1000000;
    int f1=int(x1/10000);
  
    int x2=x1-f1*10000;
    int f2=int(x2/100);

    int x3=x2-f2*100;
    int f3=int(x3);

    f0=decToBcd(f0);
    f1=decToBcd(f1);
    f2=decToBcd(f2);
    f3=decToBcd(f3);

    BCDBuf[0]=f0;
    BCDBuf[1]=f1;
    BCDBuf[2]=f2;
    BCDBuf[3]=f3;


 }
//#*---------------------------------------------------------------------------
//#* sendSerial
//#* send buffer over serial link
//#*---------------------------------------------------------------------------
void CAT817::sendSerial(byte* BCDBuf,int len) {
    for(int j = 0; j < len; j++) {
       serialPutchar(serial_port,(char)BCDBuf[j]);
    }
}
//#*---------------------------------------------------------------------------
//#* sendStatus callback
//#* After any command changing status the callback is made
//#*---------------------------------------------------------------------------
void CAT817::sendStatus() {
    if (changeStatus != NULL) {
       changeStatus();
    }
} 
//#*---------------------------------------------------------------------------
//#* sendMode callback
//#* After any command changing the transceiver mode the callback is made
//#*---------------------------------------------------------------------------
void CAT817::sendMode() {
    if (changeMode != NULL) {
       changeMode();
    }
} 
//#*---------------------------------------------------------------------------
//#* processCAT
//#* After a full command has been received process it
//#*---------------------------------------------------------------------------
void CAT817::processCAT(byte* rxBuffer) {

    byte BCDBuf[6];
    char buffer[18];

    BCDBuf[0]=0x00;
    BCDBuf[1]=0x00;
    BCDBuf[2]=0x00;
    BCDBuf[3]=0x00;
    BCDBuf[4]=0x00;
    BCDBuf[5]=0x00;

    switch(rxBuffer[4]) {
      case 0x01:  {       //* Set Frequency
       int f=BCD2Dec(&rxBuffer[0]);
       SetFrequency=f;

       if (changeFreq != NULL) {
          changeFreq();
       }
       BCDBuf[4]=0x01;
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x01 Resp(%s)",buffer);
       sendStatus();
       return;}
      case 0x03:  {        //* Get Frequency and Mode
       int f=(int)SetFrequency;
       dec2BCD(&BCDBuf[0],f);
       BCDBuf[4]=MODE;
       hex2str(&buffer[0],&BCDBuf[0],5);
       sprintf(msg,"Command 0x03 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],5);
       return;}
      case 0x00: {   //* LOCK status flip
       if(getWord(FT817,LOCK)==true) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,LOCK,true);
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x00 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],1);
       sendStatus();
       return; }
      case 0x02: { //* SPLIT status flip
       if(getWord(FT817,SPLIT)==true) {
         setWord(&FT817,SPLIT,false);
         BCDBuf[0]=0xF0;
       } else {
         setWord(&FT817,SPLIT,true);
         BCDBuf[0]=0x00;
       }
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x02 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],1);
       sendStatus();
       return;}
      case 0x05: { //* RIT status flip
       if(getWord(FT817,RIT)==true) {
         setWord(&FT817,RIT,false);
         BCDBuf[0]=0xF0;
       } else {
         setWord(&FT817,RIT,true);
         BCDBuf[0]=0x00;
       }
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x05 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],1);
       sendStatus();
       return;}
      case 0x07: {      //* Transceiver mode change
       byte mode=rxBuffer[0] & MODEWORD;       //* Prevent invalid values to be set
       if (mode != LSB && mode != USB && mode != CW && mode != CWR && mode != AM && mode != WFM && mode != FM && mode != DIG && mode != PKT) {
          hex2str(&buffer[0],&rxBuffer[0],1);
          sprintf(msg,"Command 0x07 Invalid mode (%s)",buffer);
          return;
       }
       MODE=mode;
       sendMode();
       return;}
      case 0x08: {     //* Undocummented feature returns 0x00 if transceiver unkeyed 0xF0 if keyed
       if(getWord(FT817,PTT)==true) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x08 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],1);
       sendStatus();
       return;}
      case 0x80: {      //* Turn the LOCK off
       if(getWord(FT817,LOCK)==false) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,LOCK,false);
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x80 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],1);
       sendStatus();
       return; }
      case 0x81:  {
       BCDBuf[0]=0x00;
       if (getWord(FT817,VFO)==false) {
          setWord(&FT817,VFO,true);
       } else {
          setWord(&FT817,VFO,false);
       }
       sendStatus();
       return;}
      case 0x82: {      //* Turn the SPLIT off
       if(getWord(FT817,SPLIT)==false) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,SPLIT,false);
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x81 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],1);
       sendStatus();
       return; }
      case 0x85: {      //* Turn the RIT off
       if(getWord(FT817,RIT)==false) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,RIT,false);
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x05 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],1);
       sendStatus();
       return; }
      case 0x88: {      //* Turn the LOCK off
       if(getWord(FT817,PTT)==false) {
         BCDBuf[0]=0xF0;
       } else {
         BCDBuf[0]=0x00;
       }
       setWord(&FT817,PTT,false);
       hex2str(&buffer[0],&BCDBuf[0],1);
       sprintf(msg,"Command 0x08 Resp(%s)",buffer);
       sendSerial(&BCDBuf[0],1);
       sendStatus();
       return; }
      case 0xBB:  {     //* falta controlar que sea solo dirección 0x55
       BCDBuf[0]=0x80;
       if (getWord(FT817,VFO)==true) {
          BCDBuf[0]=BCDBuf[0] | 0x01;
       }
       BCDBuf[1]=0x00;
       sendSerial(&BCDBuf[0],2);
       return;}
      case 0xF5: {      //* Set Clarifier Frequency (not implemented yet)
       byte offsetBuf[5];
       offsetBuf[2]=rxBuffer[2];
       offsetBuf[3]=rxBuffer[3];
       offsetBuf[0]=0x00;
       offsetBuf[1]=0x00;
       offsetBuf[4]=0xF5;
       int f=BCD2Dec(&offsetBuf[0]);
       f=f/10;
       if (rxBuffer[0]==0x00) {
          RITOFS=+f;
       } else {
          RITOFS=(-1)*f;
       }
       sendStatus();
       return; }  
      case 0xE7:  {     //* Receiver Status

       if (getRX!=NULL) {
          getRX();
       }

       BCDBuf[0]=RX;
       sendSerial(&BCDBuf[0],1);
       return;}
      case 0xF7:  {     //* Transmitter status
       if (getWord(FT817,PTT)==false) { //* Minimum report is based on current known status
          BCDBuf[0]=BCDBuf[0] | 0B10000000;
       }

       if (getWord(FT817,SPLIT)==false){
          BCDBuf[0]=BCDBuf[0] | 0B00100000;
       }
       if(getTX!=NULL) {       //* Ask caller thru a callback
         getTX();
       }
       BCDBuf[0]=TX;
       sendSerial(&BCDBuf[0],1);
       return;}
      case 0xBD:  {   //* TX Metering
       if (getWord(FT817,PTT)==false) {
          BCDBuf[0]=0x00;
          sendSerial(&BCDBuf[0],1);
          return;
       }
       BCDBuf[0]=0x99;
       BCDBuf[1]=0x99;
       sendSerial(&BCDBuf[0],1);
       return; }
      default:    {
       hex2str(&buffer[0],&rxBuffer[4],1);
       sprintf(msg,"Invalid or unknown code %s",&buffer[0]);
       printf(msg);
       return;}
     } 


}
//*-------------------------------------------------------------------------
//* open
//* open the serial link and enable the CAT operation if successful
//*-------------------------------------------------------------------------
void CAT817::open(char* port, int speed) {
    if ((serial_port = serialOpen (port, speed)) < 0)
    {
       fprintf (stderr, "Unable to open serial device: %s rc(%s)\n", port, strerror (errno)) ;
       return ;
    }
}
//*-------------------------------------------------------------------------
//* get
//* get a character from the  serial port and process it
//*-------------------------------------------------------------------------
void CAT817::get() {

    if (serial_port < 0) {
       return;
    }

    while(serialDataAvail(serial_port)) {
       char c = serialGetchar(serial_port);
       byte k=(byte)c;
       rxBuffer[n]=k;
       n++;
       if (n==5) {
          char buffer[18];
          if (rxBuffer[4] != 0xBD && rxBuffer[4] != 0xF7 && rxBuffer[4] != 0xBB && rxBuffer[4] != 0xE7 && rxBuffer[4] != 0x03) {
              hex2str(&buffer[0],&rxBuffer[0],n);
              printf ("Received Serial hex2str (%s)\n",buffer);
          }
          processCAT(&rxBuffer[0]);
          fflush (stdout) ;
          n=0;
       }
   }
}




