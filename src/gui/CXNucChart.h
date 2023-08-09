#ifndef CXNucChart_H
#define CXNucChart_H

#include "TGFrame.h"
#include <RQ_OBJECT.h>

#include "tkmanager.h"

class TGListBox;
class CXMainWindow;
class CXCanvas;
class TGStatusBar;
class TGComboBox;
class TH2D;
class NucData;
class TGTextEntry;
class TRootEmbeddedCanvas;

class CXNucChart: public TGTransientFrame
{
    RQ_OBJECT("CXNucChart");
public:
    enum NucChartType {
        M_LifeTime,
        M_1stIsomer,
        M_2ndIsomer,
        M_1rstExcitedState,
        M_DecayMode,
        M_BE2E2B2,
        M_BE2WU,
        M_NucInfo,
        M_LevelsInfo,
        M_GammaInfos,
    };

private:

    CXMainWindow *fMainWindow;
    TGListBox *fInfoBox;
    TRootEmbeddedCanvas *fRootCanvas;
    CXCanvas *fCanvas;
    TGStatusBar *fStatusBar;
    TGComboBox *fViewMode;
    TGComboBox *fPrintMode;
    TGComboBox *fDataSetMode;

    Int_t fLastEventType;
    Int_t fLastEventX;
    Int_t fLastEventY;
    TObject *fLastSelected;

    Double_t fLastXPosition;
    Double_t fLastYPosition;

    Bool_t fCTRL = false;

    TH2D *fNucChartHist;

    TList *fMagicList;
    TList *fListOfBoxes;

    bool fPlotMagics = true;

    Double_t fNMin;
    Double_t fNMax;
    Double_t fZMin;
    Double_t fZMax;

    TString fLastSelectedBox;
    shared_ptr<tkn::tknucleus> fSelectedNucleus;
    shared_ptr<tkn::tklevel_scheme> fSelectedLevelScheme;
    TString fCurrentDataSet = "ADOPTED LEVELS, GAMMAS";

    TGTextEntry *fNucleusTextEntry;

    bool fDoUpdateRange = false;

public:

    CXNucChart(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, CXMainWindow *mainwindow);
    virtual ~CXNucChart();

    void UpdateNucChart();
    void PrintInListBox(TString mess, Int_t Type=-1);

    void ProcessedKeyEvent(Event_t *event);
    void HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected);
    void UpdateRange();
    void PrintInfos(bool inprompt = false);
    void NucNotValidated();
    void UpdateNucFromSymb();
    void UpdatePrintMode(){PrintInfos(true);}
    void UpdateDataSet();
    static void SetPalette(Int_t Mode);


protected:

    void SelectNucleus(Int_t Z, Int_t N);
    void ShowMagicNumbers(bool On=true);
    void PlotBoxes();

    TString PrintNucleusGammas(shared_ptr<tkn::tklevel_scheme> lev, TString NucName, bool print);
    TString PrintNucleusLevels(shared_ptr<tkn::tklevel_scheme> lev, TString NucName, bool print);

    ClassDef(CXNucChart,0)

};


#endif //CXNucChart_H

