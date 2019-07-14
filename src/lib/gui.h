
#include <string.h>
#include <cstring>
using namespace std;


char gui[80];
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
   lcd.setCursor(13,0);
   switch(S) {
     case 0 : {lcd.print(" ");lcd.print(" ");lcd.print(" "); break;}
     case 1 : {lcd.write(1);lcd.print(" ");lcd.print(" "); break;}
     case 2 : {lcd.write(2);lcd.print(" ");lcd.print(" "); break;}
     case 3 : {lcd.write(3);lcd.print(" ");lcd.print(" "); break;}
     case 4 : {lcd.write(4);lcd.print(" ");lcd.print(" "); break;}
     case 5 : {lcd.write(5);lcd.print(" ");lcd.print(" "); break;}
     case 6 : {lcd.write(5);lcd.write(1);lcd.print(" "); break;}
     case 7 : {lcd.write(5);lcd.write(2);lcd.print(" "); break;}
     case 8 : {lcd.write(5);lcd.write(3);lcd.print(" "); break;}
     case 9 : {lcd.write(5);lcd.write(4);lcd.print(" "); break;}
     case 10: {lcd.write(5);lcd.write(5);lcd.print(" "); break;}
     case 11: {lcd.write(5);lcd.write(5);lcd.write(1); break;}
     case 12: {lcd.write(5);lcd.write(5);lcd.write(2); break;}
     case 13: {lcd.write(5);lcd.write(5);lcd.write(3); break;}
     case 14: {lcd.write(5);lcd.write(5);lcd.write(4); break;}
     case 15: {lcd.write(5);lcd.write(5);lcd.write(5); break;}
   }
   return;
}
//*----------------------------------------------------------------------------$
//* Show the VFO being used (A or B)
//*----------------------------------------------------------------------------$
void showVFO() {
   lcd.setCursor(0,0);
   (vx.vfoAB==VFOA ? lcd.print("A") : lcd.print("B"));
}

//*----------------------------------------------------------------------------$
//* Show the RIT offset
//*----------------------------------------------------------------------------$
void showRit() {
   if (rit.mItem==0) {return;}

   sprintf(gui,"%+04d",RITOFS);
   lcd.setCursor(11,1);
   lcd.print(gui);

}
//*----------------------------------------------------------------------------$
//* Show the PTT status
//*----------------------------------------------------------------------------$

void showPTT() {
   lcd.setCursor(2,0);
   (getWord(FT817,PTT)==true ? lcd.write(0) : lcd.print(" "));

}
//*----------------------------------------------------------------------------$
//* Show the keyer status
//*----------------------------------------------------------------------------$

void showKeyer() {

   lcd.setCursor(4,0);
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

   lcd.setCursor(6,0);
   (spl.mItem == 0 ? lcd.print(" ") : lcd.print("S"));


}
//*----------------------------------------------------------------------------$
//* Show the current mode
//*----------------------------------------------------------------------------$
void showMode(){

   lcd.setCursor(8,0);
   int i=mod.get();
   
   if (mod.init==true) {
      lcd.print((char*)mod.getText(0)); 
   } else {
      lcd.print(" ");
   }
}
//*----------------------------------------------------------------------------$
//* Show the Wifi connection status
//*----------------------------------------------------------------------------$
void showWlan0() {

   lcd.setCursor(14,1);
   if (wtd.mItem != 0) {
       if (wlan0 == true) {
         lcd.print("*");
      } else {
         lcd.print(" ");
      }
   } else {
     lcd.print("-");
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
      showSMeter(2*dds.power);
   }
   showRit();
}
//*--------------------------------------------------------------------------------------------
//* showPanel
//* show frequency or menu information at the display
//*--------------------------------------------------------------------------------------------
void showPanel() {

   
//*---- Panel when in VFO mode (CMD=false)

   if (getWord(MSW,CMD)==false) {
      lcd.clear();
      lcd.setCursor(0,0);
      showGUI();
      return;
   }

//*--- Panel in CMD mode (CMD=true)

   byte i=menuRoot.get();
   MenuClass* z=menuRoot.getChild(i);
   char gui[80];

//*--- GUI=false (meaning parameter read and navigation)

   if (getWord(MSW,GUI)==false) {
      lcd.clear();
      lcd.setCursor(0,0);
      sprintf(gui,"<%d> %s",i,menuRoot.getCurrentText());
      lcd.print((char*)gui);

      lcd.setCursor(1,1);
      sprintf(gui,"  %s ",(char*)z->getText(0));
      lcd.print((char*)gui);
      return;
   } else {

//*---- GUI=true (means parameter being changed)

      lcd.clear();
      lcd.setCursor(0,0);
      sprintf(gui,"<%d> %s",i,menuRoot.getText(menuRoot.get()));
      lcd.print(gui);
      lcd.setCursor(0,1);
      sprintf(gui,"> %s<",(char*)z->getText(0));
      lcd.print(gui);
      return;
   }

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
         //vx.set(vx.vfoAB,vx.get(vx.vfoAB));
         cat.SetFrequency=(float)vx.get(vx.vfoAB);
         (vx.vfoAB==VFOA ? setWord(&cat.FT817,VFO,false) : setWord(&cat.FT817,VFO,true));
         CATchangeFreq();
         CATchangeStatus();         
      }

      bool split=(spl.mItem == 0 ? split=false : split=true);
      if (split != getWord(cat.FT817,SPLIT)) {   // Change in split configuration
         setWord(&cat.FT817,SPLIT,split);
         CATchangeStatus();
      }

      if (bck.mItem != backlight) {     //Change in backlight condition
         backlight=bck.mItem;
      }
      
      if (kyr.mItem != cw_keyer_mode) { //Change in keyer mode
         cw_keyer_mode=kyr.mItem;
      }

      if (mod.mItem != cat.MODE) {
         cat.MODE=mod.mItem;
         CATchangeMode();       // Trigger a pseudo-CAT mode change
      } 

      if (stp.mItem != STEP) {
         STEP=stp.mItem;
         CATchangeStatus();
      }

      if ((500+50*shf.mItem)!=SHIFT) {
         SHIFT=500+50*shf.mItem;
         CATchangeStatus();
      }

     if (spd.mItem != cw_keyer_speed) {
        cw_keyer_speed=spd.mItem;
        CATchangeStatus();
     }
     if (drv.mItem != DDSPOWER) {
        DDSPOWER=drv.mItem;
        CATchangeFreq();
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
         showFreq();
         setWord(&TSW,FVFO,false);
      }

