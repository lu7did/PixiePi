//*=====================================================================================================================
//* Manage transceiver functionality and link between VFO, CAT, GUI  and DDS subsystems
//*=====================================================================================================================
void freqVfoHandler(float f) {  // handler to receiver VFO upcalls for frequency changes

char* b;

   if (vfo==nullptr) {
      return;
   }
   b=(char*)malloc(128);
   vfo->vfo2str(vfo->vfo,b);
   showFrequency();
   showChange();
   if (cat!=nullptr) {
      cat->f=vfo->get(vfo->vfo);
   }
   (TRACE>=0x02 ? fprintf(stderr,"%s:freqVfoHandler() VFO(%s) f(%5.0f) fA(%5.0f) fB(%5.0f) PTT(%s)\n",PROGRAMID,b,f,vfo->get(VFOA),vfo->get(VFOB),BOOL2CHAR(getWord(vfo->FT817,PTT))) : _NOP);

}
void ritVfoHandler(float r) {
char* b;

   if (vfo==nullptr) {
      return;
   }

   b=(char*)malloc(128);
   (TRACE>=0x02 ? fprintf(stderr,"%s:ritVfoHandler() VFO(%s) RIT(%5.0f) PTT(%s)\n",PROGRAMID,b,vfo->valueRIT(),BOOL2CHAR( getWord(vfo->FT817,PTT) ) ) : _NOP);
   if (cat!=nullptr) {
      cat->RITOFS=r;
   }
   showRIT();
}

void changeVfoHandler(byte S) {

   if (getWord(S,SPLIT)==true) {
      (TRACE>=0x02 ? fprintf(stderr,"%s:changeVfoHandler() change SPLIT S(%s) On\n",PROGRAMID,BOOL2CHAR(getWord(vfo->FT817,SPLIT))) : _NOP);
      if (cat!=nullptr) {
         setWord(&cat->FT817,SPLIT,getWord(vfo->FT817,SPLIT));
      }
      showVFO();
      showChange();
   }
   if (getWord(S,RITX)==true) {
      (TRACE>=0x02 ? fprintf(stderr,"%s:changeVfoHandler() change RIT S(%s) On\n",PROGRAMID,BOOL2CHAR(getWord(vfo->FT817,RITX))) : _NOP);
      if (cat!=nullptr) {
         setWord(&cat->FT817,RITX,getWord(vfo->FT817,RITX));
      }
      showRIT();
   }
   if (getWord(S,PTT)==true) {
      (TRACE>=0x02 ? fprintf(stderr,"%s:changeVfoHandler() change PTT S(%s) On\n",PROGRAMID,BOOL2CHAR(getWord(vfo->FT817,PTT))) : _NOP);
      if (cat!=nullptr) {
         setWord(&cat->FT817,PTT,getWord(vfo->FT817,PTT));
      }
      showPTT();
   }
   if (getWord(S,VFO)==true) {
      (TRACE>=0x02 ? fprintf(stderr,"%s:changeVfoHandler() change VFO S(%s) On\n",PROGRAMID,BOOL2CHAR(getWord(vfo->FT817,VFO))) : _NOP);
      if (cat!=nullptr) {
         setWord(&cat->FT817,VFO,getWord(vfo->FT817,VFO));
      }
      showVFO();
      showChange();
   }

}

