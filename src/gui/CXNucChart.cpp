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

#include "CXNucChart.h"
#include "cubix_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "TGListBox.h"
#include "KeySymbols.h"
#include "TGLabel.h"
#include "TGStatusBar.h"
#include "TRootEmbeddedCanvas.h"
#include "TGComboBox.h"
#include "TFrame.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TObjArray.h"
#include "TList.h"
#include "TLine.h"
#include "TLatex.h"
#include "TMath.h"
#include "THistPainter.h"
#include "TGLabel.h"
#include "TGTextEntry.h"
#include "TExec.h"
#include "TPaletteAxis.h"
#include "TVirtualX.h"
#include "TSystem.h"

#if (OS_TYPE == OS_LINUX)
#include "TGResourcePool.h"
#endif

#include "CXMainWindow.h"
#include "CXCanvas.h"
#include "CXNucleusBox.h"

#include "tkmanager.h"

using namespace std;

CXNucChart::CXNucChart(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, CXMainWindow *mainwindow) :
    TGTransientFrame(p, main, w, h, kVerticalFrame),
    fMainWindow(mainwindow)
{
    TGCompositeFrame *Toolbar = new TGCompositeFrame(this,600,100,kHorizontalFrame);
    AddFrame(Toolbar,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,10,10,0,-5));

    TGGroupFrame *gFrame = new TGGroupFrame(Toolbar, "Tools", kVerticalFrame);
    gFrame->SetTextColor(CXblue);
    gFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    Toolbar->AddFrame(gFrame, new TGLayoutHints( kLHintsExpandX  | kLHintsExpandY , 0, 0, 0, -10) );

    TGHorizontalFrame *hframe = new TGHorizontalFrame(gFrame);
    gFrame->AddFrame(hframe, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,-10,-5,0,-2));

    TGLabel *label = new TGLabel(hframe,"View: ");
    label->SetTextColor(CXred);
    hframe->AddFrame(label, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,5,0,0));

    fViewMode = new TGComboBox(hframe);
    fViewMode->Resize(150,20);
    fViewMode->AddEntry("Life time (s)",M_LifeTime);
    fViewMode->AddEntry("1st Isomer life time (s)",M_1stIsomer);
    fViewMode->AddEntry("2nd Isomer life time (s)",M_2ndIsomer);
    fViewMode->AddEntry("1rst excited state (keV)",M_1rstExcitedState);
    fViewMode->AddEntry("Decay mode",M_DecayMode);
    fViewMode->AddEntry("B(E2) (e2b2)",M_BE2E2B2);
    fViewMode->AddEntry("B(E2) (W.u.)/A",M_BE2WU);

    fViewMode->Select(M_LifeTime);
    fViewMode->Connect("Selected(Int_t)", "CXNucChart", this, "UpdateNucChart()");
    hframe->AddFrame(fViewMode, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));

    label = new TGLabel(hframe,"Print: ");
    label->SetTextColor(CXred);
    hframe->AddFrame(label, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,10,5,0,0));

    fPrintMode = new TGComboBox(hframe);
    fPrintMode->Resize(100,20);
    fPrintMode->AddEntry("Nucleus",M_NucInfo);
    fPrintMode->AddEntry("Levels",M_LevelsInfo);
    fPrintMode->AddEntry("Gamma-rays",M_GammaInfos);
    fPrintMode->Select(M_NucInfo);
    fPrintMode->Connect("Selected(Int_t)", "CXNucChart", this, "UpdatePrintMode()");
    hframe->AddFrame(fPrintMode, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));

    label = new TGLabel(hframe,"Nucleus: ");
    label->SetTextColor(CXred);
    hframe->AddFrame(label, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,10,5,0,0));

    fNucleusTextEntry = new TGTextEntry(hframe,"");
    fNucleusTextEntry->SetWidth(50);
    hframe->AddFrame(fNucleusTextEntry, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));
    fNucleusTextEntry->Connect("TextChanged(const char *)", "CXNucChart", this, "NucNotValidated()");
    fNucleusTextEntry->Connect("ReturnPressed()", "CXNucChart", this, "UpdateNucFromSymb()");

    label = new TGLabel(hframe,"Data set: ");
    label->SetTextColor(CXred);
    hframe->AddFrame(label, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,10,5,0,0));

    fDataSetMode = new TGComboBox(hframe);
    fDataSetMode->Resize(200,20);
    fDataSetMode->Connect("Selected(Int_t)", "CXNucChart", this, "UpdateDataSet()");
    hframe->AddFrame(fDataSetMode, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));

    TGCompositeFrame *Global = new TGCompositeFrame(this,600,100,kHorizontalFrame);
    AddFrame(Global,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,5,5,5,5));

    TGVerticalFrame *Main = new TGVerticalFrame(Global,600,100,kFixedWidth);
    Global->AddFrame(Main,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY,5,5,5,5));
    Main->SetWidth(220);

    gFrame = new TGGroupFrame(Main, "Selected nucleus", kVerticalFrame);
    gFrame->SetTextColor(CXblue);
    gFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    Main->AddFrame(gFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX  | kLHintsExpandY , 0, 0, -5, -10) );
    fInfoBox = new TGListBox(gFrame);
    fInfoBox->GetContainer()->RemoveInput(kButtonReleaseMask | kButtonMotionMask);
    gFrame->AddFrame(fInfoBox, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,-10,-10,3,0));

    //Canvas Panel

    Main = new TGVerticalFrame(Global,400,100);
    Global->AddFrame(Main,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,0,5,5,5));
    Main->SetWidth(800);

    fRootCanvas = new TRootEmbeddedCanvas("NCCanvas", Main);
    Main->AddFrame(fRootCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 5, 5));
    Int_t canvasid = fRootCanvas->GetCanvasWindowId();
    delete fRootCanvas->GetCanvas();  // To delete the original canvas, without this, it causes crashed on macos
    fCanvas = new CXCanvas("NuclearChartCanvas", 10, 10,canvasid);
    fCanvas->SetMainWindow(fMainWindow);
    //    fCanvas->ToggleToolBar();
    fRootCanvas->AdoptCanvas(fCanvas);
    fRootCanvas->GetContainer()->Connect("ProcessedEvent(Event_t*)", "CXNucChart", this, "ProcessedKeyEvent(Event_t*)");
    fCanvas->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "CXNucChart", this, "HandleMovement(Int_t,Int_t,Int_t, TObject*)");
    fCanvas->Connect("RangeChanged()","CXNucChart",this,"UpdateRange()");
    fCanvas->Connect("UnZoomed()","CXNucChart",this,"UpdateRange()");

    fCanvas->SetRightMargin(0.08);
    fCanvas->SetBottomMargin(0.09);
    fCanvas->SetLeftMargin(0.07);
    fCanvas->SetTopMargin(0.04);

    // status bar
    Int_t parts[] = {20 , 20 , 20 , 20 ,20};
    fStatusBar = new TGStatusBar(Main,50,10,kHorizontalFrame);
    fStatusBar->SetParts(parts,5);
    fStatusBar->Draw3DCorner(kFALSE);
    Main->AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0));

    fMagicList = new TList;
    fMagicList->SetOwner();

    fListOfBoxes = new TList;
    fListOfBoxes->SetOwner();

    int NMax, ZMax;
    auto itr = gmanager->get_map_of_nuclei_per_z().end();itr--;
    ZMax = itr->first;
    itr = gmanager->get_map_of_nuclei_per_n().end();itr--;
    NMax = itr->first;

    fNucChartHist = new TH2D("NucChart","NucChart",NMax+5, -0.5 , NMax+5-0.5,ZMax+5, -0.5 , ZMax+5-0.5);
    fNucChartHist->GetXaxis()->SetTitle("N");fNucChartHist->GetXaxis()->CenterTitle();
    fNucChartHist->GetYaxis()->SetTitle("Z");fNucChartHist->GetYaxis()->CenterTitle();

    UpdateNucChart();
    UpdateRange();

    SetCleanup(kDeepCleanup);
    SetWindowName("NuclearChart");
    CenterOnParent();
    MapSubwindows();
    Layout();
    MapWindow();
}

