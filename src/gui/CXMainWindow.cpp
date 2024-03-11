/********************************************************************************
 *   Copyright (c) : Université de Lyon 1, CNRS/IN2P3, UMR5822,                 *
 *                   IP2I, F-69622 Villeurbanne Cedex, France                   *
 *   Contibutor(s) :                                                            *
 *      Jérémie Dudouet jeremie.dudouet@cnrs.fr [2023]                          *
 *                                                                              *
 *    This software is governed by the CeCILL-B license under French law and    *
 *    abiding by the  rules of distribution of free  software.  You can use,    *
 *    modify  and/ or  redistribute  the  software under  the  terms of  the    *
 *    CeCILL-B license as circulated by CEA, CNRS and INRIA at the following    *
 *    URL \"http://www.cecill.info\".                                           *
 *                                                                              *
 *    As a counterpart to the access  to the source code and rights to copy,    *
 *    modify  and redistribute granted  by the  license, users  are provided    *
 *    only with a limited warranty  and the software's author, the holder of    *
 *    the economic  rights, and the  successive licensors have  only limited    *
 *    liability.                                                                *
 *                                                                              *
 *    In this respect, the user's attention is drawn to the risks associated    *
 *    with loading,  using, modifying  and/or developing or  reproducing the    *
 *    software by the user in light of its specific status of free software,    *
 *    that  may mean that  it is  complicated to  manipulate, and  that also    *
 *    therefore  means that it  is reserved  for developers  and experienced    *
 *    professionals having in-depth  computer knowledge. Users are therefore    *
 *    encouraged  to load  and test  the software's  suitability  as regards    *
 *    their  requirements  in  conditions  enabling the  security  of  their    *
 *    systems  and/or data to  be ensured  and, more  generally, to  use and    *
 *    operate it in the same conditions as regards security.                    *
 *                                                                              *
 *    The fact that  you are presently reading this means  that you have had    *
 *    knowledge of the CeCILL-B license and that you accept its terms.          *
 ********************************************************************************/

#include "CXMainWindow.h"

#include <thread>
#include <chrono>

#include "TStyle.h"
#include "TG3DLine.h"
#include "TApplication.h"
#include "TGMenu.h"
#include "TGSplitter.h"
#include "TRootEmbeddedCanvas.h"
#include "TGTab.h"
#include "TGStatusBar.h"
#include "TVirtualPadEditor.h"
#include "TFrame.h"
#include "KeySymbols.h"
#include "TClassMenuItem.h"
#include "TROOT.h"
#include "TBrowser.h"
#include "TClass.h"
#include "TVirtualX.h"
#include "TSystem.h"
#include "TGFileDialog.h"
#include "TRootBrowser.h"

#include "cubix_config.h"

#include "CXBgdUtility.h"
#include "CXGuiToolbar.h"
#include "CXFileList.h"
#include "CXGuiENSDFPlayer.h"
#include "CXHist1DPlayer.h"
#include "CXHist1DCalib.h"
#include "CXFitEfficiency.h"
#include "CXAngCorrPlayer.h"
#include "CXHist2DPlayer.h"
#include "CXRad2DPlayer.h"
#include "CXGammaSearch.h"
#include "CXNucChart.h"
#include "CXRadReader.h"
#include "CXRadCubePlayer.h"
#include "CXSavedList.h"
#include "CXRadCubeTH1Proj.h"
#include "CXArrow.h"
#include "CXBashColor.h"
#include "CXDialogBox.h"

#include "tkmanager.h"

using namespace std;

ULong_t CXred;
ULong_t CXblue;
ULong_t CXblack;
ULong_t CXgreen;
ULong_t CXorange;

CXMainWindow::CXMainWindow(const TGWindow *p, UInt_t w, UInt_t h) : TGMainFrame(p, w, h)
{
    loadingFuture = std::async(std::launch::async, &CXMainWindow::load_tkn_db, this);

    Init();
}

