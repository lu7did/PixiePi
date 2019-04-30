//*--------------------------------------------------------------------------------------------------
//* DDSPlus Firmware Version 1.0
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de DDS  en su version inicial
//* Implementación 
//*         PICOFM      Transceiver basado en DRA881V
//*         SINPLEA     Receiver SDR usando shield Arduino Elektor
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//* Plataforma: Arduino NANO/UNO/Mega
//* LCD: 16x2 HD44780 o equivalente
//* Standard Arduino PIN Assignment
//*   A0 - LCDKeyShield (Joystick)
//*   A1 - DDS DATA (reserved)
//*   A2 - DDS RESET (reserved)
//*   A3 - PAD CW
//*   A4 - Power (HI/LO)
//*   A5 - Meter 
//*   
//*   LCD Handling
//*   D2 - Encoder 
//*   D3 - Encoder
//*   D4 - DB4
//*   D5 - DB5
//*   D6 - DB6
//*   D7 - DB7
//*   D8 - E
//*   D9 - RW
//*   D10- BackLigth
//*
//*   Transceiver control
//*   D0 - RxD
//*   D1 - TxD

//*   D11- DDS W_CLK (reserved)
//*   D12- DDS FU_UD (reserved)
//*   D13- KEYER
//*
//*-------------------------------------------------------------------------------------------------------
//*----- Program Reference data
//*-------------------------------------------------------------------------------------------------------

#define DEBUG         false
#define PICOFM        false
#define SINPLEA       true


//*-------- Copyright and Program Build information

#define PROG_BUILD  "080"
#define COPYRIGHT "(c) LU7DID 2018"


//*----------------------------------------------------------------------------------
//*  System Status Word
//*----------------------------------------------------------------------------------
//*--- Master System Word (MSW)

#define CMD       B00000001
#define GUI       B00000010
#define PTT       B00000100
#define DRF       B00001000
#define DOG       B00010000
#define LCLK      B00100000
#define SQL       B01000000
#define BCK       B10000000

//*----- Master Timer and Event flagging (TSW)

#define FT1       B00000001
#define FT2       B00000010
#define FT3       B00000100
#define FT4       B00001000
#define FCLOCK    B00010000
#define FTS       B00100000
#define FDOG      B01000000
#define FBCK      B10000000

//*----- UI Control Word (USW)

#define BBOOT     B00000001
#define BMULTI    B00000010
#define BCW       B00000100
#define BCCW      B00001000
#define SQ        B00010000
#define MIC       B00100000
#define KDOWN     B01000000
 
#define BUSY      B10000000       //* Used for Squelch Open in picoFM and for connected to DDS on sinpleA
#define CONX      B10000000

//*----- Joystick Control Word (JSW)

#define JLEFT     B00000001
#define JRIGHT    B00000010
#define JUP       B00000100
#define JDOWN     B00001000

//*----- EEPROM signature

#if SINPLEA
#define EEPROM_COOKIE  0x1f
#endif

#if PICOFM
#define EEPROM_COOKIE  0x0f
#endif

#define EEPROM_RESET   false
#define SAVE_TIME      2000


//*----------------------------------------[DEFINITION]----------------------------------------------

//*--- Control delays

#define DELAY_DISPLAY 4000     //delay to show the initial display in mSecs
#define DELAY_SAVE    1000     //delay to consider the frequency needs to be saved in mSecs
#define LCD_DELAY     1000
#define CMD_DELAY      100
#define PTY_DELAY      200
#define DIM_DELAY    30000
#define DOG_DELAY    60000
#define BLINK_DELAY   1000
#define SAVE_DELAY    2000

#define FORCEFREQ     0
#define LCD_ON        1
#define LCD_OFF       0

#define QUEUEMAX  16        // Queue of incoming characters 

//*----------------------------------------[INCLUDE]-------------------------------------------------
#include <LiquidCrystal.h>
#include <stdio.h>
#include <EEPROM.h>

#include "MemSizeLib.h"
#include "VFOSystem.h"
#include "ClassMenu.h"
#include "CATSystem.h"

//*---- Debug buffer
char hi[80];

//*-------------------------------------------------------------------------------------------------
//* Define callback functions
//*-------------------------------------------------------------------------------------------------
void  Encoder_san();

//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//*------ define root for all the menu system
//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
MenuClass menuRoot(NULL);



//*-------------------------------------------------------------------------------------------------
//* Define class to manage VFO
//*-------------------------------------------------------------------------------------------------
void showFreq();                       //Prototype fur display used on callback
VFOSystem vx(showFreq,NULL,NULL,NULL);

//*-------------------------------------------------------------------------------------------------
//* Define class to manage CAT
//*-------------------------------------------------------------------------------------------------
void catAPI();
CATSystem ft817(catAPI);

//*--------------------------------------------------------------------------------------------
//*---- Definitions for various LCD display shields
//*--------------------------------------------------------------------------------------------
       LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  
       
       //==============================================
       //Set Encoder pin
       //==============================================
       const int Encoder_A =  3;            // Incremental Encoder singal A is PD3 
       const int Encoder_B =  2;            // Incremental Encoder singal B is PD2 
       
       unsigned int Encoder_number=0;
       
       int state=0;

       // define some values used by the panel and buttons
       int lcd_key     = 0;
       int adc_key_in  = 0;
       
       #define btnRIGHT  0
       #define btnUP     1 
       #define btnDOWN   2 
       #define btnLEFT   3
       #define btnSELECT 4
       #define btnNONE   5   
       #define btnEncodeOK  6
        