CXNucChart::~CXNucChart()
{
    fMainWindow->fNucChartWindow = nullptr;

    delete fMagicList;
    delete fListOfBoxes;
    delete fNucChartHist;

    UnmapWindow();
    CloseWindow();
}

void CXNucChart::ShowMagicNumbers(bool On)
{
    fMagicList->Clear();

    if(On == false) {
        return;
    }

    Int_t fNm[7] = {2,8,20,28,50,82,126};
    Double_t fNmMin[7] = {0.,1.,12.,18.,46.,93.,98.};
    Double_t fNmMax[7] = {10.,22.,40.,52.,90.,140.,126.*3};
    Double_t fZmMin[7] = {0.2,0.2,6.,10.,26.,44.,74.};
    Double_t fZmMax[7] = {9.,17.,30.,35.,53.,74.,95.};

    for (int i = 0; i < 7; i++) {
        Int_t num = fNm[i];
        if ((num >= fNMin + 1) && (num <= fNMax - 1) && (fZmMin[i] < fZMax)) {
            TLine* ll1 = new TLine(num - 0.5, TMath::Max(fZMin, fZmMin[i]), num - 0.5, TMath::Min(fZMax, fZmMax[i]));
            if(fListOfBoxes->GetEntries()) ll1->SetLineWidth(2);
            ll1->SetBit(kCannotPick); ll1->SetBit(kCannotPick);
            TLine* ll2 = new TLine(num + 0.5, TMath::Max(fZMin, fZmMin[i]), num + 0.5, TMath::Min(fZMax, fZmMax[i]));
            if(fListOfBoxes->GetEntries()) ll2->SetLineWidth(2);
            ll2->SetBit(kCannotPick); ll2->SetBit(kCannotPick);
            fMagicList->AddLast(ll1);
            fMagicList->AddLast(ll2);
        }
    }

    for (int i = 0; i < 7; i++) {
        Int_t num = fNm[i];
        if ((num >= fZMin + 1) && (num <= fZMax - 1) && (fNmMin[i] < fNMax)) {
            TLine* ll1 = new TLine(TMath::Max(fNMin, fNmMin[i]), num - 0.5, TMath::Min(fNMax, fNmMax[i]), num - 0.5);
            if(fListOfBoxes->GetEntries()) ll1->SetLineWidth(2);
            ll1->SetBit(kCannotPick); ll1->SetBit(kCannotPick);
            TLine* ll2 = new TLine(TMath::Max(fNMin, fNmMin[i]), num + 0.5, TMath::Min(fNMax, fNmMax[i]), num + 0.5);
            if(fListOfBoxes->GetEntries()) ll2->SetLineWidth(2);
            ll2->SetBit(kCannotPick); ll2->SetBit(kCannotPick);
            fMagicList->AddLast(ll1);
            fMagicList->AddLast(ll2);
        }
    }

    fMagicList->Execute("Draw", "");
}

