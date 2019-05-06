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
   lcd.write(3);

   lcd.setCursor(6,0);
   lcd.write(4);

   lcd.setCursor(8,0);
   lcd.print("CW");

   lcd.setCursor(13,0);
   lcd.write(7);
   lcd.write(6);
   lcd.write(5);

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
    
      
   if (getWord(MSW,GUI)==false) {
      
      lcd.clear();
      lcd.setCursor(0,0);
      sprintf(gui,"<%d> %s",i,menuRoot.getCurrentText());
      lcd.print(gui);
      lcd.setCursor(1,1);
      sprintf(gui," %s",z->getCurrentText());
      lcd.print(gui);

      return;
      
   } else {
 
      lcd.clear();
      lcd.setCursor(0,0);
      sprintf(gui,"<%s>",menuRoot.getText(menuRoot.get()));
      lcd.print(gui);
      lcd.setCursor(0,1);
      sprintf(gui,">%s",z->getCurrentText());
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
      lcd.print("Saving....");
}
//*----
//* Show Mark
//*----
void showMark(){
      lcd.setCursor(0,1);
      lcd.print(">"); 
}
//*--- Save Menu
void saveMenu() {

      byte i=menuRoot.get();
      MenuClass* z=menuRoot.l.get(i)->mChild;
      byte j=z->mItem;
      byte k=z->mItemBackup;

      if (vx.vfoAB != vfo.mItem) {   //Switch from VFO A to B or viceversa
         vx.vfoAB=vfo.mItem;
         vx.set(vx.vfoAB,vx.get(vx.vfoAB));
      }

}
//*--------------------------------------------------------------------------------------------
//* doSave
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void doSave() {

      showSave();      
//      usleep(1000000);

      

//**************************************************
//* Device specific parameter saving               *
//**************************************************
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
       printf("DEBUG: <CMD> Push Button Briefly-->Go into <VFO> mode back\n");
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&MSW,CMD,false);
       showPanel();
       showFreq();
       return;
     }

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==true) {  //in CMD mode and long push (pass to GUI mode)
       printf("DEBUG: <CMD> Push Button Lengthly-->Go into <GUI> mode\n");
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&MSW,GUI,true);
       showMark();     
       backupFSM();
       showPanel();
       return;
     }

     if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=1 operates Menu at first level and clear signals
         printf("DEBUG: <CMD> Rotate encoder CW(%d) CCW(%d)\n", getWord(USW,BCW), getWord(USW,BCCW));
         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
      }

     return;
   }

//*--- It is here if in CMD mode && GUI update enabled

   if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==true) {    // It is in CMD and GUI mode

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==false) {    //In GUI mode and brief push then go to CLI mode
       printf("DEBUG: <GUI> Push button shortly, back to <CMD>\n");
       setWord(&USW,BMULTI,false);
       setWord(&USW,KDOWN,false);
       setWord(&MSW,CMD,true);
       restoreFSM();
       setWord(&MSW,GUI,false);
       menuFSM();
       return;   
     }

     if (getWord(USW,BMULTI)==true && getWord(USW,KDOWN)==true) {    //In GUI mode and long push then save and go to CLI mode
       printf("DEBUG: <GUI> Push button lengthly, save and back to <CMD>\n");
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
         printf("DEBUG: <GUI> Rotate encoder CW(%d) CCW(%d)\n", getWord(USW,BCW), getWord(USW,BCCW));

         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;             
     }
   }

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

if (bck.mItem < 60 && bck.CW == true) {
      bck.mItem++;
  }
  if (bck.mItem > 0 && bck.CCW == true) {
      bck.mItem--;
  }
  sprintf(gui,"%i secs",bck.mItem);
  bck.l.get(0)->mText=(char*)gui;
  printf("DEBUG: gui buffer(%s)",gui);
  printf("DEBUG: bck.mText %s",bck.l.get(0)->mText);
 
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
void SplitUpdate() {

}
void KeyerUpdate() {

}
void WatchDogUpdate() {

}
void LockUpdate() {

}
void setDDSFreq() {

}