void modeVfoHandler(byte m) {

char* b;

   b=(char*)malloc(128);
   vfo->code2mode(m,b);
   if (cat!=nullptr) {
      cat->MODE=vfo->MODE;
   }
   (TRACE>=0x02 ? fprintf(stderr,"%s:modeVfoHandler() mode(%s) On\n",PROGRAMID,b) : _NOP);

}
//---------------------------------------------------------------------------
// CATchangeFreq()
// CAT Callback when frequency changes
//---------------------------------------------------------------------------
void CATchangeFreq() {

   if (cat==nullptr) {return;}

   if (getWord(MSW,PTT)==false) {
     (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeFreq() cat.SetFrequency(%d) request while transmitting, ignored!\n",PROGRAMID,(int)cat->f) : _NOP);
     vfo->set(cat->f);
     return;
   }

  cat->f=vfo->get(); 
  //cat->f=f;
  (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeFreq() Frequency change is not allowed(%d)\n",PROGRAMID,(int)f) : _NOP);

}
//-----------------------------------------------------------------------------------------------------------
// CATchangeMode
// Validate the new mode is a supported one
// At this point only CW,CWR,USB and LSB are supported
//-----------------------------------------------------------------------------------------------------------
void CATchangeMode() {

  (TRACE>=0x02 ? fprintf(stderr,"%s:CATchangeMode() requested MODE(%d) not supported\n",PROGRAMID,cat->MODE) : _NOP);

  vfo->MODE=cat->MODE;
  return;

}
//------------------------------------------------------------------------------------------------------------
// CATchangeStatus
// Detect which change has been produced and operate accordingly
//------------------------------------------------------------------------------------------------------------
void CATchangeStatus() {

  (TRACE >= 0x03 ? fprintf(stderr,"%s:CATchangeStatus() FT817(%d) cat.FT817(%d)\n",PROGRAMID,vfo->FT817,cat->FT817) : _NOP);

  if (getWord(cat->FT817,PTT) != getWord(vfo->FT817,PTT)) {
     (TRACE>=0x02 ? fprintf(stderr,"%s:CATchangeStatus() PTT change request cat.FT817(%d) now is PTT(%s)\n",PROGRAMID,cat->FT817,BOOL2CHAR(getWord(cat->FT817,PTT))) : _NOP);
     vfo->setPTT(getWord(cat->FT817,PTT));
  }

  if (getWord(cat->FT817,RITX) != getWord(vfo->FT817,RITX)) {        // RIT Changed
     (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeStatus() RIT change request cat.FT817(%d) RIT(%s)\n",PROGRAMID,cat->FT817,BOOL2CHAR(getWord(cat->FT817,RITX))) : _NOP); 
     vfo->setRIT(getWord(cat->FT817,RITX));
  }

  if (getWord(cat->FT817,LOCK) != getWord(vfo->FT817,LOCK)) {      // LOCK Changed
     (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeStatus() LOCK change request cat.FT817(%d) LOCK(%s)\n",PROGRAMID,cat->FT817,BOOL2CHAR(getWord(cat->FT817,LOCK))) : _NOP);
     vfo->setLock(getWord(cat->FT817,LOCK));
  }

  if (getWord(cat->FT817,SPLIT) != getWord(vfo->FT817,SPLIT)) {    // SPLIT mode Changed
     (TRACE>=0x01 ? fprintf(stderr,"%s:CATchangeStatus() SPLIT change request cat.FT817(%d) SPLIT(%s)\n",PROGRAMID,cat->FT817,BOOL2CHAR(getWord(cat->FT817,SPLIT))) : _NOP);
     vfo->setSplit(getWord(cat->FT817,SPLIT));
  }

  if (getWord(cat->FT817,VFO) != getWord(vfo->FT817,VFO)) {        // VFO Changed
     (getWord(cat->FT817,VFO)==0 ? vfo->setVFO(VFOA) : vfo->setVFO(VFOB));
     (TRACE >=0x01 ? fprintf(stderr,"%s:CATchangeStatus() VFO change request now (%d)\n",PROGRAMID,vfo->vfo) : _NOP);
  }
  //vfo->FT817=cat->FT817;
  return;

}
//--------------------------------------------------------------------------------------------------
// Callback to process SNR signal (if available)
//--------------------------------------------------------------------------------------------------
void CATgetRX() {

    //cat->RX=cat->snr2code(SNR);

}
//--------------------------------------------------------------------------------------------------
// Callback to process Power/SWR signal (if available)
//--------------------------------------------------------------------------------------------------
void CATgetTX() {

}

