#ifndef CXRadCubePlayer_H
#define CXRadCubePlayer_H

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

class CXRadCubePlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXRadCubePlayer");

private:

    CXMainWindow *fMainWindow = nullptr;
    CXRadCubeTH1Proj *fCurrentProj = nullptr;

    TGCheckButton *fFixRange = nullptr;
    TGCheckButton *fBckSubtract = nullptr;

    TGListBox *fStoredSpectraBox = nullptr;
    TList *fListOfStoredSpectra = nullptr;
    TGTextEntry *fDrawOpt = nullptr;

public:
    CXRadCubePlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, CXMainWindow *window);
    ~CXRadCubePlayer();

    void SetMainWindow(CXMainWindow *w);

    void Init(CXRadReader *radreader);
    void Project();
    void AddGate1();
    void AddGate2();
    void ClearGates();
    void ApplyLastGates();

    void UpdateDrawOpt();

private:
    CXRadCubeTH1Proj *GetProj();

    ClassDef(CXRadCubePlayer,0);
};

#endif
