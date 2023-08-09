#ifndef CXMainWindow_H
#define CXMainWindow_H

#include <thread>
#include <future>

#include "TGFrame.h"

#include "RQ_OBJECT.h"

#include "CXCanvas.h"

class TGMenuBar;
class TGVSplitter;
class TRootEmbeddedCanvas;
class TGTab;
class TGStatusBar;
class TVirtualPadEditor;
class CXBgdUtility;
class CXGuiToolbar;
class TGPopupMenu;
class TGraphErrors;
class CXFileList;
class CXGuiLSPlayer;
class CXHist1DPlayer;
class CXHist1DCalib;
class CXHist2DPlayer;
class CXRad2DPlayer;
class CXRadCubePlayer;
class CXGammaSearch;
class CXNucChart;
class TGCanvas;
class CXRadReader;
class CXSavedList;

using namespace std;

class CXMainWindow : public  TGMainFrame
{
    RQ_OBJECT("CXMainWindow");

public:
    enum ETestCommandIdentifiers
    {
        M_New_Canvas,
        M_New_Browser,
        M_Save_As,
        M_Save_To_Ascii,
        M_Exit,
        M_Editor,
        M_GammaSearch,
        M_NucChart,
        M_ShowStats,
        M_ShowTitle,
        M_BkdUtility,
        M_LSPlayerUtility,
        M_Hist1DPlayer,
        M_Hist1DCalib,
        M_Hist2DPlayer,
        M_Rad2DPlayer,
        M_RadCubePlayer,
        M_FileListUtility,
        M_Eff,
        M_SavedList
    };

protected:

    TGMenuBar   *fMenuBar = nullptr;
    TGPopupMenu *fMenuFile = nullptr;
    TGPopupMenu *fMenuView = nullptr;
    TGPopupMenu *fMenuTools = nullptr;
    TGPopupMenu *fMenuOptions = nullptr;

    TGVSplitter *fSplitter = nullptr;

    TRootEmbeddedCanvas *fRootCanvas = nullptr;
    CXCanvas *fCanvas = nullptr;
    TVirtualPad *fSelectedPad = nullptr;

    TList *fListOfCanvases = nullptr;

    TGVerticalFrame *fVFLeft = nullptr;
    TGVerticalFrame *fVFRight = nullptr;

    TGTab *fMainTab = nullptr;
    TGCompositeFrame *fBSPTab = nullptr;
    TGCompositeFrame *fBkdToolTab = nullptr;
    TGCompositeFrame *fHist1DPlayerTab = nullptr;
    TGCompositeFrame *fHist1DCalibTab = nullptr;
    TGCompositeFrame *fHist2DPlayerTab = nullptr;
    TGCompositeFrame *fRad2DPlayerTab = nullptr;
    TGCompositeFrame *fRadCubePlayerTab = nullptr;
    TGCompositeFrame *fLSPlayerToolTab = nullptr;
    TGCompositeFrame *fFileListTab = nullptr;
    TGCompositeFrame *fEditorTab = nullptr;
    TGCompositeFrame *fEIntensity = nullptr;
    TGCompositeFrame *fSavedListTab = nullptr;

    TGCanvas *fTGCanvas = nullptr;

    TGTab    *fCanvasTab = nullptr;
    TList    *fCanvasTabList = nullptr;

    CXFileList      *fFileList = nullptr;

    CXGuiToolbar    *fToolBar = nullptr;
    CXBgdUtility    *fBkdSubtract = nullptr;
    CXGuiLSPlayer   *fLSPlayerTool = nullptr;
    CXHist1DPlayer  *fHist1DPlayer = nullptr;
    CXHist1DCalib  *fHist1DCalib = nullptr;
    CXHist2DPlayer  *fHist2DPlayer = nullptr;
    CXRad2DPlayer   *fRad2DPlayer = nullptr;
    CXRadCubePlayer *fRadCubePlayer = nullptr;
    CXSavedList *fSavedList = nullptr;

    TGStatusBar *fStatusBar = nullptr;
    TVirtualPadEditor *fEditor = nullptr;

    Bool_t IsEditorEnabled{};
    Bool_t IsBkdUtilityEnabled{};
    Bool_t IsLSPlayerToolEnabled{};
    Bool_t IsHist1DPlayerEnabled{};
    Bool_t IsHist1DCalibPlayerEnabled{};
    Bool_t IsHist2DPlayerEnabled{};
    Bool_t IsRad2DPlayerEnabled{};
    Bool_t IsRadCubePlayerEnabled{};
    Bool_t IsFileListUtilityEnabled{};
    Bool_t IsSavedListEnabled{};