void CXMainWindow::Init()
{
    gClient->GetColorByName("red",  CXred);
    gClient->GetColorByName("blue", CXblue);
    gClient->GetColorByName("black",CXblack);
    gClient->GetColorByName("green",CXgreen);
    gClient->GetColorByName("orange",CXorange);

//    if(gNDManager == nullptr)
//        gNDManager = new CXNDManager(this);

    auto *fHf = new TGHorizontalFrame(this,GetWidth(),GetHeight());

    //------- Menu -----------

    //----------------Menu File--------------------

    fMenuFile = new TGPopupMenu(gClient->GetRoot());
    fMenuFile->AddEntry("New Canvas", M_New_Canvas, nullptr , gClient->GetPicture("newcanvas.xpm"));
    fMenuFile->AddEntry("New multi-pad Canvas", M_New_MultiPad_Canvas, nullptr , gClient->GetPicture("multiple_pads.png"));
    fMenuFile->AddEntry("New Browser", M_New_Browser, nullptr, gClient->GetPicture("browser.xpm"));
    fMenuFile->AddEntry("Save Canvas as", M_Save_As, nullptr, gClient->GetPicture("bld_save.png"));
    fMenuFile->AddEntry("Save hist to ascii", M_Save_To_Ascii, nullptr, gClient->GetPicture("query_new.xpm"));
    fMenuFile->AddSeparator();
    fMenuFile->AddEntry("Exit", M_Exit, nullptr, gClient->GetPicture("bld_exit.png"));
    fMenuFile->Connect("Activated(Int_t)", "CXMainWindow", this, "HandleMenu(Int_t)");

    //----------------Menu View--------------------

    fMenuView = new TGPopupMenu(gClient->GetRoot());
    fMenuView->Connect("Activated(Int_t)", "CXMainWindow", this, "HandleMenu(Int_t)");

    fMenuView->AddEntry("Browse Files", M_FileListUtility, nullptr, gClient->GetPicture("ed_open.png"));
    //    fMenuView->CheckEntry(M_FileListUtility);
    IsFileListUtilityEnabled = true;

    fMenuView->AddEntry("Saved list", M_SavedList, nullptr, gClient->GetPicture("bld_listbox.xpm"));
    //    fMenuView->CheckEntry(M_SavedList);
    IsSavedListEnabled = true;

    fMenuView->AddEntry("Workspace manager", M_WSManager, nullptr, gClient->GetPicture("folders.png"));
    //    fMenuView->CheckEntry(M_SavedList);
    IsWSManagerEnabled = true;

    fMenuView->AddSeparator();

    fMenuView->AddEntry("Show editor", M_Editor,nullptr,gClient->GetPicture("bld_edit.png"));
    //    fMenuView->CheckEntry(M_Editor);
    IsEditorEnabled = true;

    //---------------- Menu Tools --------------------

    fMenuTools = new TGPopupMenu(gClient->GetRoot());

    fMenuTools->AddLabel("1D Tools...",gClient->GetPicture("h1_t.xpm"));
    fMenuTools->AddSeparator();

    fMenuTools->AddEntry("ENSDF reader", M_LSPlayerUtility,nullptr,gClient->GetPicture("levelscheme_t.png"));

    IsLSPlayerToolEnabled = true;

    fMenuTools->AddEntry("Peak fitter", M_Hist1DPlayer,nullptr,gClient->GetPicture("FitTool.xpm"));
    IsHist1DPlayerEnabled= true;

    fMenuTools->AddEntry("Energy calibration", M_Hist1DCalib,nullptr,gClient->GetPicture("calib.png"));
    IsHist1DCalibPlayerEnabled= true;

    fMenuTools->AddEntry("Efficiency fit", M_HistEffFit,nullptr,gClient->GetPicture("efficiency.png"));
    IsHistEffFitPlayerEnabled= true;

    fMenuTools->AddEntry("Background player", M_BkdUtility,nullptr,gClient->GetPicture("h1_t.xpm"));
    IsBkdUtilityEnabled = true;

    fMenuTools->AddEntry("Angular correlations", M_AngCorrPlayer,nullptr,gClient->GetPicture("angcorr.png"));
    IsAngCorrPlayerEnabled= true;

    fMenuTools->AddSeparator();
    fMenuTools->AddLabel("2D Tools...",gClient->GetPicture("h2_t.xpm"));
    fMenuTools->AddSeparator();

    fMenuTools->AddEntry("GxG standard", M_Hist2DPlayer,nullptr,gClient->GetPicture("h2_t.xpm"));
    IsHist2DPlayerEnabled= true;

    fMenuTools->AddEntry("GxG radware's style", M_Rad2DPlayer,nullptr,gClient->GetPicture("rw3.gif"));
    IsRad2DPlayerEnabled= true;

    fMenuTools->AddSeparator();
    fMenuTools->AddLabel("3D Tools...",gClient->GetPicture("h3_t.xpm"));
    fMenuTools->AddSeparator();

    fMenuTools->AddEntry("GxGxG radware's style", M_RadCubePlayer,nullptr,gClient->GetPicture("rw3.gif"));
    IsRadCubePlayerEnabled= true;

    fMenuTools->AddSeparator();
    fMenuTools->AddLabel("Others...");
    fMenuTools->AddSeparator();

    fMenuTools->AddEntry("Gamma search", M_GammaSearch,nullptr,gClient->GetPicture("ed_find.png"));
    fMenuTools->AddEntry("Nuclear Chart", M_NucChart,nullptr,gClient->GetPicture("app-map-icon.png"));

    fMenuTools->Connect("Activated(Int_t)", "CXMainWindow", this, "HandleMenu(Int_t)");

    //---------------- Menu Options --------------------

    fMenuOptions = new TGPopupMenu(gClient->GetRoot());
    fMenuOptions->AddEntry("Show Stats", M_ShowStats,nullptr,gClient->GetPicture("stats.png"));
    fMenuOptions->AddEntry("Show Title", M_ShowTitle,nullptr,gClient->GetPicture("eve_text.gif"));

    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);

    fMenuOptions->Connect("Activated(Int_t)", "CXMainWindow", this, "HandleMenu(Int_t)");

    fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
    this->AddFrame(fMenuBar, new TGLayoutHints(kLHintsExpandX));
    fMenuBar->AddPopup("File", fMenuFile, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
    fMenuBar->AddPopup("View", fMenuView, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
    fMenuBar->AddPopup("Tools", fMenuTools, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
    fMenuBar->AddPopup("Options", fMenuOptions, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));

    auto *fline = new TGHorizontal3DLine(this);
    AddFrame(fline,new TGLayoutHints(kLHintsExpandX));

    //    //////////////////******************** partie gauche ********************////////////////////////////////

    fTGCanvas = new TGCanvas(fHf, 10, 10, kFixedWidth);
    fHf->AddFrame(fTGCanvas, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));

    auto *VFrame = new TGVerticalFrame(fTGCanvas->GetViewPort());
    fTGCanvas->SetContainer(VFrame);

    fTGCanvas->Resize(GetWidth()*0.23, GetHeight());

    fMainTab = new TGTab(VFrame);
    VFrame->AddFrame(fMainTab, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    fMainTab->Connect("CloseTab(Int_t)", "CXMainWindow", this, "CloseToolsTab(Int_t)");

    TString Name;

    Name = "Saved List";
    fSavedListTab = fMainTab->AddTab(Name);
    fSavedList = new CXSavedList(fSavedListTab,10,10);
    fSavedList->SetMainWindow(this);
    fSavedList->SetName(Name);
    fSavedListTab->AddFrame(fSavedList, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsSavedListEnabled,fSavedListTab,Name);

    Name = "WS Manager";
    fWSManagerTab = fMainTab->AddTab(Name);
    fWSManager = new CXWSManager(fWSManagerTab,10,10);
    fWSManager->SetMainWindow(this);
    fWSManager->SetName(Name);
    fWSManagerTab->AddFrame(fWSManager, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsWSManagerEnabled,fWSManagerTab,Name);

    Name = "Bkgd Player";
    fBkdToolTab = fMainTab->AddTab(Name);
    fBkdSubtract = new CXBgdUtility(fBkdToolTab,10,10);
    fBkdSubtract->SetMainWindow(this);
    fBkdSubtract->SetName(Name);
    fBkdToolTab->AddFrame(fBkdSubtract, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsBkdUtilityEnabled,fBkdToolTab,Name);

    Name = "1D Player";
    fHist1DPlayerTab = fMainTab->AddTab(Name);
    fHist1DPlayer = new CXHist1DPlayer(fHist1DPlayerTab,10,10);
    fHist1DPlayer->SetMainWindow(this);
    fHist1DPlayer->SetName(Name);
    fHist1DPlayerTab->AddFrame(fHist1DPlayer, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsHist1DPlayerEnabled,fHist1DPlayerTab,Name);

    Name = "1D Calib";
    fHist1DCalibTab = fMainTab->AddTab(Name);
    fHist1DCalib = new CXHist1DCalib(fHist1DCalibTab,10,10);
    fHist1DCalib->SetMainWindow(this);
    fHist1DCalib->SetName(Name);
    fHist1DCalibTab->AddFrame(fHist1DCalib, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsHist1DCalibPlayerEnabled,fHist1DCalibTab,Name);

    Name = "Efficiency fit";
    fHistEffFitTab = fMainTab->AddTab(Name);
    fHistEffFit = new CXFitEfficiency(fHistEffFitTab,10,10);
    fHistEffFit->SetMainWindow(this);
    fHistEffFit->SetName(Name);
    fHistEffFitTab->AddFrame(fHistEffFit, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsHistEffFitPlayerEnabled,fHistEffFitTab,Name);

    Name = "AngCorr";
    fAngCorrPlayerTab = fMainTab->AddTab(Name);
    fAngCorrPlayer = new CXAngCorrPlayer(fAngCorrPlayerTab,10,10);
    fAngCorrPlayer->SetMainWindow(this);
    fAngCorrPlayer->SetName(Name);
    fAngCorrPlayerTab->AddFrame(fAngCorrPlayer, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsAngCorrPlayerEnabled,fAngCorrPlayerTab,Name);

    Name = "2D Player";
    fHist2DPlayerTab = fMainTab->AddTab(Name);
    fHist2DPlayer = new CXHist2DPlayer(fHist2DPlayerTab,10,10,this);
    fHist2DPlayer->SetName(Name);
    fHist2DPlayerTab->AddFrame(fHist2DPlayer, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsHist2DPlayerEnabled,fHist2DPlayerTab,Name);

    Name = "Rad2D Player";
    fRad2DPlayerTab = fMainTab->AddTab(Name);
    fRad2DPlayer = new CXRad2DPlayer(fRad2DPlayerTab,10,10,this);
    fRad2DPlayer->SetName(Name);
    fRad2DPlayerTab->AddFrame(fRad2DPlayer, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsRad2DPlayerEnabled,fRad2DPlayerTab,Name);

    Name = "RadCube Player";
    fRadCubePlayerTab = fMainTab->AddTab(Name);
    fRadCubePlayer = new CXRadCubePlayer(fRadCubePlayerTab,10,10,this);
    fRadCubePlayer->SetMainWindow(this);
    fRadCubePlayer->SetName(Name);
    fRadCubePlayerTab->AddFrame(fRadCubePlayer, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsRadCubePlayerEnabled,fRadCubePlayerTab,Name);

    Name = "LS Player";
    fLSPlayerToolTab = fMainTab->AddTab(Name);
    fLSPlayerTool = new CXGuiENSDFPlayer(fLSPlayerToolTab,10,10);
    fLSPlayerTool->SetMainWindow(this);
    fLSPlayerTool->SetName(Name);
    fLSPlayerToolTab->AddFrame(fLSPlayerTool, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    ToggleTab(IsLSPlayerToolEnabled,fLSPlayerToolTab,Name);

    Name = "Files";
    fFileListTab = fMainTab->AddTab(Name);
    fFileList = new CXFileList(fFileListTab,GetWidth(), GetHeight());
    fFileList->SetMainWindow(this);
    fFileList->SetName(Name);
    fFileListTab->AddFrame(fFileList, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    fMainTab->GetTabTab(fFileList->GetName())->ShowClose();
    ToggleTab(IsFileListUtilityEnabled,fFileListTab,fFileList->GetName());

    //    //////////////////******************** partie droite ********************////////////////////////////////

    fSplitter = new TGVFileSplitter(fHf,2,2);
    fSplitter->SetFrame(fTGCanvas, kTRUE);
    fHf->AddFrame(fSplitter, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));
    fSplitter->Connect("LayoutHeader(TGFrame *)", "CXMainWindow", this, "DoRefresh()");

    fVFRight = new TGVerticalFrame(fHf, 10, 10);
    fHf->AddFrame(fVFRight, new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY));

    fListOfCanvases = new TList();

    fCanvasTab = new TGTab(fVFRight);
    fCanvasTab->Connect("CloseTab(Int_t)", "CXMainWindow", this, "CloseCanvasTab(Int_t)");
    fCanvasTab->Connect("Selected(Int_t)", "CXMainWindow", this, "DoTab(Int_t)");
    fVFRight->AddFrame(fCanvasTab, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));

    NewTab();
    DoTab();

    //------- Editor -----------
    fEditorTab = fMainTab->AddTab("Editor");
    fEditorTab->SetEditable();
    fEditor = TVirtualPadEditor::LoadEditor();
    fEditor->SetGlobal(kFALSE);
    fEditorTab->SetEditable(kFALSE);
    ToggleTab(IsEditorEnabled,fEditorTab,"Editor");

    // status bar
    array<Int_t,5> parts{20 , 20 , 20 , 20 ,20};
    fStatusBar = new TGStatusBar(fVFRight,50,10,kHorizontalFrame);
    fStatusBar->SetParts(parts.data(),5);
    fStatusBar->Draw3DCorner(kFALSE);
    fVFRight->AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 0, 0, 2, 0));

    AddFrame(fHf, new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY));

    UpdateContextMenus();

    // What to clean up in destructor
    SetCleanup(kDeepCleanup);

    // Set a name to the main frame
    SetWindowName("Cubix Spectra Player");
    SetIconPixmap("Cubix.png");
    SetIconName("Cubix");

    MapSubwindows();
    Layout();
    Resize(GetDefaultSize());
    MapWindow();

    gSystem->ProcessEvents();

    ToggleTab(IsFileListUtilityEnabled,fFileListTab,fFileList->GetName());

    fListOfSavedGates = new TList;
    fListOfSavedGates->SetOwner();
    fListOfSavedGates->SetName("ListOfGates");

    TH1::AddDirectory(kFALSE);

    // HandleMenu(M_AngCorrPlayer);
}

