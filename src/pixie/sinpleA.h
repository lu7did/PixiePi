#if SINPLEA
//*--------------------------------------------------------------------------------------------------
//* Specific Headers for sinpleA implementation
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//*--- Program & version identification


#define PROGRAMID "sinpleA"
#define PROG_VERSION   "1.0"

//*-----------------------------------------------------------------------------------------------
//* Control lines and VFO Definition [Project dependent]
//*-----------------------------------------------------------------------------------------------

#define VFO_SHIFT            1000
#define VFO_START          100000
#define VFO_END          60000000
#define VFO_BAND_START          0

#define VFO_PLL_LOWER        1000

#define FI_LOW     0
#define FI_HIGH   15

//*=======================================================================================================================================================
//* SI5351 Library
//*=======================================================================================================================================================
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define SI5351 true

//*------------------------------------------------------------------------------------------------
//* Set here SINPLEA Menu definitions
//*------------------------------------------------------------------------------------------------

void BandUpdate();
void ShiftUpdate();
void vfoUpdate();
void StepUpdate();
void LckUpdate();
void ModUpdate();

//void setDDSFreq();

void showBand();
void showDDS();

void showPanel();
void showFreq();

void setWord(byte* SysWord,byte v, boolean val);
boolean getWord (byte SysWord, byte v);

MenuClass band(BandUpdate);
MenuClass vfo(vfoUpdate);
MenuClass stp(StepUpdate);
MenuClass shf(ShiftUpdate);
MenuClass lck(LckUpdate);
MenuClass mod(ModUpdate);

#include "DDS_SI5351.h"

//*--------------------------------------------------------------------------------------------
//* readEEPROM
//* Read specific configuration
//*--------------------------------------------------------------------------------------------
void doSetGroup() {
  return;
}
//*--------------------------------------------------------------------------------------------
//* defineMenu
//* Define devide specific menu configuration
//*--------------------------------------------------------------------------------------------
void defineMenu(){
//*============================================================================================
//* Define master menu and lower level tree for simpleA
//*============================================================================================
  
  menuRoot.add((char*)"Band",&band);
  menuRoot.add((char*)"VFO",&vfo);
  menuRoot.add((char*)"Step",&stp);
  menuRoot.add((char*)"IF Shift",&shf);
  menuRoot.add((char*)"Lock",&lck);
  menuRoot.add((char*)"Mode",&mod);

  band.add((char*)"Off      ",NULL);
  band.set(0);

  shf.add((char*)" 0 Hz",NULL);
  shf.set(FI_LOW);

  vfo.add((char*)" A",NULL);
  vfo.set(VFOA);

//*---- Establish different and default step
  stp.add((char*)"  1 KHz",NULL);
  stp.set(3);

  lck.add((char*)"Off",NULL);  
  lck.set(0);
  
  mod.add((char*)"DDS",NULL);
  mod.set(0);

}
//*--------------------------------------------------------------------------------------------
//* pinSetup
//* Setup pin
//*--------------------------------------------------------------------------------------------
void pinSetup() {
  return;
}  