//*--------------------------------------------------------------------------------
//*--- Pseudo Real Time Clock  
//*--------------------------------------------------------------------------------

byte mm=0;
byte ss=0;

unsigned long Tclk=1000;
byte btnPrevio=btnNONE;

//*--------------------------------------------------------------------------------
//*--- Timer related definitions
//*--------------------------------------------------------------------------------

#define T_1mSec 65473 //Timer pre-scaler for 1 KHz or 1 msec

uint16_t T1=0;
uint16_t T2=0;
uint16_t T3=0;
uint16_t T4=0;
uint16_t TV=0;
uint16_t TS=0;
uint16_t TDOG=0;
uint16_t TBCK=0;
uint16_t TDIM=0;

//*-----------------------------------------------------------------------------------
//*--- Define System Status Words
//*-----------------------------------------------------------------------------------

byte MSW = 0;
byte TSW = 0;
byte USW = 0;
byte JSW = 0;

//*-----------------------------------------------------------------------------------
//*--- Serial port management areas
//*-----------------------------------------------------------------------------------

char serialQueue[QUEUEMAX]; // Actual Queue space [a0,a1,...,an]
byte pQueue = 0;            // Pointer to next position to use
byte inState= 0;
byte inCmd=0;


int_fast32_t timepassed = millis(); // int to hold the arduino miilis since startup
int_fast32_t menupassed = millis();

int memstatus   = 1;           // value to notify if memory is current or old. 0=old, 1=current.


//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//*  Model specific includes
//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "picoFM.h"
#include "sinpleA.h"

//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//*  Setup
//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {

  //*--- PinOut Definition

  lcd.begin(16, 2);   // start the LCD library  
  pinMode(10,OUTPUT);
   
   //*---- Turn On LED Backlight (1) or off (0)
   
  digitalWrite(10,LCD_ON);
  TDIM=DIM_DELAY;
   
   
  lcd.setCursor(0,0);  
   
  pinMode(Encoder_A, INPUT); 
  pinMode(Encoder_B, INPUT); 
   
  digitalWrite(Encoder_A, 1);
  digitalWrite(Encoder_B, 1);
   
//*========================================
  attachInterrupt(1, Encoder_san, FALLING);        //interrupts: numbers 0 (on digital pin 2) and 1 (on digital pin 3).

//*****************************
//* Module specific           *
//*****************************
pinSetup();
  
//*=====================================================================================
  
  lcd.setCursor(0, 0);        // Place cursor at [0,0]
  lcd.print(String(PROGRAMID)+" v"+String(PROG_VERSION)+"-"+String(PROG_BUILD));

  lcd.setCursor(0, 1);        // Place cursor at [0,1]
  lcd.print(String(COPYRIGHT));

  delay(DELAY_DISPLAY);
  lcd.clear();

//**********************************
//* Device specific initialization *
//**********************************
  setSysOM();

//*---- Define the VFO System parameters (Initial Firmware conditions)

  vx.setVFOdds(setDDSFreq);

  vx.setVFOBand(VFOA,VFO_BAND_START);
  vx.set(VFOA,VFO_START);
  vx.setVFOStep(VFOA,VFO_STEP_1KHz);
  vx.setVFOLimit(VFOA,VFO_START,VFO_END);

  vx.setVFOBand(VFOB,VFO_BAND_START);
  vx.set(VFOB,VFO_START);
  vx.setVFOStep(VFOB,VFO_STEP_1KHz);
  vx.setVFOLimit(VFOB,VFO_START,VFO_END);

  vx.setVFO(VFOA);

  //*ft817.v=vx;
 
//*--- Interrupt manipulation
//#if LCD_STANDARD
//
//  PCICR  = 0b00000010;        // 1. PCIE1: Pin Change Interrupt Enable 1
//  PCMSK1 = 0b00011111;        // Enable Pin Change Interrupt for A0, A1, A2, A3, A4
//
//#endif
  
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  //*---> TCNT1 = 34286; // preload timer 65536-16MHz/256/2Hz
  TCNT1 = T_1mSec; // preload timer 65536- [(16000000/256)/f(Hz)]
  TCCR1B |= (1 << CS12); // 256 prescaler
  TIMSK1 |= (1 << TOIE1); // enable timer overflow interrupt
  interrupts(); // enable all interrupts

//*--- Init serial port and establish handshake with DRA018F (Work as an object later)

  Serial.begin(9600);
  serialQueue[0]=0x00;
  
//*****************************
//* Module specific           *
//*****************************
  defineMenu();   

//*============================================================================================
//*--- Load the stored configuration in EEPROM (if enabled)
//*============================================================================================