CXMainWindow::~CXMainWindow() = default;

void CXMainWindow::HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected)
{
    SetPalette();

    fSelectedPad = fCanvas->GetClickSelectedPad();

    if(gPad && selected != nullptr) {
        const char *text0, *text1, *text2, *text4;
        char text3[50];
        text0 = selected->ClassName();
        SetStatusText(text0,0);
        text1 = selected->GetTitle();
        SetStatusText(text1,1);
        text2 = selected->GetName();
        SetStatusText(text2,2);

        if(fCanvas->GetCrosshair() == 1 && (selected->InheritsFrom("TFrame") || selected->InheritsFrom("TH1") || selected->InheritsFrom("CXCanvas") || selected->InheritsFrom("TPad"))) {
            if (EventType == kButton2) {
                fRefX = gPad->AbsPixeltoX(EventX);
                fRefY = gPad->AbsPixeltoY(EventY);
            }
            if (EventType == kButton1) {
                fRefX = 0;
                fRefY = 0;
            }
        }

        if (EventType == kKeyPress)
            snprintf(text3, sizeof(text3), "%c", (char) EventY);
        else
            snprintf(text3, sizeof(text3), "%.2f,%.2f", gPad->AbsPixeltoX(EventX)- fRefX, gPad->AbsPixeltoY(EventY) - fRefY );
        SetStatusText(text3,3);

        text4 = selected->GetObjectInfo(EventX,EventY);
        SetStatusText(text4,4);
    }

    //    if (EventType == kButton1Down)
    //    {
    //        if(fMainTab->GetCurrentTab() == fMainTab->GetTabTab("Editor")) fMainTab->CloseTab(fMainTab->GetCurrent());
    //    }

    /// Recuperation de la derniere position de la souris

    if(EventType == kMouseMotion) {
        fCanvas->AbsPixeltoXY(EventX,EventY,fLastXPosition,fLastYPosition);
    }
    if(EventType == kKeyPress) {
        if((EKeySym)EventY==kKey_f && !fCTRL) {
            if(IsHist1DPlayerEnabled==false)
                ToggleTab(IsHist1DPlayerEnabled,fHist1DPlayerTab,fHist1DPlayer->GetName());
            fMainTab->SetTab(fHist1DPlayer->GetName());

            fHist1DPlayer->NewFit();
        }
        if((EKeySym)EventY==kKey_f && fCTRL) {
            fHist1DPlayer->DoFit();
            fCanvas->Update();
        }
        if((EKeySym)EventY==kKey_s && !fCTRL) {
            if(IsHist1DPlayerEnabled==false)
                ToggleTab(IsHist1DPlayerEnabled,fHist1DPlayerTab,fHist1DPlayer->GetName());
            fMainTab->SetTab(fHist1DPlayer->GetName());

            fHist1DPlayer->PeakSearchClear();
            fHist1DPlayer->PeakSearch();
        }
        if((EKeySym)EventY==kKey_c && !fCTRL) {
            fHist1DPlayer->ClearFits();
            fHist1DPlayer->PeakSearchClear();
        }
        if((EKeySym)EventY==kKey_a && fCTRL) {
            if(IsHist1DCalibPlayerEnabled==false)
                ToggleTab(IsHist1DCalibPlayerEnabled,fHist1DCalibTab,fHist1DCalib->GetName());
            fMainTab->SetTab(fHist1DCalib->GetName());
            fHist1DCalib->Calibrate();
        }
        if((EKeySym)EventY==kKey_n && !fCTRL) {
            NewTab();
        }
        if((EKeySym)EventY==kKey_N && !fCTRL) {
            HandleMenu(M_New_MultiPad_Canvas);
        }
        if((EKeySym)EventY==kKey_S && !fCTRL && selected && (selected->InheritsFrom("TH1") || selected->InheritsFrom("TGraph") || selected->InheritsFrom("TF1") )) {
            AddToStoredSpectra(selected);
        }
        if((EKeySym)EventY==kKey_r && !fCTRL && selected && (selected->InheritsFrom(CXArrowBox::Class()))) {
            ((CXArrowBox*)selected)->Clean();
        }
        if((EKeySym)EventY==kKey_z) {
            // save range
            TFrame *frame = gPad->GetFrame();
            fSavedRangeXMin = frame->GetX1();
            fSavedRangeXMax = frame->GetX2();
            gbash_color->InfoMessage(Form("Saved range: [ %d ; %d ]",(int)fSavedRangeXMin,(int)fSavedRangeXMax));
        }
        if((EKeySym)EventY==kKey_Z) {
            // apply save range
            TH1 *hist = GetHisto();
            if(hist && (fSavedRangeXMin != fSavedRangeXMax)) {
                hist->GetXaxis()->SetRangeUser(fSavedRangeXMin,fSavedRangeXMax);
                gPad->Modified();
                gPad->Update();
            }
        }
    }

    /// Don't work with the touch pad
    //    if(EventType == kMouseMotion && fLastEventType == kButton1Up && EventX==fLastEventX && EventY==fLastEventY ) //equivalent to escape
    //    {
    //        if(fHist1DPlayer->IsCurrentFit())
    //            fHist1DPlayer->EndFit();
    //    }

    fLastEventType = EventType;
    fLastEventX = EventX;
    fLastEventY = EventY;
    fLastSelected = selected;

}

void CXMainWindow::SetStatusText(const char *txt, Int_t pi)
{
    // Set text in status bar.
    if(fStatusBar!=nullptr) fStatusBar->SetText(txt,pi);
}

void CXMainWindow::CloseToolsTab(Int_t tabnr)
{
    TGTabElement *tab = fMainTab->GetTabTab(tabnr);
    TString Name = tab->GetText()->Data();
    if(Name==fFileList->GetName())
        ToggleTab(IsFileListUtilityEnabled,fFileListTab,Name);
    else if(Name==fLSPlayerTool->GetName())
        ToggleTab(IsLSPlayerToolEnabled,fLSPlayerToolTab,Name);
    else if(Name==fHist1DPlayer->GetName())
        ToggleTab(IsHist1DPlayerEnabled,fHist1DPlayerTab,Name);
    else if(Name==fHist1DCalib->GetName())
        ToggleTab(IsHist1DCalibPlayerEnabled,fHist1DCalibTab,Name);
    else if(Name==fHistEffFit->GetName())
        ToggleTab(IsHistEffFitPlayerEnabled,fHistEffFitTab,Name);
    else if(Name==fAngCorrPlayer->GetName())
        ToggleTab(IsAngCorrPlayerEnabled,fAngCorrPlayerTab,Name);
    else if(Name==fHist2DPlayer->GetName())
        ToggleTab(IsHist2DPlayerEnabled,fHist2DPlayerTab,Name);
    else if(Name==fRad2DPlayer->GetName())
        ToggleTab(IsRad2DPlayerEnabled,fRad2DPlayerTab,Name);
    else if(Name==fRadCubePlayer->GetName())
        ToggleTab(IsRadCubePlayerEnabled,fRadCubePlayerTab,Name);
    else if(Name=="Editor")
        ToggleTab(IsEditorEnabled,fEditorTab,Name);
    else if(Name==fBkdSubtract->GetName())
        ToggleTab(IsBkdUtilityEnabled,fBkdToolTab,Name);
    else if(Name==fSavedList->GetName())
        ToggleTab(IsSavedListEnabled,fSavedListTab,Name);
    else if(Name==fWSManager->GetName())
        ToggleTab(IsWSManagerEnabled,fWSManagerTab,Name);
}

