#ifndef CXNucleusBox_H
#define CXNucleusBox_H

#include "TBox.h"
#include "TString.h"
#include "TLatex.h"

#include "tkmanager.h"

class TLatex;

class CXNucleusBox : public TBox {
protected:
   Int_t fZ, fN;
   TString fName;
   TLatex *fText;
   TLatex *fTextLT = nullptr;

public:
   CXNucleusBox(shared_ptr<tkn::tknucleus> nuc, Double_t size = 0.5, Int_t colStable = kBlack, Int_t colRadio = kGray + 1, Int_t textsize=1, bool withlt=false, Int_t mode=0);

   CXNucleusBox(const CXNucleusBox&) ;
   virtual ~CXNucleusBox();
   void Copy(TObject&) const;

   TLatex *GetText(){return fText;}

   virtual const char *GetName() const {return fName.Data();}
   virtual const char *ClassName() const{return fName.Data();}

   virtual void  Draw(Option_t *option="");

   ClassDef(CXNucleusBox, 1)
};

#endif
