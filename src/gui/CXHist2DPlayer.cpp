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

#include "CXHist2DPlayer.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "TGButton.h"
#include "TGTextEntry.h"
#include "TGLabel.h"
#include "TROOT.h"
#include "TFrame.h"
#include "TGListBox.h"
#include "TGComboBox.h"
#include "TGNumberEntry.h"

#include "CXMainWindow.h"
#include "CXTH1Proj.h"
#include "CXGateBox.h"
#include "CXSavedList.h"
#include "CXBashColor.h"

using namespace std;

CXHist2DPlayer::CXHist2DPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, CXMainWindow *window) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    fMainWindow = window;

    /// GxG
    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Gamma Gamma utility", kVerticalFrame);
    fGroupFrame->SetTextColor(CXred);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0));

    TGGroupFrame *fSubGroupFrame = new TGGroupFrame(fGroupFrame, "Projections", kVerticalFrame);
    fSubGroupFrame->SetTextColor(CXblue);
    fSubGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    fGroupFrame->AddFrame(fSubGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, -10, -10, 0, 0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Gate on axis: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,10,0,0));
    fProjectionAxis = new TGComboBox(fHorizontalFrame);
    fHorizontalFrame->AddFrame(fProjectionAxis,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX | kLHintsExpandY ,1,3,0,0));
    fProjectionAxis->AddEntry("X",0);
    fProjectionAxis->AddEntry("Y",1);
    fProjectionAxis->Select(0);
    fProjectionAxis->Connect("Selected(Int_t)", "CXHist2DPlayer", this, "UpdateProjection()");
    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *GateButton = new TGTextButton(fHorizontalFrame, "Gate");
    GateButton->SetTextColor(CXred);
    GateButton->Connect("Clicked()", "CXHist2DPlayer", this, "AddGate()");
    fHorizontalFrame->AddFrame(GateButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,2,2,0,0));

    TGTextButton *BackgroundButton = new TGTextButton(fHorizontalFrame, "Backgd");
    BackgroundButton->SetTextColor(CXblue);
    BackgroundButton->Connect("Clicked()", "CXHist2DPlayer", this, "AddBackgd()");
    fHorizontalFrame->AddFrame(BackgroundButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,2,2,0,0));

    TGTextButton *ClearButton = new TGTextButton(fHorizontalFrame, "Clear");
    ClearButton->SetTextColor(CXgreen);
    ClearButton->Connect("Clicked()", "CXHist2DPlayer", this, "ClearGates()");
    fHorizontalFrame->AddFrame(ClearButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,2,2,0,0));

    TGTextButton *LastGateButton = new TGTextButton(fHorizontalFrame, "Last");
    LastGateButton->Connect("Clicked()", "CXHist2DPlayer", this, "ApplyLastGate()");
    fHorizontalFrame->AddFrame(LastGateButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,2,2,0,0));

    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *ProjectButton = new TGTextButton(fHorizontalFrame, "Project");
    ProjectButton->Connect("Clicked()", "CXHist2DPlayer", this, "Project()");
    fHorizontalFrame->AddFrame(ProjectButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));

    fFixRange = new TGCheckButton(fHorizontalFrame, "Fix Range", 0);
    fFixRange->SetState(kButtonUp);
    fHorizontalFrame->AddFrame(fFixRange,new TGLayoutHints(kLHintsLeft | kLHintsCenterY ,5,10,0,0));

    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Rebin projection: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,20,0,0));
    fRebinValue = new TGNumberEntry(fHorizontalFrame, 1, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fRebinValue,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,5,5));

    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fSubGroupFrame = new TGGroupFrame(MotherFrame, "Stored spectra", kVerticalFrame);
    fSubGroupFrame->SetTextColor(CXblue);
    fSubGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fSubGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Draw Options:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fDrawOpt = new TGTextEntry(fHorizontalFrame, "hist");
    fDrawOpt->SetToolTipText("hist, same, norm, add, add(fact)");
    fHorizontalFrame->AddFrame(fDrawOpt,new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsCenterY ,0,0,0,0));
    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fStoredSpectraBox = new TGListBox(fSubGroupFrame);
    fSubGroupFrame->AddFrame(fStoredSpectraBox, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,-10,-10,0,0));
    fStoredSpectraBox->Connect("DoubleClicked(Int_t)", "CXHist2DPlayer", this, "UpdateDrawOpt()");
    fStoredSpectraBox->Connect("DoubleClicked(Int_t)", "CXSavedList", fMainWindow->GetSaveList(), "DoubleClicked(Int_t)");
    fStoredSpectraBox->GetContainer()->RemoveInput(kButtonReleaseMask | kButtonMotionMask);

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *Button = new TGTextButton(fHorizontalFrame, "Remove");
    Button->Connect("Clicked()", "CXSavedList", fMainWindow->GetSaveList(), "RemoveSelectedEntry()");
    fHorizontalFrame->AddFrame(Button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    Button = new TGTextButton(fHorizontalFrame, "Clear");
    Button->Connect("Clicked()", "CXSavedList", fMainWindow->GetSaveList(), "ClearStoredList()");
    fHorizontalFrame->AddFrame(Button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    Button = new TGTextButton(fHorizontalFrame, "Save");
    Button->Connect("Clicked()", "CXSavedList", fMainWindow->GetSaveList(), "SaveStoredList()");
    fHorizontalFrame->AddFrame(Button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fListOfStoredSpectra = fMainWindow->GetSaveList()->GetListOfStoredSpectra();
    fMainWindow->GetSaveList()->AddListBox(fStoredSpectraBox);
}

CXHist2DPlayer::~CXHist2DPlayer(){
}

CXTH1Proj *CXHist2DPlayer::GetProj(){

    if(((TString)fMainWindow->GetCanvas()->GetName()).BeginsWith("GxG")){
        fMainWindow->SetSelectedPad(fMainWindow->GetCanvas()->GetPad(1));
        fMainWindow->GetCanvas()->SetSelectedPad((TPad*)fMainWindow->GetCanvas()->GetPad(1));
        fMainWindow->GetCanvas()->SetClickSelectedPad((TPad*)fMainWindow->GetCanvas()->GetPad(1));
        gPad = fMainWindow->GetCanvas()->GetSelectedPad();
    }

    TH1 *hist = fMainWindow->GetHisto();

    if(hist && hist->InheritsFrom("CXTH1Proj")){
        return dynamic_cast<CXTH1Proj*>(hist);
    }
    else{
        gbash_color->WarningMessage("No Projection for GxG found in the current pad");
        return nullptr;
    }
}

void CXHist2DPlayer::Project(){
    fCurrentProj = GetProj();

    if(fCurrentProj){
        fCurrentProj->Project(fFixRange->GetState(), fRebinValue->GetIntNumber());

        TList *gates = fCurrentProj->GetGatesList();
        fMainWindow->GetSavedGatesList()->Clear();
        for(int i=0 ; i<gates->GetEntries() ; i++){
            fMainWindow->GetSavedGatesList()->Add(gates->At(i)->Clone());
        }
    }
}

void CXHist2DPlayer::ApplyLastGate(){
    fCurrentProj = GetProj();

    ClearGates();

    if(fCurrentProj){
        TList *gates = fCurrentProj->GetGatesList();
        gates->Clear();
        for(int i=0 ; i<fMainWindow->GetSavedGatesList()->GetEntries() ; i++){
            CXGateBox *box = (CXGateBox*)fMainWindow->GetSavedGatesList()->At(i)->Clone();
            box->SetPad(fCurrentProj->fCurrentPad);
            gates->Add(box);
            box->Draw();
        }
        fCurrentProj->UpdateGates();
    }
}

void CXHist2DPlayer::AddBackgd(){
    fCurrentProj = GetProj();

    if(fCurrentProj)
        fCurrentProj->AddBackgd();
}

void CXHist2DPlayer::AddGate(){
    fCurrentProj = GetProj();

    if(fCurrentProj)
        fCurrentProj->AddGate();
}

void CXHist2DPlayer::ClearGates(){
    fCurrentProj = GetProj();

    if(fCurrentProj)
        fCurrentProj->ClearGates();
}

void CXHist2DPlayer::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXHist2DPlayer::UpdateProjection()
{
    if(fAxisProj == fProjectionAxis->GetSelected())
        return;

    if(fProjectionAxis->GetSelected()==0)
        fAxisProj = 0;
    else
        fAxisProj = 1;

    CXTH1Proj *proj = GetProj();

    if(proj == nullptr) return;

    proj->UpdateProjection(fAxisProj);

    Project();
}

void CXHist2DPlayer::InitGG(TH2 *hist_in)
{
    TH2 *hist;

    TH1D *TotalProj = nullptr;

    if(hist_in == nullptr)
        hist = dynamic_cast<TH2*>(fMainWindow->GetHisto());
    else
        hist = hist_in;

    if(hist == nullptr) {
        cout<<"No 2D histogram found in the current pad, ignored"<<endl;
        return;
    }

    if(fAxisProj==0) {
        TotalProj = hist->ProjectionX(Form("%s_2DTotProjX",hist->GetName()));
        TotalProj->SetName(Form("%s_TotProjX",hist->GetName()));
        TotalProj->SetTitle(Form("%s TotProjX",hist->GetTitle()));
    }
    else if(fAxisProj==1){
        TotalProj = hist->ProjectionY(Form("%s_2DTotProjY",hist->GetName()));
        TotalProj->SetName(Form("%s_TotProjY",hist->GetName()));
        TotalProj->SetTitle(Form("%s TotProjY",hist->GetTitle()));
    }
    else {
        gbash_color->ErrorMessage("Unkown projection axis... ");
        return;
    }

    CXTH1Proj *NewProj = new CXTH1Proj(*TotalProj);
    NewProj->SetMainWindow(fMainWindow);
    NewProj->SetPlayer(this);
    NewProj->SetTH2(hist);
    fMainWindow->NewTab(1,2,"GxG");

    TVirtualPad *pad = fMainWindow->GetCanvas()->cd(1);
    NewProj->Draw("hist");
    NewProj->SetCurrentPad(dynamic_cast<TPad*>(pad));
    pad->Update();
    pad->SetBit(TPad::kCannotMove);
    pad->GetFrame()->SetBit(TObject::kCannotPick);

    pad = fMainWindow->GetCanvas()->GetPad(2);
    NewProj->SetProjPad(dynamic_cast<TPad*>(pad));

    gROOT->SetSelectedPad(fMainWindow->GetCanvas()->GetPad(1));
}

void CXHist2DPlayer::UpdateDrawOpt()
{
    fMainWindow->GetSaveList()->SetListDrawOption(fDrawOpt->GetText());
}

ClassImp(CXHist2DPlayer)