void CXNucChart::PlotBoxes()
{
    fListOfBoxes->Clear();

    bool withLT = false;

    if((fZMax-fZMin)<40 && (fNMax-fNMin)<40 ) withLT = true;

    if((fZMax-fZMin)>70 || (fNMax-fNMin)>70 ) return;

    for (int zz = fZMin+1; zz <= fZMax; zz++) {
        for (int nn = fNMin+1; nn <= fNMax; nn++) {
            if(gmanager->known_nucleus(zz,nn+zz)) {
                Int_t charheight;
                Float_t pad_width  = gPad->XtoPixel(gPad->GetX2());
                Float_t pad_height = gPad->YtoPixel(gPad->GetY1());
                if (pad_width < pad_height)  charheight = pad_width/(fNMax-fNMin);
                else                         charheight = pad_height/(fZMax-fZMin);

                CXNucleusBox* nb;
                if(withLT) {
                    nb = new CXNucleusBox(gmanager->get_nucleus(zz,nn+zz),0.5,kBlack,kGray+1,charheight*0.25,true, fViewMode->GetSelected());
                }
                else
                    nb = new CXNucleusBox(gmanager->get_nucleus(zz,nn+zz),0.5,kBlack,kGray+1,charheight*0.5);

                fListOfBoxes->Add(nb);
            }
        }
    }

    fCanvas->DisableClass("TLine");
    fCanvas->DisableClass("CXNucleusBox");
    fCanvas->DisableClass("TLatex");

    fListOfBoxes->Execute("Draw", "");

    CXNucleusBox *box = (CXNucleusBox*)fListOfBoxes->FindObject(fLastSelectedBox);
    if(box) {
        box->SetLineColor(kRed);
        box->SetLineWidth(2);
    }
}

void CXNucChart::SetPalette(Int_t Mode)
{
    if(Mode == M_LifeTime || Mode == M_1stIsomer || Mode == M_2ndIsomer ) {
        Int_t NColors = 33;
        int ColNbr[NColors];
        for(int i=0 ; i<NColors ;i++) ColNbr[i] = 2000+i;
        int icol=0;
        if(gROOT->GetColor(ColNbr[icol]) == nullptr) {
            new TColor(ColNbr[icol++],255./256.,139./256.,109./256.); // <1e-15
            new TColor(ColNbr[icol++],255./256.,175./256.,222./256.); // 1e-15
            new TColor(ColNbr[icol++],255./256.,175./256.,222./256.); // 1e-14
            new TColor(ColNbr[icol++],255./256.,175./256.,222./256.); // 1e-13
            new TColor(ColNbr[icol++],255./256.,175./256.,222./256.); // 1e-12
            new TColor(ColNbr[icol++],255./256.,175./256.,222./256.); // 1e-11
            new TColor(ColNbr[icol++],255./256.,175./256.,222./256.); // 1e-10
            new TColor(ColNbr[icol++],255./256.,175./256.,222./256.); // 1e-9
            new TColor(ColNbr[icol++],255./256.,175./256.,222./256.); // 1e-8
            new TColor(ColNbr[icol++],255./256.,196./256.,160./256.); // 1e-7
            new TColor(ColNbr[icol++],255./256.,234./256.,194./256.); // 1e-6
            new TColor(ColNbr[icol++],246./256.,255./256.,147./256.); // 1e-5
            new TColor(ColNbr[icol++],242./256.,255./256.,0./256.); // 1e-4
            new TColor(ColNbr[icol++],217./256.,255./256.,122./256.); // 1e-3
            new TColor(ColNbr[icol++],195./256.,255./256.,5./256.); // 1e-2
            new TColor(ColNbr[icol++],149./256.,238./256.,89./256.); // 1e-1
            new TColor(ColNbr[icol++],0./256.,196./256.,77./256.); // 1e0
            new TColor(ColNbr[icol++],76./256.,194./256.,180./256.); // 1e1
            new TColor(ColNbr[icol++],84./256.,197./256.,223./256.); // 1e2
            new TColor(ColNbr[icol++],0./256.,163./256.,199./256.); // 1e3
            new TColor(ColNbr[icol++],0./256.,159./256.,148./256.); // 1e4
            new TColor(ColNbr[icol++],0./256.,129./256.,166./256.); // 1e5
            new TColor(ColNbr[icol++],0./256.,129./256.,166./256.); // 1e6
            new TColor(ColNbr[icol++],69./256.,56./256.,167./256.); // 1e7
            new TColor(ColNbr[icol++],69./256.,56./256.,167./256.); // 1e8
            new TColor(ColNbr[icol++],69./256.,56./256.,167./256.); // 1e9
            new TColor(ColNbr[icol++],60./256.,0./256.,109./256.); // 1e10
            new TColor(ColNbr[icol++],60./256.,0./256.,109./256.); // 1e11
            new TColor(ColNbr[icol++],60./256.,0./256.,109./256.); // 1e12
            new TColor(ColNbr[icol++],60./256.,0./256.,109./256.); // 1e13
            new TColor(ColNbr[icol++],60./256.,0./256.,109./256.); // 1e14
            new TColor(ColNbr[icol++],60./256.,0./256.,109./256.); // 1e15
            new TColor(ColNbr[icol++],0./256.,0./256.,0./256.); // >1e15
        }
        Int_t palette[NColors];
        for(int i=0 ; i<NColors ;i++) palette[i] = ColNbr[i];
        gStyle->SetPalette(NColors,palette,0.);
    }
    else if(Mode == M_DecayMode) {
        Int_t NColors = 7;
        int ColNbr[NColors];
        for(int i=0 ; i<NColors ;i++) ColNbr[i] = 2100+i;
        int icol=0;
        if(gROOT->GetColor(ColNbr[icol]) == nullptr) {
            new TColor(ColNbr[icol++],0./256.,0./256.,0./256.); // STABLE => 1
            new TColor(ColNbr[icol++],255./256.,196./256.,0./256.); // P => 2
            new TColor(ColNbr[icol++],253./256.,234./256.,0./256.); // A => 3
            new TColor(ColNbr[icol++],255./256.,116./256.,89./256.); // B+ => 4
            new TColor(ColNbr[icol++],81./256.,187./256.,216./256.); // B- => 5
            new TColor(ColNbr[icol++],187./256.,218./256.,228./256.); // N => 6
            new TColor(ColNbr[icol++],0./256.,149./256.,0./256.); // SF => 7
        }
        Int_t palette[NColors];
        for(int i=0 ; i<NColors ;i++) palette[i] = ColNbr[i];
        gStyle->SetPalette(NColors,palette,0.);
    }
    if(Mode == M_1rstExcitedState ) {
        gStyle->SetPalette(1);
    }
    if(Mode == M_BE2E2B2 ) {
        gStyle->SetPalette(1);
    }
    if(Mode == M_BE2WU ) {
        gStyle->SetPalette(1);
    }
}

