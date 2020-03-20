

#include <string.h>
#include <cstring>
using namespace std;
#define FT817_STR_CAL { 16, \
                { \
                    { 0x00, -54 }, /* S0 */ \
                    { 0x01, -48 }, \
                    { 0x02, -42 }, \
                    { 0x03, -36 }, \
                    { 0x04, -30 }, \
                    { 0x05, -24 }, \
                    { 0x06, -18 }, \
                    { 0x07, -12 }, \
                    { 0x08, -6 }, \
                    { 0x09,  0 },  /* S9 */ \
                    { 0x0A,  10 }, /* +10 */ \
                    { 0x0B,  20 }, /* +20 */ \
                    { 0x0C,  30 }, /* +30 */ \
                    { 0x0D,  40 }, /* +40 */ \
                    { 0x0E,  50 }, /* +50 */ \
                    { 0x0F,  60 }  /* +60 */ \
                } }

char gui[80];
char bar[]="     ";
bool rFlag=false;

//*-----------------------------------------------------------------------------------------------------
//*--- guiDrawBar
//*-----------------------------------------------------------------------------------------------------

void guiDrawBar(char *s,int v, int vmax) {

  if (vmax==0) { return; }
  if (v>vmax)  { v=vmax; }
  float r=(float)v/(float)vmax;
  r=r*MAXBAR;
  int  l=(int)r;
  for (int i=0;i<MAXBAR;i++) {
   if (l>i) {
     s[i]=char(255);
   } else {
     s[i]=' ';
   }
  }
  return;
}

//*---------------------------------------------------------------------------
//* showSMeter
//* represents a linear SMeter composed by 3 segments with 0..5 bars each
//* input signal must be in the 0..15 range [S0..S9..+10..+60] and assigned
//* to the range. The caller is responsible to comform the signal to a 
//* linear representation.
//*---------------------------------------------------------------------------
void timer_SMeter();
void showSMeter(int S) {

   int V=S;
   if (S<0) {
      S=0;
   } else {
     if (S>15) {
        S=15;
     }
   }
   lcd.setCursor(12,1);
   switch(S) {
     case 0 :
     case 1 :
     case 2 :
     case 3 : {lcd.print(" ");lcd.print(" ");lcd.print(" "); break;}
     case 4 :
     case 5 :
     case 6 :
     case 7 : {lcd.write(255);lcd.print(" ");lcd.print(" "); break;}
     case 8 :
     case 9 :
     case 10:
     case 11: {lcd.write(255);lcd.write(255);lcd.print(" "); break;}
     case 12:
     case 13:
     case 14:
     case 15: {lcd.write(255);lcd.write(255);lcd.write(255); break;}
   }
   return;
}
//*----------------------------------------------------------------------------$
//* Show the VFO being used (A or B)
//*----------------------------------------------------------------------------$
void showVFO() {
   lcd.setCursor(8,0);
   (vx.vfoAB==VFOA ? lcd.print("A") : lcd.print("B"));
}

//*----------------------------------------------------------------------------$
//* Show the RIT offset
//*----------------------------------------------------------------------------$
void showRit() {

   lcd.setCursor(6,1);
   if (rit.mItem==0) {lcd.print("   "); return;}

   sprintf(gui,"%0+3d",ritofs/10);
   lcd.print(gui);

}
//*----------------------------------------------------------------------------$
//* Show the PTT status
//*----------------------------------------------------------------------------$

