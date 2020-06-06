//*--------------------------------------------------------------------------------------------------
//* MMS Menu Management System Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* this is part of the PixiePi platform
//* Only for ham radio usages
//* Copyright 2018,2020 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef MMS_h
#define MMS_h

class MMS
{
public:

typedef struct {
} menuList;
typedef void (*CALLCHANGE)(MMS* m);
typedef void (*CALLUPDATE)(MMS* m);

          MMS(int val,char* text,CALLCHANGE pChange,CALLUPDATE pUpdate);
    void  list();
    void  list(int n);
    void  add(MMS* child);
    MMS*  get();
    void  set(int m);
    void  set(char* t);
    void  lowerLimit(int m);
    void  upperLimit(int m);
    MMS*  move(int CW);
    void  setChild(int m);
    void  save();
    void  backup();
    void  restore();

         int   mVal;
         byte  TRACE=0x02;
         char  mText[32];
         int   bupmVal;
         char  bupmText[32];
         int   lastmove=0;
         int   mLower=-1;
         int   mUpper=-1;
         MMS*  bupcurr;
         MMS*  prev;
         MMS*  next;
         MMS*  child;
         MMS*  curr;
         MMS*  last;
         MMS*  parent;
    CALLCHANGE procChange;
    CALLUPDATE procUpdate;

         int   size=0;
//-------------------- GLOBAL VARIABLES ----------------------------
const char   *PROGRAMID="MMS";
const char   *PROG_VERSION="1.0";
const char   *PROG_BUILD="00";
const char   *COPYRIGHT="(c) LU7DID 2018,2020";

private:


};
#endif
//---------------------------------------------------------------------------------------------------
// MMS CLASS Implementation
// Master class to manage menues
//--------------------------------------------------------------------------------------------------

//*-------------------------------------------------------------------------------------------------
//* Constructor
//*-------------------------------------------------------------------------------------------------
MMS::MMS(int val,char* text,CALLCHANGE pChange,CALLUPDATE pUpdate) {


   this->mVal=val;
   this->procChange=(CALLCHANGE)pChange;
   this->procUpdate=(CALLUPDATE)pUpdate;

   this->parent=NULL;
   this->child=NULL;
   this->curr=NULL;
   this->last=NULL;
   this->bupcurr=NULL; 
   this->bupmVal=0;
   this->mLower=-1;
   this->mUpper=-1;

   strcpy(this->mText,text);
   strcpy(this->bupmText,"");

}
//*-------------------------------------------------------------------------------------------------
//* save - commit changes 
//*-------------------------------------------------------------------------------------------------
void MMS::save() {

   if (this->curr == NULL) {
      (TRACE>=0x02 ? fprintf(stderr,"%s::save() pointer is NULL, ignored\n",PROGRAMID) : _NOP);
       return;
   }
   if (this->curr->mVal != this->bupmVal || strcmp(this->curr->mText,this->bupmText) != 0 || this->curr != this->bupcurr) {
      (TRACE>=0x02 ? fprintf(stderr,"%s::save() conditions for update met mVal(%d) bupmVal(%d) mText(%s) bupmText(%s)\n",PROGRAMID,this->mVal,this->bupmVal,this->mText,this->bupmText) : _NOP);
      if (this->procUpdate != NULL) {
          this->procUpdate(this->curr);
          (TRACE>=0x02 ? fprintf(stderr,"%s::save() procUpdate(%s)\n",PROGRAMID,this->curr->mText) : _NOP);
      }
      this->backup();
   } else {
     (TRACE>=0x02 ? fprintf(stderr,"%s::save() procUpdate condition not met\n",PROGRAMID) : _NOP);
   }

   return;
}
//*-------------------------------------------------------------------------------------------------
//* lowerLimit - set lower limit for mVal variation
//*-------------------------------------------------------------------------------------------------
void MMS::lowerLimit(int m) {
     this->mLower=m;
     return;
}
//*-------------------------------------------------------------------------------------------------
//* upperLimit - set upper limit for mVal variation
//*-------------------------------------------------------------------------------------------------
void MMS::upperLimit(int m) {
     this->mUpper=m;
     return;
}
//*-------------------------------------------------------------------------------------------------
//* set the value of the child currently pointed if within lower/upper limites (if enabled)
//*-------------------------------------------------------------------------------------------------
void MMS::set(int m) {

   if (this->curr==NULL) {
      return;
   }

   if (mLower != -1 && m < mLower) {
      return;
   }

   if (mUpper != -1 && m > mUpper) {
      return;
   }

   this->curr->mVal=m;
   if (this->procChange!=NULL) {this->procChange(this->curr);}
   return;
}
//*-------------------------------------------------------------------------------------------------
//* set the text of the child currently pointed 
//*-------------------------------------------------------------------------------------------------
void MMS::set(char* t) {
   if (this->curr==NULL) {
      return;
   }
   strcpy(this->curr->mText,t);
   return;
}
void MMS::setChild(int m) {
   if (this->curr==NULL) {
      return;
   }
MMS* p=this->child;
  (TRACE>=0x02 ? fprintf(stderr,"%s::setChild() Pointing to value (%d)\n",PROGRAMID,m) : _NOP);
   while (p->next!=NULL) {
      (TRACE>=0x02 ? fprintf(stderr,"%s::setChild() current value(%d)\n",PROGRAMID,p->mVal) : _NOP);
       if (p->mVal == m) {
          this->curr=p;
         (TRACE>=0x02 ? fprintf(stderr,"%s::setChild() pointer set(%d)\n",PROGRAMID,p->mVal) : _NOP);
          return;
       }
       p=p->next;
   }

}
//*-------------------------------------------------------------------------------------------------
//* add a child beneath a parent, link both and place child at the end of the current list
//*-------------------------------------------------------------------------------------------------
void MMS::add(MMS* child) {

   if (child==NULL) return;

   MMS* parent=this;

   if (parent->child==NULL) {
      parent->child=child;
      parent->curr=child;
      parent->last=child;
      child->parent=parent;
      child->TRACE=parent->TRACE;
      child->next=NULL;
      child->prev=NULL;
      child->last=NULL;
      if(parent->procChange!=NULL) {parent->procChange(child);}
      (TRACE>=0x03 ? fprintf(stderr,"%s::add() tracelevel(%d)\n",PROGRAMID,TRACE) : _NOP);
      return;
   }

   MMS* z=parent->child;
   MMS* p=NULL;
   while(z!=NULL) {
     p=z;
     z=z->next;
   }

   if (p!=NULL) {
      p->next=child;
      child->prev=p;
   }
   child->next=NULL;
   child->parent=parent;
   child->TRACE=parent->TRACE;
   parent->last=child;
   if(parent->procChange!=NULL) {parent->procChange(child);}
   (TRACE>=0x02 ? fprintf(stderr,"%s::constructor() tracelevel(%d)\n",PROGRAMID,TRACE) : _NOP);

}
//*-------------------------------------------------------------------------------------------------
//* get a pointer to the current child
//*-------------------------------------------------------------------------------------------------
// ---------

