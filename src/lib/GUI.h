//*=====================================================================================================================
//* VFO Panel presentation handlers
//*=====================================================================================================================
//*----------------------
void showVFO() {

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

     if (vfo->vfo == VFOA) {
        row=0;
        alt=1;
     } else {
        row=1;
        alt=0;
     }

     if (getWord(GSW,FBLINK)==true) {

        lcd->setCursor(1,row);
        lcd->typeChar((char)126);

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


     if (getWord(MSW,PTT)==true) {
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


     if (getWord(MSW,PTT)==false) {  //inverted for testing
        strcpy(LCD_Buffer," ");
        lcd->println(11,1,LCD_Buffer);
        return;
     }
     lcd->setCursor(11,1);
     lcd->write(0);

}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showKeyer() {

     switch(keyer->get()->mVal) {
        case 0 : strcpy(LCD_Buffer,"S"); break;
        case 1 : strcpy(LCD_Buffer,"1"); break;
        case 2 : strcpy(LCD_Buffer,"2"); break;
     }

     lcd->println(12,1,LCD_Buffer);

}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showMeter() {

     lcd->setCursor(13,1);   //Placeholder Meter till a routine is developed for it
     lcd->write(byte(255));
     lcd->write(byte(255));
     lcd->write(byte(255));

}
//*---------------------------------- Show Split
void showSplit() {

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

     showVFO();
     showChange();
     showFrequency();
     showPTT();
     showRIT();
     showSplit();
     showKeyer();
     showMeter();

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

     
     (v==true ? TBCK=BACKLIGHT : _NOP);
     lcd->backlight(v);
     lcd->setCursor(0,0);
     setWord(&SSW,FBCK,false);


}
//====================================================================================================================== 
// analyze events coming from the hardware, evaluate changes on transceiver and apply
//====================================================================================================================== 
void processGui() {


//*----- CMD=false GUI=* this is VFO panel

     if (getWord(MSW,CMD)==false) {

        if (getWord(GSW,ECW)==true) {  //increase f
           setWord(&GSW,ECW,false);
           if (getWord(MSW,PTT)==false && vfo->getRIT()==false) { 
              f=vfo->up();
              TVFO=3000;
              setWord(&GSW,FBLINK,true);
              showFrequency();
              showChange();
           } else {
              if (vfo->getRIT()==true) {
                 float r=vfo->updateRIT(+1);
                 showRIT();
              }
           }
        }


        if (getWord(GSW,ECCW)==true) {  //decrease f

           setWord(&GSW,ECCW,false);
           if (getWord(MSW,PTT)==false && vfo->getRIT()==false) { 
              f=vfo->down();
              TVFO=3000;
              setWord(&GSW,FBLINK,true);
              showFrequency();
              showChange();
           } else {
              if (vfo->getRIT()==true) {
                 float r=vfo->updateRIT(-1);
                 showRIT();
              }
           }
        }

        if (getWord(SSW,FVFO)==true) {
           setWord(&GSW,FBLINK,false);
           setWord(&SSW,FVFO,false);
           showChange();
        }

        if (getWord(GSW,FAUX)==true) {
           setWord(&GSW,FAUX,false);
           vfo->swapVFO();
           showVFO();
           showChange();
        }

        if (getWord(GSW,FAUXL)==true) {
           setWord(&GSW,FAUXL,false);
           vfo->setRIT(vfo->vfo,!vfo->getRIT(vfo->vfo));
           showRIT();
        }

        if (getWord(GSW,FSW)==true) {
           setWord(&GSW,FSW,false);
           setWord(&MSW,CMD,true);
           setWord(&MSW,GUI,false);

           lcd->clear();
           strcpy(LCD_Buffer,"Menu!");
           lcd->println(0,0,LCD_Buffer);

        }

        if (getWord(GSW,FSWL)==true) {
           setWord(&GSW,FSWL,false);
           vfo->setSplit(!vfo->getSplit());
           showSplit();
        }

     }


//*----- CMD=true GUI=false this is Menu panel

     if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==false) {

        if (getWord(GSW,FSW)==true) {
           setWord(&GSW,FSW,false);
           setWord(&MSW,CMD,false);
           showLCDVFO();
        }


        if (getWord(GSW,FSWL)==true) {
           setWord(&GSW,FSWL,false);
           setWord(&MSW,GUI,true);

           lcd->clear();
           strcpy(LCD_Buffer,"Options Menu!");
           lcd->println(0,0,LCD_Buffer);

        }




     }
//*----- CMD=true GUI=true this is Menu item option panel

     if (getWord(MSW,CMD)==true && getWord(MSW,GUI)==true) {

        if (getWord(GSW,FSW)==true) {
           setWord(&GSW,FSW,false);
           setWord(&MSW,GUI,false);

           lcd->clear();
           strcpy(LCD_Buffer,"Options Restoring");
           lcd->println(0,0,LCD_Buffer);

        }


        if (getWord(GSW,FSWL)==true) {
           setWord(&GSW,FSWL,false);
           setWord(&MSW,GUI,false);

           lcd->clear();
           strcpy(LCD_Buffer,"Options Saving");
           lcd->println(0,0,LCD_Buffer);

        }


     }


//*------ Miscellaneaous services

//*--- Manage backlight timeout

     if (getWord(SSW,FBCK)==true) {
        setBacklight(false);
     }
}
//====================================================================================================================== 
// change pin handler (GPIO_AUX and GPIO_SW buttons)
//====================================================================================================================== 
void gpiochangePin(int pin,int state) {
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
void gpiochangeEncoder(int clk,int dt,int state) {

//*--- Manage backlight

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

}
//*--------------------------------------------------------------------------------------------------
//* handler for GUI update events
//*--------------------------------------------------------------------------------------------------
void procCoolUpdate(MMS* p) {

     (TRACE>=0x02 ? fprintf(stderr,"%s:procCoolUpdate() \n",PROGRAMID) : _NOP);

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
void procDriveChange(MMS* p) {

char f[8];
char l[2];

     strcpy(l,"X");
     strcpy(f,"       ");
     for (int i=0;i<p->mVal;i++) {
         f[i]=l[0];
     }
     (TRACE>=0x02 ? fprintf(stderr,"%s:procDriveChange %d [%s] \n",PROGRAMID,p->mVal,f) : _NOP);
     sprintf(p->mText," %d [%s]",p->mVal,f);

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

     keyer=new MMS(0,(char*)"Keyer",NULL,procKeyerUpdate);
     speed=new MMS(1,(char*)"Speed",procSpeedChange,procSpeedUpdate);
     step= new MMS(1,(char*)"Step",procStepChange,procStepUpdate);
     shift=new MMS(7,(char*)"Shift",procShiftChange,procShiftUpdate);
     drive=new MMS(5,(char*)"Drive",procDriveChange,procDriveUpdate);
     backl=new MMS(3,(char*)"Backlight",NULL,procBackUpdate);
     cool= new MMS(3,(char*)"Cooler",NULL,procCoolUpdate);

//*--- Associate menu items to root to establish navigation

     root->add(keyer);
     root->add(speed);
     root->add(shift);
     root->add(drive);
     root->add(backl);
     root->add(cool);

//*--- Create second level items, actual options

     straight=new MMS(0,(char*)"Straight",NULL,NULL);
     iambicA =new MMS(1,(char*)"Iambic A",NULL,NULL);
     iambicB =new MMS(2,(char*)"Iambic B",NULL,NULL);

     spval=new MMS(3,(char*)"*",NULL,NULL);
     stval=new MMS(1,(char*)"*",NULL,NULL);
     shval=new MMS(6,(char*)"*",NULL,NULL);
     drval=new MMS(6,(char*)"*",NULL,NULL);

     backon=new MMS(0,(char*)"Backlight On",NULL,NULL);
     backof=new MMS(1,(char*)"Backlight Off",NULL,NULL);
     coolon=new MMS(0,(char*)"Cooler On",NULL,NULL);
     coolof=new MMS(1,(char*)"Cooler Off",NULL,NULL);

//*--- associate options to menu items

     keyer->add(straight);
     keyer->add(iambicA);
     keyer->add(iambicB);

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

     backl->add(backon);
     backl->add(backof);

     cool->add(coolon);
     cool->add(coolof);

     (TRACE>=0x02 ? root->list() : (void) _NOP );

}
//*--------------------------------------------------------------------------------------------------
//* setupLCD setup the LCD definitions
//*--------------------------------------------------------------------------------------------------
void setupLCD() {


//*--- Special characters used in the GUI

byte TX[8] = {0B11111,0B10001,0B11011,0B11011,0B11011,0B11011,0B11111}; // Inverted T (Transmission Mode)
byte SP[8] = {31,17,23,17,29,17,31};    //Inverted S (Split)
byte S1[8] = {0B10000,0B10000,0B10000,0B10000,0B10000,0B10000,0B10000}; // S1 Signal
byte S2[8] = {0B11000,0B11000,0B11000,0B11000,0B11000,0B11000,0B11000}; // S2 Signal
byte S3[8] = {0B11100,0B11100,0B11100,0B11100,0B11100,0B11100,0B11100}; // S3 Signal
byte S4[8] = {0B11110,0B11110,0B11110,0B11110,0B11110,0B11110,0B11110}; // S4 Signal
byte S5[8] = {0B11111,0B11111,0B11111,0B11111,0B11111,0B11111,0B11111}; // S5 Signal
byte NA[8] = {0B11111,0B11011,0B10101,0B10101,0B10101,0B10001,0B10101}; // Inverted A
byte NB[8] = {0B11111,0B10001,0B10101,0B10011,0B10101,0B10101,0B10001}; // Inverted B
byte NS[8] = {0B11111,0B11001,0B10111,0B10011,0B11101,0B11101,0B10011}; // Inverted S


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
//*--------------------------------------------------------------------------------------------------
//* setupGPIO setup the GPIO definitions
//*--------------------------------------------------------------------------------------------------
void setupGPIO() {

    gpio=new gpioWrapper(gpiochangePin);
    gpio->TRACE=TRACE;

   (TRACE>=0x01 ? fprintf(stderr,"%s:setupGPIO() GPIO system initialization\n",PROGRAMID) : _NOP);

//     if (gpio->setPin(GPIO_PTT,GPIO_OUT,GPIO_PUP,GPIO_NLP) == -1) {
//        (TRACE>=0x00 ? fprintf(stderr,"%s:main() failure to initialize pin(%s)\n",PROGRAMID,(char*)GPIO_PTT) : _NOP);
//        exit(16);
//     }
//     if (gpio->setPin(GPIO_PA,GPIO_OUT,GPIO_PUP,GPIO_NLP) == -1) {
//        (TRACE>=0x0 ? fprintf(stderr,"%s:main() failure to initialize pin(%s)\n",PROGRAMID,(char*)GPIO_PA) : _NOP);
//        exit(16);
//     }

     if (gpio->setPin(GPIO_AUX,GPIO_IN,GPIO_PUP,GPIO_NLP) == -1) {
        (TRACE>=0x0 ? fprintf(stderr,"%s:main() failure to initialize pin(%s)\n",PROGRAMID,(char*)GPIO_AUX) : _NOP);
        exit(16);
     }
     if (gpio->setPin(GPIO_SW,GPIO_IN,GPIO_PUP,GPIO_NLP) == -1) {
        (TRACE>=0x0 ? fprintf(stderr,"%s:main() failure to initialize pin(%s)\n",PROGRAMID,(char*)GPIO_SW) : _NOP);
        exit(16);
     }
//     if (gpio->setPin(GPIO_KEYER,GPIO_IN,GPIO_PUP,GPIO_NLP) == -1) {
//        (TRACE>=0x00 ? fprintf(stderr,"%s:main() failure to initialize pin(%s)\n",PROGRAMID,(char*)GPIO_KEYER) : _NOP);
//        exit(16);
//     }

//     if (gpio->setPin(GPIO_COOLER,GPIO_OUT,GPIO_PUP,GPIO_NLP) == -1) {
//        (TRACE>=0x00 ? fprintf(stderr,"%s:main() failure to initialize pin(%s)\n",PROGRAMID,(char*)GPIO_COOLER) : _NOP);
//        exit(16);
//     }
    if (gpio->setEncoder(gpiochangeEncoder) == -1) {
        (TRACE>=0x00 ? fprintf(stderr,"%s:main() failure to initialize pin(%s-%s)\n",PROGRAMID,(char*)GPIO_CLK,(char*)GPIO_DT) : _NOP);
        exit(16);
     }

     if (gpio->start() == -1) {
        (TRACE>=0x00 ? fprintf(stderr,"%s:main() failure to start gpioWrapper object\n",PROGRAMID) : _NOP);
        exit(8);
     }

     usleep(1000);

}