void showPTT() {
   lcd.setCursor(5,1);
   (getWord(FT817,PTT)==true ? lcd.write(255) : lcd.print(" "));
   //(getWord(FT817,PTT)==true ? fprintf(stderr,"showPTT(On)\n") : fprintf(stderr,"showPTT(Off)\n"));

}
//*----------------------------------------------------------------------------$
//* Show the keyer status
//*----------------------------------------------------------------------------$
void showKeyer() {

   lcd.setCursor(10,0);
   switch(kyr.mItem) {
     case 0x00: {lcd.print("S"); break;}
     case 0x01: {lcd.print("1"); break;}
     case 0x02: {lcd.print("2"); break;}
   }
}
//*----------------------------------------------------------------------------$
//* Show the split status
//*----------------------------------------------------------------------------$
void showSplit() {

   lcd.setCursor(9,0);
   (spl.mItem == 0 ? lcd.print(" ") : lcd.print("S"));


}
//*----------------------------------------------------------------------------$
//* Show the current mode
//*----------------------------------------------------------------------------$
void showMode(){

   char* s=(char*)"*"; 

   lcd.setCursor(11,0);
   switch(mod.mItem) {
    case  0:                          {s=(char*)"L";break;};
    case  1:                          {s=(char*)"U";break;};
    case  2:                          {s=(char*)"C ";break;};
    case  3:                          {s=(char*)"C";break;};
    case  4:                          {s=(char*)"A ";break;};
    case  5:                          {s=(char*)"* ";break;};
    case  6:                          {s=(char*)"W";break;};
    case  7:                          {s=(char*)"*";break;};
    case  8:                          {s=(char*)"F ";break;};
    case  9:                          {s=(char*)"*";break;};
    case 10:                          {s=(char*)"D";break;};
    case 11:                          {s=(char*)"*";break;};
    case 12:                          {s=(char*)"P";break;};
   }

   if (mod.init==true) {
      lcd.print((char*)s); 
   } else {
      lcd.print(" ");
   }
  

}
//*----------------------------------------------------------------------------$
//* Show the standard panel in VFO mode (CLI=false)
//*----------------------------------------------------------------------------$
void showGUI() {

   showFreq();
   showVFO();
   showPTT();
   showSplit();
   showKeyer();
   showMode();

   if (getWord(MSW,PTT)==false) {
     showSMeter(0);
   } else {
     showSMeter(2*dds->power);
   }
   showRit();
}
//*--------------------------------------------------------------------------------------------
//* showPanel
//* show frequency or menu information at the display
//*--------------------------------------------------------------------------------------------
void showPanel() {

   if (rFlag != false) {
      return;
   }
   rFlag=true;
   
//*---- Panel when in VFO mode (CMD=false)


   if (getWord(MSW,CMD)==false) {
      lcd.clear();
      lcd.setCursor(0,0);
      showGUI();
      rFlag=false;
      return;
   }

//*--- Panel in CMD mode (CMD=true)

   byte i=menuRoot.get();
   MenuClass* z=menuRoot.getChild(i);

   char gui[80];

//*--- GUI=false (meaning parameter read and navigation)

   if (z->lock==true) {return;}

   if (getWord(MSW,GUI)==false) {
      lcd.clear();
      lcd.setCursor(0,0);
      sprintf(gui,"<%d> %s",i,menuRoot.getCurrentText());
      lcd.print((char*)gui);
      lcd.setCursor(1,1);
      if (z->update != NULL) {
         z->update();
         sprintf(gui,"  %-12s ",(char*)z->g);
      } else {
         sprintf(gui,"  %-12s ",(char*)z->getText(z->mItem));
      }
      lcd.print((char*)gui);
      rFlag=false;
      return;
   } else {

//*---- GUI=true (means parameter being changed)

      lcd.clear();
      lcd.setCursor(0,0);
      sprintf(gui,"<%d> %s",i,menuRoot.getText(menuRoot.get()));
      lcd.print(gui);
      lcd.setCursor(0,1);
      if (z->update != NULL) {
         z->update();
         sprintf(gui,"> %-12s<",(char*)z->g);
      } else {
         sprintf(gui,"> %-12s<",(char*)z->getText(z->mItem));
      }
      lcd.print(gui);
      rFlag=false;
      return;
   }
   rFlag=false;
   return;
}
//*--------------------------------------------------------------------------------------------
//* showSave
//* show very briefly the save legend
//*--------------------------------------------------------------------------------------------
void showSave(){
      lcd.clear();
      lcd.setCursor(0,0);
      usleep(DELAY_SAVE);
      lcd.print("Saving....");
}
//*-------------------------------------------------------------------------------------------
//* Show the parameter being updated mark 
//*-------------------------------------------------------------------------------------------
void showMark(){
      lcd.setCursor(0,1);
      lcd.print(">"); 
}
//*-------------------------------------------------------------------------------------------
//*--- Perform when a change has been made on parameters
//*-------------------------------------------------------------------------------------------
void saveMenu() {

      byte i=menuRoot.get();
      MenuClass* z=menuRoot.l.get(i)->mChild;
      byte j=z->mItem;
      byte k=z->mItemBackup;

      if (vx.vfoAB != vfo.mItem) {      //Switch from VFO A to B or viceversa
         vx.vfoAB=vfo.mItem;
         cat.SetFrequency=(float)vx.get(vx.vfoAB);
         (vx.vfoAB==VFOA ? setWord(&cat.FT817,VFO,false) : setWord(&cat.FT817,VFO,true));
         CATchangeFreq();
         CATchangeStatus();         
      }

      bool split=false;
      (spl.mItem == 0 ? split=false : split=true);
      if (split != getWord(cat.FT817,SPLIT)) {   // Change in split configuration
         setWord(&cat.FT817,SPLIT,split);
         CATchangeStatus();
      }

      if (bck.mItem != backlight) {     //Change in backlight condition
         backlight=bck.mItem;
         if (backlight != 0) {
            TBCK=backlight;
         }
      }
      
      if (kyr.mItem != cw_keyer_mode) { //Change in keyer mode
         cw_keyer_mode=kyr.mItem;
      }

      if (mod.mItem != cat.MODE) {
         cat.MODE=mod.mItem;
         CATchangeMode();       // Trigger a pseudo-CAT mode change
      } 

      if (stp.mItem != step) {
         step=stp.mItem;
         CATchangeStatus();
      }

      if ((500+50*shf.mItem)!=shift) {
         shift=500+50*shf.mItem;
         CATchangeStatus();
      }

     if (spd.mItem != cw_keyer_speed) {
        cw_keyer_speed=spd.mItem;
        CATchangeStatus();
     }
     if (drv.mItem != ddspower) {
        ddspower=drv.mItem;
        CATchangeStatus();
     }
}
//*-------------------------------------------------------------------------------------------
//*--- Exit the GUI mode saving the changes
//*-------------------------------------------------------------------------------------------

