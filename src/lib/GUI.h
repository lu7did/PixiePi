
//*=====================================================================================================================
//* VFO Panel presentation handlers
//*=====================================================================================================================

void showMode() {

char m[8];

   if (vfo==nullptr) {
      strcpy(m,"cw");   
      usleep(100000);
      return;
   } else {
     vfo->code2mode(vfo->MODE,m);
   }
   lcd->println(13,0,m);

}
//*----------------------
void showVFO() {

     if (getWord(MSW,CMD)==true) {return;}

     if (vfo->vfo == VFOA) {
        lcd->setCursor(0,0);
        lcd->write(byte(6));
        strcpy(LCD_Buffer,"B");
        lcd->println(0,1,LCD_Buffer);

     } else {
        strcpy(LCD_Buffer,"A");
        lcd->println(0,0,LCD_Buffer);
        lcd->setCursor(0,1);
        lcd->write(byte(7));
     }
}

//*----------------------
void showChange() {

int row=0;
int alt=0;

     if (getWord(MSW,CMD)==true) {return;}

     if (vfo->vfo == VFOA) {
        row=0;
        alt=1;
     } else {
        row=1;
        alt=0;
     }

     if (getWord(GSW,FBLINK)==true) {

        lcd->setCursor(1,row);

        (vfo->vfodir == 1 ? lcd->typeChar((char)126) : lcd->typeChar((char)127) );
        strcpy(LCD_Buffer," ");
        lcd->println(1,alt,LCD_Buffer);

     } else {

        strcpy(LCD_Buffer," ");
        lcd->println(1,0,LCD_Buffer);
        lcd->println(1,1,LCD_Buffer);

     }

}
//*----------------------
void showFrequency() {

     if (getWord(MSW,CMD)==true) {return;}

     strcpy(LCD_Buffer," ");
     lcd->println(9,0,LCD_Buffer);
     lcd->println(9,1,LCD_Buffer);

     sprintf(LCD_Buffer,"  %5.1f",vfo->get(VFOA)/1000.0);
     lcd->println(1,0,LCD_Buffer);

     sprintf(LCD_Buffer,"  %5.1f",vfo->get(VFOB)/1000.0);
     lcd->println(1,1,LCD_Buffer);

}
//*---------------------
void showRIT() {

     if (getWord(MSW,CMD)==true) {return;}
     if (vfo==nullptr) {return;}
     if (vfo->getPTT()==true) {
        return;
     }

     strcpy(LCD_Buffer,"    ");
     lcd->println(10,0,LCD_Buffer) ;

     if (vfo->getRIT(vfo->vfo)==true) {
        sprintf(LCD_Buffer,"%+04.0f",vfo->valueRIT(vfo->vfo));
        lcd->println(10,0,LCD_Buffer) ;
     }

}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showPTT() {

     if (getWord(MSW,CMD)==true) {return;}
     if (vfo==nullptr) {return; }
     if (vfo->getPTT()==false) {  //inverted for testing
        strcpy(LCD_Buffer," ");
        lcd->println(11,1,LCD_Buffer);
        return;
     }
     lcd->setCursor(11,1);
     lcd->write(0);
}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showKeyer() {

     if (getWord(MSW,CMD)==true) {return;}

     switch(keyer->get()->mVal) {
        case 0 : strcpy(LCD_Buffer,"S"); break;
        case 1 : strcpy(LCD_Buffer,"1"); break;
        case 2 : strcpy(LCD_Buffer,"2"); break;
     }

     lcd->println(12,1,LCD_Buffer);
}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showMeter() {

     if (getWord(MSW,CMD)==true) {return;}
     lcd->setCursor(13,1);   //Placeholder Meter till a routine is developed for it

     if (vfo==nullptr) {
        (TRACE>=0x02 ? fprintf(stderr,"%s:showMeter() vfo pointer is NULL, request ignored\n",PROGRAMID) : _NOP);
        return;}

     
int  n= ((vfo->POWER+1)*2) & 0x0f;      
     (TRACE>=0x02 ? fprintf(stderr,"%s:showMeter() POWER(%d) segments(%d)\n",PROGRAMID,vfo->POWER,n) : _NOP);

     switch(n) {

      case 0 : {lcd->write(byte(1))    ;lcd->write(byte(32))  ;lcd->write(byte(32));break;}
      case 1 : {lcd->write(byte(2))    ;lcd->write(byte(32))  ;lcd->write(byte(32));break;}
      case 2 : {lcd->write(byte(3))    ;lcd->write(byte(32))  ;lcd->write(byte(32));break;}
      case 3 : {lcd->write(byte(4))    ;lcd->write(byte(32))  ;lcd->write(byte(32));break;}
      case 4 : {lcd->write(byte(255))  ;lcd->write(byte(32))  ;lcd->write(byte(32));break;}
      case 5 : {lcd->write(byte(255))  ;lcd->write(byte(1))   ;lcd->write(byte(32));break;}
      case 6 : {lcd->write(byte(255))  ;lcd->write(byte(2))   ;lcd->write(byte(32));break;}
      case 7 : {lcd->write(byte(255))  ;lcd->write(byte(3))   ;lcd->write(byte(32));break;}
      case 8 : {lcd->write(byte(255))  ;lcd->write(byte(4))   ;lcd->write(byte(32));break;}
      case 9 : {lcd->write(byte(255))  ;lcd->write(byte(255)) ;lcd->write(byte(32));break;}
      case 10: {lcd->write(byte(255))  ;lcd->write(byte(255)) ;lcd->write(byte(1));break;}
      case 11: {lcd->write(byte(255))  ;lcd->write(byte(255)) ;lcd->write(byte(2));break;}
      case 12: {lcd->write(byte(255))  ;lcd->write(byte(255)) ;lcd->write(byte(3));break;}
      case 13: {lcd->write(byte(255))  ;lcd->write(byte(255)) ;lcd->write(byte(4));break;}
      case 14: {lcd->write(byte(255))  ;lcd->write(byte(255)) ;lcd->write(byte(255));break;}
      case 15: {lcd->write(byte(255))  ;lcd->write(byte(255)) ;lcd->write(byte(255));break;}
     }

}
//*---------------------------------- Show Split
void showSplit() {

     if (getWord(MSW,CMD)==true) {return;}

     if (vfo->getSplit()==true) {
       lcd->setCursor(10,1);
       lcd->write(byte(5));
     } else {
       strcpy(LCD_Buffer," ");
       lcd->println(10,1,LCD_Buffer);
     }
}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showLCDVFO() {

     if (getWord(MSW,CMD)==true) {return;}

     showVFO();
     showChange();
     showFrequency();
     showPTT();
     showRIT();
     showSplit();
     showKeyer();
     showMeter();
     showMode();

     return;
}
//*---------------------------------- Show Main Transceiver Menu mode (BROWSE)
void showLCDCMD() {

}
//*---------------------------------- Show Main Transceiver Menu update mode (Menu Items)
void showLCDGUI() {

}
//====================================================================================================================== 
// evaluate if a change in the GUI is needed and apply it
//====================================================================================================================== 
void showGui() {

    if (getWord(GSW,FGUI)==false) return;
    setWord(&GSW,FGUI,false);

    if (getWord(MSW,CMD)==false) {  //CLI mode, main mode of operation
       showLCDVFO();


       return;
    }

    if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==false) { //CLI mode, BROWSE mode
       showLCDCMD();

       return;
    }

