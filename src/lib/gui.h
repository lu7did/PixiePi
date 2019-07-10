#include <string.h>
#include <cstring>
using namespace std;


char gui[80];
//*----------------------------------------------------------------------------$
//* Show the VFO being used (A or B)
//*----------------------------------------------------------------------------$
void showVFO() {
   lcd.setCursor(0,0);
   (vx.vfoAB==VFOA ? lcd.write(1) : lcd.write(2));

   //if (vx.vfoAB==VFOA) {
   //   lcd.write(1);
   //} else {
   //   lcd.write(2);
   //}
}
//*----------------------------------------------------------------------------$
//* Show the PTT status
//*----------------------------------------------------------------------------$

void showPTT() {
   lcd.setCursor(2,0);
   (getWord(FT817,PTT)==true ? lcd.write(0) : lcd.print(" "));
   //if (getWord(FT817,PTT)==true) { 
   //   lcd.write(0);
   //} else {
   //   lcd.print(" ");
   //}

}
//*----------------------------------------------------------------------------$
//* Show the keyer status
//*----------------------------------------------------------------------------$

void showKeyer() {

   lcd.setCursor(4,0);
   (kyr.mItem == 0 ? lcd.print("S") : lcd.write(3));

   //if (kyr.mItem == 0) {
   //   lcd.print("S");
   //} else {
   //   lcd.write(3);
   //}

}
//*----------------------------------------------------------------------------$
//* Show the split status
//*----------------------------------------------------------------------------$

void showSplit() {

   lcd.setCursor(6,0);
   (spl.mItem == 0 ? lcd.print(" ") : lcd.write(4));

   //if (spl.mItem == 0) {
   //  lcd.print(" ");
   //} else {
   //  lcd.write(4);
   //}

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

   showVFO();
   showPTT();
   showSplit();
   showKeyer();
   showMode();
   showWlan0();
   
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
      sprintf(gui," %s",(char*)z->getText(0));
      lcd.print((char*)gui);
      return;
   } else {

//*---- GUI=true (means parameter being changed)

      lcd.clear();
      lcd.setCursor(0,0);
      sprintf(gui,"<%d> %s",i,menuRoot.getText(menuRoot.get()));
      lcd.print(gui);
      lcd.setCursor(0,1);
      sprintf(gui,"> %s",(char*)z->getText(0));
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

      if (mod.mItem != MODE) {
         cat.MODE=mod.mItem;
         CATchangeMode();       // Trigger a pseudo-CAT mode change

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
  if (mod.mItem <= 12 && mod.CW == true) {
      mod.mItem++;
  }
  if (mod.mItem > 0 && mod.CCW == true) {
      mod.mItem--;
  }

//*---- Apply policy for modes not implemented making them not selectable

  if ((mod.mItem == 5 || mod.mItem == 7 || mod.mItem ==11) && mod.CCW) {
     mod.mItem--;
  } else {
     mod.mItem++;
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

  mod.setText(0,s);
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
    case 0:                          {s=(char*)"A";break;};                            
    case 1:                          {s=(char*)"B";break;};
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
        shf.mItem=shf.mItem-1;
     } else {
        shf.mItem = 6;
     }
  }
   
  sprintf(gui,"%i Hz",500+((int)shf.mItem)*50);
  shf.setText(0,(char*)gui);

  shf.CW=false;
  shf.CCW=false;
 
  return;

}

//*---- NOT IMPLEMENTED YET
//*---- Step content management
void StepUpdate() {

}

void LockUpdate() {

}
void setDDSFreq() {

}