void doSave() {

      showSave();      
      saveMenu();
      menuRoot.save();


}

//*----------------------------------------------------------------------------------------------------
//* backupFSM
//* come here when CMD=true and GUI became true to save the contents of the object
//*----------------------------------------------------------------------------------------------------
void backupFSM() {
     
     byte i=menuRoot.get();
     MenuClass* z=menuRoot.getChild(i);
     z->backup();

}
//*----------------------------------------------------------------------------------------------------
//* restoreFSM
//* come here with CMD=true and GUI transitioning from true to false when  saving is not required
//*----------------------------------------------------------------------------------------------------
void restoreFSM() {

      byte i=menuRoot.get();
      MenuClass* z=menuRoot.getChild(i);
      z->restore();  
}

//*----------------------------------------------------------------------------------------------------
//* menuFSM
//* handle the menu navigation at high level
//*----------------------------------------------------------------------------------------------------
void menuFSM() {


   if (getWord(MSW,CMD)==false) {   //* VFO mode, no business here!
      return;
   }


   if (getWord(MSW,GUI)==false) {  //It's the first level of command navigation, CMD=true && GUI=false
      menuRoot.move(getWord(USW,BCW),getWord(USW,BCCW));
      showPanel();
      return;
   }

//*---- Here CMD==true and GUI==true so it's a second level menu meant to update the parameter

     byte i=menuRoot.get();
     MenuClass* z=menuRoot.getChild(i);
     z->move(getWord(USW,BCW),getWord(USW,BCCW));
     showPanel();
     return;
   
}

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages the command and VFO handling FSM
//*--------------------------------------------------------------------------------------------------
void CMD_FSM() {

//*---- CMD=false, so it's VFO mode

   if (getWord(MSW,CMD)==false) {      //S=0 VFO Mode   

//*--- Handle rotation of the Encoder

      if(getWord(TSW,FVFO)==true){
      //

         showFreq();
         setWord(&TSW,FVFO,false);
      }

//*--- Handle the push button of the encoder (checking for ENCODER SW button)

      if (getWord(USW,BMULTI)==true) {
         setWord(&USW,BMULTI,false);
         setWord(&MSW,CMD,true);
         showPanel();
      }

      if (getWord(USW,BAUX)==true) {
         setWord(&USW,BAUX,false);
         if (getWord(FT817,PTT)==false) {
            setPTT(true);
            showGUI();
         } else {
            setPTT(false);
            showGUI();
         }
      }
      return;

   }

//******************************************************************************************
//*--- If here CMD=1 (command mode)
//******************************************************************************************
//* Process menu commands
//******************************************************************************************

   if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==false) {    // It is in pure CMD mode

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==false) {  //in CMD mode and brief push (return to VFO mode)
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&MSW,CMD,false);
       showPanel();
       showFreq();
       return;
     }

     if ((getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==true) || (getWord(USW,BAUX)==true)) {  //in CMD mode and long push (pass to GUI mode)
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&USW,BAUX,false);
       setWord(&MSW,GUI,true);
       showMark();     
       backupFSM();
       showPanel();
       return;
     }

     if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=1 operates Menu at first level and clear signals
       menuFSM();
       setWord(&USW,BCW,false);
       setWord(&USW,BCCW,false);
      }

     return;
   }

