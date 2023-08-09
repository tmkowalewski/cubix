#ifndef CXRadCubeTH1Proj_H
#define CXRadCubeTH1Proj_H

#include "RQ_OBJECT.h"
#include "KeySymbols.h"
#include "TH1.h"

#include "CXRadReader.h"

using namespace std;

class TH2;
class TPad;
class TVirtualPad;
class CXMainWindow;
class CXGateBox;
class TBox;
class CXRad2DPlayer;
class CXRadCubePlayer;


class CXRadCubeTH1Proj : public  TH1D
{
    RQ_OBJECT("CXRadCubeTH1Proj");

public:
    CXRadReader *fRadReader = nullptr;
    TPad *fProjPad = nullptr;
    TPad *fCurrentPad = nullptr;

private:

    CXMainWindow *fMainWindow = nullptr;

    Bool_t fAddNewGate1 = false;
    Bool_t fAddNewGate2 = false;

    Bool_t   moved = false;
    Double_t oldx, oldy;
    Double_t xinit, yinit;
    Double_t xmin, xmax, ymin, ymax;

    TList *fListOfGates = nullptr;

    TBox *fGateVirtualBox = nullptr;

    CXRadCubePlayer *fRadCubePlayer = nullptr;
    CXRad2DPlayer   *fRad2DPlayer = nullptr;

    Float_t fLenghtFactor = 1.;
    Float_t fWidth        = 6.;
    Float_t fFWHM_0       = 9.;
    Float_t fFWHM_1       = 0.004;
    Float_t fFWHM_2       = 0.;

    TString fMode;

    EKeySym fLastSym;

public:
    CXRadCubeTH1Proj(CXRadReader *radreader);
    CXRadCubeTH1Proj(TH2 *hist);
    CXRadCubeTH1Proj();

    ~CXRadCubeTH1Proj();

    void SetMainWindow(CXMainWindow *w);

    void SetProjPad(TPad *pad);
    void SetCurrentPad(TPad *pad);
    void SetCubePlayer(CXRadCubePlayer *player);
    void Set2DPlayer(CXRad2DPlayer *player);
    void HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected);
    void Project(Bool_t FixRange=false, Bool_t BGSubtract = true, Int_t Rebin=1);

    void AddGate1(){fAddNewGate1 = true;}
    void AddGate1(Float_t TheMean, Float_t TheWidth); //*MENU*
    void AddGate2(){fAddNewGate2 = true;}
    void AddGate2(Float_t Mean, Float_t Width); //*MENU*
    void ClearGates();
    TList *GetGatesList(){return fListOfGates;}
    void UpdateGates();
    void RemoveGate(CXGateBox *box);

    TH1 *GetTotalProj(){return fRadReader->GetTotalProjection();}
    TH1 *GetBackground(){return fRadReader->GetBackground();}

    Float_t GetLenghtFactor() { return fLenghtFactor; }
    Float_t GetWidth() { return fWidth; }
    Float_t GetFWHM_0() { return fFWHM_0; }
    Float_t GetFWHM_1() { return fFWHM_1; }
    Float_t GetFWHM_2() { return fFWHM_2; }
    void SetBackground(Float_t LenghtFactor, Float_t Width, Float_t FWHM_0, Float_t FWHM_1, Float_t FWHM_2); //*MENU* *ARGS={LenghtFactor=>fLenghtFactor,Width=>fWidth,FWHM_0=>fFWHM_0,FWHM_1=>fFWHM_1,FWHM_2=>fFWHM_2}
    void SaveBackground(); //*MENU*

    TObject*         Clone(const char* newname=0) const {return fRadReader->GetTotalProjection()->Clone(newname);}

    ClassDef(CXRadCubeTH1Proj,0);
};

#endif