void CXNucChart::UpdateNucChart()
{
    fCanvas->cd();
    fNucChartHist->Reset();
    fNucChartHist->GetListOfFunctions()->Clear();

    if(fViewMode->GetSelected() == M_LifeTime || fViewMode->GetSelected() == M_1stIsomer || fViewMode->GetSelected() == M_2ndIsomer ) {
        fNucChartHist->SetContour(33);
        TExec *ex1 = new TExec("ex1",Form("CXNucChart::SetPalette(%d)",M_LifeTime));
        fNucChartHist->GetListOfFunctions()->Add(ex1);

        gPad->Modified();
        gPad->Update();
        gSystem->ProcessEvents();
    }
    if(fViewMode->GetSelected() == M_DecayMode) {
        fNucChartHist->SetContour(7);
        TExec *ex1 = new TExec("ex1",Form("CXNucChart::SetPalette(%d)",M_DecayMode));
        fNucChartHist->GetListOfFunctions()->Add(ex1);

        gPad->Modified();
        gPad->Update();
        gSystem->ProcessEvents();
    }
    if(fViewMode->GetSelected() == M_1rstExcitedState) {
        fNucChartHist->SetContour(20);
        TExec *ex1 = new TExec("ex1",Form("CXNucChart::SetPalette(%d)",M_1rstExcitedState));
        fNucChartHist->GetListOfFunctions()->Add(ex1);

        gPad->Modified();
        gPad->Update();
        gSystem->ProcessEvents();
    }
    if(fViewMode->GetSelected() == M_BE2E2B2) {
        fNucChartHist->SetContour(20);
        TExec *ex1 = new TExec("ex1",Form("CXNucChart::SetPalette(%d)",M_BE2E2B2));
        fNucChartHist->GetListOfFunctions()->Add(ex1);

        gPad->Modified();
        gPad->Update();
        gSystem->ProcessEvents();
    }
    if(fViewMode->GetSelected() == M_BE2WU) {
        fNucChartHist->SetContour(20);
        TExec *ex1 = new TExec("ex1",Form("CXNucChart::SetPalette(%d)",M_BE2WU));
        fNucChartHist->GetListOfFunctions()->Add(ex1);

        gPad->Modified();
        gPad->Update();
        gSystem->ProcessEvents();
    }

    for(const auto &nuc : gmanager->get_nuclei()) {
        auto level_scheme = nuc->get_level_scheme();
        auto ground_state = nuc->get_ground_state();
        if(!ground_state) continue;

        vector<shared_ptr<tkn::tklevel>> isomers;
        if(fViewMode->GetSelected() == M_1stIsomer || fViewMode->GetSelected() == M_2ndIsomer) {
            isomers = nuc->get_level_scheme()->get_levels( [](auto lvl) {
                return lvl->is_isomer();
            });
        }

        if(fViewMode->GetSelected() == M_LifeTime && !isnan(ground_state->get_lifetime())) {
            if(nuc->is_stable()) fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),1e30);
            else fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),ground_state->get_lifetime());
        }
        if(fViewMode->GetSelected() == M_1stIsomer && isomers.size()>=1 && !isnan(isomers.at(0)->get_lifetime())) {
            if(isomers.at(0)->is_stable()) fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),1e30);
            else fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),isomers.at(0)->get_lifetime());
        }
        if(fViewMode->GetSelected() == M_2ndIsomer && isomers.size() >=2 && !isnan(isomers.at(1)->get_lifetime())) {
            if(isomers.at(1)->is_stable()) fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),1e30);
            else fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),isomers.at(1)->get_lifetime());
        }

        if(fViewMode->GetSelected() == M_DecayMode) {
            Int_t value;

            tkn::tkstring decay = nuc->get_property("decay_modes");
            auto token = decay.replace_all("]","[").tokenize("[");
            if(token.size() == 0) decay = "";
            else decay = token.front();

            if(nuc->is_stable()) value = 1;
            else if(decay.begins_with("B-;")) value = 5;
            else if(decay.begins_with("2B-")) value = 5;
            else if(decay.begins_with("B+")) value = 4;
            else if(decay.begins_with("EC")) value = 4;
            else if(decay.begins_with("2EC")) value = 4;
            else if(decay.begins_with("P")) value = 2;
            else if(decay.begins_with("2P")) value = 2;
            else if(decay.begins_with("N")) value = 6;
            else if(decay.begins_with("2N")) value = 6;
            else if(decay.begins_with("A")) value = 3;
            else if(decay.begins_with("SF")) value = 7;
            else if(decay.begins_with("EF")) value = 7;
            else continue;

            fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),value);
        }
        if(fViewMode->GetSelected() == M_1rstExcitedState) {
            if(level_scheme->get_levels().size()>1) fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),level_scheme->get_levels().at(1)->get_energy());
        }
        if(fViewMode->GetSelected() == M_BE2E2B2 && (nuc->get_z()%2==0 && nuc->get_n()%2==0)) {
            auto gamma = level_scheme->get_decay<tkn::tkgammadecay>("2+1->0+1",false);
            if(!gamma) continue;
            // here we ask, if known, the BE2 value (1: electric, 2:L, 1: Weisskopf units)
            auto BE2 = gamma->get_trans_prob(true,2,false);
            if(BE2<=0||std::isnan(BE2)) continue;
            fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),BE2);
        }
        if(fViewMode->GetSelected() == M_BE2WU) {
            auto gamma = level_scheme->get_decay<tkn::tkgammadecay>("2+1->0+1",false);
            if(!gamma) continue;
            // here we ask, if known, the BE2 value (1: electric, 2:L, 1: Weisskopf units)
            auto BE2W = gamma->get_trans_prob(true,2,true);
            if(BE2W<=0||std::isnan(BE2W)) continue;
            fNucChartHist->Fill(nuc->get_n(),nuc->get_z(),BE2W/((double)nuc->get_a()));
        }
    }

    if(fViewMode->GetSelected() == M_LifeTime || fViewMode->GetSelected() == M_1stIsomer || fViewMode->GetSelected() == M_2ndIsomer ) {
        fNucChartHist->SetMinimum(1e-16);
        fNucChartHist->SetMaximum(1e16);
        fNucChartHist->Draw("colz");
        fCanvas->SetLogz();
    }
    else if(fViewMode->GetSelected() == M_DecayMode) {
        fNucChartHist->SetMinimum(1);
        fNucChartHist->SetMaximum(7);
        fNucChartHist->Draw("colz");
        fCanvas->SetLogz(0);
    }
    else if(fViewMode->GetSelected() == M_1rstExcitedState) {
        fNucChartHist->SetMinimum();
        fNucChartHist->SetMaximum();
        fNucChartHist->Draw("colz");
        fCanvas->SetLogz();
    }
    else if(fViewMode->GetSelected() == M_BE2E2B2) {
        fNucChartHist->SetMinimum(1e-3);
        fNucChartHist->SetMaximum();
        fNucChartHist->Draw("colz");
        fCanvas->SetLogz();
    }
    else if(fViewMode->GetSelected() == M_BE2WU) {
        fNucChartHist->SetMinimum();
        fNucChartHist->SetMaximum();
        fNucChartHist->Draw("colz");
        fCanvas->SetLogz(0);
    }

    UpdateRange();

    TPaletteAxis *palette = (TPaletteAxis*)fNucChartHist->GetListOfFunctions()->FindObject("palette");
    if(palette) {
        palette->SetX1NDC(0.925);
        palette->SetX2NDC(0.95);
        palette->SetLineWidth(0);
        palette->SetBit(TObject::kCannotPick);
        fCanvas->DisableClass("TPaletteAxis");
        if(fViewMode->GetSelected() == M_DecayMode)
            fNucChartHist->GetZaxis()->SetLabelSize(0.);
        else
            fNucChartHist->GetZaxis()->SetLabelSize(0.04);

        fNucChartHist->GetZaxis()->SetLabelOffset(0.);
        fCanvas->Update();
    }

    fCanvas->GetFrame()->SetBit(TObject::kCannotPick);
}

