//*--------------------------------------------------------------------------------------------------
//* VFOSystem VFO Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef VFOSystem_h
#define VFOSystem_h

//*--- Definition for VFO parameters and limits

#define VFOA    0
#define VFOB    1
#define VFOMAX  2

#define VFO_STEP_1Hz            1
#define VFO_STEP_10Hz          10
#define VFO_STEP_100Hz        100
#define VFO_STEP_500Hz        500
#define VFO_STEP_1KHz        1000
#define VFO_STEP_5KHz        5000
#define VFO_STEP_10KHz      10000
#define VFO_STEP_50KHz      50000
#define VFO_STEP_100KHz    100000
#define VFO_STEP_1MHz     1000000

typedef unsigned char byte;
typedef bool boolean;
typedef void (*CALLBACK)();

typedef struct {
  byte ones;
  byte tens;
  byte hundreds;
  byte thousands;
  byte tenthousands;
  byte hundredthousands;
  byte millions;
 
} FSTR;

//*============================================================================================
//* Define band limits
//*============================================================================================
//*---- HF band definition

#define BANDMAX 6


//*---------------------------------------------------------------------------------------------------
//* VFOSystem CLASS
//*---------------------------------------------------------------------------------------------------
class VFOSystem
{
  public: 
  
      VFOSystem(CALLBACK c,CALLBACK t,CALLBACK r,CALLBACK d);
      void setVFOdds(CALLBACK d);
      
      void set(unsigned char VFO,long int f);
      long int get(byte VFO);
      void setVFO(byte VFO);
      void setVFOFreq(byte VFO,long int fVFO);
      void setVFOShift(byte VFO,long int shiftVFO);
      void setVFOStep(byte VFO,long int stepVFO);
      void setVFOLimit(byte VFO,long int fMIN, long int fMAX);
      void setVFOBand(byte VFO,byte band);
      byte findBand(long int f);

  
      
      boolean isVFOChanged(byte VFO);
      boolean isVFOLocked();

      void resetVFO(byte VFO);
      //void resetVFOFreq(byte VFO);
      void computeVFO(long int f, FSTR* v);
      

      void getStr(byte VFO);
      void updateVFO(byte VFO, long int vstep);

      long int code2step(byte b);
      byte step2code(long int s);
      
      //long int getVFOFreq(byte VFO);
      //void tx(boolean s);
      
      long int vfo[VFOMAX];
      boolean VFOlock;
      long int VFOIFShift;
      
      
      long int vfoshift[VFOMAX];
      long int vfostep[VFOMAX];
      byte     vfoband[VFOMAX];
      
      long int vfomin[VFOMAX];
      long int vfomax[VFOMAX];
      byte     vforpt[VFOMAX];
      byte     vfoAB=VFOA;
      void     swapVFO();
      
      FSTR     vfostr[VFOMAX];
      
      CALLBACK changeVFO=NULL;
      CALLBACK changeTX=NULL;
      CALLBACK changeDDS=NULL;
      CALLBACK changeReset=NULL;
 

      long int loFreq[BANDMAX+1]={100,1800,3500,7000,14000,21000,28000};
      long int hiFreq[BANDMAX+1]={60000,2000,3800,7300,14350,21450,29700};
      //long int bandvfo[VFOMAX][BANDMAX+1];
      

  
  private:
      boolean _boot;
      boolean _tx;
      long int _rxa[VFOMAX];

};