//*--------------------------------------------------------------------------------------------
//* savesinpleA
//* save specifics of sinpleA
//*--------------------------------------------------------------------------------------------
void saveMenu() {

   if (vx.vfoAB != vfo.mItem) {   //Switch from VFO A to B or viceversa
      vx.vfoAB=vfo.mItem;
      
      band.mItem=(vx.vfoband[vx.vfoAB]);
      BandUpdate();
      
      stp.mItem=(vx.step2code(vx.vfostep[vx.vfoAB]));
      StepUpdate();
       
   } else {

      if (vx.vfoband[vx.vfoAB]!=band.mItem) { //Switch Band
         vx.vfoband[vx.vfoAB]=band.mItem;
         vx.set(vx.vfoAB,vx.loFreq[vx.vfoband[vx.vfoAB]]*1000);
    }

         if (vx.vfostep[vx.vfoAB]!=vx.code2step(stp.mItem)) { //Change tuning step
            vx.vfostep[vx.vfoAB]=vx.code2step(stp.mItem);
         }
   }

   vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);
   vx.set(vx.vfoAB,vx.get(vx.vfoAB));

}
//*--------------------------------------------------------------------------------------------
//* Band Update   (CALLBACK from Menu System)
//* Set band label and limits
//*--------------------------------------------------------------------------------------------
void BandUpdate() {

  char* s=(char*)"                  ";

#if DEBUG

  sprintf(hi,"BandUpdate ANTES band.mItem=%u %s",band.mItem,band.l.get(0)->mText);
  Serial.println(hi);
  
#endif
  
  if (band.mItem < BANDMAX && band.CW == true) {
      band.mItem++;
  }
  if (band.mItem > 0 && band.CCW == true) {
      band.mItem--;
  }
   
  
  switch(band.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"160m";break;};
    case 2:                          {s=(char*)"80m";break;}; 
    case 3:                          {s=(char*)"40m";break;}; 
    case 4:                          {s=(char*)"20m";break;}; 
    case 5:                          {s=(char*)"15m";break;}; 
    default:                         {s=(char*)"10m";break;}; 
  }


  band.l.get(0)->mText=s;
  band.CW=false;
  band.CCW=false;
  
  return;
  
}
//*--------------------------------------------------------------------------------------------
//* FI Shift Update   (CALLBACK from Menu System)
//* Set FI shift label and limits
//*--------------------------------------------------------------------------------------------
void ShiftUpdate() {
  
  if (shf.mItem < 15 && shf.CW == true) {
      shf.mItem++;
  }
  if (shf.mItem > 0 && shf.CCW == true) {
      shf.mItem--;
  }
  char* s=(char*)"                  "; 


  sprintf(s,"%i KHz",shf.mItem);
      
  shf.l.get(0)->mText=s;
  shf.CW=false;
  shf.CCW=false;
 
  return;
}
//*--------------------------------------------------------------------------------------------
//* FI Shift Update   (CALLBACK from Menu System)
//* Set FI shift label and limits
//*--------------------------------------------------------------------------------------------
void vfoUpdate() {

  char* s=(char*)"  "; 

  if (vfo.mItem < VFOB && vfo.CW == true) {
      vfo.mItem++;
  }
  if (vfo.mItem > VFOA && vfo.CCW == true) {
      vfo.mItem--;
  }

  if (vfo.mItem==VFOA){s=(char*)"A";}
  if (vfo.mItem==VFOB){s=(char*)"B";}
  
  vfo.l.get(0)->mText=s;
  
  vfo.CW=false;
  vfo.CCW=true;
  
  return;
}

//*--------------------------------------------------------------------------------------------
//* LckUpdate Callback
//* Control VFO Lock
//*--------------------------------------------------------------------------------------------
void LckUpdate(){
  char* s=(char*)"    "; 

  if (lck.mItem < 1 && lck.CW == true) {
      lck.mItem++;
  }
  if (lck.mItem > 0 && lck.CCW == true) {
      lck.mItem--;
  }
  switch(lck.mItem) {
    case 0:                          {s=(char*)"Off";vx.VFOlock=false;break;};     
    default:                         {s=(char*)"On";vx.VFOlock=true;break;};  
  }
  
  lck.l.get(0)->mText=s;
  lck.CW=false;
  lck.CCW=false;
  return;
}
//*--------------------------------------------------------------------------------------------
//* ModUpdate Callback
//* Control VFO Mode (DDS/VFO)
//*--------------------------------------------------------------------------------------------
void ModUpdate(){
  char* s=(char*)"    "; 

  if (mod.mItem < 1 && mod.CW == true) {
      mod.mItem++;
  }
  if (mod.mItem > 0 && mod.CCW == true) {
      mod.mItem--;
  }

  switch(mod.mItem) {
      case 0:              {s=(char*)"DDS";break;};
      default:             {s=(char*)"VFO";break;};  
  }
  
  mod.l.get(0)->mText=s;
  mod.CW=false;
  mod.CCW=false;
  return;
}
//*--------------------------------------------------------------------------------------------
//* Step Update
//* Set step limit
//*--------------------------------------------------------------------------------------------
void StepUpdate() {

  char* s=(char*)"            "; 

  if (stp.mItem < 6 && stp.CW == true) {
      stp.mItem++;
  }
  if (stp.mItem > 0 && stp.CCW == true) {
      stp.mItem--;
  }
  switch(stp.mItem) {
    case 0:                          {s=(char*)"   1 Hz";break;};                            
    case 1:                          {s=(char*)"  10 Hz";break;};
    case 2:                          {s=(char*)" 100 Hz";break;}; 
    case 3:                          {s=(char*)"  1 KHz";break;}; 
    case 4:                          {s=(char*)" 10 KHz";break;}; 
    case 5:                          {s=(char*)"100 KHz";break;}; 
    default:                         {s=(char*)"  1 MHz";break;}; 
  }

  stp.l.get(0)->mText=s;
  stp.CW=false;
  stp.CCW=false;
  return;
}
//*--------------------------------------------------------------------------------------------
//* readEEPROM
//* Read specific configuration
//*--------------------------------------------------------------------------------------------
void readEEPROM(){

#if DEBUG 
     sprintf(hi,"EEPROM initialization Band(%i)",EEPROM.read(31));
     Serial.println(hi);
#endif

     byte c=EEPROM.read(35);
     byte b=EEPROM.read(36);

     if ((b>=0) && (b<=1)){lck.set(b);}
     if ((c>=0) && (c<=1)){mod.set(b);}
             
     if ((EEPROM.read(18) >= FI_LOW) && (EEPROM.read(18)<= FI_HIGH)) {
        shf.set(EEPROM.read(18));
     } else {
        shf.set(FI_LOW);  
     }

     return;
}    