//*--- It is here if in CMD mode && GUI update enabled

   if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==true) {    // It is in CMD and GUI mode

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==false) {    //In GUI mode and brief push then go to CLI mode
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&MSW,CMD,true);
       restoreFSM();
       setWord(&MSW,GUI,false);
       menuFSM();
       return;   
     }

     if ((getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==true) || (getWord(USW,BAUX)==true)) {    //In GUI mode and long push then save and go to CLI mode
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&USW,BAUX,false);
       setWord(&MSW,CMD,true);
       doSave();
       showPanel();
       setWord(&MSW,GUI,false);
       menuFSM();
       return;
     }
     
     if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=1 operates Menu at first level and clear signals
         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;             
     }
   }

   return;


}
//*-----------------------------------------------------------------------------------------------------------------
//* Callback for menu objects
//* Called from the MenuClass objects when a callback is defined to manage content
//*-----------------------------------------------------------------------------------------------------------------
//*--- Keyer content
void KeyerUpdate() {

  if (kyr.mItem < 2 && kyr.CW == true) {
      kyr.mItem++;
  }
  if (kyr.mItem > 0 && kyr.CCW == true) {
      kyr.mItem--;
  }

  kyr.CW=false;
  kyr.CCW=false;

  char* s=(char*)"                  "; 
  switch(kyr.mItem) {
    case 0:                          {s=(char*)"Straight";break;};                            
    case 1:                          {s=(char*)"Iambic A";break;};
    case 2:                          {s=(char*)"Iambic B";break;};
  }
  kyr.g=s;
  showPanel();

  return;

}

