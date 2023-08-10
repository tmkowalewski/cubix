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

#ifndef CXBgdUtility_H
#define CXBgdUtility_H

#include "TGFrame.h"
#include "TGButton.h"
#include "RQ_OBJECT.h"

#include "CXMainWindow.h"
#include <array>

class TH2;
class TGLabel;
class TGNumberEntry;
class CXBgdUtility : public  TGVerticalFrame
{
    RQ_OBJECT("CXBgdUtility");

public:

protected:

private:

    CXMainWindow      *fMainWindow{};

    TGLabel *fLabel;
    TGGroupFrame *fGroupFrame;
    TGCompositeFrame *fHorizontalFrame;

    TGCheckButton *fActivate;
    TGCheckButton *fDoSmoothing;
    TGCheckButton *fDoCompton;

    int fNumberIterations1D;
    int fNumberIterations2DX;
    int fNumberIterations2DY;

    int fDirection1D;
    int fDirection2D;

    int fFilterOrder1D;
    int fFilterOrder2D;
    bool fSmoothing;
    int fSmoothingWindow;
    bool fCompton;

    TGNumberEntry *fNItersButton1D;
    TGNumberEntry *fNItersButton2D[2]={};

    TGRadioButton *fDirectionButton1D[2]={};

    array<TGRadioButton*,4> fFilterOrderButton1D={};
    array<TGRadioButton*,7> fSmoothingButton1D={};

    TGTextButton *fSubtractButton;
    TGTextButton *f2DEvalButton;

    TGRadioButton *fDirectionButton2D[2]={};
    TGRadioButton *fFilterOrderButton2D[2]={};

    TList *fListOfButtons1D = nullptr;


    TH2 *fCurrent2DHist = nullptr;
    TH2 *fCurrent2DBackd = nullptr;
    TH2 *fCurrent2DSubtract = nullptr;

    ULong_t red={};
    ULong_t blue={};
    ULong_t black={};

public:
    CXBgdUtility(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
        ~CXBgdUtility();

    Bool_t IsActivated(){return fActivate->GetState();}
    void DoActivate(){fActivate->SetState(EButtonState::kButtonDown,true);}

    void SetMainWindow(CXMainWindow *w){fMainWindow = w;}

    void ToggleBckSupp();
    void HandleWindowButtons(Int_t id=-1);
    void HandleFilterButtons(Int_t id=-1);
    void HandleSmoothButtons(Int_t id=-1);
    void GetParams();
    void DoSubtract();

    void Do2DEvaluation(TH1 *hist_in=nullptr);

protected:

private:

    void SetButtonsStatus(bool on);

    ClassDef(CXBgdUtility,0)
};

#endif