void CXMainWindow::CloseTab(TGTab *tab, Int_t tabnr)
{
    for(int i=0 ; i<fCanvas->GetListOfPrimitives()->GetEntries() ; i++)
    {
        TObject *o = fCanvas->GetListOfPrimitives()->At(i);
        if(o->InheritsFrom("TH1"))
            delete o;
    }

    TGFrameElement *el = nullptr;
    if (tab->GetTabContainer(tabnr))
        el = (TGFrameElement *)tab->GetTabContainer(tabnr)->GetList()->First();

    if (el && el->fFrame) {
        el->fFrame->Disconnect("ProcessedConfigure(Event_t*)");
        el->fFrame->SetFrameElement(nullptr);
        if (el->fFrame->InheritsFrom("TGMainFrame")) {
            Bool_t sleep = (el->fFrame->InheritsFrom("TRootCanvas")) ? kTRUE : kFALSE;
            ((TGMainFrame *)el->fFrame)->CloseWindow();
            if (sleep)
                gSystem->Sleep(150);
            gSystem->ProcessEvents();
        }
        else
            delete el->fFrame;
        el->fFrame = nullptr;
        if (el->fLayout && (el->fLayout != fgDefaultHints) &&
            (el->fLayout->References() > 0)) {
            el->fLayout->RemoveReference();
            if (!el->fLayout->References()) {
                delete el->fLayout;
            }
        }
        tab->GetTabContainer(tabnr)->GetList()->Remove(el);
        delete el;
        tab->RemoveTab(tabnr);
    }

    if(tab == fCanvasTab) fListOfCanvases->RemoveAt(tabnr);
}

void CXMainWindow::DoTab(Int_t tabnr)
{
    fCanvas = (CXCanvas*)fListOfCanvases->At(tabnr);
    fSelectedPad = fCanvas->cd();
    if(fCanvas->GetUniqueID() == 666)
        fSelectedPad = fCanvas->cd(1);
    fCanvas->SetSelectedPad((TPad*)fSelectedPad);
    fCanvas->Update();

    //------- Editor -----------
    if(fEditorTab != nullptr)
    {
        delete fEditor;
        fEditorTab->SetEditable();
        fEditor = TVirtualPadEditor::LoadEditor();
        fEditor->SetGlobal(kFALSE);
        fEditorTab->SetEditable(kFALSE);
    }

    TString name = fCanvasTab->GetTabTab(tabnr)->GetText()->Data();

    if(name.BeginsWith("RadCube")) {
        if(IsRadCubePlayerEnabled)
            fMainTab->SetTab(fRadCubePlayer->GetName());
        else
            ToggleTab(IsRadCubePlayerEnabled,fRadCubePlayerTab,fRadCubePlayer->GetName());
    }
    else if(name.BeginsWith("RadGG")) {
        if(IsRad2DPlayerEnabled)
            fMainTab->SetTab(fRad2DPlayer->GetName());
        else
            ToggleTab(IsRad2DPlayerEnabled,fRad2DPlayerTab,fRad2DPlayer->GetName());
    }
    else if(name.BeginsWith("GxG")) {
        if(IsHist2DPlayerEnabled)
            fMainTab->SetTab(fHist2DPlayer->GetName());
        else
            ToggleTab(IsHist2DPlayerEnabled,fHist2DPlayerTab,fHist2DPlayer->GetName());
    }
}