/* ============================================================================================
 *  EEPROM Map
 *  ===========================================================================================
 * 00 Squelch
 * 01..07 VFO A
 * 08..14 VFO B
 * 15 VFO A/B
 * 16 PWR
 * 17 WDG
 * 18 SHIFT
 * 19 RPT
 * 20 SPD
 * 21 BDW
 * 22 HPF
 * 23 LPT
 * 24
 * 25 CTCSS
 * 26 VFO
 * 27 MSW
 * 28 USW
 * 29 TSW
 * 30 BAND VFO A
 * 31 BAND VFO B
 * 32 STEP VFOA
 * 33 STEP VFOB
 * 34 EEPROM COOKIE
 * 35 MODE (VFO/DDS)
 * 36 VFO LOCK
 * 
 */
#if EEPROM_RESET
    EEPROM.write(34,0);
#endif    

  if (FORCEFREQ == 0) {
      
     if (EEPROM.read(34)==EEPROM_COOKIE) {


            //*---- Recover frequency for VFOA
            char hi[12];    
            sprintf(hi,"%3d%1d%1d%1d%1d%1d%1d",EEPROM.read(1),EEPROM.read(2),EEPROM.read(3),EEPROM.read(4),EEPROM.read(5),EEPROM.read(5),EEPROM.read(6),EEPROM.read(7));
            vx.set(VFOA,String(hi).toInt());

            
            //*---- Recover frequency for VFOB
            sprintf(hi,"%3d%1d%1d%1d%1d%1d%1d",EEPROM.read(8),EEPROM.read(9),EEPROM.read(10),EEPROM.read(11),EEPROM.read(12),EEPROM.read(13),EEPROM.read(14));
            vx.set(VFOB,String(hi).toInt());            

#if DEBUG
//*----------------------------------------------------------------------
            sprintf(hi,"EEPROM Read VFOA=%ld",vx.get(VFOA));
            Serial.println(hi);

            sprintf(hi,"EEPROM Read VFOB=%ld",vx.get(VFOB));
            Serial.println(hi);
//*----------------------------------------------------------------------            
#endif            

            vx.vfoAB = EEPROM.read(15);        
            vfo.set(EEPROM.read(26));
            
            vx.vfostep[VFOA]=vx.code2step(EEPROM.read(32));
            vx.vfostep[VFOB]=vx.code2step(EEPROM.read(33));            
            
            vx.vfoband[VFOA]=EEPROM.read(30);
            vx.vfoband[VFOB]=EEPROM.read(31);
           
            readEEPROM();

            MSW = EEPROM.read(27);
            USW = EEPROM.read(28);
            TSW = EEPROM.read(29);
     }
  } 

//*********************************
//* Module specific function      *
//*********************************
  checkBandLimit();
  

//**********************************************************************************************
//*--- Initial value for system operating modes
//**********************************************************************************************

  setWord(&MSW,CMD,false);
  setWord(&MSW,GUI,false);
  setWord(&MSW,PTT,true);
  setWord(&MSW,DRF,false);
  setWord(&MSW,DOG,false);
  setWord(&MSW,LCLK,false);
  setWord(&MSW,SQL,false);

  setWord(&USW,BBOOT,true);
  setWord(&USW,BMULTI,false);
  setWord(&USW,BCW,false);
  setWord(&USW,BCCW,false);
  setWord(&USW,SQ,false);
  setWord(&USW,MIC,false);
  setWord(&USW,KDOWN,false);

  
  setWord(&JSW,JLEFT,false);
  setWord(&JSW,JRIGHT,false);
  setWord(&JSW,JUP,false);
  setWord(&JSW,JDOWN,false);


  //vx.updateVFO(vx.vfoAB,vx.vfo[vx.vfoAB]);

//*=========================================================================================================
#if DEBUG

//*--- Print Serial Banner (TEST Mode Mostly)
  sprintf(hi,"%s %s Compiled %s %s",PROGRAMID,PROG_VERSION,__TIME__, __DATE__);
  Serial.println(hi);
  sprintf(hi,"(c) %s",COPYRIGHT);
  Serial.print("RAM Free=");
  Serial.println(freeMemory());

#endif
//*=========================================================================================================

//***********************************
//* End of initialization and setup *
//***********************************
  showPanel();
  showFreq();

}
//*****************************************************************************************************
//*                               Menu Finite Status Machine (FSM)
//*
//*****************************************************************************************************
//*--------------------------------------------------------------------------------------------
//* menuText
//* defines the text based on Menu FSM state
//*--------------------------------------------------------------------------------------------
String menuText(byte mItem) {

//*---- Here CMD==true and GUI==true so it's a second level menu

   byte i=menuRoot.get();
   MenuClass* z=menuRoot.getChild(i);
   return z->getText(i);
 
}
//*--------------------------------------------------------------------------------------------
//* showFreq
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void showFreq() {

  FSTR v;  
    
  long int f=vx.get(vx.vfoAB); 
  vx.computeVFO(f,&v);

#if DEBUG

  sprintf(hi,"showFreq f=%ld",f);
  Serial.println(hi);

#endif


//*---- Prepare to display
  lcd.setCursor(2, 1);
  if (v.millions <10) {
    lcd.print(" ");
  }
  
  lcd.print(v.millions);
  lcd.print(".");
  lcd.print(v.hundredthousands);
  lcd.print(v.tenthousands);
  lcd.print(v.thousands);

//**************************************
//* Setup device specific frequency    *
//**************************************
  setFrequencyHook(f,&v); 
  timepassed = millis();
  memstatus = 0; // Trigger memory write

};


