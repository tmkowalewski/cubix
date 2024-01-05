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

#include "CXRadCubePlayer.h"

#include <iostream>

#include "TGButton.h"
#include "TGTextEntry.h"
#include "TGLabel.h"
#include "TROOT.h"
#include "TFrame.h"
#include "TGListBox.h"
#include "TGNumberEntry.h"

#include "CXMainWindow.h"
#include "CXRadCubeTH1Proj.h"
#include "CXGateBox.h"
#include "CXRadReader.h"
#include "CXSavedList.h"
#include "CXBashColor.h"

using namespace std;

CXRadCubePlayer::CXRadCubePlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, CXMainWindow *window) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    fMainWindow = window;

    /// GxG
    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Radware's Cube reader", kVerticalFrame);
    fGroupFrame->SetTextColor(CXred);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0));

    TGGroupFrame *fSubGroupFrame = new TGGroupFrame(fGroupFrame, "Projections", kVerticalFrame);
    fSubGroupFrame->SetTextColor(CXblue);
    fSubGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    fGroupFrame->AddFrame(fSubGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, -10, -10, 0, 0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "N projs: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,5,0,0));
    fNProjections = new TGNumberEntry(fHorizontalFrame, 1, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fNProjections->Connect("ValueSet(Long_t)", "CXRadCubePlayer", this, "ChangeNProjections()");
    fHorizontalFrame->AddFrame(fNProjections,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,5,5));
    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Use FWHM: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,3,0,0,0));
    fUseFWHM = new TGCheckButton(fHorizontalFrame);
    fUseFWHM->SetState(kButtonUp);
    fUseFWHM->Connect("Clicked()","CXRad2DPlayer", this, "ToggleFWHM()");
    fHorizontalFrame->AddFrame(fUseFWHM,new TGLayoutHints(kLHintsLeft | kLHintsCenterY ,3,0,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Gate frac: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));
    fFWHMGateFraction = new TGNumberEntry(fHorizontalFrame, 1, 3, 0, TGNumberFormat::kNESRealOne, TGNumberFormat::kNEAPositive);
    fHorizontalFrame->AddFrame(fFWHMGateFraction,new TGLayoutHints(kLHintsLeft | kLHintsCenterY | kLHintsExpandX ,3,3,0,0));

    ToggleFWHM();

    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *GateButton = new TGTextButton(fHorizontalFrame, "Gate 1");
    GateButton->SetTextColor(CXred);
    GateButton->Connect("Clicked()", "CXRadCubePlayer", this, "AddGate1()");
    fHorizontalFrame->AddFrame(GateButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,2,2,0,0));

    TGTextButton *BackgroundButton = new TGTextButton(fHorizontalFrame, "Gate 2");
    BackgroundButton->SetTextColor(CXgreen);
    BackgroundButton->Connect("Clicked()", "CXRadCubePlayer", this, "AddGate2()");
    fHorizontalFrame->AddFrame(BackgroundButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,2,2,0,0));

    TGTextButton *ClearButton = new TGTextButton(fHorizontalFrame, "Clear");
    ClearButton->SetTextColor(CXorange);
    ClearButton->Connect("Clicked()", "CXRadCubePlayer", this, "ClearGates()");
    fHorizontalFrame->AddFrame(ClearButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,2,2,0,0));

    TGTextButton *LastGateButton = new TGTextButton(fHorizontalFrame, "Last");
    LastGateButton->Connect("Clicked()", "CXRadCubePlayer", this, "ApplyLastGates()");
    fHorizontalFrame->AddFrame(LastGateButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,2,2,0,0));

    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *ProjectButton = new TGTextButton(fHorizontalFrame, "Project");
    ProjectButton->Connect("Clicked()", "CXRadCubePlayer", this, "Project()");
    fHorizontalFrame->AddFrame(ProjectButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));

    fFixRange = new TGCheckButton(fHorizontalFrame, "Fix Range", 0);
    fFixRange->SetState(kButtonUp);
    fHorizontalFrame->AddFrame(fFixRange,new TGLayoutHints(kLHintsLeft | kLHintsCenterY ,5,10,0,0));

    fBckSubtract = new TGCheckButton(fHorizontalFrame, "BG sub", 0);
    fBckSubtract->SetState(kButtonDown);
    fHorizontalFrame->AddFrame(fBckSubtract,new TGLayoutHints(kLHintsLeft | kLHintsCenterY ,5,10,0,0));

    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Rebin projection: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,5,0,0));
    fRebinValue = new TGNumberEntry(fHorizontalFrame, 1, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fRebinValue,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,5,5));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Dynamic mode: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fDynamicProj = new TGCheckButton(fHorizontalFrame, "", 0);
    fDynamicProj->SetState(kButtonUp);
    fHorizontalFrame->AddFrame(fDynamicProj,new TGLayoutHints(kLHintsLeft | kLHintsCenterY ,0,5,0,0));

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
    fStoredSpectraBox->GetContainer()->RemoveInput(kButtonReleaseMask | kButtonMotionMask);
    fSubGroupFrame->AddFrame(fStoredSpectraBox, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,-10,-10,0,0));
    fStoredSpectraBox->Connect("DoubleClicked(Int_t)", "CXRadCubePlayer", this, "UpdateDrawOpt()");
    fStoredSpectraBox->Connect("DoubleClicked(Int_t)", "CXSavedList", fMainWindow->GetSaveList(), "DoubleClicked(Int_t)");

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

