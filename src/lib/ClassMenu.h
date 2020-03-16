//*--------------------------------------------------------------------------------------------------
//* ClassMenu Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef MenuClass_h
#define MenuClass_h
//enum byte : std::uint8_t {};
#include "LinkedList.h"
#include <stdio.h>
#include <stdlib.h>

class MenuClass

{
  public:
  typedef void (*CALLBACK)();
  typedef struct {
         char* mText=(char*)"                    ";
         MenuClass* mChild;
         } List;

      
      unsigned char mItem;
      unsigned char mItemBackup;
      
      bool CW;
      bool CCW;
      bool init;
      List x;
      LinkedList<List*> l = LinkedList<List*>();      

      
      MenuClass(CALLBACK u);
      
      void  add(char* t,MenuClass* m);
      char* getText(unsigned char i);
      char* getCurrentText();
      void  set(unsigned char i);
      unsigned char  get();
      unsigned char  getBackup();
      void  refresh();
      
      void  setText(unsigned char i,char* c);
      bool  isUpdated();
      bool  last;


      
      MenuClass* getChild(unsigned char i);
      void  move(bool CW,bool CCW);
      void  backup();
      void  restore();
      void  save();
      //List* getmem();
      
      CALLBACK update;
      char* g=(char*)"                    ";

      
  private:    
      char gui[80];      
      
  
};

//*-------------------------------------------------------------------------------------------------
//* Constructor
//*-------------------------------------------------------------------------------------------------
MenuClass::MenuClass(CALLBACK u) {
   
   update=u;
 
   mItem=0;
   mItemBackup=0;
   CW=false;
   CCW=false;
   init=false;
   
   return;
}
//*-------------------------------------------------------------------------------------------------
//* Set current Index by force
//*-------------------------------------------------------------------------------------------------
void MenuClass::refresh() {
  if (update!=NULL){
     fprintf(stderr,"MenuClass::refresh()\n");
     update();
  }
  return;
}
//*----------------------------------------------------------------------------
//* Refresh content for actual value is dynamically created
//*---------------------------------------------------------------------------
void MenuClass::set(unsigned char i) {
  mItem=i; 
  if (update!=NULL){
    update();
  }
  return;
}
//*-------------------------------------------------------------------------------------------------
//* Add element to the menu, the child menu is optional
//*-------------------------------------------------------------------------------------------------
void MenuClass::add(char* t,MenuClass* m) {

     List* x = (List*)malloc(sizeof(List));
     init=true;
     x->mText=t;
     x->mChild=m;
     l.add(x);
}
//*-------------------------------------------------------------------------------------------------
//* Get the item pointer associated with the menu element
//*------------------------------------------------------------------------------------------------ 
unsigned char MenuClass::get() {
  return mItem;
}
//*-------------------------------------------------------------------------------------------------
//* Get the item pointer associated with the menu element
//*------------------------------------------------------------------------------------------------ 
unsigned char MenuClass::getBackup() {
  return mItemBackup;
}
//*-------------------------------------------------------------------------------------------------
//* Get the text associated with the ith menu element
//*------------------------------------------------------------------------------------------------ 
char* MenuClass::getText(unsigned char i) {
  return (char*)l.get(i)->mText;
}
//*-------------------------------------------------------------------------------------------------
//* Get the text associated with the ith menu element
//*------------------------------------------------------------------------------------------------ 
void MenuClass::setText(unsigned char i,char* c) {

  fprintf(stderr,"MenuClass::setText i=%d,char* %s\n",i,c);
  //free(l.get(i)->mText);
  //char* l.get(i)->mText=malloc(strlen(c)+1);
  //strcpy(l.get(i)->mText,c);
  l.get(i)->mText=c;
  return;
}

//*-------------------------------------------------------------------------------------------------
//* Get the pointer to the menu associated with the ith menu element (optional)
//*------------------------------------------------------------------------------------------------ 
MenuClass* MenuClass::getChild(unsigned char i) {
  return l.get(i)->mChild;
}
//*-------------------------------------------------------------------------------------------------
//* Advance or retry menu position, take care of borders
//*------------------------------------------------------------------------------------------------  
void MenuClass::move(bool cCW,bool cCCW){

  CW=cCW;
  CCW=cCCW;
    
  if (l.size() == 0) {return; }  //Is list empty? Return
  if (update!=NULL) {      
      update();
      init=true;
      return;
  }
  if (mItem < l.size()-1 && CW==true) {
     mItem++;
     return;
  }
  if (mItem>0 && CCW==true) {
     mItem--;
     return;
  }

  if (mItem ==l.size()-1 && CW==true) {
     mItem=0;
     return;
  }
  if (mItem==0 && CCW==true) {
     mItem=l.size()-1;
     return;
  }
  
} 
//*-------------------------------------------------------------------------------------------------
//* Backup current object pointer
//*------------------------------------------------------------------------------------------------       
char* MenuClass::getCurrentText(){

  if (update!=NULL){
    refresh();
    return getText(0);
  }
  return getText(mItem);
  
}
//*-------------------------------------------------------------------------------------------------
//* Backup current object pointer
//*------------------------------------------------------------------------------------------------       
void MenuClass::backup(){
  
  mItemBackup=mItem;
  
}
//*-------------------------------------------------------------------------------------------------
//* Restore previous object pointer (reverse a change)
//*------------------------------------------------------------------------------------------------       
void MenuClass::restore(){
  mItem=mItemBackup;
}
//*-------------------------------------------------------------------------------------------------
//* Restore previous object pointer (reverse a change)
//*------------------------------------------------------------------------------------------------       
void MenuClass::save(){
  
  mItemBackup=mItem;

 
}
//*-------------------------------------------------------------------------------------------------
//* Restore previous object pointer (reverse a change)
//*------------------------------------------------------------------------------------------------       
bool MenuClass::isUpdated(){
  unsigned char i=get();
  MenuClass* z=getChild(i);   
   if(z->mItem != z->mItemBackup) {
     return true;
   } else {
     return false;
   }
   return false;
  
}   
#endif