//*--- CLI mode, UPDATE mode
    showLCDGUI();
}
//====================================================================================================================== 
// setBacklight
//====================================================================================================================== 
void setBacklight(bool v) {

     if (getWord(MSW,BCK)==false) {
        TBCK=BACKLIGHT;
        return;
     }

     if (TBCK>0) {
        TBCK=BACKLIGHT;
        setWord(&SSW,FBCK,false);
        return;
     }
     TBCK=BACKLIGHT; 
     lcd->backlight(v);
     lcd->setCursor(0,0);
     setWord(&SSW,FBCK,false);

     usleep(10000);
}
//====================================================================================================================== 
// showMenu()
//====================================================================================================================== 
void showMenu() {

     MMS*  menu=root->curr;
     int   i=menu->mVal;
     char* m=menu->mText;

     MMS*  child=menu->curr;
     int   j=child->mVal;
     char* t=child->mText;

     sprintf(LCD_Buffer," %02d %s",i,m);
     lcd->println(1,0,LCD_Buffer);

     sprintf(LCD_Buffer," %s",t);
     lcd->println(1,1,t);

     if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==true) {
        lcd->setCursor(0,1);
        lcd->typeChar((char)126);
     }
}
//*---  Progress to next Chile

void nextChild(int dir) {
     root->curr->move(dir);
}
//*---  Progress to next Menu item

void nextMenu(int dir) {
     root->move(dir);
}

//*---  backup menu

void backupMenu() {
     root->curr->backup();
}
//*---  save menu

void saveMenu() {
     root->curr->save();
}
//*---  restore menu