CXRadCubePlayer::~CXRadCubePlayer(){
}

CXRadCubeTH1Proj *CXRadCubePlayer::GetProj(){

    if(((TString)fMainWindow->GetCanvas()->GetName()).BeginsWith("RadCube")){
        fMainWindow->SetSelectedPad(fMainWindow->GetCanvas()->GetPad(1));
        fMainWindow->GetCanvas()->SetSelectedPad((TPad*)fMainWindow->GetCanvas()->GetPad(1));
        fMainWindow->GetCanvas()->SetClickSelectedPad((TPad*)fMainWindow->GetCanvas()->GetPad(1));
        gPad = fMainWindow->GetCanvas()->GetSelectedPad();
    }

    TH1 *hist = fMainWindow->GetHisto();

    if(hist && hist->InheritsFrom("CXRadCubeTH1Proj")){
        return dynamic_cast<CXRadCubeTH1Proj*>(hist);
    }
    else{
        cout<<"No Projection for RadCube found in the current pad"<<endl;
        return nullptr;
    }
}

void CXRadCubePlayer::Project()
{
    fCurrentProj = GetProj();

    if(fCurrentProj) {
        fCurrentProj->Project(fFixRange->GetState(),fBckSubtract->GetState(),fRebinValue->GetIntNumber());

        TList *gates = fCurrentProj->GetGatesList();
        fMainWindow->GetSavedGatesList()->Clear();
        for(int i=0 ; i<gates->GetEntries() ; i++){
            fMainWindow->GetSavedGatesList()->Add(gates->At(i)->Clone());
        }
    }
}

void CXRadCubePlayer::ApplyLastGates()
{
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

void CXRadCubePlayer::AddGate1(){
    fCurrentProj = GetProj();

    if(fCurrentProj)
        fCurrentProj->AddNewGate1();
}

void CXRadCubePlayer::AddGate2(){
    fCurrentProj = GetProj();

    if(fCurrentProj)
        fCurrentProj->AddNewGate2();
}

void CXRadCubePlayer::ClearGates(){
    fCurrentProj = GetProj();

    if(fCurrentProj)
        fCurrentProj->ClearGates();
}

void CXRadCubePlayer::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXRadCubePlayer::Init(CXRadReader *radreader)
{
    TH1 *Background = nullptr;

    if(radreader == nullptr) {
        cout<<"No RadCube found, ignored"<<endl;
        return;
    }

    CXRadCubeTH1Proj *NewProj = new CXRadCubeTH1Proj(radreader, fNProjs);
    NewProj->SetMainWindow(fMainWindow);
    NewProj->SetCubePlayer(this);
    fMainWindow->NewTab(1,fNProjs+1,"RadCube");

    Background = NewProj->GetBackground();
    Background->SetName(Form("%s_Background",radreader->GetName().Data()));
    Background->SetTitle(Form("%s_Background",radreader->GetName().Data()));
    Background->SetLineColor(kRed);
    Background->SetLineStyle(kDashed);

    TVirtualPad *pad = fMainWindow->GetCanvas()->cd(1);
    NewProj->Draw("hist");
    Background->Draw("hist same");
    NewProj->SetCurrentPad(dynamic_cast<TPad*>(pad));
    pad->Update();
    pad->SetBit(TPad::kCannotMove);
    pad->GetFrame()->SetBit(TObject::kCannotPick);

    for(int i=0 ; i<fNProjs ; i++) {
        pad = fMainWindow->GetCanvas()->GetPad(i+2);
        NewProj->SetProjPad(i,dynamic_cast<TPad*>(pad));
    }

    gROOT->SetSelectedPad(fMainWindow->GetCanvas()->GetPad(1));
}

void CXRadCubePlayer::UpdateDrawOpt()
{
    fMainWindow->GetSaveList()->SetListDrawOption(fDrawOpt->GetText());
}

void CXRadCubePlayer::ToggleFWHM()
{
    if(fUseFWHM->GetState() == kButtonUp) {
        fFWHMGateFraction->SetState(0);
    }
    else {
        fFWHMGateFraction->SetState(1);
    }
}

bool CXRadCubePlayer::UseFWHM()
{
    if(!fMainWindow) return false;
    if(!fMainWindow->GetWSManager()->GetActiveWorkspace() || !fMainWindow->GetWSManager()->GetActiveWorkspace()->fFWHMFunction) {
        if(fUseFWHM->GetState()==kButtonDown) gbash_color->WarningMessage(Form("No FWHM function in the active workspace: %s -> remove FWHM option",fMainWindow->GetWSManager()->GetActiveWSName().Data()));
        return false;
    }
    if(fUseFWHM->GetState()==kButtonUp) return false;
    return true;
}

void CXRadCubePlayer::ChangeNProjections()
{
    fNProjs = fNProjections->GetIntNumber();
    if(GetProj()) {
        Init(GetProj()->fRadReader);
    }
}

ClassImp(CXRadCubePlayer)
