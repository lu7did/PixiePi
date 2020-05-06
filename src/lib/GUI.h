// ======================================================================================================================
// change pin handler (GPIO_AUX and GPIO_SW buttons)
// ======================================================================================================================
void gpiochangePin(int pin,int state) {


  (TRACE>=0x02 ? fprintf(stderr,"%s:gpiochangePin() received upcall from gpioWrapper object state pin(%d) state(%d)\n",PROGRAMID,pin,state) : _NOP);
  if (pin==GPIO_AUX) {
     (TRACE >=0x01 ? fprintf(stderr,"%s:gpiochangePin() manual PTT operation thru AUX button pin(%d) value(%d)\n",PROGRAMID,pin,state) : _NOP);
  }
 
 if (pin==GPIO_SW) {
     (TRACE >=0x01 ? fprintf(stderr,"%s:gpiochangePin() manual PTT software encoder\n",PROGRAMID) : _NOP);
  }
  
}
// ======================================================================================================================
// change pin handler (GPIO_AUX and GPIO_SW buttons)
// ======================================================================================================================
void gpiochangeEncoder(int clk,int dt,int state) {

     (TRACE >=0x01 ? fprintf(stderr,"%s:gpiochangeEncoder() enconder changed (%d)\n",PROGRAMID,state) : _NOP);

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

MMS* root=new MMS(0,(char*)"root",NULL,NULL);
     root->TRACE=0x00;

MMS* keyer=new MMS(0,(char*)"Keyer",NULL,procKeyerUpdate);
MMS* speed=new MMS(1,(char*)"Speed",procSpeedChange,procSpeedUpdate);
MMS* step= new MMS(1,(char*)"Step",procStepChange,procStepUpdate);
MMS* shift=new MMS(7,(char*)"Shift",procShiftChange,procShiftUpdate);
MMS* drive=new MMS(5,(char*)"Drive",procDriveChange,procDriveUpdate);
MMS* backl=new MMS(3,(char*)"Backlight",NULL,procBackUpdate);
MMS* cool= new MMS(3,(char*)"Cooler",NULL,procCoolUpdate);

     root->add(keyer);
     root->add(speed);
     root->add(shift);
     root->add(drive);
     root->add(backl);
     root->add(cool);

MMS* straight=new MMS(0,(char*)"Straight",NULL,NULL);
MMS* iambicA =new MMS(1,(char*)"Iambic A",NULL,NULL);
MMS* iambicB =new MMS(2,(char*)"Iambic B",NULL,NULL);

     keyer->add(straight);
     keyer->add(iambicA);
     keyer->add(iambicB);

MMS* spval=new MMS(3,(char*)"*",NULL,NULL);

     speed->add(spval);
     speed->lowerLimit(1);
     speed->upperLimit(10);

MMS* stval=new MMS(1,(char*)"*",NULL,NULL);
     step->add(stval);
     step->lowerLimit(1);
     step->upperLimit(5);

MMS* shval=new MMS(6,(char*)"*",NULL,NULL);
     shift->add(shval);
     shift->lowerLimit(4);
     shift->upperLimit(8);

MMS* drval=new MMS(6,(char*)"*",NULL,NULL);
     drive->add(drval);
     drive->lowerLimit(0);
     drive->upperLimit(8);

MMS* backon=new MMS(0,(char*)"Backlight On",NULL,NULL);
MMS* backof=new MMS(1,(char*)"Backlight Off",NULL,NULL);
     backl->add(backon);
     backl->add(backof);

MMS* coolon=new MMS(0,(char*)"Cooler On",NULL,NULL);
MMS* coolof=new MMS(1,(char*)"Cooler Off",NULL,NULL);
     cool->add(coolon);
     cool->add(coolof);

     (TRACE>=0x02 ? root->list() : (void) _NOP );

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