void CXMainWindow::NewTab(Int_t px, Int_t py, const TString &name)
{
    TString TabName = Form("Canvas %d",fCanvasTab->GetNumberOfTabs());
    if(px>1 || py>1) TabName = Form("MultiPad %d",fCanvasTab->GetNumberOfTabs());
    if(name != "") TabName = name;

    while(fCanvasTab->GetTabTab(TabName)) {
        TObjArray *arr = TabName.Tokenize("_");
        TString last = arr->Last()->GetName();
        delete arr;

        if(last.IsDigit()) {
            Int_t n = last.Atoi();
            TabName.ReplaceAll(Form("_%d",n),Form("_%d",n+1));
        }
        else {
            TabName.Append("_2");
        }
    }

    TGCompositeFrame *atab = fCanvasTab->AddTab(TabName);
    fCanvasTab->GetTabTab(TabName)->ShowClose();

    fToolBar = new CXGuiToolbar(atab, 60, 20, kHorizontalFrame);

    atab->AddFrame(fToolBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

    fRootCanvas = new TRootEmbeddedCanvas("ec1", atab);
    atab->AddFrame(fRootCanvas, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    Int_t canvasid = fRootCanvas->GetCanvasWindowId();
    delete fRootCanvas->GetCanvas();
    fCanvas = new CXCanvas(TabName.Copy().ReplaceAll(" ","_"), 10, 10, canvasid);
    fCanvas->SetMainWindow(this);
    fListOfCanvases->Add(fCanvas);
    fCanvas->ToggleToolBar();
    fRootCanvas->AdoptCanvas(fCanvas);
//    fRootCanvas->GetContainer()->Connect("ProcessedEvent(Event_t*)", "CXMainWindow", this, "ProcessedKeyEvent(Event_t*)");
    fRootCanvas->GetClient()->Connect("ProcessedEvent(Event_t *, Window_t)", "CXMainWindow",this, "ProcessedKeyEvent(Event_t*)");

    fCanvas->SetRightMargin(0.03);
    fCanvas->SetBottomMargin(0.09);
    fCanvas->SetLeftMargin(0.09);
    fCanvas->SetTopMargin(0.04);

    fCanvas->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "CXMainWindow", this, "HandleMovement(Int_t,Int_t,Int_t, TObject*)");

    if(px*py>1) {
        fCanvas->SetUniqueID(666);
        fCanvas->cd();
        fCanvas->Divide(px,py,0.001,0.001);
        fCanvas->Update();

        for(int i=0 ;i<px*py ; i++) {
            TVirtualPad *pad = fCanvas->GetPad(i+1);
            pad->cd();
            pad->SetLeftMargin(0.06);
            if(px>1) pad->SetLeftMargin(0.06*px*0.85);
            pad->SetRightMargin(fCanvas->GetRightMargin());
            pad->SetBottomMargin(fCanvas->GetBottomMargin());
            if(py>1) pad->SetBottomMargin(fCanvas->GetBottomMargin()*py*0.7);
            pad->SetTopMargin(fCanvas->GetTopMargin());
            pad->DrawFrame(0,0,1,1);
            pad->Update();
            pad->SetBit(TPad::kCannotMove);
            pad->GetFrame()->SetBit(TObject::kCannotPick);
        }

        fCanvas->SetClickSelectedPad((TPad*) fCanvas->GetPad(1));
        fSelectedPad = (TPad*) fCanvas->GetPad(1);
    }
    else {
        fCanvas->DrawFrame(0,0,1,1);
        fCanvas->SetClickSelectedPad((TPad*)fCanvas);
        fSelectedPad = (TPad*) fCanvas;
    }

    fCanvas->SetNPads(px*py);

    fCanvas->Update();
    fCanvas->GetFrame()->SetBit(TObject::kCannotPick);

    fCanvasTab->SetTab(fCanvasTab->GetNumberOfTabs()-1);

    fToolBar->SetCanvas(fCanvas);

    fCanvasTab->MapSubwindows();
    fCanvasTab->Layout();
}

void CXMainWindow::HandleMenu(Int_t id)
{
    // Handle menu items.
    switch (id) {
    case M_New_Canvas:
        NewTab();
        break;
    case M_New_MultiPad_Canvas: {
        TString px="1", py="2";
        CXDialogBox *diag = new CXDialogBox(this->GetMainFrame(),"New multi pad canvas:");
        diag->Add("NX",px);
        diag->Add("NY",py);
        diag->Popup();
        NewTab(px.Atoi(),py.Atoi());
        break;
    }
    case M_New_Browser:
        new TBrowser;
        break;
    case M_Save_As:
        fCanvas->SaveCanvasAs();
        break;
    case M_Save_To_Ascii:
        SaveToAscii();
        break;
    case M_Exit:
        CloseWindow();
        break;
    case M_Editor:
        ToggleTab(IsEditorEnabled,fEditorTab,"Editor");
        break;
    case M_BkdUtility:
        ToggleTab(IsBkdUtilityEnabled,fBkdToolTab,fBkdSubtract->GetName());
        break;
    case M_SavedList:
        ToggleTab(IsSavedListEnabled,fSavedListTab,fSavedList->GetName());
        break;
    case M_WSManager:
        ToggleTab(IsWSManagerEnabled,fWSManagerTab,fWSManager->GetName());
        break;
    case M_Hist1DPlayer:
        ToggleTab(IsHist1DPlayerEnabled,fHist1DPlayerTab,fHist1DPlayer->GetName());
        break;
    case M_Hist1DCalib:
        ToggleTab(IsHist1DCalibPlayerEnabled,fHist1DCalibTab,fHist1DCalib->GetName());
        break;
    case M_HistEffFit:
        ToggleTab(IsHistEffFitPlayerEnabled,fHistEffFitTab,fHistEffFit->GetName());
        break;
    case M_AngCorrPlayer: {
#ifdef HAS_MATHMORE
        ToggleTab(IsAngCorrPlayerEnabled,fAngCorrPlayerTab,fAngCorrPlayer->GetName());
        break;
#else
        gbash_color->ErrorMessage("ROOT needs to be compiled with mathmore to use the angular correlations utility");
        break;
#endif
    }
    case M_Hist2DPlayer:
        ToggleTab(IsHist2DPlayerEnabled,fHist2DPlayerTab,fHist2DPlayer->GetName());
        break;
    case M_Rad2DPlayer:
        ToggleTab(IsRad2DPlayerEnabled,fRad2DPlayerTab,fRad2DPlayer->GetName());
        break;
    case M_RadCubePlayer:
        ToggleTab(IsRadCubePlayerEnabled,fRadCubePlayerTab,fRadCubePlayer->GetName());
        break;
    case M_LSPlayerUtility:
        ToggleTab(IsLSPlayerToolEnabled,fLSPlayerToolTab,fLSPlayerTool->GetName());
        break;
    case M_FileListUtility:
        ToggleTab(IsFileListUtilityEnabled,fFileListTab,fFileList->GetName());
        break;
    case M_ShowStats:
        IsStatsShown = 1-IsStatsShown;
        if(IsStatsShown) {
            fMenuOptions->CheckEntry(M_ShowStats);
            gStyle->SetOptStat(1000011);
        }
        else {
            fMenuOptions->UnCheckEntry(M_ShowStats);
            gStyle->SetOptStat(0);
        }
        RefreshPads();
        break;
    case M_ShowTitle:
        IsTitleShown = 1-IsTitleShown;
        if(IsTitleShown) {
            fMenuOptions->CheckEntry(M_ShowTitle);
            gStyle->SetOptTitle(1);
        }
        else {
            fMenuOptions->UnCheckEntry(M_ShowTitle);
            gStyle->SetOptTitle(0);
        }
        RefreshPads();
        break;
    case M_GammaSearch:
        if(fGammaSearchWindow == nullptr) {
//            wait();
            fGammaSearchWindow = new CXGammaSearch(gClient->GetRoot(),gClient->GetRoot(),1200,500, this);
        }
        else
            fGammaSearchWindow->MapRaised();
        break;
    case M_NucChart:
        if(fNucChartWindow == nullptr) {
//            wait();
            fNucChartWindow = new CXNucChart(gClient->GetRoot(),gClient->GetRoot(),1200,550, this);
        }
        else
            fNucChartWindow->MapRaised();
        break;

        if(gPad != nullptr) {
            gPad->Modified();
            gPad->Update();
        }
        break;
    }
}

void CXMainWindow::ToggleTab(Bool_t &Enable, TGCompositeFrame *tab, const char * name)
{
    if(!Enable) {
        fMainTab->AddTab(name,tab);

        fMainTab->SetTab(name);
        fMainTab->GetTabTab(name)->ShowClose();

        if(tab != fEditorTab) tab->MapSubwindows();

        if(fMainTab->GetNumberOfTabs()>3) {
            TString Name = fMainTab->GetTabTab(0)->GetText()->GetString();

            if(Name==fFileList->GetName())
                ToggleTab(IsFileListUtilityEnabled,fFileListTab,Name);
            else if(Name==fLSPlayerTool->GetName())
                ToggleTab(IsLSPlayerToolEnabled,fLSPlayerToolTab,Name);
            else if(Name==fHist1DPlayer->GetName())
                ToggleTab(IsHist1DPlayerEnabled,fHist1DPlayerTab,Name);
            else if(Name==fHist1DCalib->GetName())
                ToggleTab(IsHist1DCalibPlayerEnabled,fHist1DCalibTab,Name);
            else if(Name==fHistEffFit->GetName())
                ToggleTab(IsHistEffFitPlayerEnabled,fHistEffFitTab,Name);
            else if(Name==fAngCorrPlayer->GetName())
                ToggleTab(IsAngCorrPlayerEnabled,fAngCorrPlayerTab,Name);
            else if(Name==fHist2DPlayer->GetName())
                ToggleTab(IsHist2DPlayerEnabled,fHist2DPlayerTab,Name);
            else if(Name==fRad2DPlayer->GetName())
                ToggleTab(IsRad2DPlayerEnabled,fRad2DPlayerTab,Name);
            else if(Name==fRadCubePlayer->GetName())
                ToggleTab(IsRadCubePlayerEnabled,fRadCubePlayerTab,Name);
            else if(Name=="Editor")
                ToggleTab(IsEditorEnabled,fEditorTab,Name);
            else if(Name==fBkdSubtract->GetName())
                ToggleTab(IsBkdUtilityEnabled,fBkdToolTab,Name);
            else if(Name==fSavedList->GetName())
                ToggleTab(IsSavedListEnabled,fSavedListTab,Name);
            else if(Name==fWSManager->GetName())
                ToggleTab(IsWSManagerEnabled,fWSManagerTab,Name);
        }

        fMainTab->SetTab(name);

        Layout();
    }
    else {
        fMainTab->SetTab(name);
        fMainTab->RemoveTab(fMainTab->GetCurrent());
    }

    Enable = !Enable;
}


void CXMainWindow::SaveCanvasAs()
{

    const char* SaveAsTypes[] = {
        "PDF",          "*.pdf",
        "PostScript",   "*.ps",
        "Encapsulated PostScript", "*.eps",
        "SVG",          "*.svg",
        "TeX",          "*.tex",
        "GIF",          "*.gif",
        "ROOT macros",  "*.C",
        "ROOT files",   "*.root",
        "XML",          "*.xml",
        "PNG",          "*.png",
        "XPM",          "*.xpm",
        "JPEG",         "*.jpg",
        "TIFF",         "*.tiff",
        "XCF",          "*.xcf",
        "All files",    "*",
        nullptr,        nullptr
    };

    TString workdir = gSystem->WorkingDirectory();
    static TString dir(".");
    static Int_t typeidx = 0;
    static Bool_t overwr = kFALSE;
    TGFileInfo fi;
    fi.fFileTypes   = SaveAsTypes;
    fi.fIniDir      = StrDup(dir);
    fi.fFileTypeIdx = typeidx;
    fi.fOverwrite   = overwr;
    new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), kFDSave, &fi);
    gSystem->ChangeDirectory(workdir.Data());
    if (!fi.fFilename) return;
    TString fn = fi.fFilename;
    dir     = fi.fIniDir;
    typeidx = fi.fFileTypeIdx;
    overwr  = fi.fOverwrite;

    if(!fn.EndsWith(fi.fFileTypes[1]))
        fn.Append(fi.fFileTypes[1]);

    fCanvas->SaveAs(fn);
}