void CXNucChart::PrintInListBox(TString mess, Int_t Type)
{

#if (OS_TYPE == OS_LINUX)
    const TGFont *ufont;         // will reflect user font changes
    ufont = gClient->GetFont("-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
    // ufont = gClient->GetFont("-adobe-times-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
    if (!ufont)
        ufont = fClient->GetResourcePool()->GetDefaultFont();

    TGGC   *uGC;           // will reflect user GC changes
    // graphics context changes
    GCValues_t val;
    val.fMask = kGCFont;
    val.fFont = ufont->GetFontHandle();
    uGC = gClient->GetGC(&val, kTRUE);

    TGTextLBEntry *entry = new TGTextLBEntry(fInfoBox->GetContainer(), new TGString(mess), fInfoBox->GetNumberOfEntries()+1, uGC->GetGC(), ufont->GetFontStruct());
#else
    TGTextLBEntry *entry = new TGTextLBEntry(fInfoBox->GetContainer(), new TGString(mess), fInfoBox->GetNumberOfEntries()+1);
#endif

    if(Type == kError)
        entry->SetBackgroundColor((Pixel_t)0xff0000);
    else if(Type == kInfo)
        entry->SetBackgroundColor((Pixel_t)0x87a7d2);
    else if(Type == kWarning)
        entry->SetBackgroundColor((Pixel_t)0xdfdf44);
    else if(Type == kPrint)
        entry->SetBackgroundColor((Pixel_t)0x90f269);

    fInfoBox->AddEntry((TGLBEntry *)entry, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX));
    fInfoBox->Layout();
}

void CXNucChart::SelectNucleus(Int_t Z, Int_t N)
{
    fSelectedNucleus = gmanager->get_nucleus(Z,N+Z);

    fDataSetMode->RemoveAll();

    if(fSelectedNucleus == nullptr) {
        PrintInListBox(Form("Nucleus: Z=%d, N=%d unknown",Z,N),kError);
        return;
    }

    fSelectedLevelScheme = fSelectedNucleus->get_level_scheme();
    int i=0;
    for(auto &dataset: fSelectedLevelScheme->get_datasets()) {
        fDataSetMode->AddEntry(dataset.second->get_name().data(),i);
        i++;
    }
    fDataSetMode->Select(0);

    if(fDataSetMode->GetNumberOfEntries() == 0) PrintInfos(true);

    fNucleusTextEntry->SetTitle(fSelectedNucleus->get_symbol().data());
    fNucleusTextEntry->SetTextColor(CXblack);

    CXNucleusBox *box = (CXNucleusBox*)fListOfBoxes->FindObject(Form("(Z=%d,N=%d)",Z,N));
    if(box) {
        box->SetLineColor(kRed);
        box->SetLineWidth(2);
    }

    CXNucleusBox *lastbox = (CXNucleusBox*)fListOfBoxes->FindObject(fLastSelectedBox);
    if(lastbox && lastbox != box) {
        lastbox->SetLineColor(kBlack);
        lastbox->SetLineWidth(1);
    }

    fLastSelectedBox = Form("(Z=%d,N=%d)",Z,N);

    fCanvas->Modified();
    fCanvas->Update();
}