#if PICOFM
//****************************************************
//* picoFM GUI and Panel related unique functions    *
//****************************************************

//*--------------------------------------------------------------------------------------------
//* showRpt
//* show repeater operation mode at the display
//*--------------------------------------------------------------------------------------------
void showRpt() {

  
  lcd.setCursor(0,0);
  if (rpt.get()==0){lcd.print(" ");} else {lcd.print("S");}


};
//*--------------------------------------------------------------------------------------------
//* showCTC
//* show CTCSS operation mode at the display
//*--------------------------------------------------------------------------------------------
void showCTC() {


  lcd.setCursor(1,0);
  if (ctc.get()==0){lcd.print(" ");} else {lcd.print("T");}

};
//*--------------------------------------------------------------------------------------------
//* showPwr
//* show power level at the display
//*--------------------------------------------------------------------------------------------
void showPwr() {

  
  lcd.setCursor(11,0);
  if (pwr.get()==0) {lcd.print("L");} else {lcd.print("H");}
  return;


}
//*--------------------------------------------------------------------------------------------
//* showMet
//* show meter
//*--------------------------------------------------------------------------------------------
void showMet() {


     //showMeter(&sqlMeter,sqlMeter.v);
     return;

}
//*--------------------------------------------------------------------------------------------
//* showSPD
//* show HILO operation mode at the display
//*--------------------------------------------------------------------------------------------
void showSPD() {


  lcd.setCursor(5,0);
  if (spd.get()==0) {lcd.print(" ");} else {lcd.print("Z");}


};
//*--------------------------------------------------------------------------------------------
//* showSQL
//* show SQL operation mode at the display
//*--------------------------------------------------------------------------------------------
void showSQL() {


  lcd.setCursor(12,0);
  if (getWord(MSW,CMD)==true) {return;}
  if (digitalRead(A3)==LOW && getWord(MSW,CMD)==false) {lcd.write(byte(5));} else {lcd.write(byte(0));}

  
};

//*--------------------------------------------------------------------------------------------
//* showPTT
//* show PTT operation mode at the display
//*--------------------------------------------------------------------------------------------
void showPTT() {

  lcd.setCursor(4,0);
  if (getWord(MSW,DOG)==true && wdg.get()!=0){lcd.write(byte(7)); lcd.setCursor(4,0);lcd.blink();return;}
  if (getWord(MSW,PTT)==false) {lcd.write(byte(6)); lcd.setCursor(4,0);lcd.noBlink();} else {lcd.print(" ");lcd.setCursor(4,0);lcd.noBlink();}


};

//*--------------------------------------------------------------------------------------------
//* showDRF
//* show DRF filter at the display
//*--------------------------------------------------------------------------------------------
void showDRF() {
  
  lcd.setCursor(3,0);
  if (getWord(MSW,DRF)==false){lcd.print(char(174));} else {lcd.print(char(42));}


};
//*--------------------------------------------------------------------------------------------
//* showDog
//* show if watchdog is enabled or not
//*--------------------------------------------------------------------------------------------
void showDog() {

  
  lcd.setCursor(9,0);
  if (wdg.get()==0){lcd.print(" ");} else {lcd.print("W");}
  

};

#endif


#if SINPLEA
//****************************************************
//* sinpleA GUI and Panel related unique functions    *
//****************************************************
//*--------------------------------------------------------------------------------------------
//* showDDS
//* show DDS connection or error condition at the display
//*--------------------------------------------------------------------------------------------
void showDDS() {

#if DEBUG
  Serial.println("showDDS");
#endif
  
  lcd.setCursor(13,0);
  lcd.print("[");
  if (getWord(USW,CONX)==true) {lcd.print("*");} else {lcd.print(" ");}
  lcd.print("]");
 
};

#endif

//****************************************************
//* Common GUI and Panel related unique functions    *
//****************************************************
//*--------------------------------------------------------------------------------------------
//* showVFO
//* show VFO filter at the display
//*--------------------------------------------------------------------------------------------
void showVFO() {

  showVFOHook();
  

};


