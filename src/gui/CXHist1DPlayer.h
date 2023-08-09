#ifndef CXHist1DPlayer_H
#define CXHist1DPlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TArrow.h"
#include "TObject.h"
#include "TGComboBox.h"
#include "TGNumberEntry.h"

using namespace std;

class CXMainWindow;
class TF1;
class TGraphErrors;
class TH1;
class TGCheckButton;
class TGListBox;
class TVirtualPad;
class CXArrow;
class CXHist1DPlayer;
class CXFit;
class CXBgdFit;

class CXHist1DPlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXHist1DPlayer");

public:
    TGCheckButton *fUseLT, *fUseRT;
    TGCheckButton *fFixFWHM, *fFixLT, *fFixRT, *fFixMean, *fFixAmpli;
    TGCheckButton *fBckStep, *fBckPol1, *fBckExp;
    TGCheckButton *fBckPol1_Bgd, *fBckExp_Bgd;
    TGNumberEntry *fNE_FWHM[3];
    TGNumberEntry *fNE_LT[3];
    TGNumberEntry *fNE_RT[3];
    TGTextEntry   *fFitOptions;

    Bool_t fUseStep = true;
    Bool_t fUsePol1 = false;
    Bool_t fUseExp = false;

    Bool_t fUseBgdPol1 = false;
    Bool_t fUseBgdExp = false;

    Bool_t DoNewFit = false;
    Bool_t DoNewBgdFit = false;

private:

    CXMainWindow *fMainWindow = nullptr;

    TGNumberEntry *fSigmaSPEntry = nullptr;
    TGNumberEntry *fThresholdSPEntry = nullptr;
    TGNumberEntry *fTextSize = nullptr;

    TGComboBox *fMinimizer = nullptr;
    TGComboBox *fAlgorithm = nullptr;
    TGNumberEntry *fTolenrance = nullptr;
    TGNumberEntry *fPrintLevel = nullptr;

    TList *fListOfFitObjects = nullptr;
    TList *fListOfBgdFitObjects = nullptr;

    vector<Float_t> fEnergies;
    vector<Float_t> fFitEnergies;
    vector<Float_t> fFitBackgd;

    TH1 *fCurrentHist = nullptr;

    TF1 *fFitFunction = nullptr;
    TF1 *fBackFunction = nullptr;
    TF1 *fResidue = nullptr;
    TList *fListOfPeakFunctions = nullptr;

    TGListBox *fFitResultsBox = nullptr;

    CXFit     *fCurrentFit = nullptr;
    CXBgdFit  *fCurrentBgdFit = nullptr;

public:
    CXHist1DPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXHist1DPlayer();

    Bool_t IsCurrentFit(){return DoNewFit;}

    void PeakSearch();
    void PeakSearchClear();
    void NewFit();
    void NewBgdFit();

    void EndFit(){DoNewFit = false;}
    void DoFit();
    void DoBgdFit();
    void ClearFits();
    void RemoveFit(CXFit *fit);
    void RemoveBgdFit(CXBgdFit *fit);

    void HandleMouse(Int_t EventType,Int_t EventX,Int_t EventY, TObject* selected);
    void HandleMyButton();

    TGListBox *GetFitResultsBox(){return fFitResultsBox;}
    void PrintInListBox(TString mess, Int_t Type);

    CXMainWindow *GetMainWindow(){return fMainWindow;}
    void SetMainWindow(CXMainWindow *w);

    void UpdateMinimizer();
    const char *GetMinimizer() {return fMinimizer->GetSelectedEntry()->GetTitle();}
    const char *GetAlgorithm() {return fAlgorithm->GetSelectedEntry()->GetTitle();}
    Double_t GetTolerance() {return fTolenrance->GetNumber();}
    Int_t GetPrintLevel() {return fPrintLevel->GetNumber();}

    ClassDef(CXHist1DPlayer,0);
};

#endif
