#include <string.h>
#include <cstring>
using namespace std;
char gui[80];
//*----------------------------------------------------------------------------$
//* Show the standard panel in VFO mode (CLI=false) mode
//*----------------------------------------------------------------------------$
void showGUI() {

   lcd.setCursor(0,0);
   lcd.write(1);

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
      //showVFO();


//*--- Device specific GUI builter
      showGUI();
//*----           
      return;
   }


//*--- if here then CMD==true

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
      sprintf(gui,"> %s",z->getCurrentText());
      lcd.print(gui);
      return;
   }

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