void restoreMenu() {
     root->curr->restore();
}
//====================================================================================================================== 
// actuators for GPIO controlled lines
//====================================================================================================================== 
//*----- Cooler
void setCooler(bool f) {

    (f==true ? gpioWrite(GPIO_COOLER,1) : gpioWrite(GPIO_COOLER,0));
    setWord(&MSW,COOLER,f);
    (TRACE>=0x02 ? fprintf(stderr,"%s:setCooler() Cooler set(%s)\n",PROGRAMID,BOOL2CHAR(f)) : _NOP);

    return;
}
//*----- PTT

void setPTT(bool f) {

    (f==true ? gpioWrite(GPIO_PTT,1) : gpioWrite(GPIO_PTT,0));
    setWord(&MSW,PTT,f);
    if (vfo!=nullptr) vfo->setPTT(f);
    //showPTT();
    (TRACE>=0x02 ? fprintf(stderr,"%s:setPTT() PTT set(%s)\n",PROGRAMID,BOOL2CHAR(f)) : _NOP);
    return;
}

//====================================================================================================================== 
// analyze events coming from the hardware, evaluate changes on transceiver and apply
//====================================================================================================================== 
void processGui() {


//*----- CMD=false GUI=* this is VFO panel

     if (getWord(MSW,CMD)==false) {

        if (getWord(GSW,ECW)==true) {  //increase f
           setWord(&GSW,ECW,false);
           setWord(&GSW,ECCW,false);
           if (vfo->getPTT()==false && vfo->getRIT()==false) { 
              f=vfo->up();
              TVFO=3000;
              setWord(&GSW,FBLINK,true);
              //showFrequency();
              //showChange();
           } else {
              if (vfo->getRIT()==true) {
                 float r=vfo->updateRIT(+1);
                 //showRIT();
              }
           }
        }


        if (getWord(GSW,ECCW)==true) {  //decrease f

           setWord(&GSW,ECCW,false);
           setWord(&GSW,ECW,false);
           if (vfo->getPTT()==false && vfo->getRIT()==false) { 
              f=vfo->down();
              TVFO=3000;
              setWord(&GSW,FBLINK,true);
              //showFrequency();
              //showChange();
           } else {
              if (vfo->getRIT()==true) {
                 float r=vfo->updateRIT(-1);
                 //showRIT();
              }
           }
        }

        if (getWord(SSW,FVFO)==true) {  // manage blinking
           setWord(&GSW,FBLINK,false);
           setWord(&SSW,FVFO,false);
           showChange();
        }

        if (getWord(GSW,FAUX)==true) {  // swap VFO
           setWord(&GSW,FAUX,false);
           setWord(&SSW,FBLINK,true);
           vfo->swapVFO();
           //showVFO();
           //showChange();
        }

        if (getWord(GSW,FAUXL)==true) { // enable RIT
           setWord(&GSW,FAUXL,false);
           vfo->setRIT(vfo->vfo,!vfo->getRIT(vfo->vfo));
           //showRIT();
        }

        if (getWord(GSW,FSW)==true) {   // switch to menu mode
           setWord(&GSW,FSW,false);
           setWord(&MSW,CMD,true);
           setWord(&MSW,GUI,false);
           lcd->clear();
           showMenu();

        }

        if (getWord(GSW,FSWL)==true) {  // switch to Split operation
           setWord(&GSW,FSWL,false);
           vfo->setSplit(!vfo->getSplit());
           //showSplit();
           if (vfo->vfo != VFOA) { // split operates VFOA as primary and VFOB as secondary, force that configuration
              setWord(&GSW,FAUX,true);              
              processGui(); // recursive call to operate the VFO change
           }
        }

     }


//*----- CMD=true GUI=false this is Menu panel

     if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==false) {

        if (getWord(GSW,FSW)==true) {  //return to VFO mode
           setWord(&GSW,FSW,false);
           setWord(&MSW,CMD,false);
           showLCDVFO();
        }

        if (getWord(GSW,ECW)==true) {  //while in menu mode turn knob clockwise
           setWord(&GSW,ECW,false);
           nextMenu(+1);
	   lcd->clear();
           showMenu();
        }

        if (getWord(GSW,ECCW)==true) {  //while in menu mode turn knob counterclockwise
           setWord(&GSW,ECCW,false);
           nextMenu(-1);
           lcd->clear();
           showMenu();
        }


        if (getWord(GSW,FSWL)==true) {  //while in menu mode transition to GUI mode (backup)
           setWord(&GSW,FSWL,false);
           setWord(&MSW,GUI,true);
           lcd->clear();
           backupMenu();
           showMenu();
        }

        if (getWord(SSW,FSAVE)==true) { //restore menu after saving message
           setWord(&SSW,FSAVE,false);
           lcd->clear();
           showMenu();
        }
     }