void CXNucChart::HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected)
{
    SetPalette(fViewMode->GetSelected());

    if(selected != nullptr) {
        const char *text0, *text1, *text2, *text4;
        char text3[50];
        text0 = selected->ClassName();
        fStatusBar->SetText(text0,0);
        text1 = selected->GetTitle();
        fStatusBar->SetText(text1,1);
        text2 = selected->GetName();
        fStatusBar->SetText(text2,2);

        if (EventType == kKeyPress)
            snprintf(text3, sizeof(text3), "%c", (char) EventY);
        else
            snprintf(text3, sizeof(text3), "%.2f,%.2f", fCanvas->AbsPixeltoX(EventX), fCanvas->AbsPixeltoY(EventY) );
        fStatusBar->SetText(text3,3);

        text4 = selected->GetObjectInfo(EventX,EventY);
        fStatusBar->SetText(text4,4);
    }

    if(EventType == kButton1Up && fLastEventType == kButton1Down) {
        Int_t Z = TMath::Nint(fNucChartHist->GetYaxis()->GetBinCenter(fNucChartHist->GetYaxis()->FindBin(fCanvas->AbsPixeltoY(EventY))));
        Int_t N = TMath::Nint(fNucChartHist->GetXaxis()->GetBinCenter(fNucChartHist->GetXaxis()->FindBin(fCanvas->AbsPixeltoX(EventX))));
        SelectNucleus(Z,N);
    }
    //    if(EventType == kButton1Double) {
    //        PrintInfos(true);
    //    }

    /// Recuperation de la derniere position de la souris
    if(EventType == kMouseMotion) {
        fCanvas->AbsPixeltoXY(EventX,EventY,fLastYPosition,fLastXPosition);
    }

    fLastEventType = EventType;
    fLastEventX = EventX;
    fLastEventY = EventY;
    fLastSelected = selected;

    if(fDoUpdateRange) {
        UpdateRange();
        fDoUpdateRange = false;
    }
}

void CXNucChart::PrintInfos(bool inprompt)
{
    if(fSelectedNucleus == nullptr) {
        if(inprompt) cout<<Form("No selected nucleus")<<endl;
        return;
    }
    fInfoBox->RemoveAll();

    if(fPrintMode->GetSelected() == M_NucInfo) {
        if(inprompt) {
            TString text = Form("** Nucleus: %s (Z:%d, N:%d) **",fSelectedNucleus->get_symbol().data(),fSelectedNucleus->get_z(),fSelectedNucleus->get_n());
            TString stars; for (int i=0 ; i<text.Length() ; i++) stars.Append("*");
            cout<<endl<<endl<<stars<<endl<<text<<endl<<stars<<endl<<endl;
        }
        PrintInListBox(Form("Nucleus: %s (Z:%d, N:%d)",fSelectedNucleus->get_symbol().data(),fSelectedNucleus->get_z(),fSelectedNucleus->get_n()),kPrint);

        auto gamma = fSelectedLevelScheme->get_decay<tkn::tkgammadecay>("2+1->0+1",false);
        if(gamma) {
            auto BE2 = gamma->get_trans_prob_measure(true,2,false);
            auto BE2W = gamma->get_trans_prob_measure(true,2,true);
            if(BE2) {
                if(inprompt) cout<<BE2 << endl;
                ostringstream s; s << BE2;
                PrintInListBox(s.str().data(),kPrint);
            }
            if(BE2W) {
                if(inprompt) cout<<BE2W << endl;
                ostringstream s; s << BE2W;
                PrintInListBox(s.str().data(),kPrint);
            }
        }

        auto ground_state = fSelectedNucleus->get_ground_state();
        if(inprompt) {
            cout<<endl;
            if(ground_state) {
                cout<<Form("Energy (keV): %g",ground_state->get_energy())<<endl;
                cout<<Form("Jpi         : %s",ground_state->get_spin_parity_str().data())<<endl;
            }
            if(fSelectedNucleus->get_mass_excess_measure()) {
                cout<<Form("Mass Excess : %g",fSelectedNucleus->get_mass_excess())<<endl;
            }
            if(fSelectedNucleus->get_lifetime_measure()) {
                cout<<Form("T 1/2       : %s",ground_state->get_lifetime_str().data())<<endl;
            }
            if(fSelectedNucleus->get_abundance()>0.) cout<<Form("Abundance   : %g %%",fSelectedNucleus->get_abundance())<<endl;
        }
        PrintInListBox("");
        if(ground_state) {
            PrintInListBox(Form("Energy (keV): %g",ground_state->get_energy()),kInfo);
            PrintInListBox(Form("Jpi         : %s",ground_state->get_spin_parity_str().data()),kInfo);
        }
        if(fSelectedNucleus->get_mass_excess_measure()) {
            PrintInListBox(Form("Mass Excess : %g",fSelectedNucleus->get_mass_excess()),kInfo);
        }
        if(fSelectedNucleus->get_lifetime_measure()) {
            PrintInListBox(Form("T 1/2       : %s",ground_state->get_lifetime_str().data()),kInfo);
        }
        if(fSelectedNucleus->get_abundance()>0.) PrintInListBox(Form("Abundance   : %g %%",fSelectedNucleus->get_abundance()),kInfo);
        if(!fSelectedNucleus->is_stable()) {
            for(auto &dec: fSelectedNucleus->get_decay_modes()) {
                if(inprompt) cout<<Form("decay       : %s -- %g %%",dec.first.data(), dec.second)<<endl;
                PrintInListBox(Form("decay       : %s -- %g %%",dec.first.data(), dec.second),kInfo);
            }
        }

        vector<shared_ptr<tkn::tklevel>> isomers;
        isomers = fSelectedLevelScheme->get_levels( [](auto lvl) {
            return lvl->is_isomer();
        });

        for(auto &iso: isomers) {
            if(inprompt) {
                cout<<endl;
                cout<<"Isomer N° " << iso->get_isomer_level() << endl;
                        cout<<Form("Energy (keV): %g",iso->get_energy())<<endl;
                cout<<Form("Jpi         : %s",iso->get_spin_parity_str().data())<<endl;
                if(iso->get_lifetime_measure()) {
                    cout<<Form("T 1/2       : %s",iso->get_lifetime_str().data())<<endl;
                }
            }
            PrintInListBox("");
            PrintInListBox(Form("Isomer N°   : %d",iso->get_isomer_level()),kInfo);
                PrintInListBox(Form("Energy (keV): %g",iso->get_energy()),kInfo);
            PrintInListBox(Form("Jpi         : %s",iso->get_spin_parity_str().data()),kInfo);
            if(iso->get_lifetime_measure()) {
                PrintInListBox(Form("T 1/2       : %s",iso->get_lifetime_str().data()),kInfo);
            }
        }
    }
    else if(fPrintMode->GetSelected() == M_LevelsInfo) {
        TString Text = PrintNucleusLevels(fSelectedLevelScheme,fSelectedNucleus->get_symbol(),inprompt);
        PrintInListBox(Form("Energy (keV)"),kInfo);

        TObjArray *arr = Text.Tokenize("!");
        for(int i=0 ; i<arr->GetEntries() ; i++) {
            PrintInListBox(arr->At(i)->GetName());
        }
        delete arr;
    }
    else if(fPrintMode->GetSelected() == M_GammaInfos) {
        TString Text = PrintNucleusGammas(fSelectedLevelScheme,fSelectedNucleus->get_symbol(),inprompt);

        PrintInListBox(Form("Energy (keV)"),kInfo);

        TObjArray *arr = Text.Tokenize("!");
        for(int i=0 ; i<arr->GetEntries() ; i++) {
            PrintInListBox(arr->At(i)->GetName());
        }
        delete arr;
    }
}