//*--------------------------------------------------------------------------------------------------
//*---- Split options
//*--------------------------------------------------------------------------------------------------
void SplitUpdate() {

  if (spl.mItem < 1 && spl.CW == true) {
      spl.mItem++;
  }
  if (spl.mItem > 0 && spl.CCW == true) {
      spl.mItem--;
  }
  spl.CW=false;
  spl.CCW=false;

  char* s=(char*)"                  "; 
  switch(spl.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"On";break;};
  }
  
  spl.g=s;
  showPanel();

  return;

}
//*-------------------------------------------------------------------------------------------
//*---- Mode update
//*-------------------------------------------------------------------------------------------
void ModeUpdate() {


  if (mod.mItem < 12 && mod.CW == true) {
      mod.mItem++;

  }
  if (mod.mItem > 0 && mod.CCW == true) {
      mod.mItem--;

  }


//*---- Apply policy for modes not implemented making them not selectable

  if ((mod.mItem == 5 || mod.mItem == 7 || mod.mItem ==11) && mod.CW) {
     mod.mItem++;

  }
  if ((mod.mItem == 5 || mod.mItem == 7 || mod.mItem ==11) && mod.CCW) {
     mod.mItem--;
  }

  mod.CW=false;
  mod.CCW=false;


  char* s=(char*)"                  "; 
  switch(mod.mItem) {
    case  0:                          {s=(char*)"LSB";break;};
    case  1:                          {s=(char*)"USB";break;};
    case  2:                          {s=(char*)"CW ";break;};
    case  3:                          {s=(char*)"CWR";break;};
    case  4:                          {s=(char*)"AM ";break;};
    case  5:                          {s=(char*)"N/I";break;};
    case  6:                          {s=(char*)"WFM";break;};
    case  7:                          {s=(char*)"N/I";break;};
    case  8:                          {s=(char*)"FM ";break;};
    case  9:                          {s=(char*)"N/I";break;};
    case 10:                          {s=(char*)"DIG";break;};
    case 11:                          {s=(char*)"N/I";break;};
    case 12:                          {s=(char*)"PKT";break;};
  }
  
  mod.g=s;
  showPanel();
  

  return;

}
//*-----------------------------------------------------------------------------------
//*---- Watchdog enable/disable content
//*-----------------------------------------------------------------------------------
void WatchDogUpdate() {


  if (wtd.mItem < 1 && wtd.CW == true) {
      wtd.mItem++;
  }
  if (wtd.mItem > 0 && wtd.CCW == true) {
      wtd.mItem--;
  }

  wtd.CW=false;
  wtd.CCW=false;

  char* s=(char*)"                  "; 
  switch(wtd.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"On";break;};
  }
  
  wtd.g=s;
  showPanel();
  


  return;
    
}
//*----------------------------------------------------------------------------------------
//*---- VFO update
//*----------------------------------------------------------------------------------------
void VfoUpdate() {


  if (vfo.mItem < 1 && vfo.CW == true) {
      vfo.mItem++;
  }
  if (vfo.mItem > 0 && vfo.CCW == true) {
      vfo.mItem--;
  }

  vfo.CW=false;
  vfo.CCW=false;

  char* s=(char*)"                  "; 
  switch(vfo.mItem) {
    case 0:                          {s=(char*)"Vfo A";break;};                            
    case 1:                          {s=(char*)"Vfo B";break;};
  }

  vfo.g=s;
  showPanel();

 
  return;
 
}
//*-------------------------------------------------------------------------------------------
//*--- BackLight mode management
//*-------------------------------------------------------------------------------------------
void BackLightUpdate() {



  if (bck.CW == true) {
     if (bck.mItem < 60) {
        bck.mItem++;
     } else {
        bck.mItem = 0;
     }
  }
  if (bck.CCW == true) {
     if (bck.mItem > 0) {
        bck.mItem--;
     } else {
        bck.mItem = 60;
     }
  }

  bck.CW=false;
  bck.CCW=false;


  guiDrawBar(bar,bck.mItem,60);
  sprintf(gui,"%3d[%5s]%3s",bck.mItem,bar,(char*)" % ");
  bck.g=(char*)gui;
  showPanel();


 
  return;

}
//*-----------------------------------------------------------------------------------------------------
//*--- Speed Update
//*-----------------------------------------------------------------------------------------------------
void SpeedUpdate() {


  if (spd.init == false) {return;}

  if (spd.CW == true && spd.mItem < 40) {
     spd.mItem++;
  }

  if (spd.CCW==true && spd.mItem > 0) {
     spd.mItem--;
  }

  spd.CW=false;
  spd.CCW=false;


  //char g[80];
  //char g[17];
  char s[17]; 

  guiDrawBar(bar,spd.mItem,40);
  sprintf(gui,"%3d[%5s]%3s",spd.mItem,bar,(char*)"WPM");
  spd.g=(char*)gui;
  showPanel();
  //spd.setText(0,(char*)gui);
  //strcpy((char*)g,(char*)gui);
  //spd.setText(0,(char*)g);

  //showPanel();


}