//*----- CMD=true GUI=true this is Menu item option panel

     if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==true) {

        if (getWord(GSW,ECW)==true) {  //while in menu mode turn knob clockwise
           setWord(&GSW,ECW,false);
           nextChild(+1);
	   lcd->clear();
           showMenu();
        }

        if (getWord(GSW,ECCW)==true) {  //while in menu mode turn knob counterclockwise
           setWord(&GSW,ECCW,false);
           nextChild(-1);
           lcd->clear();
           showMenu();
        }

        if (getWord(GSW,FSW)==true) {  //in GUI mode return to menu mode without commit of changes
           setWord(&GSW,FSW,false);
           setWord(&MSW,GUI,false);
           lcd->clear();
           restoreMenu();

        }


        if (getWord(GSW,FSWL)==true) {  //in GUI mode return to menu mode with commit of changes
           setWord(&GSW,FSWL,false);
           setWord(&MSW,GUI,false);

           lcd->clear();
           strcpy(LCD_Buffer,"Saving..");
           lcd->println(0,0,LCD_Buffer);
           saveMenu();
           TSAVE=3000;

        }




     }


//*------ Miscellaneaous services

//*--- Manage backlight timeout

     if (getWord(SSW,FBCK)==true) {  //backlight timeout
        setBacklight(false);
     }
}
//====================================================================================================================== 
// change pin handler (GPIO_AUX and GPIO_SW buttons)
//====================================================================================================================== 
void gpiochangePin(int pin,int state,uint32_t tick) {

  (TRACE>=0x03 ? fprintf(stderr,"%s:gpiochangePin() received upcall from gpioWrapper object state pin(%d) state(%d)\n",PROGRAMID,pin,state) : _NOP);

//*--- Manager backligth

  setBacklight(true);

//*--- process interrupt

  if (getWord(GSW,FAUX)==true || getWord(GSW,FAUXL)==true || getWord(GSW,FSW)==true || getWord(GSW,FSWL)==true) {
     (TRACE>=0x00 ? fprintf(stderr,"%s:gpiochangePin() *** ERROR *** previous button event not consumed yet pin(%d) state(%d)\n",PROGRAMID,pin,state) : _NOP);
  }

  if (pin==GPIO_AUX) {
     (TRACE >=0x02 ? fprintf(stderr,"%s:gpiochangePin() GPIO AUX button pin(%d) value(%d)\n",PROGRAMID,pin,state) : _NOP);
     if (state==2) {
        setWord(&GSW,FAUXL,true);
     } else {
        setWord(&GSW,FAUX,true);
     }
     return;
  }
 
 if (pin==GPIO_SW) {
     (TRACE >=0x02 ? fprintf(stderr,"%s:gpiochangePin() manual PTT software encoder\n",PROGRAMID) : _NOP);
     if (state==2) {
        setWord(&GSW,FSWL,true);
     } else {
        setWord(&GSW,FSW,true);
     }
     return;
  }

  return;
  
}
// ======================================================================================================================
// change pin handler (GPIO_AUX and GPIO_SW buttons)
// ======================================================================================================================
void gpiochangeEncoder(int gpio,int state,uint32_t tick) {

//*--- Manage backlight

return;

     setBacklight(true);

//*--- Process encoder interrupt

     (TRACE >=0x03 ? fprintf(stderr,"%s:gpiochangeEncoder() Encoder changed (%d)\n",PROGRAMID,state) : _NOP);
     if (getWord(GSW,ECW)==true || getWord(GSW,ECCW)==true) {
        (TRACE>=0x00 ? fprintf(stderr,"%s:gpiochangeEncoder() *** ERROR *** previous encoder event not consumed yet state(%d)\n",PROGRAMID,state) : _NOP);
     }

//*--- Rotary event to left or right

     if (state>0) {
        setWord(&GSW,ECW,true);
        return;
     }
     setWord(&GSW,ECCW,true);
     return;

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procKeyerUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procKeyerUpdate() \n",PROGRAMID) : _NOP);
     if (p->mVal < 0) {
        p->mVal=0;
     }
     if (p->mVal > 2) {
        p->mVal=2;
     }
     cw_keyer_mode=p->mVal;

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procSpeedUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procSpeedUpdate() \n",PROGRAMID) : _NOP);

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procStepUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procStepUpdate() \n",PROGRAMID) : _NOP);

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procModeUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procModeUpdate() \n",PROGRAMID) : _NOP);

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procShiftUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procShiftUpdate() \n",PROGRAMID) : _NOP);

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procDriveUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procDriveUpdate() \n",PROGRAMID) : _NOP);

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procBackUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procBackUpdate() \n",PROGRAMID) : _NOP);
     if (p->mVal==0) {
        setWord(&MSW,BCK,false);
        TBCK=BACKLIGHT;
        return;
     }

     setWord(&MSW,BCK,true);
     TBCK=p->mVal*1000;
     lcd->backlight(true);
     lcd->setCursor(0,0);

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procCoolUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procCoolUpdate()\n",PROGRAMID) : _NOP);

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procBeaconUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procBeaconUpdate()\n",PROGRAMID) : _NOP);

}

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
//*--------------------------------------------------------------------------------------------------
//* handler for GUI change events
//*--------------------------------------------------------------------------------------------------
void procSpeedChange(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procSpeedChange() \n",PROGRAMID) : _NOP);
     sprintf(p->mText,"%2d wpm",p->mVal*5);

}
//*--------------------------------------------------------------------------------------------------
void procKeyerChange(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procKeyerChange() \n",PROGRAMID) : _NOP);
     if (p->mVal>2) {p->mVal=2;}
     if (p->mVal<0) {p->mVal=0;}
     switch(p->mVal) {
       case 0 : { sprintf(p->mText,"%2d Straight",p->mVal); break;}
       case 1 : { sprintf(p->mText,"%2d Iambic A",p->mVal); break;}
       case 2 : { sprintf(p->mText,"%2d Iambic B",p->mVal); break;}
     }
}
//*--------------------------------------------------------------------------------------------------
void procModeChange(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procModeChange() \n",PROGRAMID) : _NOP);
     if (vfo==nullptr) {
        strcpy(p->mText,"cw");
     } else {
        vfo->code2mode(p->mVal,p->mText);
     }

}
//*--------------------------------------------------------------------------------------------------
void procStepChange(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procStepChange() \n",PROGRAMID) : _NOP);
     sprintf(p->mText,"%3d Hz",p->mVal*100);

}
//*--------------------------------------------------------------------------------------------------
void procShiftChange(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procShiftChange() \n",PROGRAMID) : _NOP);
     sprintf(p->mText,"%3d Hz",p->mVal*100);

}
//*--------------------------------------------------------------------------------------------------
void procBacklChange(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procBacklChange() \n",PROGRAMID) : _NOP);
     sprintf(p->mText,"%3d secs",p->mVal);

}
//*--------------------------------------------------------------------------------------------------
void procDriveChange(MMS* p) {

char f[8];
char lev[2];
int  k = 255;
char c = k;
     sprintf(lev,"%c", c); //prints A
     p->mVal=p->mVal & 0x0f;
     strcpy(f,"       ");
     for (int i=0;i<p->mVal;i++) {
         f[i]=lev[0];
     }
     (TRACE>=0x02 ? fprintf(stderr,"%s:procDriveChange %d [%s] \n",PROGRAMID,p->mVal,f) : _NOP);
     sprintf(p->mText," %d [%s]",p->mVal,f);

     if (vfo!=nullptr) {vfo->POWER=p->mVal;cat->POWER=vfo->POWER;}
     showMeter();

}

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

