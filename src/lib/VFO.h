//*=====================================================================================================================
//* Manage transceiver functionality and link between VFO, CAT, GUI  and DDS subsystems
//*=====================================================================================================================
void freqVfoHandler(float f) {  // handler to receiver VFO upcalls for frequency changes

char* b;

   if (vfo==nullptr) {
      return;
   }
   b=(char*)malloc(128);

   (TRACE>=0x02 ? fprintf(stderr,"%s:freqVfoHandler() calling vfo2str\n",PROGRAMID) : _NOP);
   vfo->vfo2str(vfo->vfo,b);

   (TRACE>=0x02 ? fprintf(stderr,"%s:freqVfoHandler() VFO(%s) f(%5.0f) fA(%5.0f) fB(%5.0f) PTT(%s)\n",PROGRAMID,b,f,vfo->get(VFOA),vfo->get(VFOB),BOOL2CHAR(getWord(vfo->FT817,PTT))) : _NOP);

}
