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

#ifndef CXGateBox_h
#define CXGateBox_h

#include "TBox.h"
#include "TString.h"

class TH1;
class TVirtualPad;

class CXGateBox : public TBox
{
public:

private:
    TString fName;
    TString fTitle;

    Float_t fWidth;
    Float_t fCentroid;

    Bool_t fIsBGD   = false;
    Bool_t fIsGate1 = false;
    Bool_t fIsGate2 = false;

    TVirtualPad *fAssociatedPad = nullptr;

    TH1 *fAssociatedHist = nullptr;

    Bool_t fIsButtonReleased = true;

public:
    CXGateBox(Float_t E, Float_t width, TVirtualPad *pad);
    ~CXGateBox(){;}

    void SetWidth(Float_t width){fWidth = width; UpdateBox();} // *MENU*
    Float_t GetWidth(){return fWidth;}

    void SetCentroid(Float_t centroid){fCentroid = centroid; UpdateBox();} // *MENU*
    Float_t GetCentroid(){return fCentroid;}

    void Update();
    void UpdateBox();

    void SetPad(TVirtualPad *pad){fAssociatedPad = pad;}
    TVirtualPad *GetPad(){return fAssociatedPad;}

    TH1 *GetHisto();

    Bool_t IsBGD(){return fIsBGD;}
    Bool_t IsGate1(){return fIsGate1;}
    Bool_t IsGate2(){return fIsGate2;}

    void SetBGD(); // *MENU*
    void SetGate1(); // *MENU*
    void SetGate2(); // *MENU*

    TBox *GetBox();

    void SetTitle(const char* title){fTitle = title;}
    void SetName(const char* title){fName = title;}

    virtual const char *GetName() const {return fName.Data();}
    virtual const char *ClassName() const{return fTitle.Data();}


    //Methodes bidon pour les enlever du ContextMenu
    virtual void        Delete(Option_t *option=""){TBox::Delete(option);}
    virtual void        DrawClass() const {TBox::DrawClass();}
    virtual TObject    *DrawClone(Option_t *option="") const {return TBox::DrawClone(option);}
    virtual TObject    *Clone(const char* newname=0) const;
    virtual void        Dump() const{TBox::Dump();}
    virtual void        Inspect() const {TBox::Inspect();}
    virtual void        SaveAs(const char *filename="",Option_t *option="") const {TBox::SaveAs(filename,option);}
    virtual void        SetDrawOption(Option_t *option=""){TBox::SetDrawOption(option);}

    virtual void        SetLineAttributes(){TBox::SetLineAttributes();}
    virtual void        SetFillAttributes(){TBox::SetFillAttributes();}

    virtual Int_t       DistancetoPrimitive(Int_t px, Int_t py);
    virtual void        ExecuteEvent(Int_t event, Int_t px, Int_t py);

    virtual void        ls(Option_t *option="") const;

private:

    ClassDef(CXGateBox,0)
};


#endif
