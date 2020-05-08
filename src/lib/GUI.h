//====================================================================================================================== 
// analyze events coming from the hardware, evaluate changes on transceiver and apply
//====================================================================================================================== 
void processGui() {

}
//*----------------------
void showFrequency() {

char LCDBuffer[33];

     if (vfo->vfo == VFOA) {
        lcd->setCursor(0,0);
        lcd->write(1);
        strcpy(LCDBuffer,"B");
        lcd->println(0,1,LCDBuffer);
     } else {
        strcpy(LCDBuffer,"A");
        lcd->println(0,0,LCDBuffer);
        lcd->setCursor(0,0);
        lcd->write(2);
     }

     sprintf(LCDBuffer,"  %5.2f",vfo->get(VFOA)/1000.0);
     lcd->println(1,0,LCDBuffer);

     sprintf(LCDBuffer,"  %5.2f",vfo->get(VFOB)/1000.0);
     lcd->println(1,1,LCDBuffer);

}
//*---------------------
void showRIT() {

char LCDBuffer[33];

     if (getWord(MSW,PTT)==true) {
        return;
     }
     sprintf(LCDBuffer,"%+03d",vfo->getRIT(vfo->vfo));
     if (getWord(FT817,RIT)==true) {
         lcd->println(10,0,LCDBuffer) ;
     }

}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showPTT() {

char LCDBuffer[33];
     if (getWord(MSW,PTT)==true) {  //inverted for testing
        strcpy(LCDBuffer," ");
        lcd->println(11,1,LCDBuffer);
        return;
     }
     lcd->setCursor(11,1);
     lcd->write(0);

}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showKeyer() {
char LCDBuffer[33];
     switch(keyer->get()->mVal) {
        case 0 : strcpy(LCDBuffer,"S"); break;
        case 1 : strcpy(LCDBuffer,"1"); break;
        case 2 : strcpy(LCDBuffer,"2"); break;
     }
     lcd->println(12,1,LCDBuffer);

}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showMark() {

char LCDBuffer[33];

     if (getWord(MSW,PTT)==true) {
        strcpy(LCDBuffer,"P0");
        lcd->println(14,0,LCDBuffer);
        return;
     }
     strcpy(LCDBuffer,"Rx");
     lcd->println(14,0,LCDBuffer);

}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showMeter() {

     lcd->setCursor(13,1);   //Placeholder Meter till a routine is developed for it
     lcd->write(5);
     lcd->write(5);
     lcd->write(5);

}
//*---------------------------------- Show Main Transceiver LCD Dialog (VFO)
void showLCDVFO() {

     showFrequency();
     showPTT();
     showRIT();
     showKeyer();
     showMark();
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
// change pin handler (GPIO_AUX and GPIO_SW buttons)
//====================================================================================================================== 
void gpiochangePin(int pin,int state) {
  (TRACE>=0x03 ? fprintf(stderr,"%s:gpiochangePin() received upcall from gpioWrapper object state pin(%d) state(%d)\n",PROGRAMID,pin,state) : _NOP);

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

     (TRACE >=0x03 ? fprintf(stderr,"%s:gpiochangeEncoder() Encoder changed (%d)\n",PROGRAMID,state) : _NOP);
     if (getWord(GSW,ECW)==true || getWord(GSW,ECCW)==true) {
        (TRACE>=0x00 ? fprintf(stderr,"%s:gpiochangeEncoder() *** ERROR *** previous encoder event not consumed yet state(%d)\n",PROGRAMID,state) : _NOP);
     }

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

     root=new MMS(0,(char*)"root",NULL,NULL);
     root->TRACE=0x00;

     keyer=new MMS(0,(char*)"Keyer",NULL,procKeyerUpdate);
     speed=new MMS(1,(char*)"Speed",procSpeedChange,procSpeedUpdate);
     step= new MMS(1,(char*)"Step",procStepChange,procStepUpdate);
     shift=new MMS(7,(char*)"Shift",procShiftChange,procShiftUpdate);
     drive=new MMS(5,(char*)"Drive",procDriveChange,procDriveUpdate);
     backl=new MMS(3,(char*)"Backlight",NULL,procBackUpdate);
     cool= new MMS(3,(char*)"Cooler",NULL,procCoolUpdate);

     root->add(keyer);
     root->add(speed);
     root->add(shift);
     root->add(drive);
     root->add(backl);
     root->add(cool);

     straight=new MMS(0,(char*)"Straight",NULL,NULL);
     iambicA =new MMS(1,(char*)"Iambic A",NULL,NULL);
     iambicB =new MMS(2,(char*)"Iambic B",NULL,NULL);

     keyer->add(straight);
     keyer->add(iambicA);
     keyer->add(iambicB);


     spval=new MMS(3,(char*)"*",NULL,NULL);

     speed->add(spval);
     speed->lowerLimit(1);
     speed->upperLimit(10);

     stval=new MMS(1,(char*)"*",NULL,NULL);
     step->add(stval);
     step->lowerLimit(1);
     step->upperLimit(5);

     shval=new MMS(6,(char*)"*",NULL,NULL);
     shift->add(shval);
     shift->lowerLimit(4);
     shift->upperLimit(8);

     drval=new MMS(6,(char*)"*",NULL,NULL);
     drive->add(drval);
     drive->lowerLimit(0);
     drive->upperLimit(8);

     backon=new MMS(0,(char*)"Backlight On",NULL,NULL);
     backof=new MMS(1,(char*)"Backlight Off",NULL,NULL);
     backl->add(backon);
     backl->add(backof);

     coolon=new MMS(0,(char*)"Cooler On",NULL,NULL);
     coolof=new MMS(1,(char*)"Cooler Off",NULL,NULL);
     cool->add(coolon);
     cool->add(coolof);

     (TRACE>=0x02 ? root->list() : (void) _NOP );

}
//*--------------------------------------------------------------------------------------------------
//* setupLCD setup the LCD definitions
//*--------------------------------------------------------------------------------------------------
void setupLCD() {

byte TX[8] = {0B11111,0B10001,0B11011,0B11011,0B11011,0B11011,0B11111}; // Inverted T (Transmission Mode)
byte SP[8] = {31,17,23,17,29,17,31};    //Inverted S (Split)
byte S1[8] = {0B10000,0B10000,0B10000,0B10000,0B10000,0B10000,0B10000}; // S1 Signal
byte S2[8] = {0B11000,0B11000,0B11000,0B11000,0B11000,0B11000,0B11000}; // S2 Signal
byte S3[8] = {0B11100,0B11100,0B11100,0B11100,0B11100,0B11100,0B11100}; // S3 Signal
byte S4[8] = {0B11110,0B11110,0B11110,0B11110,0B11110,0B11110,0B11110}; // S4 Signal
byte S5[8] = {0B11111,0B11111,0B11111,0B11111,0B11111,0B11111,0B11111}; // S5 Signal
//byte NA[8] = {0B01110,0B10001,0B10001,0B10001,0B11111,0B10001,0B10001}; //inverted A
//byte NB[8] = {0B11110,0B10001,0B10001,0B11110,0B10001,0B10001,0B11110}; //inverted B
byte NB[8] = {
	0B00001,
	0B01110,
	0B01110,
	0B00001,
	0B01110,
	0B01110,
	0B00001};

byte NA[8] = {
	0B10001,
	0B01110,
	0B01110,
	0B01110,
	0B00000,
	0B01110,
	0B01110};

   lcd=new LCDLib(NULL);
   (TRACE>=0x01 ? fprintf(stderr,"%s:setupLCD() LCD system initialization\n",PROGRAMID) : _NOP);

   lcd->begin(16,2);
   lcd->clear();

   lcd->createChar(0,TX);
   lcd->createChar(1,S1);
   lcd->createChar(2,S2);
   lcd->createChar(3,S3);
   lcd->createChar(4,S4);
   lcd->createChar(5,S5);

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
