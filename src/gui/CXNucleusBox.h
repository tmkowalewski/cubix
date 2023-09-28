/********************************************************************************
 *   Copyright (c) : Université de Lyon 1, CNRS/IN2P3, UMR5822,                 *
 *                   IP2I, F-69622 Villeurbanne Cedex, France                   *
 *   Contibutor(s) :                                                            *
 *      Jérémie Dudouet jeremie.dudouet@cnrs.fr [2023]                          *
 *                                                                              *
 *    This software is governed by the CeCILL-B license under French law and    *
 *    abiding by the  rules of distribution of free  software.  You can use,    *
 *    modify  and/ or  redistribute  the  software under  the  terms of  the    *
 *    CeCILL-B license as circulated by CEA, CNRS and INRIA at the following    *
 *    URL \"http://www.cecill.info\".                                           *
 *                                                                              *
 *    As a counterpart to the access  to the source code and rights to copy,    *
 *    modify  and redistribute granted  by the  license, users  are provided    *
 *    only with a limited warranty  and the software's author, the holder of    *
 *    the economic  rights, and the  successive licensors have  only limited    *
 *    liability.                                                                *
 *                                                                              *
 *    In this respect, the user's attention is drawn to the risks associated    *
 *    with loading,  using, modifying  and/or developing or  reproducing the    *
 *    software by the user in light of its specific status of free software,    *
 *    that  may mean that  it is  complicated to  manipulate, and  that also    *
 *    therefore  means that it  is reserved  for developers  and experienced    *
 *    professionals having in-depth  computer knowledge. Users are therefore    *
 *    encouraged  to load  and test  the software's  suitability  as regards    *
 *    their  requirements  in  conditions  enabling the  security  of  their    *
 *    systems  and/or data to  be ensured  and, more  generally, to  use and    *
 *    operate it in the same conditions as regards security.                    *
 *                                                                              *
 *    The fact that  you are presently reading this means  that you have had    *
 *    knowledge of the CeCILL-B license and that you accept its terms.          *
 ********************************************************************************/

#ifndef CXNucleusBox_H
#define CXNucleusBox_H

#include "TBox.h"
#include "TString.h"
#include "TLatex.h"

#include "tknucleus.h"

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
