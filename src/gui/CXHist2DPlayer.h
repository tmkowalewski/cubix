#ifndef CXHist2DPlayer_H
#define CXHist2DPlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"

using namespace std;

class CXMainWindow;
class TH2;
class CXTH1Proj;
class TGListBox;
class TGLBEntry;
class TGCheckButton;
class TGTextEntry;
class TGComboBox;
class TGNumberEntry;

class CXHist2DPlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXHist2DPlayer");

private:

    CXMainWindow *fMainWindow = nullptr;

    Int_t fAxisProj = 0; // X => 0 ; Y => 1

    CXTH1Proj *fCurrentProj = nullptr;

    TGListBox *fStoredSpectraBox = nullptr;
    TList *fListOfStoredSpectra = nullptr;
    TGTextEntry *fDrawOpt = nullptr;
    TGCheckButton *fFixRange = nullptr;

    TGNumberEntry *fRebinValue = nullptr;

    TGComboBox *fProjectionAxis = nullptr;

public:
    CXHist2DPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, CXMainWindow *window);
    ~CXHist2DPlayer();

    void SetMainWindow(CXMainWindow *w);

    void InitGG(TH2 *hist_in =nullptr);
    void UpdateProjection();

    void Project();
    void AddBackgd();
    void AddGate();
    void ClearGates();
    void ApplyLastGate();

    void UpdateDrawOpt();

private:
    CXTH1Proj *GetProj();

    ClassDef(CXHist2DPlayer,0);
};

#endif
