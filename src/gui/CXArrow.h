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

#ifndef CXArrow_H
#define CXArrow_H

#include "TArrow.h"
#include "TBox.h"

class CXFit;
class CXBgdFit;

class TLatex;
class TBox;
class TH1;
class TVirtualPad;

class CXArrowBox;

class CXArrow : public  TArrow
{

private:
    CXFit *fFit = nullptr;
    CXBgdFit *fBgdFit = nullptr;

    TLatex *fLatex = nullptr;
    CXArrowBox *fBox = nullptr;

    Float_t fTextSize = 0.03;

public:

    CXArrow(CXFit *fit, Double_t E,Double_t y1 ,Double_t y2,Float_t arrowsize=0.05, Float_t textsize = 0.03, Option_t *option=">");
    CXArrow(CXBgdFit *fit, Double_t E,Double_t y1 ,Double_t y2,Float_t arrowsize=0.05, Float_t textsize = 0.03, Option_t *option=">");
    CXArrow(const CXArrow &other);  // Copy constructor
    CXArrow &operator=(const CXArrow &other); // Copy assignment operator
    virtual ~CXArrow();

    virtual TObject* Clone(const char* newname = "") const override;

    CXFit *GetFit(){return fFit;}
    CXBgdFit *GetBgdFit(){return fBgdFit;}

    //! Energy
    void SetEnergy(Float_t E); // *MENU* *ARGS={E=>fX1}*
    Double_t GetEnergy() const {return fX1;}

    void Set(Double_t X, Double_t Y1, Double_t Y2);

    void RemoveArrow(); // *MENU*
    void RemoveFit(); // *MENU*

    void SetText(TH1 *hist, const TString &text, const TString &tooltip);

    void ClearPad(TVirtualPad *pad = nullptr, bool refresh = true);

    virtual void Paint(Option_t *option = "") override;

    //! Sort
    virtual Bool_t  IsSortable() const override {return kTRUE;}
    virtual Int_t Compare(const TObject *obj) const override;

    ClassDefOverride(CXArrow,0);
};

class CXArrowBox : public  TBox
{
private:
    CXArrow *fArrow = nullptr;

public:
    CXArrowBox(CXArrow *arrow) : TBox() {fArrow = arrow;}
    ~CXArrowBox(){;}

    void Clean(){if(fArrow) {fArrow->ClearPad(); delete fArrow; fArrow = nullptr;}}

    ClassDefOverride(CXArrowBox,0);
};

#endif
