#ifndef CXHist1DCalib_H
#define CXHist1DCalib_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TArrow.h"
#include "TObject.h"

#include "CXRecalEnergy.h"

using namespace std;

class TGTextEntry;
class CXMainWindow;
class TGNumberEntry;
class TF1;
class TGComboBox;
class TGCheckButton;
class CXCanvas;
class TCanvas;

class CXHist1DCalib : public  TGVerticalFrame
{
    RQ_OBJECT("CXHist1DCalib");

public:

private:

    TGTextEntry *fSources = nullptr;
    CXMainWindow *fMainWindow = nullptr;

    TGNumberEntry *fRangeMin = nullptr;
    TGNumberEntry *fRangeMax = nullptr;
    TGNumberEntry *fFWHMSPEntry = nullptr;
    TGNumberEntry *fThresholdSPEntry = nullptr;
    TGNumberEntry *fVerboseLevel = nullptr;

    TGNumberEntry *fCalibOrder = nullptr;
    TGCheckButton *fNoOffset = nullptr;

    TGCheckButton *fLeftTail = nullptr;
    TGCheckButton *fRightTail = nullptr;
    TGCheckButton *f2DSearch = nullptr;

    TList *fListOfObjects = nullptr;

    TString fSourcesFolder;

    vector< Double_t > fEnergies;
    Double_t fERef=0.;

    CXRecalEnergy *fRecalEnergy = nullptr;
    TCanvas *fCalibCanvas = nullptr;

public:

    CXHist1DCalib(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXHist1DCalib();

    CXMainWindow *GetMainWindow(){return fMainWindow;}
    void SetMainWindow(CXMainWindow *w);

    void UpdateSources();
    void UpdateText();
    void CleanCalib();
    void GetCurrentRange();

    void HandleMouse(Int_t EventType,Int_t EventX,Int_t EventY, TObject* selected);
    void HandleMyButton();

    void Calibrate();

    double DinoFct(double*xx,double*pp);
    TF1 *GetDinoFct(TString Name, double min, double max, int Npar);

    ClassDef(CXHist1DCalib,0);
};

#endif
