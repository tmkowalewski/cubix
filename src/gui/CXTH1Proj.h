#ifndef CXTH1Proj_H
#define CXTH1Proj_H

#include "RQ_OBJECT.h"
#include "TH1.h"
#include "KeySymbols.h"

using namespace std;

class TH2;
class TPad;
class TVirtualPad;
class CXMainWindow;
class CXGateBox;
class TBox;
class CXHist2DPlayer;

class CXTH1Proj : public  TH1D
{
    RQ_OBJECT("CXTH1Proj");

public:
    TH2 *fGGHist = nullptr;
    TPad *fProjPad = nullptr;
    TPad *fCurrentPad = nullptr;

private:

    CXMainWindow *fMainWindow = nullptr;

    Bool_t fAddNewGate = false;
    Bool_t fAddNewBacground = false;

    Bool_t   moved = false;
    Double_t oldx{}, oldy{};
    Double_t xinit{}, yinit{};
    Double_t xmin{}, xmax{}, ymin{}, ymax{};

    TList *fListOfGates = nullptr;

    TBox *fGateVirtualBox = nullptr;

    CXHist2DPlayer *f2DPlayer = nullptr;

    Int_t fProjectionAxis = 0;

    EKeySym fLastSym;

public:
    CXTH1Proj(const TH1D &hist);
    CXTH1Proj();
    ~CXTH1Proj();

    void SetMainWindow(CXMainWindow *w);

    void UpdateProjection(Int_t Axis);
    void SetTH2(TH2 *hist);
    void SetProjPad(TPad *pad);
    void SetCurrentPad(TPad *pad);
    void SetPlayer(CXHist2DPlayer *player);
    void HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected);

    void Project(Bool_t FixRange=false, int _rebin_value=1);
    void AddBackgd(){fAddNewBacground = true;}
    void AddBackgd(Float_t Mean, Float_t Width); //*MENU*
    void AddGate(){fAddNewGate = true;}
    void AddGate1(Float_t Mean, Float_t Width); //*MENU*
    void ClearGates();
    TList *GetGatesList(){return fListOfGates;}
    void UpdateGates();
    void RemoveGate(CXGateBox *box);

private:

    ClassDef(CXTH1Proj,0);
};

#endif