void CXMainWindow::DoRefresh()
{
    fMainTab->Resize(10,10);
    Layout();
}

void CXMainWindow::RefreshPads()
{
    TList *pList = fCanvas->GetListOfPrimitives();
    TObjOptLink *lnk = nullptr;
    if (pList) lnk = (TObjOptLink*)pList->FirstLink();
    TObject *obj;
    while (lnk) {
        obj = lnk->GetObject();
        if (obj->InheritsFrom(TPad::Class())) {
            ((TPad*)obj)->Modified();
        }
        lnk = (TObjOptLink*)lnk->Next();
    }

    gPad->Modified();
    gPad->Update();

    fCanvas->Modified();
    fCanvas->Update();
}

void CXMainWindow::CloseWindow()
{
    if(fGammaSearchWindow)
        fGammaSearchWindow->CloseWindow();

    if(fNucChartWindow)
        fNucChartWindow->CloseWindow();

    for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
        if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("Calibration Results")) {
            gROOT->GetListOfCanvases()->RemoveAt(i);
            i--;
        }
    }
    for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
        if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("FWHM fit")) {
            gROOT->GetListOfCanvases()->RemoveAt(i);
            i--;
        }
    }
    for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
        if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("Efficiency fit")) {
            gROOT->GetListOfCanvases()->RemoveAt(i);
            i--;
        }
    }

    cout << " Bye Bye Cubix!" <<endl;
    fstop_db_loading = true;

    // UnmapWindow();
    // DeleteWindow();//  launch a delete but after a short time like a thread.

    gApplication->Disconnect("Terminate(Int_t)", this, "CloseWindow()");
    gApplication->SetReturnFromRun(false);
    gApplication->Terminate(false);
}

TH1 *CXMainWindow::GetHisto(TVirtualPad *pad, bool GetFirst)
{
    TH1 *hist = nullptr;
    if(pad == nullptr && fSelectedPad == nullptr) {
        fCanvas->cd();
        fSelectedPad = fCanvas->cd();
    }
    if(pad == nullptr)
        pad = fSelectedPad;

    TObjOptLink *lnk = nullptr;
    TObject *obj = nullptr;

    TList *pList = pad->GetListOfPrimitives();
    if (pList) lnk = (TObjOptLink*)pList->FirstLink();

    while (lnk) {
        obj = lnk->GetObject();
        if (obj->InheritsFrom(TH1::Class()) && strcmp(obj->GetName() ,"hframe") != 0) {
            hist = dynamic_cast<TH1*>(obj);
            if(GetFirst) return hist;
        }
        lnk = (TObjOptLink*)lnk->Next();
    }
    return hist;
}

TGraph *CXMainWindow::GetGraph(TVirtualPad *pad, bool GetFirst)
{
    TGraph *graph = nullptr;

    if(pad == nullptr && fSelectedPad == nullptr) {
        fCanvas->cd();
        fSelectedPad = fCanvas->cd();
    }
    if(pad == nullptr)
        pad = fSelectedPad;

    TObjOptLink *lnk = nullptr;
    TObject *obj = nullptr;

    TList *pList = pad->GetListOfPrimitives();
    if (pList) lnk = (TObjOptLink*)pList->FirstLink();

    while (lnk) {
        obj = lnk->GetObject();
        if (obj->InheritsFrom(TGraph::Class()) && strcmp(obj->GetName() ,"hframe") != 0) {
            graph = dynamic_cast<TGraph*>(obj);
            if(GetFirst) return graph;
        }
        lnk = (TObjOptLink*)lnk->Next();
    }
    return graph;
}

void CXMainWindow::UpdateContextMenus()
{
    /// TH1F ///
    TClass *cl = gROOT->GetClass("TH1F");
    cl->MakeCustomMenuList();
    TList * ml = cl->GetMenuList();

    TClassMenuItem *n;

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Copy","CopyObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Cut","CutObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Undraw","UndrawObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Normalize","HistNorm",this,"TObject*", 0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Scale","HistScale",this,"Float_t,TObject*", 1); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Apply calibration","HistCalib",this,"TObject*", 1); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl); ml->AddFirst(n);

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Add to Stored spectra","AddToStoredSpectra",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Fit Peaks","PopUpFitPeaks",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Find Peaks","PopUpFindPeaks",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Show Background","PopUpShowBackground",this,"TObject *",0); ml->AddFirst(n);

    for(int i=0 ; i<6 ; i++) ml->RemoveAt(ml->GetEntries()-1);
    ml->RemoveAt(ml->GetEntries()-3);
    for(int i=0 ; i<2 ; i++) ml->RemoveAt(ml->GetEntries()-4);
    for(int i=0 ; i<2 ; i++) ml->RemoveAt(ml->GetEntries()-10);
    ml->RemoveAt(ml->GetEntries()-15);
    for(int i=0 ; i<3 ; i++) ml->RemoveAt(ml->GetEntries()-16);

    /// TH1D///
    cl = gROOT->GetClass("TH1D");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Copy","CopyObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Cut","CutObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Undraw","UndrawObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Normalize","HistNorm",this,"TObject*", 0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Scale","HistScale",this,"Float_t,TObject*", 1); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Apply calibration","HistCalib",this,"TObject*", 1); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl); ml->AddFirst(n);

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Add to Stored spectra","AddToStoredSpectra",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Fit Peaks","PopUpFitPeaks",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Find Peaks","PopUpFindPeaks",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Show Background","PopUpShowBackground",this,"TObject *",0); ml->AddFirst(n);

    for(int i=0 ; i<6 ; i++) ml->RemoveAt(ml->GetEntries()-1);
    ml->RemoveAt(ml->GetEntries()-3);
    for(int i=0 ; i<2 ; i++) ml->RemoveAt(ml->GetEntries()-4);
    for(int i=0 ; i<2 ; i++) ml->RemoveAt(ml->GetEntries()-10);
    ml->RemoveAt(ml->GetEntries()-15);
    for(int i=0 ; i<3 ; i++) ml->RemoveAt(ml->GetEntries()-16);

    /// TH1Proj ///
    cl = gROOT->GetClass("CXTH1Proj");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Copy","CopyObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Undraw","UndrawObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Normalize","HistNorm",this,"TObject*", 0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Scale","HistScale",this,"Float_t,TObject*", 1); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl); ml->AddFirst(n);

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Add to Stored spectra","AddToStoredSpectra",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Fit Peaks","PopUpFitPeaks",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Find Peaks","PopUpFindPeaks",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Show Background","PopUpShowBackground",this,"TObject *",0); ml->AddFirst(n);

    for(int i=0 ; i<6 ; i++) ml->RemoveAt(ml->GetEntries()-1);
    ml->RemoveAt(ml->GetEntries()-3);
    for(int i=0 ; i<2 ; i++) ml->RemoveAt(ml->GetEntries()-4);
    for(int i=0 ; i<2 ; i++) ml->RemoveAt(ml->GetEntries()-10);
    ml->RemoveAt(ml->GetEntries()-15);
    for(int i=0 ; i<3 ; i++) ml->RemoveAt(ml->GetEntries()-16);

    /// CXRadCubeTH1Proj ///
    cl = gROOT->GetClass("CXRadCubeTH1Proj");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Copy","CopyObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Undraw","UndrawObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Normalize","HistNorm",this,"TObject*", 0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Scale","HistScale",this,"Float_t,TObject*", 1); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl); ml->AddFirst(n);

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Add to Stored spectra","AddToStoredSpectra",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Fit Peaks","PopUpFitPeaks",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Find Peaks","PopUpFindPeaks",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Show Background","PopUpShowBackground",this,"TObject *",0); ml->AddFirst(n);

    for(int i=0 ; i<6 ; i++) ml->RemoveAt(ml->GetEntries()-1);
    ml->RemoveAt(ml->GetEntries()-3);
    for(int i=0 ; i<2 ; i++) ml->RemoveAt(ml->GetEntries()-4);
    for(int i=0 ; i<2 ; i++) ml->RemoveAt(ml->GetEntries()-10);
    ml->RemoveAt(ml->GetEntries()-15);
    for(int i=0 ; i<3 ; i++) ml->RemoveAt(ml->GetEntries()-16);

    /// TH2F ///
    cl = gROOT->GetClass("TH2F");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Copy","CopyObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Cut","CutObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Undraw","UndrawObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl); ml->AddFirst(n);

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Add to Stored spectra","AddToStoredSpectra",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Eval Background","PopUpEval2DBackground",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Init Radware GxG","PopUpInitRadGG",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Init GxG","PopUpInitGG",this,"TObject *",0); ml->AddFirst(n);

    for(int i=0 ; i<6 ; i++) ml->RemoveAt(ml->GetEntries()-1);
    for(int i=0 ; i<4 ; i++) ml->RemoveAt(ml->GetEntries()-3);
    ml->RemoveAt(ml->GetEntries()-9);
    for(int i=0 ; i<7 ; i++) ml->RemoveAt(ml->GetEntries()-12);

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Rebin","Rebin2D",this,"Int_t, Int_t, TObject*", 2); ml->AddAt(n,12);
    //    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl);ml->AddAt(n,ml->GetEntries()-4);

    /// TH2D ///
    cl = gROOT->GetClass("TH2D");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Copy","CopyObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Cut","CutObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Undraw","UndrawObject",fCanvas,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl); ml->AddFirst(n);

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Add to Stored spectra","AddToStoredSpectra",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Eval Background","PopUpEval2DBackground",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Init Radware GxG","PopUpInitRadGG",this,"TObject *",0); ml->AddFirst(n);
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl, "Init GxG","PopUpInitGG",this,"TObject *",0); ml->AddFirst(n);

    for(int i=0 ; i<6 ; i++) ml->RemoveAt(ml->GetEntries()-1);
    for(int i=0 ; i<4 ; i++) ml->RemoveAt(ml->GetEntries()-3);
    ml->RemoveAt(ml->GetEntries()-9);
    for(int i=0 ; i<7 ; i++) ml->RemoveAt(ml->GetEntries()-12);

    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Rebin","Rebin2D",this,"Int_t, Int_t, TObject*", 2); ml->AddAt(n,12);
    //    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl);ml->AddAt(n,ml->GetEntries()-4);

    /// CXArrow ///
    cl = gROOT->GetClass("CXArrow");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();

    for(int i=0 ; i<18 ; i++) ml->RemoveAt(ml->GetEntries()-1);

    /// TGraph ///
    cl = gROOT->GetClass("TGraph");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Scale","GraphScale",this,"Float_t,TObject*", 1); ml->AddAt(n,3);

    /// TGraphErrors ///
    cl = gROOT->GetClass("TGraphErrors");
    cl->MakeCustomMenuList();
    ml = cl->GetMenuList();
    n = new TClassMenuItem(TClassMenuItem::kPopupUserFunction,cl,"Scale","GraphScale",this,"Float_t,TObject*", 1); ml->AddAt(n,3);
    //    n = new TClassMenuItem(TClassMenuItem::kPopupSeparator,cl);ml->AddAt(n,ml->GetEntries()-4);
}