//*--------------------------------------------------------------------------------------------
//* showPanel
//* show frequency or menu information at the display
//*--------------------------------------------------------------------------------------------
void showPanel() {

   
   if (getWord(MSW,CMD)==false) {
      
      lcd.clear();
      lcd.setCursor(0,0);
      
      //showFreq();
      showVFO();


//*--- Device specific GUI builter
      showGUI();
      showDDS();
//*----           
      return;
   }


//*--- if here then CMD==true

   byte i=menuRoot.get();
   MenuClass* z=menuRoot.getChild(i);
    
      
   if (getWord(MSW,GUI)==false) {
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("<"+String(i)+"> "+String(menuRoot.getCurrentText()));
      lcd.setCursor(1,1);
      lcd.print("  "+String(z->getCurrentText() ));

      return;
      
   } else {
 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("<"+String(menuRoot.get())+"> "+String(menuRoot.getText(menuRoot.get())));
      lcd.setCursor(0,1);
      lcd.print(">");
      lcd.print("  "+String(z->getCurrentText()));
      return;
   }

}
//*----
//* Show Mark
//*----
void showMark(){
      lcd.setCursor(0,1);
      lcd.print(">"); 
}
//*----
//* UnshowMark
//*----
void unshowMark(){
       lcd.setCursor(0,1);
      lcd.print(" "); 
}
//*----------------------------------------------------------------------------------------------------
//* menuFSM
//* come here with CLI==true so it's either one of the two menu levels
//*----------------------------------------------------------------------------------------------------
void menuFSM() {

   if (getWord(MSW,CMD)==false) {   //* VFO mode, no business here!
      return;
   }

   if (getWord(MSW,GUI)==false) {  //It's the first level     
      menuRoot.move(getWord(USW,BCW),getWord(USW,BCCW));
      showPanel();
      return;
   }

//*---- Here CMD==true and GUI==true so it's a second level menu

     byte i=menuRoot.get();
     MenuClass* z=menuRoot.getChild(i);
     z->move(getWord(USW,BCW),getWord(USW,BCCW));
     showPanel();
     return;
   
}
//*--------------------------------------------------------------------------------------------
//* showSave
//* save legend
//*--------------------------------------------------------------------------------------------
void showSave(){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Saving....");
  
}


//*--------------------------------------------------------------------------------------------
//* doSave
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void doSave() {

      showSave();      
      delay(DELAY_SAVE);

      

//**************************************************
//* Device specific parameter saving               *
//**************************************************
      saveMenu();
     
//***************************************************

      menuRoot.save();

      setWord(&MSW,CMD,false);
      setWord(&MSW,GUI,false);

      
      showPanel();
      showFreq();
}

//*****************************************************************************************************
//*                            Command Finite Status Machine (FSM)
//*
//*****************************************************************************************************
//*----------------------------------------------------------------------------------------------------
//* backupFSM
//* come here with CLI==true so it's either one of the two menu levels
//*----------------------------------------------------------------------------------------------------
void backupFSM() {
     
     byte i=menuRoot.get();
     MenuClass* z=menuRoot.getChild(i);
     z->backup();

}
//*----------------------------------------------------------------------------------------------------
//* restoreFSM
//* come here with CLI==true so it's either one of the two menu levels
//*----------------------------------------------------------------------------------------------------
void restoreFSM() {

      byte i=menuRoot.get();
      MenuClass* z=menuRoot.getChild(i);
      z->restore();  
}