MMS* MMS::get() {
   return this->curr;
}
//*-------------------------------------------------------------------------------------------------
//* move - advance the child by one. If a change handler is defined mVal is increased and the handler
//* is caller thereafter. If a handler isn't present then the childs are forward or backward traversed
//*-------------------------------------------------------------------------------------------------
MMS* MMS::move(int CW) {

   if (this->curr==NULL) {
      return this->curr;
   }

   if (this->procChange!=NULL) {
      if (this->curr!=NULL) {
         if (this->mLower != -1 && this->curr->mVal+CW < this->mLower) {
            return this->curr;
         }

         if (this->mUpper != -1 && this->curr->mVal+CW > this->mUpper) {
            return this->curr;
         }
         this->curr->mVal=this->curr->mVal+CW;
         this->procChange(this->curr);

      }
      return this->curr;
   }

   if (CW>0) {
      MMS* z=this->curr;
      if (z!=NULL) {
         this->curr=this->curr->next;
         if (this->curr==NULL) {
            this->curr=this->child;
         }
      } else {
        this->curr=this->child;
      }
      return this->curr;
   }

   if (CW<0) {
      MMS* z=this->curr;
      if (z!=NULL) {
         this->curr=this->curr->prev;
         if (this->curr==NULL) {
            this->curr=this->last;
         }
      } else {
        return this->curr;
      }
   }

   return this;

}
// --------
//*-------------------------------------------------------------------------------------------------
//* backup take a checkpoint of the current situation (mostly the current child situation)
//*-------------------------------------------------------------------------------------------------

void MMS::backup() {

    if (this->curr!=NULL) {
       this->bupmVal=this->curr->mVal;
       strcpy(this->bupmText,this->curr->mText);
       this->bupcurr=this->curr;
    }
    return;
}
// --------
//*-------------------------------------------------------------------------------------------------
//* restore to the previous checkpoint
//*-------------------------------------------------------------------------------------------------
void MMS::restore() {

    if (this->curr!=NULL) {
       this->curr->mVal=this->bupmVal;
       strcpy(this->curr->mText,this->bupmText);
       this->curr=this->bupcurr;
    }
    return;

}
// --------
//*-------------------------------------------------------------------------------------------------
//* List the whole tree starting from a node
//*-------------------------------------------------------------------------------------------------
void MMS::list(int n) {

  MMS* p=this;
  while (p!=NULL) {
     fprintf(stderr,"<%d> %*s Val(%d)--Text(%s) First(%s) Current Child(%s) Lower(%d) Upper(%d) Last(%s)\n",n, n, "", p->mVal,p->mText,(p->child==NULL ? "Nil" : p->child->mText),(p->curr==NULL ? "Nil" : p->curr->mText),p->mLower,p->mUpper,(p->last==NULL ? "Nil" : p->last->mText));
     MMS* z=p->child;
     if (z!=NULL) {
        z->list(n+1);
        if (n==1) {return;}
     }
     p=p->next;
  }

}
// -------
void MMS::list() {

  this->list(1);

}