//*-----------------------------------------------------------------------------------------------------
//*--- RIT Update
//*-----------------------------------------------------------------------------------------------------
void RitUpdate() {

  if (rit.mItem < 1 && rit.CW == true) {
      rit.mItem++;
  }
  if (rit.mItem > 0 && rit.CCW == true) {
      rit.mItem--;
  }

  rit.CW=false;
  rit.CCW=false;

  char* s=(char*)"                  "; 
  switch(rit.mItem) {
    case 0:                          {s=(char*)"Off ";break;};                            
    case 1:                          {s=(char*)"On  ";break;};
  }

  rit.g=s;
  showPanel();


  //rit.l.get(0)->mText=s;
  //showPanel();

}
//*-----------------------------------------------------------------------------------------------------
//*--- Driver Update
//*-----------------------------------------------------------------------------------------------------
void DriverUpdate() {


  if (drv.init == false) {return;}

  if (drv.CW == true && drv.mItem < 7) {
     drv.mItem++;
  } 
  if (drv.CCW == true && drv.mItem > 0) {
     drv.mItem--;
  }

  drv.CW=false;
  drv.CCW=false;

  float r=100.0*((float)drv.mItem/7.0);
  guiDrawBar(bar,drv.mItem,7);
  sprintf(gui,"%3d[%5s]%3s",(int)r,(char*)bar,(char*)" % ");
  drv.g=(char*)gui;
  showPanel();
  return;
     
}
//*-----------------------------------------------------------------------------------------------------
//*--- Shift Update
//*-----------------------------------------------------------------------------------------------------
void ShiftUpdate() {

  if (shf.CW == true) {         //* Varies Tone shift between 500 and 800 Hz
     if (shf.mItem < 6) {
        shf.mItem++;
     } else {
        shf.mItem = 0;
     }
  }
  if (shf.CCW == true) {
     if (shf.mItem > 0) {
        shf.mItem--;
     } else {
        shf.mItem = 6;
     }
  }
  char* s=(char*)"           ";
  shf.CW=false;
  shf.CCW=false;


  int k=shf.mItem*50+500;
  guiDrawBar(bar,shf.mItem,6);
  sprintf(gui,"%3d[%5s]%3s",k,bar,(char*)" Hz");
  shf.g=(char*)gui;
  
  //shf.setText(0,(char*)g);
  //shf.setText(0,(char*)s);
  //shf.l.get(0)->mText=gui;
  //showPanel();

 
  return;

}
//*-------------------------------------------------------------------------
//*---- Step content management
//*-------------------------------------------------------------------------
void StepUpdate() {

  if (stp.CW == true && stp.mItem < 6) {         //* Varies Tone shift between 500 and 800 Hz
      stp.mItem++;
  }

  if (stp.CCW == true && stp.mItem > 0) {
      stp.mItem--;
  }

  stp.CW=false;
  stp.CCW=false;

  char* s=(char*)"                  ";
  switch(stp.mItem) {
    case 0 : {s=(char*)" 100 Hz "; break;}
    case 1 : {s=(char*)" 500 Hz "; break;}
    case 2 : {s=(char*)"   1 KHz"; break;}
    case 3 : {s=(char*)"   5 KHz"; break;}
    case 4 : {s=(char*)"  10 KHz"; break;}
    case 5 : {s=(char*)"  50 KHz"; break;}
    case 6 : {s=(char*)" 100 KHz"; break;}
  } 

  stp.g=(char*)s;
  showPanel();

  updatestep(VFOA,stp.mItem);
  updatestep(VFOB,stp.mItem);

  return;
}
//*----------------------------------------------------------------------------
//* LockUpdate
//*----------------------------------------------------------------------------
void LockUpdate() {

  if (lck.mItem < 1 && lck.CW == true) {
      lck.mItem++;
  }
  if (lck.mItem > 0 && lck.CCW == true) {
      lck.mItem--;
  }

  lck.CW=false;
  lck.CCW=false;


  char* s=(char*)"                  "; 
  switch(lck.mItem) {
    case 0:                          {s=(char*)"Off ";break;};                            
    case 1:                          {s=(char*)"On  ";break;};
  }

  lck.g=s;
  //lck.l.get(0)->mText=s;
  showPanel();

}
void setDDSFreq() {

}