//*----------------------------------------------------------------------------------------------------
//* processVFO
//* come here to update the VFO
//*----------------------------------------------------------------------------------------------------
void processVFO() {

   
   if (getWord(USW,BCW)==true) {

       if (vx.isVFOLocked()==false){
          vx.updateVFO(vx.vfoAB,vx.vfostep[vx.vfoAB]);      
          lcd.setCursor(0,1);
          lcd.print((char)126);
       } else {
          vx.updateVFO(vx.vfoAB,0);      
          lcd.setCursor(0,1);
          lcd.print((char)183);
       }
   
   }
   
   if (getWord(USW,BCCW)==true) {
       
       if (vx.isVFOLocked()==false){
          vx.updateVFO(vx.vfoAB,-vx.vfostep[vx.vfoAB]); 
          lcd.setCursor(0,1);
          lcd.print((char)127);
       } else {
          vx.updateVFO(vx.vfoAB,0);
          lcd.setCursor(0,1);
          lcd.print((char)183);
       }
   
   }
   
   T4=LCD_DELAY;
 
}
//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages the reading of the KeyShield buttons
//*--------------------------------------------------------------------------------------------------
void readKeyShield() {

 

 if(readButton()==true && getWord(USW,KDOWN)==true) {
  
   if (millis()-menupassed>=SAVE_DELAY) {
      setWord(&USW,KDOWN,false);
      setWord(&USW,BMULTI,false);
      setWord(&MSW,LCLK,true);
      return;
   }
   
   setWord(&USW,KDOWN,false);
   setWord(&USW,BMULTI,true);
   setWord(&MSW,LCLK,false);
   return;
 }
 

 if(readButton()==false && getWord(USW,KDOWN)==false) {
  
    setWord(&USW,KDOWN,true);
    menupassed=millis();
   return;
 }
 return; 

}

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages the command and VFO handling FSM
//*--------------------------------------------------------------------------------------------------
void CMD_FSM() {

   readKeyShield();

#if PICOFM

   //*-------------------------------------------------------------------------
   //* Operate Watchdog
   //*-------------------------------------------------------------------------
       if (getWord(TSW,FDOG)==true && wdg.get() !=0) {
          setWord(&TSW,FDOG,false);
          setWord(&MSW,DOG,true);
          //showMeter(&sqlMeter,sqlMeter.v);
          digitalWrite(PTTPin,LOW);
          
          if (rpt.get()!=0) {vx.swapVFO();showVFO();showFreq();} //If split enabled        
          showPTT();        
       }

#endif

   //*-------------------------------------------------------------------------------
   //* Handle PTT  (detect MIC PTT pressed and PTT signal not activated) just pressed
   //*-------------------------------------------------------------------------------
       if (digitalRead(A4)==LOW && getWord(MSW,PTT)==true){

        
           setWord(&MSW,PTT,false);

#if PICOFM
           
           digitalWrite(PTTPin,HIGH);    //*-- Prende TX
           //showMeter(&pwrMeter,pwrMeter.v);
           if (rpt.get()!=0) {vx.swapVFO();showVFO();showFreq();} //If split enabled
#endif
           
           digitalWrite(10,LCD_ON);
           TDIM=DIM_DELAY; 

#if PICOFM           
           if (wdg.get()!=0) {TDOG=DOG_DELAY;}
           
           setWord(&MSW,DOG,false);
           showPTT();
#endif
           
       } else {
        
          if (digitalRead(A4)==HIGH && getWord(MSW,PTT)==false) {
              
              setWord(&MSW,PTT,true);
#if PICOFM
              
              digitalWrite(PTTPin,LOW);   //*-- Apaga TX
              //showMeter(&sqlMeter,sqlMeter.v);
              if (rpt.get()!=0) {vx.swapVFO();showVFO();} //If split enabled
              //showFreq();
              setWord(&MSW,DOG,false);
              showPTT();
#endif              
              showFreq();
 
          
          }   
       }

#if PICOFM
       
   //*-------------------------------------------------------------------------
   //* PTT == true means not activated (RX mode)
   //*-------------------------------------------------------------------------
   if (getWord(MSW,PTT) == true) {

     showSQL();
   }  
     //*--- Lectura analógica int s=readV(SQLPIN,SQLMAX,SQLSCALE);
     //sqlMeter.v=readMeter(&sqlMeter);

#endif   
   
//*-----------------------------------------------------------------------------
//* Menu management with KEY Shield cualquier cambio enciende LED
//*-----------------------------------------------------------------------------

   if ( getWord(USW,BMULTI)==true ||
        getWord(USW,BCW)==true    ||
        getWord(USW,BCCW)==true   ||
        getWord(MSW,LCLK)==true ) {
          
        digitalWrite(10,LCD_ON);
        TDIM=DIM_DELAY; 
   }

//*------------------------------------------------------------------------------
//* If PTT==false (TX) clear all signals, thus disable any activity of the keys
//*------------------------------------------------------------------------------
   if (getWord(MSW,PTT)==false) {
    
      setWord(&USW,BMULTI,false);
      setWord(&USW,BCCW,false);
      setWord(&USW,BCW,false);
      setWord(&MSW,LCLK,false);
      setWord(&JSW,JLEFT,false);
      setWord(&JSW,JRIGHT,false);
      setWord(&JSW,JUP,false);
      setWord(&JSW,JDOWN,false);
      
   }

//*-------------------------------------------------------------------------------
//* PTT==true (RX) and CMD==false (VFO)
//*-------------------------------------------------------------------------------
   if (getWord(MSW,CMD)==false && getWord(MSW,PTT)==true) {      //S=0 VFO Mode   

      
//*----------------------------------------------------------------------------
//*---- Process MULTI Button (VFO Mode)
//*----------------------------------------------------------------------------
      
      if (getWord(USW,BMULTI)==true) { //S (0)->(1) enter CMD mode
         
         setWord(&MSW,CMD,true);
         setWord(&MSW,GUI,false);
         showPanel();
         menuFSM();
         setWord(&USW,BMULTI,false);
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);

         return;
      }


//*----------------------------------------------------------------------------
//*---- Erase the GUI update mark after a timeout
//*----------------------------------------------------------------------------     

      if (getWord(TSW,FT4)) {  //Clear GUI clue if present
         lcd.setCursor(0,1);
         lcd.print(" ");
         setWord(&TSW,FT4,false);
      }
      
//*--------------------------------------------------------------------------------------
//*---- Process rotation of VFO encoder   (VFO Mode)
//*--------------------------------------------------------------------------------------
      
      if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=0 operates VFO and clear signals
         processVFO();
      }

      if (vx.isVFOChanged(vx.vfoAB)==true) {

 //************************************
 //* Device specific reset            *
 //************************************
         doSetGroup();
         vx.resetVFO(vx.vfoAB);
       }
       
       setWord(&USW,BCW,false);
       setWord(&USW,BCCW,false);
       return;             

   }
//******************************************************************************************
//* Process menu commands
//******************************************************************************************
//*---- If here is in S=1 or higher (CMD mode)

   if(readButton()==false && getWord(USW,KDOWN)==true && (millis()-menupassed>=SAVE_DELAY) && getWord(MSW,CMD)==true){
     if (getWord(MSW,GUI)==false) {  
         showMark();
     } else {
         //showSave();
         doSave();
     }
   }
   
   if (getWord(MSW,GUI)==false && getWord(MSW,PTT)==true) {   //S=1 pure command mode

//*--- Process S=1 transitions
      
      if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=1 operates Menu at first level and clear signals
         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;             
       }

       if (getWord(USW,BMULTI)==true) { //S=1 (1)->(0)
          setWord(&USW,BMULTI,false);
          setWord(&MSW,CMD,false);
          setWord(&MSW,GUI,false);
          showPanel();
          setWord(&USW,BCW,false);
          setWord(&USW,BCCW,false);
          return;             
       }
       
       if (getWord(MSW,LCLK)==true) { //S=1 (1)->(2)
          setWord(&MSW,LCLK,false);
          setWord(&MSW,CMD,true);
          setWord(&MSW,GUI,true);
          backupFSM();
          showPanel();
          setWord(&USW,BCW,false);
          setWord(&USW,BCCW,false);
          return;             
       }
   }