//*--------------------------------------------------------------------------------------------
//* writeEEPROM
//* Write specific configuration
//*--------------------------------------------------------------------------------------------

void writeEEPROM() {
  
  
  EEPROM.write(18,shf.get());
  EEPROM.write(35,mod.get());
  EEPROM.write(36,lck.get());
  
  
  return;
}
//*--------------------------------------------------------------------------------------------
//* showBand
//* show Band setting at the display
//*--------------------------------------------------------------------------------------------
void showBand() {

  lcd.setCursor(8,0);
  lcd.print(String(band.getCurrentText()));
 
};      
//*--------------------------------------------------------------------------------------------
//* showMode
//* show DDS Mode
//*--------------------------------------------------------------------------------------------
void showMode() {

  lcd.setCursor(12,0);
  if (mod.mItem==0) {
    lcd.print("D");
  } else {
    lcd.print("V");
  }
  
 
};     
//*--------------------------------------------------------------------------------------------
//* showLock
//* show Lock Mode
//*--------------------------------------------------------------------------------------------
void showLock() {

 //*-- Not associating a symbol at the LCD panel for lock, just the "x" when tuning
 
};     
//*--------------------------------------------------------------------------------------------
//* writeEEPROM
//* Write specific configuration
//*--------------------------------------------------------------------------------------------
void showGUI(){
  
      showBand();
      showLock();
      showMode();
      return;

}
//*--------------------------------------------------------------------------------------------
//* checkBandLimit
//* Verify if the set frequency is within the band limits and correct
//*--------------------------------------------------------------------------------------------
void checkBandLimit(){

#if DEBUG
  sprintf(hi,"setup band=%i",band.get());
  Serial.println(hi);
  
  sprintf(hi,"entry with VFOA=%ld VFOB=%ld",vx.get(VFOA),vx.get(VFOB));
  Serial.println(hi);
#endif

  band.mItem=vx.vfoband[vx.vfoAB];
  stp.mItem=vx.step2code(vx.vfostep[vx.vfoAB]);
  vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);
  
  if (vx.get(VFOA)<vx.loFreq[band.get()]*1000 || vx.get(VFOA)>vx.hiFreq[band.get()]*1000) {vx.set(VFOA,vx.loFreq[band.get()]*1000);}
  if (vx.get(VFOB)<vx.loFreq[band.get()]*1000 || vx.get(VFOB)>vx.hiFreq[band.get()]*1000) {vx.set(VFOB,vx.loFreq[band.get()]*1000);}

  BandUpdate();
  StepUpdate();

#if DEBUG
  sprintf(hi,"exit checkBandLimit with VFOA=%ld VFOB=%ld",vx.get(VFOA),vx.get(VFOB));
  Serial.println(hi);
#endif
  
  return;
}

