#ifndef CXLevelSchemePlayer_H
#define CXLevelSchemePlayer_H

#include <map>
#include "TNamed.h"

#include "tkmanager.h"

class CXMainWindow;
class CXGuiLSPlayer;
class TH1;

using namespace tkn;

class CXLevelSchemePlayer : public TNamed
{

private:

    CXMainWindow *fMainWindow = nullptr;
    CXGuiLSPlayer *fGuiLSPlayer = nullptr;

    Int_t fMinStrenght;
    Int_t fMaxStrenght;

    Int_t fMinSpin;
    Int_t fMaxSpin;

    Float_t fRangeXMin;
    Float_t fRangeXMax;

    Float_t fMinELevel;
    Float_t fMaxELevel;

    Float_t fMinLifeTime;
    Float_t fMaxLifeTime;

    TH1 *fCurrentHist = nullptr;

    Color_t fColorWheel[48];

    Color_t fCurrentColor;

    TList *fListOfArrows = nullptr;
    TList *fListOfLatex = nullptr;
    TList *fListOfBoxes = nullptr;
    TList *fListOfCXArrows = nullptr;

    Bool_t fCanReplot;
    Bool_t fLevelDraw;

public:

    CXLevelSchemePlayer(const char* name="", const char *title="");
    virtual ~CXLevelSchemePlayer();

    void SetMainWindow(CXMainWindow *w){fMainWindow = w;}
    void SetGuiLSPlayer(CXGuiLSPlayer *player){fGuiLSPlayer = player;}

    void CleanArrows();
    shared_ptr<tklevel_scheme> DrawArrows(TString ListOfNuclei, TH1 *h, TString DataSet);
    shared_ptr<tklevel_scheme> DrawArrowsForNuc(TString NucName, TString DataSet, bool do_draw=true);

    Bool_t CanReplot(){return fCanReplot;}

    void ProcessedEventLevelScheme(Int_t eventType, Int_t eventX, Int_t eventY, TObject*obj);
    void RemoveArrow(Int_t ArrowIndex);

    void PlotLevelScheme(TString NucName, TString Type="ENSDF", TString DataSet="ADOPTED LEVELS, GAMMAS");

    TString PrintNucleusLevels(Int_t Z, Int_t N, bool print = false);
    TString PrintNucleusLevels(tklevel_scheme *lev, TString NucName, bool print = false);

//    TString PrintNucleusGammas(Int_t Z, Int_t N, bool print = false);
//    TString PrintNucleusGammas(tklevel_scheme *lev, TString NucName, bool print = false);

    TString GetLTString(double LifeTime);

private:

    void DrawArrow(TH1 *hist, const std::shared_ptr<tkn::tkgammadecay> gamma, tknucleus &Nuc);
    void ConnectCanvas();

    tklevel *GetLevel(tklevel *nuclev, tklevel_scheme *ls);
    void DrawLink(tklevel_scheme *ls, tkgammadecay *gammalink);
    void ShiftLink(tkgammadecay *link,Float_t XShift);
    void GetLimits(tkgammadecay *link, Float_t &xmin,Float_t &xmax,Float_t &ymin,Float_t &ymax);
    void ResizeNucLev(tklevel *nuclev,Float_t width);


    ClassDef(CXLevelSchemePlayer,0)
};

#endif // CXLevelSchemePlayer_H