void CXMainWindow::Rebin2D(Int_t RebinX, Int_t RebinY, TObject *c)
{
    TH2 *hist = dynamic_cast<TH2*>(c);
    TString DrawOpt = hist->GetDrawOption();
    if(DrawOpt=="")
        DrawOpt = "col";
    hist = hist->Rebin2D(RebinX,RebinY,hist->GetName());
    hist->Draw(DrawOpt);
    RefreshPads();
}

void CXMainWindow::PopUpEval2DBackground(TObject *c)
{
    TH2 *hist = dynamic_cast<TH2*>(c);

    if(IsBkdUtilityEnabled==false)
        ToggleTab(IsBkdUtilityEnabled,fBkdToolTab,fBkdSubtract->GetName());

    fMainTab->SetTab(fBkdSubtract->GetName());

    fBkdSubtract->Do2DEvaluation(hist);
}

void CXMainWindow::PopUpInitGG(TObject *c)
{
    TH2 *hist = dynamic_cast<TH2*>(c);

    if(IsHist2DPlayerEnabled==false)
        ToggleTab(IsHist2DPlayerEnabled,fHist2DPlayerTab,fHist2DPlayer->GetName());

    fMainTab->SetTab(fHist2DPlayer->GetName());

    fHist2DPlayer->InitGG(hist);
}

void CXMainWindow::PopUpInitRadGG(TObject *c)
{
    TH2 *hist = dynamic_cast<TH2*>(c);

    if(IsRad2DPlayerEnabled==false)
        ToggleTab(IsRad2DPlayerEnabled,fRad2DPlayerTab,fRad2DPlayer->GetName());

    fMainTab->SetTab(fRad2DPlayer->GetName());

    fRad2DPlayer->Init(hist);
}

void CXMainWindow::InitRadCubePlayer(CXRadReader *radreader)
{
    if(IsRadCubePlayerEnabled==false)
        ToggleTab(IsRadCubePlayerEnabled,fRadCubePlayerTab,fRadCubePlayer->GetName());

    fMainTab->SetTab(fRadCubePlayer->GetName());

    fRadCubePlayer->Init(radreader);
}


void CXMainWindow::HistNorm(TObject *c)
{
    TH1 *hist = dynamic_cast<TH1*>(c);

    Float_t x1,x2;
    x1=gPad->GetUxmin();
    x2=gPad->GetUxmax();
    hist->GetXaxis()->UnZoom();
    hist->Scale(1./hist->Integral());
    hist->GetXaxis()->SetRangeUser(x1,x2);

    RefreshPads();
}

void CXMainWindow::HistScale(Float_t scaleFact, TObject *c)
{
    TH1 *hist = dynamic_cast<TH1*>(c);


    hist->Scale(scaleFact);

    RefreshPads();
}

void CXMainWindow::HistCalib(TObject *c)
{
    TH1 *hist = dynamic_cast<TH1*>(c);

    if(!GetWSManager()->GetActiveWorkspace()) {
        glog << tkn::error << "No active workspace -> No energy calibration function available" << tkn::do_endl;
        return;
    }
    TF1 *func = GetWSManager()->GetActiveWorkspace()->fCalibFunction;
    if(!func) {
        glog << tkn::error << "No energy calibration function in the active workspace: " << GetWSManager()->GetActiveWorkspace()->GetName() << tkn::do_endl;
        return;
    }
    fHist1DCalib->ApplyCalibration(hist,func);

    RefreshPads();
}

void CXMainWindow::GraphScale(Float_t scaleFact, TObject *c)
{
    auto *graph = dynamic_cast<TGraph*>(c);

    for(int i=0 ; i<graph->GetN() ; i++){
        graph->SetPoint(i,graph->GetX()[i],graph->GetY()[i]*scaleFact);
    }

    RefreshPads();
}

void CXMainWindow::PopUpShowBackground(TObject *c)
{
    auto *hist = dynamic_cast<TH1*>(c);

    if(GetHisto()->GetName() != hist->GetName())
    {
        fCanvas->cd();
        hist->Draw("hist");
        RefreshPads();
    }
    if(IsBkdUtilityEnabled==false)
        ToggleTab(IsBkdUtilityEnabled,fBkdToolTab,fBkdSubtract->GetName());

    fMainTab->SetTab(fBkdSubtract->GetName());

    fBkdSubtract->DoActivate();

    RefreshPads();
}

void CXMainWindow::AddToStoredSpectra(TObject *c)
{
    TString Current = fMainTab->GetCurrentTab()->GetText()->GetString();

    if(!(Current == fHist2DPlayer->GetName()  ||
          Current == fRadCubePlayer->GetName() ||
          Current == fSavedList->GetName())) {

        if(IsSavedListEnabled==false)
            ToggleTab(IsSavedListEnabled,fSavedListTab,fSavedList->GetName());
        else
            fMainTab->SetTab(fSavedList->GetName());
    }

    TObject *o = c;
    TObject *clone = nullptr;
    if(o->InheritsFrom("CXRadCubeTH1Proj"))
        clone = ((CXRadCubeTH1Proj*)o)->GetTotalProj()->Clone();
    else
        clone = o->Clone();
    if(o->InheritsFrom(TH1::Class_Name()))
        ((TH1*)clone)->SetDirectory(nullptr);
    fSavedList->AddToStoredList(clone);
}

void CXMainWindow::PopUpFindPeaks(TObject *c)
{
    auto *hist = dynamic_cast<TH1*>(c);

    if(GetHisto()->GetName() != hist->GetName()) {
        fCanvas->cd();
        hist->Draw("hist");
        RefreshPads();
    }
    if(IsHist1DPlayerEnabled==false)
        ToggleTab(IsHist1DPlayerEnabled,fHist1DPlayerTab,fHist1DPlayer->GetName());

    fMainTab->SetTab(fHist1DPlayer->GetName());

    fHist1DPlayer->PeakSearch();
}