void CXNucChart::NucNotValidated()
{
    fNucleusTextEntry->SetTextColor(CXred);
}

void CXNucChart::UpdateNucFromSymb()
{
    if(gmanager->known_nucleus(fNucleusTextEntry->GetText())) {
        tkn::tknucleus nuc(fNucleusTextEntry->GetText());
        SelectNucleus(nuc.get_z(),nuc.get_n());
        fNucChartHist->GetXaxis()->SetRangeUser(nuc.get_n()-5,nuc.get_n()+5);
        fNucChartHist->GetYaxis()->SetRangeUser(nuc.get_z()-5,nuc.get_z()+5);
        UpdateRange();
        fNucleusTextEntry->SetTextColor(CXblack);
        fCanvas->Modified();
        fCanvas->Update();
    }
    else {
        TString text = fNucleusTextEntry->GetText();
        for(int i=0 ; i<text.Length() ; i++) {
            if(((TString)text[i]).IsDec()) {
                text.Remove(i,1);
                i--;
            }
        }
        Int_t NMin = 1000;
        Int_t NMax = -1;
        Int_t Z = 0;

        for(auto nuc: gmanager->get_nuclei()) {
            TString nucname = nuc->get_symbol();
            for(int i=0 ; i<nucname.Length() ; i++) {
                if(((TString)nucname[i]).IsDec()) {
                    nucname.Remove(i,1);
                    i--;
                }
            }
            if(nucname == text) {
                if(nuc->get_n() < NMin) NMin = nuc->get_n();
                if(nuc->get_n() > NMax) NMax = nuc->get_n();
                Z = nuc->get_z();
            }
        }
        if(NMin >= 0 && NMax >= 0) {
            Int_t N = TMath::Nint(NMax+NMin)*0.5;
            fNucleusTextEntry->SetTitle(Form("%d%s",N+Z,text.Data()));
            UpdateNucFromSymb();
        }
    }
}

void CXNucChart::ProcessedKeyEvent(Event_t *event)
{
    SetPalette(fViewMode->GetSelected());

    char input[10];
    UInt_t keysym;

    gVirtualX->LookupString(event, input, sizeof(input), keysym);

    //    std::cout << "event : " << event->fCode << " " << event->fState <<" ; "<< event->fType  << "; " << keysym << std::endl;

    if(event->fType == kGKeyPress && keysym == kKey_Control)
        fCTRL = true;
    if(event->fType == kKeyRelease && keysym == kKey_Control)
        fCTRL = false;
}

