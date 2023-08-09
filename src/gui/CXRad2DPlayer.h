#ifndef CXRad2DPlayer_H
#define CXRad2DPlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"

using namespace std;

class CXMainWindow;
class CXRadReader;
class CXRadCubeTH1Proj;
class TGListBox;
class TGLBEntry;
class TGCheckButton;
class TGTextEntry;
class TH2;
class TGNumberEntry;

class CXRad2DPlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXRad2DPlayer");

private:

    CXMainWindow *fMainWindow = nullptr;
    CXRadCubeTH1Proj *fCurrentProj = nullptr;

    TGCheckButton *fFixRange = nullptr;
    TGCheckButton *fBckSubtract = nullptr;

    TGNumberEntry *fRebinValue = nullptr;

    TGListBox *fStoredSpectraBox = nullptr;
    TList *fListOfStoredSpectra = nullptr;
    TGTextEntry *fDrawOpt = nullptr;

public:
    CXRad2DPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, CXMainWindow *window);
    ~CXRad2DPlayer();

    void SetMainWindow(CXMainWindow *w);

    void Init(TH2 *hist);
    void Project();
    void AddGate();
    void ClearGates();
    void ApplyLastGates();

    void UpdateDrawOpt();

private:
    CXRadCubeTH1Proj *GetProj();

    ClassDef(CXRad2DPlayer,0);
};

#endif
