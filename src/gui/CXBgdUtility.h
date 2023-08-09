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