//*-----------------------------------------------------------------------------------------------------------------------------------
//* setFrequencyHook
//* Device specific frequency display
//*-----------------------------------------------------------------------------------------------------------------------------------
void setFrequencyHook(long int f,FSTR* v) {
 

#if DEBUG  
  sprintf(hi,"Frequency %ld",f);
  Serial.println(hi);
  //sprintf(hi,"FrequencyDDS %ld",fDDS);
  //Serial.println(hi);
#endif
    
//*******************************
//* Setup DDS Frequency         *
//*******************************


  if (stp.get()<3) {
     lcd.print(" ");
     lcd.print((*v).hundreds);
     lcd.print((*v).tens);
  }

  if (stp.get()==0){
     lcd.print((*v).ones);
  }

  
}
//*--------------------------------------------------------------------------------------------
//* setSysOM
//* Setup specific System Vars
//*--------------------------------------------------------------------------------------------
void setSysOM(){
  
  setWord(&USW,CONX,false);
  DDSInit();
  

  return;
}
//*--------------------------------------------------------------------------------------------
//* handleTimerHook
//* Hook for timer tick
//*--------------------------------------------------------------------------------------------

void handleTimerHook(){

}
//*--------------------------------------------------------------------------------------------
//* showVFOHook
//* Hook for the VFO display
//*--------------------------------------------------------------------------------------------
void showVFOHook(){
  lcd.setCursor(0,0);
  //lcd.print("Vfo");
  if (vfo.get()==VFOA){lcd.print("A");} else {lcd.print("B");}
  //lcd.print("");
  return;
}