    Bool_t IsStatsShown = false;
    Bool_t IsTitleShown = false;

    Int_t fLastEventType{};
    Int_t fLastEventX{};
    Int_t fLastEventY{};
    TObject *fLastSelected = nullptr;

    Double_t fLastXPosition{};
    Double_t fLastYPosition{};

    Float_t fRefX = 0;
    Float_t fRefY = 0;

    Bool_t fCTRL = false;
    Bool_t fSHIFT = false;

    TList *fListOfSavedGates = nullptr;

public:

    CXGammaSearch   *fGammaSearchWindow = nullptr;

    CXNucChart   *fNucChartWindow = nullptr;

public:
    CXMainWindow(const TGWindow *p, UInt_t w, UInt_t h);
    ~CXMainWindow();

    void CloseWindow();

    void Init();

    void HandleMovement(Int_t event, Int_t EventX, Int_t EventY, TObject *selected);
    void ProcessedKeyEvent(Event_t *event);

    void SetStatusText(const char *txt, Int_t pi);

    void HandleMenu(Int_t id);

    CXCanvas *GetCanvas(){return fCanvas;}
    TRootEmbeddedCanvas *GetROOTCanvas() {return fRootCanvas;}

    TList *GetListOfCanvases(){return fListOfCanvases;}

    void SaveCanvasAs();
    void NewTab(Int_t px=1, Int_t py=1, const TString &name="");
    void CloseTab(TGTab *tab, Int_t tabnr);
    void CloseCanvasTab(Int_t n){return CloseTab(fCanvasTab,n);}
    void CloseToolsTab(Int_t tabnr);
    void DoTab(Int_t tabnr=0);

    void DoDraw(TObject *obj, TString DrawOpt);

    void RefreshPads();
    void DoRefresh();

    void OpenTreeViewer(){Emit("OpenTreeViewer()");}   //*SIGNAL*
    void OpenLS(){Emit("OpenLS()");} //*SIGNAL*

    TH1 *GetHisto(TVirtualPad *pad = nullptr, bool GetFirst = true);
    void UpdateContextMenus();
    void PopUpFindPeaks(TObject *c);
    void PopUpFitPeaks(TObject *c);
    void PopUpShowBackground(TObject *c);
    void AddToStoredSpectra(TObject *c);
    void PopUpInitGG(TObject *c);
    void PopUpInitRadGG(TObject *c);

    void InitRadCubePlayer(CXRadReader *radreader);

    void HistScale(Float_t scaleFact=1, TObject *c=nullptr);
    void GraphScale(Float_t scaleFact=1, TObject *c=nullptr);
    void HistNorm(TObject *c);
    void Rebin2D(Int_t RebinX=2, Int_t RebinY=2, TObject *c=nullptr);
    void PopUpEval2DBackground(TObject *c);

    void OpenFile(TString filename);

    TVirtualPad *GetSelectedPad(){return fSelectedPad;}
    void SetSelectedPad(TVirtualPad *pad){fSelectedPad = pad;}

    Bool_t IsCtrlOn(){return fCTRL;}
    Bool_t IsShiftOn(){return fSHIFT;}

    CXSavedList *GetSaveList(){return fSavedList;}

    TList *GetSavedGatesList(){return fListOfSavedGates;}

    CXGuiLSPlayer *GetLSPlayer(){return fLSPlayerTool;}

    static void SetPalette();

protected:

private:

    void ToggleTab(Bool_t &Enable, TGCompositeFrame *tab, const char * name);
    void SaveToAscii(){fCanvas->SaveHistToAsciiFile();}

    std::future<void> loadingFuture;

    void load_tkn_db();
    void wait();

    ClassDef(CXMainWindow,0)
};

R__EXTERN ULong_t CXred;
R__EXTERN ULong_t CXblue;
R__EXTERN ULong_t CXblack;
R__EXTERN ULong_t CXgreen;

//#define ERR_MESS  std::cout<<"\e[0;3;31m -- ERROR   : "
//#define WARN_MESS std::cout<<"\e[0;3;33m -- WARNNING: "
//#define INFO_MESS std::cout<<"\e[0;3;32m -- INFO    : "
//#define END_MESS  "\e[0;3m"
//#define ENDL END_MESS<<std::endl

#endif
