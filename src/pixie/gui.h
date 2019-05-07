#include <string.h>
#include <cstring>
using namespace std;
char gui[80];

//*--- ShowVFO
void showVFO() {
   lcd.setCursor(0,0);
   if (vx.vfoAB==VFOA) {
      lcd.write(1);
   } else {
      lcd.write(2);
   }
}

//*----------------------------------------------------------------------------$
//* Show the standard panel in VFO mode (CLI=false) mode
//*----------------------------------------------------------------------------$
void showGUI() {

   showVFO();

   lcd.setCursor(2,0);
   lcd.write(0);

   lcd.setCursor(4,0);
   if (kyr.mItem == 0) {
      lcd.print("S");
   } else {
      lcd.write(3);
   }

   lcd.setCursor(6,0);
   if (spl.mItem == 0) {
     lcd.print(" ");
   } else {
     lcd.write(4);
   }

   lcd.setCursor(8,0);
   int i=mod.get();
   printf("DEBUG: showGUI mode mod.get()=%d mod.mItem=%d \n",i,mod.mItem);
   if (fFirst==true) {
      printf("DEBUG: showGUI mod.getText(0)=%s\n",mod.getText(0));
      lcd.print((char*)mod.getText(0)); 
  } else {
      printf("DEBUG: showGUI fFirst=false\n");
   }

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
//*--------------------------------------------------------------------------------------------
//* showPanel
//* show frequency or menu information at the display
//*--------------------------------------------------------------------------------------------
void showPanel() {

   
   if (getWord(MSW,CMD)==false) {
      
      lcd.clear();
      lcd.setCursor(0,0);


//*--- Device specific GUI builter
      showGUI();
//*----           
      return;
   }

//*-----------------------------------------------------------------------------------------------
//*--- if here then CMD==true
//*-----------------------------------------------------------------------------------------------
   byte i=menuRoot.get();
   MenuClass* z=menuRoot.getChild(i);
   char gui[80];
  
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
//* save legend
//*--------------------------------------------------------------------------------------------
void showSave(){
      lcd.clear();
      lcd.setCursor(0,0);
      usleep(300000);
      lcd.print("Saving....");
}
//*----
//* Show Mark
//*----
void showMark(){
      lcd.setCursor(0,1);
      lcd.print(">"); 
}
//*-------------------------------------------------------------------------------------------
//*--- Save Menu
//*-------------------------------------------------------------------------------------------
void saveMenu() {

      byte i=menuRoot.get();
      MenuClass* z=menuRoot.l.get(i)->mChild;
      byte j=z->mItem;
      byte k=z->mItemBackup;

      if (vx.vfoAB != vfo.mItem) {   //Switch from VFO A to B or viceversa
         vx.vfoAB=vfo.mItem;
         vx.set(vx.vfoAB,vx.get(vx.vfoAB));
      }

      if (bck.mItem != backlight) {
         backlight=bck.mItem;
      }

}
//*--------------------------------------------------------------------------------------------
//* doSave
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void doSave() {

      showSave();      
      saveMenu();
      menuRoot.save();

}

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

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages the command and VFO handling FSM
//*--------------------------------------------------------------------------------------------------
void CMD_FSM() {

//*-------------------------------------------------------------------------------
//* PTT==true (RX) and CMD==false (VFO)
//*-------------------------------------------------------------------------------
   if (getWord(MSW,CMD)==false) {      //S=0 VFO Mode   

      
//*--------------------------------------------------------------------------------------
//*---- Process rotation of VFO encoder   (VFO Mode)
//*--------------------------------------------------------------------------------------
      if(getWord(TSW,FVFO)==true){
         showFreq();
         setWord(&TSW,FVFO,false);
      }

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
       //printf("DEBUG: <CMD> Push Button Briefly-->Go into <VFO> mode back\n");
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&MSW,CMD,false);
       showPanel();
       showFreq();
       return;
     }

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==true) {  //in CMD mode and long push (pass to GUI mode)
       //printf("DEBUG: <CMD> Push Button Lengthly-->Go into <GUI> mode\n");
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&MSW,GUI,true);
       showMark();     
       backupFSM();
       showPanel();
       return;
     }

     if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=1 operates Menu at first level and clear signals
         //printf("DEBUG: <CMD> Rotate encoder CW(%d) CCW(%d)\n", getWord(USW,BCW), getWord(USW,BCCW));
         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
      }

     return;
   }

//*--- It is here if in CMD mode && GUI update enabled

   if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==true) {    // It is in CMD and GUI mode

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==false) {    //In GUI mode and brief push then go to CLI mode
       //printf("DEBUG: <GUI> Push button shortly, back to <CMD>\n");
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&MSW,CMD,true);
       restoreFSM();
       setWord(&MSW,GUI,false);
       menuFSM();
       return;   
     }

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==true) {    //In GUI mode and long push then save and go to CLI mode
       //printf("DEBUG: <GUI> Push button lengthly, save and back to <CMD>\n");
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
         //printf("DEBUG: <GUI> Rotate encoder CW(%d) CCW(%d)\n", getWord(USW,BCW), getWord(USW,BCCW));

         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;             
     }
   }

   return;


}
void KeyerUpdate() {
  if (kyr.mItem < 1 && kyr.CW == true) {
      kyr.mItem++;
  }
  if (kyr.mItem > 0 && kyr.CCW == true) {
      kyr.mItem--;
  }
  char* s=(char*)"                  "; 
  switch(kyr.mItem) {
    case 0:                          {s=(char*)"Straight";break;};                            
    case 1:                          {s=(char*)"Iambic";break;};
  }
  
  kyr.l.get(0)->mText=s;
  showPanel();
  
  return;

}

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
void ModeUpdate() {
  if (mod.mItem < 6 && mod.CW == true) {
      mod.mItem++;
  }
  if (mod.mItem > 0 && mod.CCW == true) {
      mod.mItem--;
  }

  char* s=(char*)"                  "; 
  switch(mod.mItem) {
    case 0:                          {s=(char*)"CWL";break;};
    case 1:                          {s=(char*)"CWU";break;};
    case 2:                          {s=(char*)"USB";break;};
    case 3:                          {s=(char*)"LSB";break;};
    case 4:                          {s=(char*)"WSP";break;};
    case 5:                          {s=(char*)"FT8";break;};
    case 6:                          {s=(char*)"PSK";break;};
  }
  printf("DEBUG: ModeUpdate s(%s)\n",s);
  mod.setText(0,s);
  printf("DEBUG: ModeUpdate mod.mItem(%d) getText(0)=%s\n",mod.mItem,mod.getText(0));
  fFirst=true;
  showPanel();
  
  return;


}
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

//*-------------------------------x------------------------------------------------------------- 
//* VfoUpdate //* manages the content of the VFO 
//*-------------------------------------------------------------------------------------------- 
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
//*-----------------------------------------------------------------------------------------------------
//*--- Backlight management
//+-----------------------------------------------------------------------------------------------------
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

//*===================================================================================================================================$
//* Implementarion of Menu Handlers
//*===================================================================================================================================$
void StepUpdate() {

}
void ShiftUpdate() {

}
void LockUpdate() {

}
void setDDSFreq() {

}