#endif
//*---------------------------------------------------------------------------------------------------
//* VFOSystem CLASS Implementation
//*---------------------------------------------------------------------------------------------------
//*--------------------------------------------------------------------------------------------------
//* VFOSystem VFO Management Class   (CODE)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//*#include "VFOSystem.h"
VFOSystem::VFOSystem(CALLBACK c,CALLBACK t,CALLBACK r,CALLBACK d)
{
  changeVFO=NULL;
  _tx=false;
  VFOlock=false;
  VFOIFShift=0;
  
  if (c!=NULL) {changeVFO=c;}   //* Callback of change VFO frequency
  if (t!=NULL) {changeTX=t;}    //* Callback of TX mode change
  if (r!=NULL) {changeReset=r;} //* Callback of VFO Reset 
  if (d!=NULL) {changeDDS=d;}   //* Callback of DDS change
 
}
//*---------------------------------------------------------------------------------------------------
//* Create change frequency callback
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOdds(CALLBACK d) {
  
  if (d!=NULL) {changeDDS=d;}   //* Callback of DDS change

  return;
}
//*---------------------------------------------------------------------------------------------------
//* Given the frequency find the band
//*---------------------------------------------------------------------------------------------------
byte VFOSystem::findBand(long int f) {

 for (int i=1; i <= BANDMAX; i++){

   if ( (f>=loFreq[i]*1000) && (f<=hiFreq[i]*1000)){
      return i;
   }
 }

 return 0;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO step
//*---------------------------------------------------------------------------------------------------
long int VFOSystem::get(byte b){
  
  if ((b<VFOA) && (b>VFOB)) {
     return vfo[VFOA];
  }
 
  return vfo[b];   
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO step
//*---------------------------------------------------------------------------------------------------
long int VFOSystem::code2step(byte b) {

       switch(b) {
         case 0:                   {return VFO_STEP_1Hz;}
         case 1:                   {return VFO_STEP_10Hz;}
         case 2:                   {return VFO_STEP_100Hz;}
         case 3:                   {return VFO_STEP_1KHz;}
         case 4:                   {return VFO_STEP_10KHz;}
         case 5:                   {return VFO_STEP_100KHz;}
         default:                  {return VFO_STEP_1MHz;}
     }    
  return VFO_STEP_1KHz;
 
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO step
//*---------------------------------------------------------------------------------------------------
byte VFOSystem::step2code(long int s) {

       switch(s) {
         case 1:                   {return 0;}
         case 10:                  {return 1;}
         case 100:                 {return 2;}
         case 1000:                {return 3;}
         case 10000:               {return 4;}
         case 100000:              {return 5;}
         default:                  {return 6;}
     }    
  return 3;
 
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Shift
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOShift(byte VFO,long int shiftVFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}

  vfoshift[VFO]=shiftVFO;
  return;
 
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Step
//*---------------------------------------------------------------------------------------------------
void VFOSystem::swapVFO() {
   if (vfoAB==VFOA) {vfoAB=VFOB;} else {vfoAB=VFOA;}
   resetVFO(vfoAB);
   return;
}

//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Step
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOStep(byte VFO,long int stepVFO) {
   
   if (VFO<VFOA || VFO>VFOB) { return;}

   vfostep[VFO]=stepVFO;
   return;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Band
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOBand(byte VFO,byte band) {
   
   if (VFO<VFOA || VFO>VFOB) { return;}

   vfoband[VFO]=band;
   return;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Band
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOLimit(byte VFO,long int fMIN,long int fMAX) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}

  vfomin[VFO]=fMIN;
  vfomax[VFO]=fMAX;
  return;
  
}

//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Frequency
//*---------------------------------------------------------------------------------------------------
void VFOSystem::set(byte VFO,long int f) {
  char hi[80];
  
  if (VFO<VFOA || VFO>VFOB) { return;}

#if DEBUG

  sprintf(hi,"VFOSystem:set VFO(%d) f(%ld)",VFO,f);
  Serial.println(hi);

#endif  
  vfo[VFO]=f;
  _rxa[VFO]=f;
  
  if (changeVFO!=NULL) {
    changeVFO();}
  if (changeDDS!=NULL) {
     changeDDS();}
  getStr(VFO);
  resetVFO(VFO);
  
  return;
  
}
//*---------------------------------------------------------------------------------------------------
//* Return lock status for VFO
//*---------------------------------------------------------------------------------------------------
boolean VFOSystem::isVFOLocked(){

  return VFOlock;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Frequency
//*---------------------------------------------------------------------------------------------------
void VFOSystem::resetVFO(byte VFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  
  _rxa[VFO]=vfo[VFO];
  if (changeReset!=NULL) {changeReset();}
  
  return;
  
}

//*---------------------------------------------------------------------------------------------------
//* Set the focus VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFO(byte VFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  vfoAB = VFO;
  resetVFO(vfoAB);
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::getStr(byte VFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  
  long int fx=vfo[VFO];
  computeVFO(fx,&vfostr[vfoAB]);
  return;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::computeVFO(long int f, FSTR* v) {
  
  (*v).millions =       int(f / 1000000);
  (*v).hundredthousands = ((f / 100000) % 10);
  (*v).tenthousands =     ((f / 10000) % 10);
  (*v).thousands =        ((f / 1000) % 10);
  (*v).hundreds =         ((f / 100) % 10);
  (*v).tens =             ((f / 10) % 10);
  (*v).ones =             ((f / 1) % 10);
  return; 
   
}
//*---------------------------------------------------------------------------------------------------
//* check if the VFO had changed
//*---------------------------------------------------------------------------------------------------
boolean VFOSystem::isVFOChanged(byte VFO) {

  if (_rxa[VFO] == vfo[VFO]) {return false;}
  return true;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::updateVFO(byte VFO,long int vstep) {
   
   vfo[VFO]=vfo[VFO]+vstep;


#if DEBUG

   char hi[80];   
   sprintf(hi,"updateVFO VFO(%d) vfo=%ld rxa=%ld min=%ld max=%ld",VFO,vfo[VFO],_rxa[VFO],vfomin[VFO],vfomax[VFO]);
   Serial.println(hi);

#endif

   
   if (vfo[VFO] > vfomax[VFO]) {
       vfo[VFO] = vfomax[VFO];
   } // UPPER VFO LIMIT
       
   if (vfo[VFO] < vfomin[VFO]) {
       vfo[VFO] = vfomin[VFO];
   } // LOWER VFO LIMIT

   getStr(VFO);

   if (vfo[VFO]!=_rxa[VFO]) {
      resetVFO(VFO); 


      if (changeDDS!=NULL) {
         changeDDS();
      }
   
   }

   if (changeVFO!=NULL) {
         changeVFO();
   }
}
   


