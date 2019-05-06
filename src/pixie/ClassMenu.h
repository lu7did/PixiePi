
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
         char* mText;
         MenuClass* mChild;
         } List;

      
      unsigned char mItem;
      unsigned char mItemBackup;
      
      bool CW;
      bool CCW;
      List x;
      LinkedList<List*> l = LinkedList<List*>();      

      
      MenuClass(CALLBACK u);
      
      void  add(char* t,MenuClass* m);
      char* getText(unsigned char i);
      char* getCurrentText();
      void  set(unsigned char i);
      unsigned char  get();
      unsigned char  getBackup();
      
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

      
  private:    
      
      
  
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
   
   return;
}
//*-------------------------------------------------------------------------------------------------
//* Set current Index by force
//*-------------------------------------------------------------------------------------------------

void MenuClass::set(unsigned char i) {
  mItem=i; 
  if (update!=NULL){
    //printf("DEBUG MenuClass: Method: set parm=%d\n",i);
    update();
  }
  return;
}
//*-------------------------------------------------------------------------------------------------
//* Add element to the menu, the child menu is optional
//*-------------------------------------------------------------------------------------------------
void MenuClass::add(char* t,MenuClass* m) {

     List* x = (List*)malloc(sizeof(List));
     
     x->mText=t;
     x->mChild=m;
     
     l.add(x);
}
//*-------------------------------------------------------------------------------------------------
//* Get the item pointer associated with the menu element
//*------------------------------------------------------------------------------------------------ 
unsigned char MenuClass::get() {
  //printf("DEBUG MenuClass: Method: get parm=%d\n",mItem);
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
  //printf("DEBUG MenuClass: Method: getText parm=%d returned text:%s\n",i,(char*)l.get(i)->mText);
  return (char*)l.get(i)->mText;
}
//*-------------------------------------------------------------------------------------------------
//* Get the text associated with the ith menu element
//*------------------------------------------------------------------------------------------------ 
void MenuClass::setText(unsigned char i,char* c) {
  
  //printf("DEBUG MenuClass: Method: setText parm=%d provided:%s\n",i,c);

  l.get(i)->mText=c;
  return;
}

//*-------------------------------------------------------------------------------------------------
//* Get the pointer to the menu associated with the ith menu element (optional)
//*------------------------------------------------------------------------------------------------ 
MenuClass* MenuClass::getChild(unsigned char i) {
  //printf("DEBUG MenuClass: Method: getChild parm=%d\n",i);

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
      //if (display!=NULL) {display();}
      return;
  }
  if (mItem < l.size()-1 && CW==true) {
     mItem++;
     //if (display!=NULL) {display();}
     return;
  }
  if (mItem>0 && CCW==true) {
     //if (display!=NULL) {display();}
     mItem--;
     return;
  }

  if (mItem ==l.size()-1 && CW==true) {
     //if (display!=NULL) {display();}
     mItem=0;
     return;
  }
  if (mItem==0 && CCW==true) {
     //if (display!=NULL) {display();}   
     mItem=l.size()-1;
     return;
  }
  
} 
//*-------------------------------------------------------------------------------------------------
//* Backup current object pointer
//*------------------------------------------------------------------------------------------------       
char* MenuClass::getCurrentText(){

  if (update!=NULL){
    //printf("DEBUG MenuClass: Method: getCurrentText parm=(0) returned text:%s\n",(char*)getText(0));

    //update();
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