//*--------------------------------------------------------------------------------------------------
//* createMenu (create GUI structure)
//*--------------------------------------------------------------------------------------------------
void createMenu() {

     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Initialize menu structure\n",PROGRAMID) : _NOP);

//*--- Create root menu item

     root=new MMS(0,(char*)"root",NULL,NULL);
     root->TRACE=0x00;

//*--- Create first child level (actual menu)

     keyer=  new MMS(0,(char*)"Keyer",procKeyerChange,procKeyerUpdate);
     mode=   new MMS(1,(char*)"Mode",procModeChange,procModeUpdate);
     speed=  new MMS(2,(char*)"Speed",procSpeedChange,procSpeedUpdate);
     step=   new MMS(3,(char*)"Step",procStepChange,procStepUpdate);
     shift=  new MMS(4,(char*)"Shift",procShiftChange,procShiftUpdate);
     drive=  new MMS(5,(char*)"Drive",procDriveChange,procDriveUpdate);
     backl=  new MMS(6,(char*)"Backlight",procBacklChange,procBackUpdate);
     cool=   new MMS(7,(char*)"Cooler",NULL,procCoolUpdate);
     beacon= new MMS(8,(char*)"Beacon",NULL,procBeaconUpdate);

//*--- Associate menu items to root to establish navigation

     root->add(keyer);
     root->add(mode);
     root->add(speed);
     root->add(step);
     root->add(shift);
     root->add(drive);
     root->add(backl);
     root->add(cool);
     root->add(beacon);

     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() First menu structure created\n",PROGRAMID) : _NOP);


//*--- Create second level items, actual options

     straight=new MMS(0,(char*)"Straight",NULL,NULL);
     iambicA =new MMS(1,(char*)"Iambic A",NULL,NULL);
     iambicB =new MMS(2,(char*)"Iambic B",NULL,NULL);

     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Keyer created\n",PROGRAMID) : _NOP);


     bcnoff=new MMS(0,(char*)"Beacon Off",NULL,NULL);
     bcnon =new MMS(1,(char*)"Beacon On",NULL,NULL);

     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Beacon created\n",PROGRAMID) : _NOP);


//   spval=new MMS(3,(char*)"*",NULL,NULL);
     spval=new MMS((s/5),(char*)"*",NULL,NULL);
     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Keyer speed created speed(%d)\n",PROGRAMID,(s/5)) : _NOP);



     stval=new MMS(1,(char*)"*",NULL,NULL);
     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Step structure created\n",PROGRAMID) : _NOP);


int  sh=(int)x/100;
     shval=new MMS(sh,(char*)"*",NULL,NULL);
//   shval=new MMS(6,(char*)"*",NULL,NULL);
     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Shift menu structure created shift(%d)\n",PROGRAMID,sh) : _NOP);



//   drval=new MMS(6,(char*)"*",NULL,NULL);
     drval=new MMS(l,(char*)"*",NULL,NULL);  //initialize with level
     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Drive menu structure created\n",PROGRAMID) : _NOP);


     if (vfo==nullptr) {
        modval=new MMS(m,(char*)"*",NULL,NULL);
     } else {
        modval=new MMS(vfo->MODE,(char*)"*",NULL,NULL);
     }

//   modval=new MMS(2,(char*)"*",NULL,NULL);
     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Mode menu structure created\n",PROGRAMID) : _NOP);



     backval=new MMS(backlight,(char*)"*",NULL,NULL);
//     backof=new MMS(1,(char*)"Backlight Off",NULL,NULL);
     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Backlight menu structure created backlight(%d)\n",PROGRAMID,backlight) : _NOP);


     coolon=new MMS(0,(char*)"Cooler On",NULL,NULL);
     coolof=new MMS(1,(char*)"Cooler Off",NULL,NULL);
     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Cooler menu structure created\n",PROGRAMID) : _NOP);

//*--- associate options to menu items

     keyer->add(straight);
     keyer->add(iambicA);
     keyer->add(iambicB);

     mode->add(modval);

     speed->add(spval);
     speed->lowerLimit(1);
     speed->upperLimit(10);

     step->add(stval);
     step->lowerLimit(1);
     step->upperLimit(5);

     shift->add(shval);
     shift->lowerLimit(4);
     shift->upperLimit(8);

     drive->add(drval);
     drive->lowerLimit(0);
     drive->upperLimit(8);

     backl->add(backval);
     backl->lowerLimit(0);
     backl->upperLimit(60);

//     backl->add(backof);

     cool->add(coolon);
     cool->add(coolof);

     beacon->add(bcnoff);
     beacon->add(bcnon);
     (TRACE>=0x01 ? fprintf(stderr,"%s:createMenu() Menu associations structure created\n",PROGRAMID) : _NOP);
     (TRACE>=0x02 ? root->list() : (void) _NOP );

}
//*--------------------------------------------------------------------------------------------------
//* setupLCD setup the LCD definitions
//*--------------------------------------------------------------------------------------------------
void setupLCD() {


//*--- Special characters used in the GUI

byte TX[8] = {0B11111,0B10001,0B11011,0B11011,0B11011,0B11011,0B11111}; // Inverted T (Transmission Mode)
byte SP[8] = {31,17,23,17,29,17,31};    //Inverted S (Split)
byte S1[8] = {0B10000,0B10000,0B10000,0B10000,0B10000,0B10000,0B10000,0B10000}; // S1 Signal
byte S2[8] = {0B11000,0B11000,0B11000,0B11000,0B11000,0B11000,0B11000,0B11000}; // S2 Signal
byte S3[8] = {0B11100,0B11100,0B11100,0B11100,0B11100,0B11100,0B11100,0B11100}; // S3 Signal
byte S4[8] = {0B11110,0B11110,0B11110,0B11110,0B11110,0B11110,0B11110,0B11110}; // S4 Signal
byte S5[8] = {0B11111,0B11111,0B11111,0B11111,0B11111,0B11111,0B11111,0B11111}; // S5 Signal
byte NA[8] = {0B11111,0B11011,0B10101,0B10101,0B10101,0B10001,0B10101,0B00000}; // Inverted A
byte NB[8] = {0B11111,0B10001,0B10101,0B10011,0B10101,0B10101,0B10001,0B00000}; // Inverted B
byte NS[8] = {0B11111,0B11001,0B10111,0B10011,0B11101,0B11101,0B10011,0B00000}; // Inverted S


//*--- setup LCD configuration

   lcd=new LCDLib(NULL);
   (TRACE>=0x01 ? fprintf(stderr,"%s:setupLCD() LCD system initialization\n",PROGRAMID) : _NOP);

   lcd->begin(16,2);
   lcd->clear();

   lcd->createChar(0,TX);
   lcd->createChar(1,S1);
   lcd->createChar(2,S2);
   lcd->createChar(3,S3);
   lcd->createChar(4,S4);

   lcd->createChar(5,NS);
   lcd->createChar(6,NA);
   lcd->createChar(7,NB);

   lcd_light=LCD_ON;
   lcd->backlight(true);
   lcd->setCursor(0,0);
   lcd->print("Loading...");

}
//*--------------------------[Rotary Encoder Interrupt Handler]--------------------------------------
//* Interrupt handler for Rotary Encoder CW and CCW control
//*--------------------------------------------------------------------------------------------------
void updateEncoders(int gpio, int level, uint32_t tick)
{
        if (level != 0) {  //ignore non falling part of the interruption
           return;
        }

        //setBacklight(true);

        if (getWord(GSW,ECW)==true || getWord(GSW,ECCW) ==true) { //exit if pending to service a previous one
           (TRACE>=0x02 ? fprintf(stderr,"%s:updateEnconders() Last CW/CCW signal pending processsing, ignored!\n",PROGRAMID) : _NOP);
           return;
        }

        int clkState=gpioRead(GPIO_CLK);
        int dtState= gpioRead(GPIO_DT);

        endEncoder = std::chrono::system_clock::now();

        int lapEncoder=std::chrono::duration_cast<std::chrono::milliseconds>(endEncoder - startEncoder).count();

        if ( lapEncoder  < MINENCLAP )  {
           (TRACE>=0x02 ? fprintf(stderr,"%s:updateEnconders() CW/CCW signal too close from last, ignored lap(%d)!\n",PROGRAMID,lapEncoder) : _NOP);
           return;
        }

        if (dtState != clkState) {
          counter++;
          setWord(&GSW,ECCW,true);
        } else {
          counter--;
          setWord(&GSW,ECW,true);
        }

        clkLastState=clkState;        
        startEncoder = std::chrono::system_clock::now();

}