void CXMainWindow::PopUpFitPeaks(TObject *c)
{
    auto *hist = dynamic_cast<TH1*>(c);

    if(GetHisto()->GetName() != hist->GetName())
    {
        fCanvas->cd();
        hist->Draw("hist");
        RefreshPads();
    }
    if(IsHist1DPlayerEnabled==false)
        ToggleTab(IsHist1DPlayerEnabled,fHist1DPlayerTab,fHist1DPlayer->GetName());

    fMainTab->SetTab(fHist1DPlayer->GetName());

    fHist1DPlayer->NewFit();
}


void CXMainWindow::ProcessedKeyEvent(Event_t *event)
{
    char input[10];
    UInt_t keysym;

    if(gPad && ((TString)gPad->GetCanvas()->GetCanvas()->GetName()) != "NuclearChartCanvas") SetPalette();

   // gVirtualX->LookupString(event, input, sizeof(input), keysym);
   // std::cout << "event : " << event->fCode << " " << event->fState <<" ; "<< event->fType  << "; " << keysym << " -> " << input << std::endl;

    if(event->fState & kKeyControlMask) fCTRL = true;
    else fCTRL = false;
    if(event->fState & kKeyShiftMask) fSHIFT = true;
    else fSHIFT = false;
}

void CXMainWindow::SetPalette()
{
    gStyle->SetPalette();
}

void CXMainWindow::DoDraw(TObject *obj, TString DrawOpt)
{
    SetPalette();

    TH1 *hist = nullptr;

    if(obj==nullptr) return;

    gPad = GetSelectedPad();
    if(gPad==nullptr)
        gPad = GetCanvas()->cd();
    if(gPad==nullptr)
        return;

    if(obj->InheritsFrom(TH2::Class()) && gPad->GetListOfPrimitives()->FindObject("ex1") == nullptr) {
        gPad->AddExec("ex1","CXMainWindow::SetPalette()");
    }

    if(((TString)gPad->GetCanvas()->GetName()).BeginsWith("AngCorr")) {
        TGraph *g = nullptr;
        if(obj->InheritsFrom(TCanvas::Class())) {
            auto *canvas_in = (TCanvas*)obj;
            for(int i=0 ; i<canvas_in->GetListOfPrimitives()->GetEntries() ; i++){
                TObject *o = canvas_in->GetListOfPrimitives()->At(i);
                if(o->InheritsFrom(TGraph::Class())) g = dynamic_cast<TGraph*>(o);
                if(g) break;
            }
        }
        else {
            if(obj->InheritsFrom(TGraph::Class())) g = dynamic_cast<TGraph*>(obj);
        }

        if(g) {
            gPad->GetCanvas()->cd(3);
            g->Draw("ape");
            g->SetMarkerStyle(20);
            g->SetMarkerColor(kRed);
            g->SetMarkerSize(1);
            RefreshPads();
            gPad->GetFrame()->SetBit(TObject::kCannotPick);
            return;
        }
        else {
            gbash_color->WarningMessage("Only a TGraph can be drawn in an angular correlation canvas");
            return;
        }
    }


    DrawOpt.ReplaceAll(" ","");

    if(obj->InheritsFrom(TCanvas::Class())) {
        auto *canvas_in = (TCanvas*)obj;
        gPad->Clear();

        Int_t idraw = 0;

        for(int i=0 ; i<canvas_in->GetListOfPrimitives()->GetEntries() ; i++){
            TObject *o = canvas_in->GetListOfPrimitives()->At(i);
            if(o->InheritsFrom(TFrame::Class()))
                continue;
            if(o->InheritsFrom(TPad::Class()))
                o->DrawClone();
            else {
                if(idraw==0) {
                    idraw++;
                    if(o->InheritsFrom(TH2::Class()))
                        o->Draw("col");
                    else if(o->InheritsFrom(TH1::Class())) {
                        o->Draw("hist");
                    }
                    else if(o->InheritsFrom(TGraph::Class()))
                        o->Draw("APL");
                    else
                        o->Draw();
                }
                else{
                    if(o->InheritsFrom(TH2::Class()))
                        o->Draw("col same");
                    else if(o->InheritsFrom(TH1::Class()))
                        o->Draw("hist same");
                    else if(o->InheritsFrom(TGraph::Class()))
                        o->Draw("PL");
                    else
                        o->Draw("same");
                }
            }
        }
    }
    else if(obj->InheritsFrom(TH1::Class())) {

        hist = (TH1*)obj;
        hist->SetDirectory(nullptr);
        hist->SetStats(true);

        if(DrawOpt.Contains("norm")) {
            hist->Scale(1./((Float_t)hist->Integral(1,hist->GetNbinsX())));
            DrawOpt.ReplaceAll("norm","");
        }
        if(DrawOpt.Contains("add")){
            TH1 * HistToadd = GetHisto();
            if(HistToadd == nullptr)
                gbash_color->WarningMessage("No histogram to be added found in the current pad, ignored");
            else {
                Float_t Fact=1;
                if(DrawOpt.Contains("(") && DrawOpt.Contains(")")){
                    TObjArray *arr = DrawOpt.Copy().Tokenize("(");
                    TString opt = arr->At(1)->GetName();
                    delete arr;
                    arr = opt.Tokenize(")");
                    opt = arr->First()->GetName();
                    delete arr;
                    if(opt.IsFloat())
                        Fact = opt.Atof();
                    else
                        cout<<"could not decrypt the addition factor, 1 is used"<<endl;
                }
                HistToadd->Add(hist,Fact);
            }
        }
        else if(DrawOpt.Contains("div")){

            TH1 * HistToDiv = GetHisto();
            if(HistToDiv == nullptr)
                gbash_color->WarningMessage("No histogram to be divided found in the current pad, ignored");
            else {
                Float_t Fact=1;
                if(DrawOpt.Contains("(") && DrawOpt.Contains(")")){
                    TObjArray *arr = DrawOpt.Copy().Tokenize("(");
                    TString opt = arr->At(1)->GetName();
                    delete arr;
                    arr = opt.Tokenize(")");
                    opt = arr->First()->GetName();
                    delete arr;
                    if(opt.IsFloat())
                        Fact = opt.Atof();
                    else
                        cout<<"could not decrypt the addition factor, 1 is used"<<endl;
                }

                HistToDiv->Divide(HistToDiv,hist,1,Fact);
            }
        }
        else if(DrawOpt.Contains("mult")){

            TH1 * HistToMult = GetHisto();
            if(HistToMult == nullptr)
                gbash_color->WarningMessage("No histogram to be multiplied found in the current pad, ignored");
            else {
                Float_t Fact=1;
                if(DrawOpt.Contains("(") && DrawOpt.Contains(")")){
                    TObjArray *arr = DrawOpt.Copy().Tokenize("(");
                    TString opt = arr->At(1)->GetName();
                    delete arr;
                    arr = opt.Tokenize(")");
                    opt = arr->First()->GetName();
                    delete arr;
                    if(opt.IsFloat())
                        Fact = opt.Atof();
                    else
                        cout<<"could not decrypt the addition factor, 1 is used"<<endl;
                }
                HistToMult->Multiply(HistToMult,hist,1,Fact);
            }
        }
        else
            hist->Draw(DrawOpt);
    }
    else
        obj->Draw(DrawOpt);

    RefreshPads();
    gPad->GetFrame()->SetBit(TObject::kCannotPick);
}

void CXMainWindow::OpenFile(TString filename)
{
    fFileList->DisplayFile(filename);
}

void CXMainWindow::load_tkn_db() {
    glog.set_warnings(false);
    int inuc=0;
    for(auto nuc : gmanager->get_nuclei()) {
        for(auto &lvl : nuc->get_level_scheme()->get_levels()){
            if(fstop_db_loading) return;
            while(fdb_loading_paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            glog.progress_bar(gmanager->get_nuclei().size(),inuc,"Loading tkn db...");
        }
        inuc++;
    }
    cout << endl;

    gbash_color->InfoMessage("TkN database loaded !");
    fdb_loaded = true;
    return;
}

void CXMainWindow::wait() {
    gbash_color->WarningMessage("Waiting for the TkN database loading...");
    loadingFuture.wait();
}

ClassImp(CXMainWindow)