//*--- Handle the push button of the encoder

      if (getWord(USW,BMULTI)==true) {
         setWord(&USW,BMULTI,false);
         setWord(&MSW,CMD,true);
         showPanel();
      } else {
         return;
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

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==true) {  //in CMD mode and long push (pass to GUI mode)
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
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

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==true) {    //In GUI mode and long push then save and go to CLI mode
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
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
  char* s=(char*)"                  "; 
  switch(kyr.mItem) {
    case 0:                          {s=(char*)"Straight";break;};                            
    case 1:                          {s=(char*)"Iambic A";break;};
    case 2:                          {s=(char*)"Iambic B";break;};
  }
  kyr.l.get(0)->mText=s;
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
  char* s=(char*)"                  "; 
  switch(spl.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"On";break;};
  }
  
  spl.l.get(0)->mText=s;
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
  
  mod.l.get(0)->mText=s;
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
  char* s=(char*)"                  "; 
  switch(wtd.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"On";break;};
  }
  
  wtd.l.get(0)->mText=s;
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
  char* s=(char*)"                  "; 
  switch(vfo.mItem) {
    case 0:                          {s=(char*)"Vfo A";break;};                            
    case 1:                          {s=(char*)"Vfo B";break;};
  }

  vfo.l.get(0)->mText=s;
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
   
  sprintf(gui,"%i secs",bck.mItem);
  bck.setText(0,(char*)gui);

  bck.CW=false;
  bck.CCW=false;
 
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
  sprintf(gui," %d wpm",spd.mItem);
  spd.setText(0,(char*)gui);

  spd.CW=false;
  spd.CCW=false;

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
  char* s=(char*)"                  "; 
  switch(rit.mItem) {
    case 0:                          {s=(char*)"Off ";break;};                            
    case 1:                          {s=(char*)"On  ";break;};
  }

  rit.l.get(0)->mText=s;
  showPanel();

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

  switch(drv.mItem) {
     case 0 : {sprintf(gui," Off"); break;} 
     case 1 : {sprintf(gui," 1/7"); break;} 
     case 2 : {sprintf(gui," 2/7"); break;} 
     case 3 : {sprintf(gui," 3/7"); break;} 
     case 4 : {sprintf(gui," 4/7"); break;} 
     case 5 : {sprintf(gui," 5/7"); break;} 
     case 6 : {sprintf(gui," 6/7"); break;} 
     case 7 : {sprintf(gui," Max"); break;} 
  }

  drv.setText(0,(char*)gui);

  drv.CW=false;
  drv.CCW=false;
 
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
  switch(shf.mItem) {
    case 0:                          {s=(char*)"500 Hz ";break;};                            
    case 1:                          {s=(char*)"550 Hz ";break;};                            
    case 2:                          {s=(char*)"600 Hz";break;};                            
    case 3:                          {s=(char*)"650 Hz";break;};                            
    case 4:                          {s=(char*)"700 Hz";break;};                            
    case 5:                          {s=(char*)"750 Hz";break;};                            
    case 6:                          {s=(char*)"800 Hz";break;};                            
  }
  shf.setText(0,(char*)s);
  shf.CW=false;
  shf.CCW=false;
 
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

  stp.setText(0,(char*)s);
  stp.CW=false;
  stp.CCW=false;
  return;
}
//*---- NOT IMPLEMENTED YET

void LockUpdate() {

}
void setDDSFreq() {

}