//*---- Only with GUI at this level, check anyway

   if (getWord(MSW,GUI)==true && getWord(MSW,PTT)==true) {

      if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=1 operates Menu at first level and clear signals
         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;             
       }
       
       if (getWord(USW,BMULTI)==true) { //S=2 (2)->(1) Restoring
          setWord(&USW,BMULTI,false);
          setWord(&MSW,CMD,true);
          setWord(&MSW,GUI,false);
          restoreFSM();
          showPanel();
          setWord(&USW,BCW,false);
          setWord(&USW,BCCW,false);
          return;             
       }

       if (getWord(MSW,LCLK)==true) { //S=2 (2)->(1) Saving
          setWord(&MSW,LCLK,false);
          setWord(&MSW,CMD,true);
          setWord(&MSW,GUI,false);
          doSave();
          showPanel();
          setWord(&USW,BCW,false);
          setWord(&USW,BCCW,false);
          return;             
       }
   }
       
   setWord(&USW,BCW,false);
   setWord(&USW,BCCW,false);             

}


//*============================================================================================
//*--------------------------------------------------------------------------------------------
//* swapVFO
//* returns the "other" VFO
//*--------------------------------------------------------------------------------------------
byte swapVFO() {

  if (vx.vfoAB==VFOA) {
     return VFOB;
  }
  return VFOA;
  
}
//*--------------------------------------------------------------------------------------------
//* checkPriority
//* send commands related to priority channel management
//*--------------------------------------------------------------------------------------------
void checkPriority() {
  

  
}
//*------------------------------------------------------------------------------------------------------
//* serialEvent is called when a serial port related interrupt happens
//*------------------------------------------------------------------------------------------------------
void serialEvent() {
  
 while (Serial.available() && !ft817.isQueueFull()) {
      char inChar = (char)Serial.read();                //get new character from Serial port
      ft817.addQueue(inChar);    //add to Queue
#if DEBUG      
      sprintf(hi,"Recibido byte[%c]",inChar);
      Serial.println(hi);
#endif      

  }
  
  
}
//*------------------------------------------------------------------------------------------------------
//* callback to CAT API
//*------------------------------------------------------------------------------------------------------
void catAPI(){
     CATHook();
}
//*****************************************************************************************************
//*                               Manages Meter
//*
//*****************************************************************************************************

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages frequency display at the LCD
//*--------------------------------------------------------------------------------------------------
void LCD_FSM() {


  //*--- Logic to sense the SQL reference, if changed display value and then restore once finished
  
  if (getWord(MSW,CMD)==false){
    
         
  }

 
  
}

