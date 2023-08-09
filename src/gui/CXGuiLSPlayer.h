#ifndef CXGuiLSPlayer_H
#define CXGuiLSPlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"

#include "tkmanager.h"

using namespace std;

class TGCheckButton;
class CXLevelSchemePlayer;
class CXMainWindow;
class TGComboBox;

class CXGuiLSPlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXGuiLSPlayer");

public:

private:

    CXMainWindow *fMainWindow = nullptr;

    TGTextEntry *fListOfNucleus = nullptr;

    TGComboBox *fDataSetMode = nullptr;

    Bool_t fYrastMode;

    TGCheckButton *fYrastButton;

    TGNumberEntry *fTextSize;
    TGCheckButton *fFullGammaTitle;
    TGNumberEntry *fBranchingRatio[2];
    TGCheckButton *fUseBranchingRatio;
    TGNumberEntry *fELevel[2];
    TGCheckButton *fUseELevels;
    TGNumberEntry *fSpins[2];
    TGCheckButton *fUseSpins;
    TGNumberEntry *fLifeTime[2];
    TGCheckButton *fUseLifeTime;
    TGComboBox    *fLifeTimeScale[2];

    TGCheckButton *fArrowMode;

    CXLevelSchemePlayer *fLSPlayer = nullptr;

    Bool_t fNucleiAreKnown = false;
    Int_t fNumberOfNuclei = 0;

    shared_ptr<tkn::tklevel_scheme> fSelectedLevelScheme;
    TString fCurrentDataSet = "ADOPTED LEVELS, GAMMAS";
    Int_t fSelectedEntry=0;
    Bool_t fNoUpdateDataSet = false;

public:
    CXGuiLSPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXGuiLSPlayer();

    void SetMainWindow(CXMainWindow *w);

    void ManuallyAddNucleus();
    void CheckListOfNuclei();
    void UpdateGammaRays();

    void GetBranchingRatio(Int_t &min, Int_t &max);
    void GetELevels(Float_t &min, Float_t &max);
    void GetSpins(Int_t &min, Int_t &max);
    void GetLifeTime(Float_t &min, Float_t &max);

    Bool_t GetColorMode(){return fArrowMode->GetState();}

    Bool_t GetYrastMode(){return fYrastMode;}

    Float_t GetTextSize(){return fTextSize->GetNumber();}
    Bool_t IsFullTitleMode(){return fFullGammaTitle->GetState();}
    Bool_t UseBranchingRatio(){return fUseBranchingRatio->GetState();}
    Bool_t UseELevels(){return fUseELevels->GetState();}
    Bool_t UseLifeTime(){return fUseLifeTime->GetState();}
    Bool_t UseSpins(){return fUseSpins->GetState();}

    void PlotLS();

    CXLevelSchemePlayer *GetPlayer() {return fLSPlayer;}

    void UpdateDataSet();

protected:

private:

    ClassDef(CXGuiLSPlayer,0);
};

#endif