//*--------------------------------------------------------------------------------------------
//* CATHook
//* Hook for the CAT command processing API
//*--------------------------------------------------------------------------------------------
void CATHook(){

byte b[4];
unsigned long fx=0;
unsigned long k=0;

APISTR a=ft817.a;

#if DEBUG
sprintf(hi,"Entering Hook Processing");
Serial.println(hi);
#endif


switch(a.cmd) {

  case 0x00   :{
               //Serial.println("CAT Command 0x00 (LOCK) -- N/I");
              //*--- Must return 0x00 if not locked 0xf0 if locked, as locked is not supported return always CAT_LOCKED
              a.rc=0x00;
              return;
              }
  case 0x01   : {//Process CAT FREQ (OK)
               //Serial.println("CAT Command 0x01 (FREQ)");
               fx=0;
               k =1000000;
               for (int i=0;i<4;i++) {
                   b[i]=bcd2byte(a.bCAT[i]);
                   fx=fx+(b[i]*k);
                   k=k/100;
               }  
               
               ft817.a.r=false;             //* This command won't require answer, block it
               byte bx=vx.findBand(fx*10);
               if (bx!=band.mItem) {
                   band.mItem=bx;
                   saveMenu();
                   BandUpdate();
                   showPanel();
                   showFreq();
               }
               vx.set(vx.vfoAB,fx*10);
               return;
               }
case 0x03  : {
                //Serial.println("CAT Command 0x03 (READ FREQUENCY and MODE)"); (OK)
                //* Return 5 bytes with status XX XX XX XX 01
                fx=vx.get(vx.vfoAB)/10;
                k=1000000;
                unsigned long j=0;
                for (int i=0;i<4;i++) {
                   j=fx/k;
                   b[i]=byte2bcd((byte)j);
                   fx=fx-(j*k);
                   k=k/100;
                   ft817.sendCAT(b[i]);                   
                 }  
                ft817.sendCAT(0x01);
                ft817.a.r=false;             //* This command won't require answer, block it
                return;
                }
case 0x05   : { //Process CAT RIT ON/OFF command
                //Serial.println("CAT Command 0x05 (RIT ON)");
                ft817.a.rc=0x00;
                ft817.a.r=true;
                return;
                }

case 0x07  : { //Serial.println("CAT Command 0x07 (OPERATING MODE)");
                ft817.a.rc=0x00;
                ft817.a.r=false;
                return;
               }

case 0x08   :{//Serial.println("CAT Command 0x08 (PTT ON)");
              //*--- Must return 0x00 if not keyed 0xf0 if already keyed
              ft817.a.rc=0x00;
              ft817.a.r=true;
              return;              
              }

case 0x10   :{//Serial.println("CAT Command 0x10 (PTT STATUS) UNDOC");
              //*--- Must return 0x00 if not keyed 0xf0 if already keyed
              ft817.a.rc=0x00;
              ft817.a.r=true;
              return;                     
              }            

case 0x80   :{
               //Serial.println("CAT Command 0x80 (LOCK OFF)");
              //*--- Must return 0x00 if already locked 0xf0 if not already locked
              lck.mItem=0;
              ft817.a.rc=0xf0;
              ft817.a.r=true;
              showPanel();
              return;    
              } 
              
case 0x81   :{//Serial.println("CAT Command 0x81 (TOGGLE VFOA/B)");
              //*--- Toggle between VFO-A and VFO-B no response 
              vx.swapVFO();
              ft817.a.rc=0x00;
              ft817.a.r=false;
              return;
              }            

 case 0x82   :{//Serial.println("CAT Command 0x82 (SPLIT OFF)");
              //*--- Must return 0x00 if already split 0xf0 if not already split
              ft817.a.rc=0x00;
              ft817.a.r=true;
              return;                     

              }  

case 0x85   :{//Serial.println("CAT Command 0x82 (RIT OFF)");
              //*--- Must return 0x00 if already split 0xf0 if not already split
              ft817.a.rc=0x00;
              ft817.a.r=true;
              return;                     
              }           

case 0x88   :{//Serial.println("CAT Command 0x88 (PTT OFF)");
              //*--- Must return 0x00 if already keyed 0xf0 if not already keyed
              ft817.a.rc=0x00;
              ft817.a.r=true;
              return;                     
              }           

case 0xa7   :{//Serial.println("CAT Command 0xa7 (Radio Configuration) UnDoc");
              //*--- Return an area of data, fake answer based on recommendation from KA7OEI web page 
              ft817.sendCAT(0xA7);
              ft817.sendCAT(0x02);
              ft817.sendCAT(0x00);
              ft817.sendCAT(0x04);
              ft817.sendCAT(0x67);
              ft817.sendCAT(0xD8);
              ft817.sendCAT(0xBF);
              ft817.sendCAT(0xD8);
              ft817.sendCAT(0xBF);
              //*---    
              ft817.clearCAT();
              ft817.a.rc=0x00;
              ft817.a.r=true;
              return;
              }           
case 0xba   :{
               //Serial.println("CAT Command 0xba (Unknown) UnDoc");
              //*--- Returns 0x00 
              ft817.a.rc=0x00;
              ft817.a.r=true;
              return;
              }     
case 0xbd   :{//Serial.println("CAT Command 0xbd (Read TX Meter) UnDoc");
              //*--- Returns 0x00 on RX return [xx][yy] on TX with BCD encoding of PWR,VSWR,ALC,MOD 
              ft817.sendCAT(0x00);  //* PWR & VSWR     --- TO BE IMPLEMENTED FROM METER SIGNAL --
              ft817.sendCAT(0x00);  //* ALC & MOD 
              ft817.clearCAT();
              ft817.a.rc=0x00;
              ft817.a.r=false;
              return;
              }               
case 0xe7   :{//Serial.println("CAT Command 0xe7 (Read Receiver status)");
              //*--- Returns [byte] with receiver status 
              ft817.a.rc=0x00;
              ft817.a.r=true;
              return;
              }
case 0xf5   :{
               //Serial.println("CAT Command 0xf5 (Set clarifier status)");
              //*--- receive [dir][x][10/1 KHz][100/10 Hz] 
              ft817.a.rc=0x00;
              ft817.a.r=false;
              ft817.clearCAT();
              return;
              }              
case 0xf7   :{//Serial.println("CAT Command 0xf7 (Read TX Status)");
              //*--- return [byte] with status 
              ft817.a.rc=0x00;
              ft817.a.r=true;
              ft817.clearCAT();
              return;
              }              

 //*--- Several not implemented
 case 0xf9:
 case 0xbe:
 case 0xbb:
 case 0xbc:
 case 0x8f:
 case 0x0f:
 case 0x09:
 case 0x0a:
 case 0x0b:
              ft817.a.rc=0x00;
              ft817.a.r=false;
              ft817.clearCAT();
              return;
 
 default     : {
              ft817.a.rc=0x00;
              ft817.a.r=false;
              ft817.shiftCAT(); 
              return;
               }  
                
}

//*--------------------------------------------------------------------------------------------
//* Default return from processing
//*--------------------------------------------------------------------------------------------
a.rc=0x00;
return;
}

#endif