//*======================================================================================================
//*                                   Master Loop Control
//*
//*======================================================================================================
void loop() {
 
  
    //*--- Handle commands 
  CMD_FSM();
  
  //*--- Update LCD Display
  LCD_FSM();
 
  //*--- Write memory to EEPROM when it has been quiet for a while (2 secs)

  if (memstatus == 0) {
    if (timepassed + SAVE_TIME < millis()) {
      storeMEM();
    }
  } 
}
//*----------------------------------------[INTERRUPT]---------------------------------------------------
//*======================================================================================================
//*                                   Master Timer Sub-System
//*
//*======================================================================================================
//*--------------------------------------------------------------------------------------------------
//* Timer Interrupt handler
//*      (TIMER1): This is the interrupt handler for TIMER1 set as 1 msec or 1 KHz
//*--------------------------------------------------------------------------------------------------
ISR(TIMER1_OVF_vect) // interrupt service routine that wraps a user defined function supplied by attachInterrupt
{

  TCNT1 = T_1mSec; // preload timer

  
//*--- Serve Timer 1 (T1) 
  if (T1>0) { 
      T1--;
      if (T1==0){
         setWord(&TSW,FT1,true);
      }
  }
//*--- Serve Timer 2 (T2) 
  if (T2>0) { 
      T2--;
      if (T2==0){
         setWord(&TSW,FT2,true);
      }
  }
//*--- Serve Timer 3 (3) 
  if (T3>0) { 
      T3--;
      if (T3==0){
         setWord(&TSW,FT3,true);
      }
  }
//*--- Serve Timer 4 (T4)

  if (T4>0) {
      T4--;
      if (T4==0) {
         setWord(&TSW,FT4,true);         
      }
  }


//*--- Serve Timer Squelch Display (TS)

  if (TS>0) {
      TS--;
      if (TS==0) {
         setWord(&TSW,FTS,true);         
      }
  }

//*--- Serve Timer Priority Channel (TPTY)

  if (TDOG>0) {
      TDOG--;
      if (TDOG==0) {
         setWord(&TSW,FDOG,true);         
      }
  }

//*--- Serve Timer Priority Channel (TPTY)

  if (TBCK>0) {
      TBCK--;
      if (TBCK==0) {
         setWord(&TSW,FBCK,true);         
      }
  }


//*******************************************
//* Device specifig                         *
//*******************************************
  handleTimerHook();

//*--- Serve real time clock

  if (Tclk>0) {
     Tclk--;
     if (Tclk==0) {
        setWord(&TSW,FCLOCK,true);
        Tclk=1000;
        ss++;
        if (ss==60){
            ss=0;
            mm++;
            if (mm==60) {
                mm=0;
            }
        }

     }
  }
}
//*======================================================================================================
//*                                   Panel Control Sub-System
//*
//*======================================================================================================
//*----------------------------------------[INTERRUPT]---------------------------------------------------
//* System Interrupt Handler
//* Handles interrupt from
//*         * bMulti button
//*         * Rotary encoder Outputs A & B
//*--------------------------------------------------------------------------------------------------
ISR (PCINT1_vect) {

   Encoder_classic();
   
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* Encoder_classic
//* interrupt handler to manage rotary button
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Encoder_classic() {

//*----- Check SQL 
  
  if(!digitalRead(A3)) {
       setWord(&USW,SQ,true);
  }
  
  if(!digitalRead(A4)) {
    setWord(&USW,MIC,true);
  }

  if ((digitalRead(A3)) && (getWord(MSW,SQL)==false)) {
       setWord(&USW,SQ,true);    
  }

  if ((digitalRead(A4)) && (getWord(MSW,PTT)==false)) {
       setWord(&USW,MIC,true);    
  }
  
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* Encoder_san
//* interrupt handler to manage rotary button
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Encoder_san()
{  
    if(digitalRead(Encoder_B))
    {
       setWord(&USW,BCW,true);     //bRotaryCW  = true;
       Encoder_number++;
    } else {
       setWord(&USW,BCCW,true);    //bRotaryCCW = true;
       Encoder_number--;
    }     
    state=1;
    Encoder_classic();

}

//*--------------------------[System Word Handler]---------------------------------------------------
//* getSSW Return status according with the setting of the argument bit onto the SW
//*--------------------------------------------------------------------------------------------------
boolean getWord (byte SysWord, byte v) {

  return SysWord & v;

}
//*--------------------------------------------------------------------------------------------------
//* setSSW Sets a given bit of the system status Word (SSW)
//*--------------------------------------------------------------------------------------------------
void setWord(byte* SysWord,byte v, boolean val) {

  *SysWord = ~v & *SysWord;
  if (val == true) {
    *SysWord = *SysWord | v;
  }

}
//*----------------------------------------[EEPROM]---------------------------------------------------
//* Store frequency at EEPROM
//*--------------------------------------------------------------------------------------------------

void storeMEM() {


  FSTR v;   
  long int f=vx.get(VFOA);
  vx.computeVFO(f,&v);
 
  if (memstatus==1) {return; }

//*=== Recover and store VFO A & B setup
  
  EEPROM.write(1, v.millions);
  EEPROM.write(2, v.hundredthousands);
  EEPROM.write(3, v.tenthousands);
  EEPROM.write(4, v.thousands);
  EEPROM.write(5, v.hundreds);
  EEPROM.write(6, v.tens);
  EEPROM.write(7, v.ones);


  f=vx.get(VFOB);
  vx.computeVFO(f,&v); 

  EEPROM.write(8,  v.millions);
  EEPROM.write(9,  v.hundredthousands);
  EEPROM.write(10, v.tenthousands);
  EEPROM.write(11, v.thousands);
  EEPROM.write(12, v.hundreds);
  EEPROM.write(13, v.tens);
  EEPROM.write(14, v.ones);

  EEPROM.write(15, vx.vfoAB);

//*-----  
  
  EEPROM.write(26,vfo.get());  
  EEPROM.write(27,MSW);
  EEPROM.write(28,USW);
  EEPROM.write(29,TSW);
  EEPROM.write(34,EEPROM_COOKIE);

  EEPROM.write(32,vx.step2code(vx.vfostep[VFOA]));
  EEPROM.write(33,vx.step2code(vx.vfostep[VFOB]));

  EEPROM.write(30,vx.vfoband[VFOA]);
  EEPROM.write(31,vx.vfoband[VFOB]);  

 
//*---- Module dependent implementation

  writeEEPROM();
  memstatus = 1;  // Let program know memory has been written
};

//*-------------------------------------------------------------------------------------------------------
//* uppercase a character 
//*-------------------------------------------------------------------------------------------------------
int uppercase (int charbytein)
{
  if (((charbytein > 96) && (charbytein < 123)) || ((charbytein > 223) && (charbytein < 255))) {
    charbytein = charbytein - 32;
  }
  if (charbytein == 158) { charbytein = 142; }  // Å¾ -> Å½
  if (charbytein == 154) { charbytein = 138; }  // Å¡ -> Å 
  
  return charbytein;
}



//*=======================================================================================================================================================
//* Super Library ELEC Freaks LCD Key Shield
//*=======================================================================================================================================================
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* read_LCD_buttons()
//* read the buttons
//--------------------------------------------------------------------------------------------------------------------------------------------------------
boolean readButton() {
 
 int v = analogRead(A0);      
 if (v>1000) {return true;} else {return false;}
 

}