//*--------------------------[Rotary Encoder Interrupt Handler]--------------------------------------
//* Interrupt handler routine for Rotary Encoder Push button
//*--------------------------------------------------------------------------------------------------
void updateSW(int gpio, int level, uint32_t tick)
{

     setBacklight(true);
     if (level != 0) {
        endPush = std::chrono::system_clock::now();
        int lapPush=std::chrono::duration_cast<std::chrono::milliseconds>(endPush - startPush).count();
        if (getWord(GSW,FSW)==true || getWord(GSW,FSWL)==true) {
           (TRACE>=0x02 ? fprintf(stderr,"%s:updateSW() Last SW signal pending processsing, ignored!\n",PROGRAMID) : _NOP);
           return;
        }
        if (lapPush < MINSWPUSH) {
           (TRACE>=0x02 ? fprintf(stderr,"%s:updateSW() SW pulsetoo short! ignored!\n",PROGRAMID) : _NOP) ;
           return;
        } else {
           if (lapPush > MAXSWPUSH) {
              (TRACE>=0x02 ? fprintf(stderr,"%s:updateSW() SW long pulse detected lap(%d)\n",PROGRAMID,lapPush) : _NOP);
              setWord(&GSW,FSWL,true);
           } else {
              (TRACE>=0x02 ? fprintf(stderr,"%s:updateSW() SW brief pulse detected lap(%d)\n",PROGRAMID,lapPush) : _NOP);
              setWord(&GSW,FSW,true);
           }
           return;
        }
     }
     startPush = std::chrono::system_clock::now();
     int pushSW=gpioRead(GPIO_SW);
}
//*--------------------------[Rotary Encoder Interrupt Handler]--------------------------------------
//* Interrupt handler routine for Rotary Encoder Push button
//*--------------------------------------------------------------------------------------------------
void updateAUX(int gpio, int level, uint32_t tick)
{
     setBacklight(true);
     if (level != 0) {
        endAux = std::chrono::system_clock::now();
        int lapAux=std::chrono::duration_cast<std::chrono::milliseconds>(endAux - startAux).count();
        if (getWord(GSW,FAUX)==true || getWord(GSW,FAUXL)==true) {
              (TRACE>=0x02 ? fprintf(stderr,"%s:updateSW() Last AUX signal pending processsing, ignored!\n",PROGRAMID) : _NOP);
              return;
        }
        if (lapAux < MINSWPUSH) {
           (TRACE>=0x02 ? fprintf(stderr,"%s:updateSW() AUX pulsetoo short! ignored!\n",PROGRAMID) : _NOP);
           return;
        } else {
           if (lapAux > MAXSWPUSH) {
              (TRACE>=0x02 ? fprintf(stderr,"%s:updateSW() AUX long pulse detected lap(%d)\n",PROGRAMID,lapAux) : _NOP);
              setWord(&GSW,FAUXL,true);
           } else {
              (TRACE>=0x02 ? fprintf(stderr,"%s:updateSW() AUX brief pulse detected lap(%d)\n",PROGRAMID,lapAux) : _NOP);
              setWord(&GSW,FAUX,true);
           }
           return;
        }
     }
     startAux = std::chrono::system_clock::now();
     int pushAUX=gpioRead(GPIO_AUX);
}
//*--------------------------[Rotary Encoder Interrupt Handler]--------------------------------------
//* Interrupt handler routine for Rotary Encoder Push button
//*--------------------------------------------------------------------------------------------------
void updateKeyer(int gpio, int level, uint32_t tick)
{
     if (level>1) {
        (TRACE>=0x02 ? fprintf(stderr,"%s:updateKeyer() Pin(%d) Pending signals level(%d), ignored! tick(%d)\n",PROGRAMID,gpio,level,tick) : _NOP);
         return;
     }

     setBacklight(true);

     if (getWord(GSW,FKEYUP)==true || getWord(GSW,FKEYDOWN)==true) {
        (TRACE>=0x02 ? fprintf(stderr,"%s:updateKeyer() Last Keyer signal pending processsing, ignored! tick(%d)\n",PROGRAMID,tick) : _NOP);
        return;
     }

     if (level == 0) {
        (TRACE>=0x02 ? fprintf(stderr,"%s:updateKeyer() Pin(%d) level(%d) tick(%d) Keyer down\n",PROGRAMID,gpio,level,tick) : _NOP);
        setWord(&SSW,FKEYDOWN,true);
     } else {
        (TRACE>=0x02 ? fprintf(stderr,"%s:updateKeyer() Pin(%d) level(%d) tick(%d) Keyer up\n",PROGRAMID,gpio,level,tick) : _NOP);
        setWord(&SSW,FKEYUP,true);
     }
}
//*--------------------------------------------------------------------------------------------------
//* setupGPIO setup the GPIO definitions
//*--------------------------------------------------------------------------------------------------
void setupGPIO() {

    if(gpioInitialise()<0) {
        (TRACE>=0x00 ? fprintf(stderr,"%s:setupGPIO() Cannot initialize GPIO\n",PROGRAMID) : _NOP);
        exit(16);
    }


//*---- Turn cooler on

    gpioSetMode(GPIO_COOLER, PI_OUTPUT);
    gpioWrite(GPIO_COOLER, 1);
    usleep(100000);
 

    gpioSetMode(GPIO_AUX, PI_INPUT);
    gpioSetPullUpDown(GPIO_AUX,PI_PUD_UP);
    gpioSetAlertFunc(GPIO_AUX,updateAUX);
    usleep(100000);

//*---- Configure Encoder

    gpioSetMode(GPIO_SW, PI_INPUT);
    gpioSetPullUpDown(GPIO_SW,PI_PUD_UP);
    gpioSetAlertFunc(GPIO_SW,updateSW);
    usleep(100000);

    gpioSetMode(GPIO_LEFT,PI_INPUT);
    gpioSetPullUpDown(GPIO_LEFT,PI_PUD_UP);
    gpioSetAlertFunc(GPIO_LEFT,updateKeyer);
    usleep(100000);

    gpioSetMode(GPIO_CLK, PI_INPUT);
    gpioSetPullUpDown(GPIO_CLK,PI_PUD_UP);
    usleep(100000);
    gpioSetISRFunc(GPIO_CLK, FALLING_EDGE,0,updateEncoders);

    gpioSetMode(GPIO_DT, PI_INPUT);
    gpioSetPullUpDown(GPIO_DT,PI_PUD_UP);
    usleep(100000);


    for (int i=0;i<64;i++) {

        gpioSetSignalFunc(i,sighandler);

    }
}