void CXNucChart::UpdateRange()
{
    ((THistPainter*)fNucChartHist->GetPainter())->PaintFrame();

    fNMin = fCanvas->GetUxmin();
    fNMax = fCanvas->GetUxmax();
    Int_t Width = fNMax-fNMin;

    fZMin = fCanvas->GetUymin();
    fZMax = fCanvas->GetUymax();
    Int_t Height = fZMax-fZMin;

    if(Width>(Height*1.5)) {
        if(fNMin+Height<fNucChartHist->GetXaxis()->GetXmax()) {
            fNucChartHist->GetXaxis()->SetRangeUser(fNMin,fNMin+Height);
        }
        else
            fNucChartHist->GetXaxis()->SetRangeUser(fNucChartHist->GetXaxis()->GetXmax()-Height,fNucChartHist->GetXaxis()->GetXmax());

        fDoUpdateRange = true;
        fCanvas->Update();
        return;
    }

    ShowMagicNumbers(fPlotMagics);
    PlotBoxes();

    if(fViewMode->GetSelected() == M_DecayMode) {
        TLatex *l = new TLatex(0.,0.,"Stable");    l->SetNDC(); l->SetX(0.965); l->SetY(0.13);l->SetTextSizePixels(18);l->SetTextFont(132);l->SetTextAngle(45.);l->Draw();l->SetBit(kCannotPick);fMagicList->Add(l);
        l = new TLatex(0.,0.,"p");         l->SetNDC(); l->SetX(0.965); l->SetY(0.26);l->SetTextSizePixels(18);l->SetTextFont(132);l->SetTextAngle(45.);l->Draw();l->SetBit(kCannotPick);fMagicList->Add(l);
        l = new TLatex(0.,0.,"#alpha");    l->SetNDC(); l->SetX(0.965); l->SetY(0.38);l->SetTextSizePixels(18);l->SetTextFont(132);l->SetTextAngle(45.);l->Draw();l->SetBit(kCannotPick);fMagicList->Add(l);
        l = new TLatex(0.,0.,"#Beta^{+}"); l->SetNDC(); l->SetX(0.965); l->SetY(0.50);l->SetTextSizePixels(18);l->SetTextFont(132);l->SetTextAngle(45.);l->Draw();l->SetBit(kCannotPick);fMagicList->Add(l);
        l = new TLatex(0.,0.,"#Beta^{-}"); l->SetNDC(); l->SetX(0.965); l->SetY(0.63);l->SetTextSizePixels(18);l->SetTextFont(132);l->SetTextAngle(45.);l->Draw();l->SetBit(kCannotPick);fMagicList->Add(l);
        l = new TLatex(0.,0.,"n");         l->SetNDC(); l->SetX(0.965); l->SetY(0.75);l->SetTextSizePixels(18);l->SetTextFont(132);l->SetTextAngle(45.);l->Draw();l->SetBit(kCannotPick);fMagicList->Add(l);
        l = new TLatex(0.,0.,"S.F.");      l->SetNDC(); l->SetX(0.965); l->SetY(0.88);l->SetTextSizePixels(18);l->SetTextFont(132);l->SetTextAngle(45.);l->Draw();l->SetBit(kCannotPick);fMagicList->Add(l);
    }

    fCanvas->Modified();
    fCanvas->Update();
}

void CXNucChart::UpdateDataSet()
{
    TString dataset = fDataSetMode->GetSelectedEntry()->GetTitle();
    fSelectedLevelScheme = nullptr;
    if(fSelectedNucleus) fSelectedLevelScheme = fSelectedNucleus->get_level_scheme();
    fSelectedLevelScheme->select_dataset(dataset.Data());
    PrintInfos(true);
}

TString CXNucChart::PrintNucleusGammas(shared_ptr<tkn::tklevel_scheme> lev, TString NucName, bool print)
{
    if(lev == nullptr) return "";

    TString Text="";

    if(print) {
        TString text = Form("** Gamma-rays transitions for nucleus %s **",NucName.Data());
        TString stars; for (int i=0 ; i<text.Length() ; i++) stars.Append("*");
        cout<<endl<<endl<<stars<<endl<<text<<endl<<stars<<endl<<endl;

        cout<<Form("    E Gamma :      Ji (    Ei    ) -->      Jf (     Ef   ) ( I %% )")<<endl;
        cout<<     "-------------------------------------------------------------------"<<endl;

        cout<<endl;
    }

    for(auto &dec: lev->get_decays<tkn::tkgammadecay>()) {
        Float_t Energy = dec->get_energy();

        auto NucLevI = dec->get_level_from();
        auto NucLevF = dec->get_level_to();

        auto Strengh = dec->get_relative_intensity_measure();

        double ELevI   = NucLevI->get_energy();
        TString spinI_s = NucLevI->get_spin_parity_str();

        double ELevF   = NucLevF->get_energy();
        TString spinF_s = NucLevF->get_spin_parity_str();

        TString TransitionName;
        if(Strengh) TransitionName = Form(" %6.1f keV : %7s (%6.1f keV) --> %7s (%6.1f keV) (%3.f %%) ",Energy,spinI_s.Data(),ELevI,spinF_s.Data(),ELevF,Strengh->get_value());
        else  TransitionName = Form(" %6.1f keV : %7s (%6.1f keV) --> %7s (%6.1f keV) (?) ",Energy,spinI_s.Data(),ELevI,spinF_s.Data(),ELevF);
        if(print) cout<<TransitionName<<endl;
        Text += Form("!%-7g %-7s->%-7s",Energy,spinI_s.Data(),spinF_s.Data());
    }
    return Text;
}

TString CXNucChart::PrintNucleusLevels(shared_ptr<tkn::tklevel_scheme> lev, TString NucName, bool print)
{
    if(lev == nullptr) return "";

    TString Text="";

    if(print) {
        TString text = Form("** Levels info for nucleus %s **",NucName.Data());
        TString stars; for (int i=0 ; i<text.Length() ; i++) stars.Append("*");
        cout<<endl<<endl<<stars<<endl<<text<<endl<<stars<<endl<<endl;

        cout<<left<<setw(15)<<"Energy"<<setw(6)<<"Spin"<<setw(8)<<"T 1/2"<<endl;
        cout<<"--------------------------"<<endl<<endl;
    }

    for(auto &lev: lev->get_levels()) {
        Float_t ELev   = lev->get_energy();
        TString spin = lev->get_spin_parity_str();
        TString LifeTime = lev->get_lifetime_str();

        if(print) cout<<left<<setw(15)<<ELev<<setw(6)<<spin<<setw(8)<<LifeTime<<endl;
        Text += Form("!%-7g %-7s %-10s",ELev,spin.Data(),LifeTime.Data());
    }
    return Text;
}

ClassImp(CXNucChart);


