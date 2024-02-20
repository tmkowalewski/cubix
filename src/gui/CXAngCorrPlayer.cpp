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

#include "CXAngCorrPlayer.h"
#include "cubix_config.h"

#include <iostream>

#include "TGNumberEntry.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGSlider.h"
#include "TFrame.h"
#include "TROOT.h"
#include "TF1.h"
#include "TMath.h"
#include "TFitResult.h"
#include "TVirtualFitter.h"
#include "TGraphAsymmErrors.h"
#include "TGProgressBar.h"
#include "TSystem.h"
#include "TLine.h"
#include "TLatex.h"
#include "TF2.h"

#ifdef HAS_MATHMORE
#include "Math/SpecFuncMathMore.h"
#endif

#include "CXBashColor.h"
#include "CXMainWindow.h"
#include "CXDialogBox.h"

#include "tklog.h"

using namespace std;

CXAngCorrPlayer::CXAngCorrPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "New instance", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *button = new TGTextButton(fHorizontalFrame, "New instance");
    button->Connect("Clicked()", "CXAngCorrPlayer", this, "NewInstance()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    //----------------------------------------------------------------------------------------//

    fGroupFrame = new TGGroupFrame(MotherFrame, "Theoretical plot", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "2J1: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,1,0,0));
    fNESpins[0] = new TGNumberEntry(fHorizontalFrame, 8, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fNESpins[0]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "UpdateTheory()");
    fHorizontalFrame->AddFrame(fNESpins[0],new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,1,10,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "2J2: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,1,0,0));
    fNESpins[1] = new TGNumberEntry(fHorizontalFrame, 4, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fNESpins[1]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "UpdateTheory()");
    fHorizontalFrame->AddFrame(fNESpins[1],new TGLayoutHints(kLHintsCenterY | kLHintsExpandX ,1,10,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "2J3: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,1,0,0));
    fNESpins[2] = new TGNumberEntry(fHorizontalFrame, 0, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fNESpins[2]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "UpdateTheory()");
    fHorizontalFrame->AddFrame(fNESpins[2],new TGLayoutHints(kLHintsCenterY | kLHintsExpandX ,1,10,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,-10,-10,5,5));


    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Mix 1/2: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fSlider[0] = new TGHSlider(fHorizontalFrame, 100, kSlider1 | kScaleBoth, 0);
    fSlider[0]->Connect("PositionChanged(Int_t)", "CXAngCorrPlayer", this, "DoSlider(Int_t)");
    fSlider[0]->Connect("PositionChanged(Int_t)", "CXAngCorrPlayer", this, "UpdateTheory()");
    fSlider[0]->SetRange(-TMath::PiOver2()*1000.,TMath::PiOver2()*1000.);
    fSlider[0]->SetPosition(0);
    fHorizontalFrame->AddFrame(fSlider[0], new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    fNEMixing[0] = new TGNumberEntry(fHorizontalFrame, 0., 5, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNEMixing[0]->GetNumberEntry()->SetText("0.00");
    fNEMixing[0]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "DoSlider(Int_t)");
    fNEMixing[0]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "UpdateTheory()");
    fHorizontalFrame->AddFrame(fNEMixing[0],new TGLayoutHints(kLHintsCenterY | kLHintsExpandX ,1,10,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,-10,-10,5,5));


    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Mix 2/3: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fSlider[1] = new TGHSlider(fHorizontalFrame, 100, kSlider1 | kScaleBoth, 1);
    fSlider[1]->Connect("PositionChanged(Int_t)", "CXAngCorrPlayer", this, "DoSlider(Int_t)");
    fSlider[1]->Connect("PositionChanged(Int_t)", "CXAngCorrPlayer", this, "UpdateTheory()");
    fSlider[1]->SetRange(-TMath::PiOver2()*1000.,TMath::PiOver2()*1000.);
    fSlider[1]->SetPosition(0);
    fHorizontalFrame->AddFrame(fSlider[1], new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    fNEMixing[1] = new TGNumberEntry(fHorizontalFrame, 0., 5, 1, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNEMixing[1]->GetNumberEntry()->SetText("0.00");
    fNEMixing[1]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "DoSlider(Int_t)");
    fNEMixing[1]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "UpdateTheory()");
    fHorizontalFrame->AddFrame(fNEMixing[1],new TGLayoutHints(kLHintsCenterY | kLHintsExpandX ,1,10,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,-10,-10,5,5));


    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fAksLabel = new TGLabel(fHorizontalFrame, "Ak values:");
    fAksLabel->SetTextColor(CXblue);
    fAksLabel->SetTextJustify(kTextLeft);
    fHorizontalFrame->AddFrame(fAksLabel,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,10,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,-10,-10,5,5));

    //----------------------------------------------------------------------------------------//

    fGroupFrame = new TGGroupFrame(MotherFrame, "Mixing evaluation", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Fix mix 1/2: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fFixMixing[0] = new TGCheckButton(fHorizontalFrame);
    fFixMixing[0]->SetState(kButtonDown);
    fHorizontalFrame->AddFrame(fFixMixing[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft ,1,3,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Fix mix 2/3: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fFixMixing[1] = new TGCheckButton(fHorizontalFrame);
    fFixMixing[1]->SetState(kButtonUp);
    fHorizontalFrame->AddFrame(fFixMixing[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft ,1,3,0,0));

    button = new TGTextButton(fHorizontalFrame, "  Plot  ");
    button->Connect("Clicked()", "CXAngCorrPlayer", this, "PlotMixingEvaluation()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "N points: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,1,0,0));
    fNEMixingPoints = new TGNumberEntry(fHorizontalFrame, 100, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fNEMixingPoints->SetWidth(70);
    fHorizontalFrame->AddFrame(fNEMixingPoints,new TGLayoutHints(kLHintsCenterY,1,10,0,0));

    fProgressBar = new TGHProgressBar(fHorizontalFrame, TGProgressBar::kFancy, 300);
    fProgressBar->SetBarColor("lightblue");
    fProgressBar->ShowPosition(kTRUE, kFALSE, "%.0f %%");
    fHorizontalFrame->AddFrame(fProgressBar,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));

    TGPictureButton *fPictButton = new TGPictureButton(fHorizontalFrame, gClient->GetPicture("StopLoading.gif"));
    fPictButton->Connect("Clicked()", "CXAngCorrPlayer", this, "StopMixingEvaluation()");
    fHorizontalFrame->AddFrame(fPictButton,new TGLayoutHints(kLHintsCenterY,1,5,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    //----------------------------------------------------------------------------------------//

    fGroupFrame = new TGGroupFrame(MotherFrame, "Experimental analysis", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    button = new TGTextButton(fHorizontalFrame, "  Fit angular distribution  ");
    button->Connect("Clicked()", "CXAngCorrPlayer", this, "FitDistribution()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));

    fAnglesButtons[0] = new TGRadioButton(fHorizontalFrame,"deg",0);
    fAnglesButtons[0]->SetState(EButtonState::kButtonDown);
    fAnglesButtons[0]->Connect("Clicked()","CXAngCorrPlayer", this, "HandleButtons()");
    fHorizontalFrame->AddFrame(fAnglesButtons[0],new TGLayoutHints(kLHintsCenterY,5,5,0,0));
    fAnglesButtons[1] = new TGRadioButton(fHorizontalFrame,"rad",1);
    fAnglesButtons[1]->Connect("Clicked()","CXAngCorrPlayer", this, "HandleButtons()");
    fHorizontalFrame->AddFrame(fAnglesButtons[1],new TGLayoutHints(kLHintsCenterY,5,5,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    TGLabel *label;
    TString Names[2] = {"A2","A4"};

    for(int i=0 ; i<2 ; i++) {
        fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
        fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, Form("%s:",Names[i].Data())),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 10, 10, 0, 0));
        label->SetTextColor(CXblue);

        fExpAks[i][0] = new TGNumberEntry(fHorizontalFrame, 0., 5,0, TGNumberFormat::kNESRealFour, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);
        fExpAks[i][0]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "UpdateData()");

        fHorizontalFrame->AddFrame(fExpAks[i][0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,1,5,0,0));
        fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));

        fExpAks[i][1] = new TGNumberEntry(fHorizontalFrame, 0., 5,0, TGNumberFormat::kNESRealFour, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);
        fExpAks[i][1]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "UpdateData()");

        fHorizontalFrame->AddFrame(fExpAks[i][1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,1,5,0,0));
        fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));

        fExpAks[i][2] = new TGNumberEntry(fHorizontalFrame, 0., 5,0, TGNumberFormat::kNESRealFour, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);
        fExpAks[i][2]->Connect("ValueSet(Long_t)", "CXAngCorrPlayer", this, "UpdateData()");

        fHorizontalFrame->AddFrame(fExpAks[i][2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,1,5,0,0));

        fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));
    }

    // fExpAks[0][0]->SetNumber(0.1);fExpAks[0][1]->SetNumber(0.2);fExpAks[0][2]->SetNumber(0.3);
    // fExpAks[1][0]->SetNumber(-0.01);fExpAks[1][1]->SetNumber(0);fExpAks[1][2]->SetNumber(0.01);

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    button = new TGTextButton(fHorizontalFrame, "  Fit Qi factors  ");
    button->Connect("Clicked()", "CXAngCorrPlayer", this, "FitCorrectionFactors()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));

    button = new TGTextButton(fHorizontalFrame, "  Save Qi  ");
    button->Connect("Clicked()", "CXAngCorrPlayer", this, "SaveCorrectionFactors()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    TString QNames[2] = {"Q2","Q4"};

    for(int i=0 ; i<2 ; i++) {
        fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
        fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, Form("%s:",QNames[i].Data())),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 10, 10, 0, 0));
        label->SetTextColor(CXblue);

        fExpQks[i][0] = new TGNumberEntry(fHorizontalFrame, 1., 5,0, TGNumberFormat::kNESRealFour, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);

        fHorizontalFrame->AddFrame(fExpQks[i][0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,1,5,0,0));
        fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));

        fExpQks[i][1] = new TGNumberEntry(fHorizontalFrame, 1., 5,0, TGNumberFormat::kNESRealFour, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);

        fHorizontalFrame->AddFrame(fExpQks[i][1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,1,5,0,0));
        fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));

        fExpQks[i][2] = new TGNumberEntry(fHorizontalFrame, 1., 5,0, TGNumberFormat::kNESRealFour, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);

        fHorizontalFrame->AddFrame(fExpQks[i][2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,1,5,0,0));

        fFixQi[i] = new TGCheckButton(fHorizontalFrame,"",i);
        fFixQi[i]->SetState(kButtonDown);
        fFixQi[i]->Connect("Clicked()","CXAngCorrPlayer", this, "HandleButtons()");

        fHorizontalFrame->AddFrame(fFixQi[i],new TGLayoutHints(kLHintsCenterY | kLHintsLeft ,1,1,0,0));

        fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));
    }

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    button = new TGTextButton(fHorizontalFrame, "  Fit mixing  ");
    button->Connect("Clicked()", "CXAngCorrPlayer", this, "FitMixing()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fMixingLabel = new TGLabel(fHorizontalFrame, "Mixing value:");
    fMixingLabel->SetTextColor(CXblue);
    fMixingLabel->SetTextJustify(kTextLeft);
    fHorizontalFrame->AddFrame(fMixingLabel,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,10,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    button = new TGTextButton(fHorizontalFrame, "  Plot theory  ");
    button->Connect("Clicked()", "CXAngCorrPlayer", this, "PlotTheoryOnDist()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    //----------------------------------------------------------------------------------------//

    for(int i=0 ; i<4 ; i++) fAngCorrPads[i] = nullptr;

    UpdateTheory();
}

CXAngCorrPlayer::~CXAngCorrPlayer() = default;

void CXAngCorrPlayer::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXAngCorrPlayer::NewInstance()
{
    if(fMainWindow == nullptr) return;

    fMainWindow->NewTab(2,2,"AngCorr");

    UpdateTheory();
}

void CXAngCorrPlayer::HandleButtons(int id)
{
    auto *btn = static_cast<TGButton*>(gTQSender);
    if (id == -1) id = btn->WidgetId();
    if(btn->InheritsFrom(TGRadioButton::Class())) {
        if(id==0) {
            fAnglesButtons[0]->SetState(kButtonDown);
            fAnglesButtons[1]->SetState(kButtonUp);
        }
        else if(id==1) {
            fAnglesButtons[1]->SetState(kButtonDown);
            fAnglesButtons[0]->SetState(kButtonUp);
        }
    }
    if(btn->InheritsFrom(TGCheckButton::Class())) {
        if(btn->GetState()==kButtonUp) {
            for(int i=0 ; i<3 ; i++) fExpQks[id][i]->SetState(false);
        }
        else {
            for(int i=0 ; i<3 ; i++) fExpQks[id][i]->SetState(true);
        }
    }
}

void CXAngCorrPlayer::GetCurrentInstance()
{
    if(fMainWindow == nullptr) return;

    fTheoreticalDistribution = nullptr;
    for(int i=0 ; i<4 ; i++) fAngCorrPads[i] = nullptr;

    if(!fMainWindow->GetCanvas() || !((TString)fMainWindow->GetCanvas()->GetName()).BeginsWith("AngCorr")) {
        // gbash_color->ErrorMessage("Current Canvas is not a angular correlation canvas, ignored");
        return;
    }

    for(int i=0 ; i<4 ; i++) fAngCorrPads[i] = fMainWindow->GetCanvas()->GetPad(i+1);

    fAngularDistributionGraph = GetElement<TGraph>(fAngCorrPads[2]);
    fAngularDistributionFunction = GetElement<TF1>(fAngCorrPads[2],"_Exp");
    fAngularDistributionFunction_err = GetElement<TH1>(fAngCorrPads[2],"AngCorrConfidence95");
    fTheoreticalDistributionOnExp = GetElement<TF1>(fAngCorrPads[2],"_TheoOnExp");

    fTheoreticalDistribution = GetElement<TF1>(fAngCorrPads[0]);

    fA2A4MixingGraph = GetElement<TGraph>(fAngCorrPads[1],"A2A4MixingGraph");

    fA2A4ExpMarker = GetElement<TGraphAsymmErrors>(fAngCorrPads[1],"A2A4ExpMarker");

    fChi2Func = GetElement<TF1>(fAngCorrPads[3],"Chi2_1D_");

    fChi2Func2D = GetElement<TF2>(fAngCorrPads[3],"Chi2_2D_");
    fChi2Func2D_errband = GetElement<TF2>(fAngCorrPads[3],"Chi2_2DErrBand_");
}

template <typename T>
T *CXAngCorrPlayer::GetElement(TVirtualPad *_pad, TString _namepattern)
{
    if(!_pad) return nullptr;
    for(auto &&obj: *_pad->GetListOfPrimitives()) {
        if(obj->InheritsFrom(T::Class())) {
            TString name = obj->GetName();
            if(_namepattern == "" || name.Contains(_namepattern)) return dynamic_cast<T*>(obj);
        }
    }
    return nullptr;
}

void CXAngCorrPlayer::DoSlider(Int_t _pos)
{
    Int_t id=-1;
    TGFrame *frm = (TGFrame *) gTQSender;
    if (frm->IsA()->InheritsFrom(TGSlider::Class())) {
        TGSlider *sl = (TGSlider*) frm;
        id = sl->WidgetId();
        double atandelta = _pos/1000.;
        double delta = TMath::Tan(atandelta);
        fNEMixing[id]->SetNumber(delta);
    }
    if (frm->IsA()->InheritsFrom(TGNumberEntry::Class())) {
        TGNumberEntry *ne = (TGNumberEntry*) frm;
        id = ne->WidgetId();
        double delta = ne->GetNumber();
        double atandelta = TMath::ATan(delta);
        int position = TMath::Nint(atandelta*1000.);
        fSlider[id]->SetPosition(position);
    }
}

void CXAngCorrPlayer::FitCorrectionFactors()
{
    GetCurrentInstance();

    if(!fAngularDistributionGraph) {
        gbash_color->WarningMessage("No angular distribution to fit, ignored");
        return;
    }

    double min=0;
    double max=180;
    int deg=1;
    if(fAnglesButtons[1]->GetState()==kButtonDown) {
        min=-1;
        max=1;
        deg=0;
    }

    delete fAngularDistributionFunction;
    fAngularDistributionFunction = new TF1(Form("%s_Exp",fMainWindow->GetCanvas()->GetName()), this, &CXAngCorrPlayer::ExpAngCorrFunction, min, max, 6, "CXAngCorrPlayer", "ExpAngCorrFunction");
    fAngularDistributionFunction->FixParameter(5,deg);

    double mean;
    if(deg) mean = fAngularDistributionFunction->Eval(90);
    else mean = fAngularDistributionFunction->Eval(0);

    fAngularDistributionFunction->SetParNames("A0","A2","A4","Q2","Q4");

    int TwoJ1 = fNESpins[0]->GetIntNumber();
    int TwoJ2 = fNESpins[1]->GetIntNumber();
    int TwoJ3 = fNESpins[2]->GetIntNumber();

    double mix12 = fNEMixing[0]->GetNumber();
    double mix23 = fNEMixing[1]->GetNumber();

    vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,mix12,mix23);
    Float_t _A2 = 0.;
    Float_t _A4 = 0.;
    if(Akks.size()>=1) _A2 = Akks.at(0);
    if(Akks.size()>=2) _A4 = Akks.at(1);

    fAngularDistributionFunction->SetParameter(0,fAngularDistributionFunction->Eval(mean));
    fAngularDistributionFunction->SetLineColor(kBlue);
    fAngularDistributionFunction->FixParameter(1,_A2);
    fAngularDistributionFunction->FixParameter(2,_A4);

    if(fFixQi[0]->GetState()==kButtonUp) fAngularDistributionFunction->FixParameter(3,fExpQks[0][1]->GetNumber());
    else fAngularDistributionFunction->SetParameter(3,fExpQks[0][1]->GetNumber());
    // fAngularDistributionFunction->SetParLimits(3,0.,10.);

    if(fFixQi[1]->GetState()==kButtonUp) fAngularDistributionFunction->FixParameter(4,fExpQks[1][1]->GetNumber());
    else fAngularDistributionFunction->SetParameter(4,fExpQks[1][1]->GetNumber());
    // fAngularDistributionFunction->SetParLimits(3,0.,10.);    // fAngularDistributionFunction->SetParLimits(4,0.,10.);

    TFitResultPtr r = fAngularDistributionGraph->Fit(fAngularDistributionFunction,"S0R");

    double minQ2 = fAngularDistributionFunction->GetParameter(3)-fAngularDistributionFunction->GetParError(3);
    double maxQ2 = fAngularDistributionFunction->GetParameter(3)+fAngularDistributionFunction->GetParError(3);
    double minQ4 = fAngularDistributionFunction->GetParameter(4)-fAngularDistributionFunction->GetParError(4);
    double maxQ4 = fAngularDistributionFunction->GetParameter(4)+fAngularDistributionFunction->GetParError(4);

    delete fAngularDistributionFunction_err;

    if(!r->IsValid()) {
        gbash_color->WarningMessage("Warning: Fit failed");
        fAngularDistributionFunction_err = nullptr;
    }
    else {
        if(fAnglesButtons[1]->GetState() == kButtonDown) fAngularDistributionFunction_err = new TH1D(Form("%s_AngCorrConfidence95",fMainWindow->GetCanvas()->GetName()),"AngCorr 0.95 confidence band", 180, -1, 1);
        else fAngularDistributionFunction_err = new TH1D(Form("%s_AngCorrConfidence95",fMainWindow->GetCanvas()->GetName()),"AngCorr 0.95 confidence band", 180, 0, 180);

        (TVirtualFitter::GetFitter())->GetConfidenceIntervals(fAngularDistributionFunction_err);
        fAngularDistributionFunction_err->SetLineWidth(0);
        fAngularDistributionFunction_err->SetFillColor(kBlue);
        fAngularDistributionFunction_err->SetFillColorAlpha(kBlue,0.1);
        fAngularDistributionFunction_err->SetFillStyle(1001);
        fAngularDistributionFunction_err->SetStats(false);
        fAngularDistributionFunction_err->SetDirectory(nullptr);
    }

    fAngCorrPads[2]->cd();

    fAngularDistributionFunction->Draw("same");
    if(fAngularDistributionFunction_err) fAngularDistributionFunction_err->Draw("e3 same");

    fAngCorrPads[2]->Update();
    fAngCorrPads[2]->Modified();

    if(fFixQi[0]->GetState()==kButtonDown) {
        fExpQks[0][0]->SetNumber(minQ2);
        fExpQks[0][1]->SetNumber(fAngularDistributionFunction->GetParameter(3));
        fExpQks[0][2]->SetNumber(maxQ2);
    }
    if(fFixQi[1]->GetState()==kButtonDown) {
        fExpQks[1][0]->SetNumber(minQ4);
        fExpQks[1][1]->SetNumber(fAngularDistributionFunction->GetParameter(4));
        fExpQks[1][2]->SetNumber(maxQ4);
    }
}

void CXAngCorrPlayer::FitDistribution()
{
    GetCurrentInstance();

    if(!fAngularDistributionGraph) {
        gbash_color->WarningMessage("No angular distribution to fit, ignored");
        return;
    }

    double min=0;
    double max=180;
    int deg=1;
    if(fAnglesButtons[1]->GetState()==kButtonDown) {
        min=-1;
        max=1;
        deg=0;
    }

    delete fAngularDistributionFunction;
    fAngularDistributionFunction = new TF1(Form("%s_Exp",fMainWindow->GetCanvas()->GetName()), this, &CXAngCorrPlayer::ExpAngCorrFunction, min, max, 6, "CXAngCorrPlayer", "ExpAngCorrFunction");
    fAngularDistributionFunction->FixParameter(5,deg);

    double mean;
    if(deg) mean = fAngularDistributionFunction->Eval(90);
    else mean = fAngularDistributionFunction->Eval(0);

    fAngularDistributionFunction->SetParNames("A0","A2","A4","Q2","Q4");

    fAngularDistributionFunction->SetParameter(0,fAngularDistributionFunction->Eval(mean));
    fAngularDistributionFunction->SetLineColor(kBlue);
    fAngularDistributionFunction->SetParameter(1,0);
    // fAngularDistributionFunction->SetParLimits(1, -2, 2);
    fAngularDistributionFunction->SetParameter(2,0);
    // fAngularDistributionFunction->SetParLimits(2, -2, 2);


    // calculate the extreme possible values with Qfactors to determine the real A2 A4 errors
    double minA2 =  1e12;
    double maxA2 =  -1e12;
    double minA4 =  1e12;
    double maxA4 =  -1e12;

    double Q2Min = fExpQks[0][0]->GetNumber();
    double Q2Max = fExpQks[0][2]->GetNumber();
    double Q4Min = fExpQks[1][0]->GetNumber();
    double Q4Max = fExpQks[1][2]->GetNumber();

    vector<double> Q2s{Q2Min,Q2Max};
    vector<double> Q4s{Q4Min,Q4Max};

    for(auto Q2: Q2s) {
        for(auto Q4: Q4s) {
            fAngularDistributionFunction->FixParameter(3,Q2);
            fAngularDistributionFunction->FixParameter(4,Q4);
            fAngularDistributionGraph->Fit(fAngularDistributionFunction,"Q0R");
            double A2 = fAngularDistributionFunction->GetParameter(1);
            double A4 = fAngularDistributionFunction->GetParameter(2);
            minA2 = std::min(A2-fAngularDistributionFunction->GetParError(1), minA2);
            maxA2 = std::max(A2+fAngularDistributionFunction->GetParError(1), maxA2);
            minA4 = std::min(A4-fAngularDistributionFunction->GetParError(2), minA4);
            maxA4 = std::max(A4+fAngularDistributionFunction->GetParError(2), maxA4);
        }
    }

    fAngularDistributionFunction->FixParameter(3,fExpQks[0][1]->GetNumber());
    fAngularDistributionFunction->FixParameter(4,fExpQks[1][1]->GetNumber());

    TFitResultPtr r = fAngularDistributionGraph->Fit(fAngularDistributionFunction,"S0R");

    minA2 = std::min(fAngularDistributionFunction->GetParameter(1)-fAngularDistributionFunction->GetParError(1), minA2);
    maxA2 = std::max(fAngularDistributionFunction->GetParameter(1)+fAngularDistributionFunction->GetParError(1), maxA2);
    minA4 = std::min(fAngularDistributionFunction->GetParameter(2)-fAngularDistributionFunction->GetParError(2), minA4);
    maxA4 = std::max(fAngularDistributionFunction->GetParameter(2)+fAngularDistributionFunction->GetParError(2), maxA4);

    delete fAngularDistributionFunction_err;

    if(!r->IsValid()) {
        gbash_color->WarningMessage("Warning: Fit failed");
        fAngularDistributionFunction_err = nullptr;
    }
    else {
        if(fAnglesButtons[1]->GetState() == kButtonDown) fAngularDistributionFunction_err = new TH1D(Form("%s_AngCorrConfidence95",fMainWindow->GetCanvas()->GetName()),"AngCorr 0.95 confidence band", 180, -1, 1);
        else fAngularDistributionFunction_err = new TH1D(Form("%s_AngCorrConfidence95",fMainWindow->GetCanvas()->GetName()),"AngCorr 0.95 confidence band", 180, 0, 180);

        (TVirtualFitter::GetFitter())->GetConfidenceIntervals(fAngularDistributionFunction_err);
        fAngularDistributionFunction_err->SetLineWidth(0);
        fAngularDistributionFunction_err->SetFillColor(kBlue);
        fAngularDistributionFunction_err->SetFillColorAlpha(kBlue,0.1);
        fAngularDistributionFunction_err->SetFillStyle(1001);
        fAngularDistributionFunction_err->SetStats(false);
        fAngularDistributionFunction_err->SetDirectory(nullptr);
    }

    fAngCorrPads[2]->cd();

    fAngularDistributionFunction->Draw("same");
    if(fAngularDistributionFunction_err) fAngularDistributionFunction_err->Draw("e3 same");

    fAngCorrPads[2]->Update();
    fAngCorrPads[2]->Modified();

    fExpAks[0][0]->SetNumber(minA2);
    fExpAks[0][1]->SetNumber(fAngularDistributionFunction->GetParameter(1));
    fExpAks[0][2]->SetNumber(maxA2);
    fExpAks[1][0]->SetNumber(minA4);
    fExpAks[1][1]->SetNumber(fAngularDistributionFunction->GetParameter(2));
    fExpAks[1][2]->SetNumber(maxA4);

    UpdateData();
}

void CXAngCorrPlayer::FitMixing()
{
    UpdateData();

    GetCurrentInstance();

    bool DoMix12 = fFixMixing[0]->GetState()==kButtonUp;
    bool DoMix23 = fFixMixing[1]->GetState()==kButtonUp;

    if(DoMix12 && DoMix23) FitMixing2D();
    else FitMixing1D();
}

void CXAngCorrPlayer::FitMixing1D()
{
    double ExpA2 = fExpAks[0][1]->GetNumber();
    double ExpA4 = fExpAks[1][1]->GetNumber();

    while(TGraph *g = GetElement<TGraph>(fAngCorrPads[1],"A2A4BestChi2Marker")) delete g;

    delete fChi2Func;
    fChi2Func = new TF1(Form("Chi2_1D_%s",fMainWindow->GetCanvas()->GetName()), this, &CXAngCorrPlayer::EvalChi2, -90,90, 11, "CXAngCorrPlayer", "EvalChi2");

    fAngCorrPads[3]->Clear();

    bool DoMix12 = fFixMixing[0]->GetState()==kButtonUp;
    bool DoMix23 = fFixMixing[1]->GetState()==kButtonUp;

    int mode = 0;
    if(DoMix12 && DoMix23) mode = 2;
    else if(DoMix23) mode = 1;

    int TwoJ1 = fNESpins[0]->GetIntNumber();
    int TwoJ2 = fNESpins[1]->GetIntNumber();
    int TwoJ3 = fNESpins[2]->GetIntNumber();

    double mix = fNEMixing[1]->GetNumber();
    if(mode==1) mix = fNEMixing[0]->GetNumber();

    fChi2Func->SetParameters(mode,
                             TwoJ1,TwoJ2,TwoJ3,
                             mix,
                             ExpA2,fExpAks[0][0]->GetNumber(),fExpAks[0][2]->GetNumber(),
                             ExpA4,fExpAks[1][0]->GetNumber(),fExpAks[1][2]->GetNumber()
                             );
    fChi2Func->SetNpx(5000);
    if(mode==0) fChi2Func->GetXaxis()->SetTitle("ATan(#delta_{12}) (deg)");
    else if(mode==1) fChi2Func->GetXaxis()->SetTitle("ATan(#delta_{23}) (deg)");
    fChi2Func->GetYaxis()->SetTitle("#Chi^{2}/ndf");
    fChi2Func->GetXaxis()->SetTitleSize(0.05);
    fChi2Func->GetXaxis()->SetLabelSize(0.05);
    fChi2Func->GetYaxis()->SetTitleSize(0.05);
    fChi2Func->GetYaxis()->SetLabelSize(0.05);
    fChi2Func->GetYaxis()->SetTitleOffset(1.10);
    fChi2Func->GetXaxis()->SetTitleOffset(1.05);
    fChi2Func->GetXaxis()->CenterTitle();
    fChi2Func->GetYaxis()->CenterTitle();


    TF1 *fChi2FuncDeriv = new TF1("Chi2FuncDeriv",this,&CXAngCorrPlayer::EvalChi2Deriv,-90,90,0, "CXAngCorrPlayer", "EvalChi2Deriv");
    TF1 *fChi2FuncDeriv2 = new TF1("Chi2FuncDeriv2",this,&CXAngCorrPlayer::EvalChi2Deriv2,-90,90,0, "CXAngCorrPlayer", "EvalChi2Deriv2");
    fChi2FuncDeriv->SetNpx(5000);
    fChi2FuncDeriv2->SetNpx(5000);

    // Look for local minima

    double rangemin=-90;
    double rangemax=89;
    double min=0.;

    vector<double> vminima;

    gErrorIgnoreLevel=kFatal;
    while(true && rangemin<=rangemax) {
        min = fChi2FuncDeriv->GetX(0.,rangemin,rangemax);
        if(isnan(min)) break;
        if(fChi2FuncDeriv2->Eval(min)>0) {
            vminima.push_back(min);
        }
        rangemin = min+0.5;
    }

    delete fChi2FuncDeriv;
    delete fChi2FuncDeriv2;

    gErrorIgnoreLevel=kInfo;

    fAngCorrPads[3]->cd();
    fChi2Func->Draw();
    fAngCorrPads[3]->SetLogy(1);
    fAngCorrPads[3]->Modified();
    fAngCorrPads[3]->Update();
    fAngCorrPads[3]->GetFrame()->SetBit(TObject::kCannotPick);

    double BestChi2 = numeric_limits<double>::max();
    double BestDelta=0.;
    double BestDeltaTheta=0.;

    int nmin=0;
    for(auto min: vminima) {
        fAngCorrPads[3]->cd();
        fAngCorrPads[3]->Modified();
        fAngCorrPads[3]->Update();

        double y1 = TMath::Power(10,gPad->GetUymin());
        double y2 = TMath::Power(10,gPad->GetUymax());

        Color_t col = kBlue;
        if(nmin==1) col = kMagenta;
        if(nmin==2) col = kCyan;
        nmin++;

        TLine *l = new TLine(min,y1,min,y2);
        l->SetLineColor(col);
        l->SetLineStyle(kDashed);
        l->Draw();
        l->SetBit(TObject::kCannotPick);

        TString ModeStr="12";
        if(mode==1) ModeStr="23";
        TLatex *text = new TLatex(min-0.005*(gPad->GetFrame()->GetX2()-gPad->GetFrame()->GetX1()),y2,Form("#delta_{%s} = %.3g, #Chi^{2} = %.3g ",ModeStr.Data(),tan(min*TMath::DegToRad()),fChi2Func->Eval(min)));
        text->SetTextColor(col);
        text->SetTextSize(0.06);
        text->SetTextFont(132);
        text->SetTextAlign(31);
        text->SetTextAngle(90);
        text->Draw();
        text->SetBit(TObject::kCannotPick);

        double mix12 = fNEMixing[0]->GetNumber();
        double mix23 = fNEMixing[1]->GetNumber();

        if(mode==0) mix12 = tan(min*TMath::DegToRad());
        if(mode==1) mix23 = tan(min*TMath::DegToRad());

        vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,mix12,mix23);
        Float_t _A2 = 0.;
        Float_t _A4 = 0.;
        if(Akks.size()>=1) _A2 = Akks.at(0);
        if(Akks.size()>=2) _A4 = Akks.at(1);


        TGraph *gr = new TGraph;
        gr->SetName(Form("A2A4BestChi2Marker_%d",col));
        gr->SetMarkerStyle(20);
        gr->SetMarkerColor(col);
        gr->AddPoint(_A2,_A4);

        fAngCorrPads[1]->cd();
        gr->Draw("p");
        gr->SetBit(TObject::kCannotPick);

        if(fChi2Func->Eval(min)<BestChi2) {
            BestChi2=fChi2Func->Eval(min);
            BestDelta = tan(min*TMath::DegToRad());
            BestDeltaTheta = min;
        }
    }

    // estimation of mixing uncertainty

    double mindelta=0., maxdelta=0.;

    // using Chi2min+1
    double dtheta=10;
    gErrorIgnoreLevel=kFatal;
    while(true && dtheta<90) {
        mindelta = fChi2Func->GetX(BestChi2+1,BestDeltaTheta-dtheta,BestDeltaTheta);
        maxdelta = fChi2Func->GetX(BestChi2+1,BestDeltaTheta,BestDeltaTheta+dtheta);
        if(!(isnan(mindelta) || isnan(maxdelta))) {
            mindelta = tan(mindelta*TMath::DegToRad());
            maxdelta = tan(maxdelta*TMath::DegToRad());
            break;
        }
        dtheta += 10;
    }
    gErrorIgnoreLevel=kInfo;

    TF1 *ftest = new TF1("temp", this, &CXAngCorrPlayer::EvalChi2, -90,90, 11, "CXAngCorrPlayer", "EvalChi2");
    ftest->SetParameters(mode+10,
                         TwoJ1,TwoJ2,TwoJ3,
                         mix,
                         ExpA2,fExpAks[0][0]->GetNumber(),fExpAks[0][2]->GetNumber(),
                         ExpA4,fExpAks[1][0]->GetNumber(),fExpAks[1][2]->GetNumber()
                         );
    ftest->SetNpx(5000);

    double mindeltaerr=0., maxdeltaerr=0.;

    gbash_color->InfoMessage("Fit Mixing:");

    bool ok = false;

    if(!(isnan(mindelta) || isnan(maxdelta))) {
        gbash_color->InfoMessage("Mixing estimation based on Chi2min+1 evaluation:");
        fMixingLabel->SetText(Form("Mixing value: %.3g [%.3g ; %.3g]",BestDelta,mindelta,maxdelta));
        cout << Form("Mixing value: %.3g [%.3g ; %.3g]",BestDelta,mindelta,maxdelta) << endl;
        ok = true;
    }
    if(ftest->GetMaximum()>0.) {

        double mindeltaerr=0., maxdeltaerr=0.;

        double dtheta=10;
        gErrorIgnoreLevel=kFatal;
        while(true && dtheta<90) {
            double test1 = ftest->GetMaximumX(BestDeltaTheta-dtheta,BestDeltaTheta);
            if(abs(test1-mindeltaerr)>0.01) {
                mindeltaerr = test1;
                dtheta += 5;
                continue;
            }
            double test2 = ftest->GetMaximumX(BestDeltaTheta,BestDeltaTheta+dtheta);
            if(abs(test2-maxdeltaerr)>0.01) {
                maxdeltaerr = test2;
                dtheta += 5;
                continue;
            }
            break;
        }
        gErrorIgnoreLevel=kInfo;
        mindeltaerr = tan(mindeltaerr*TMath::DegToRad());
        maxdeltaerr = tan(maxdeltaerr*TMath::DegToRad());

        gbash_color->InfoMessage("Mixing estimation based on experimental error bars:");
        fMixingLabel->SetText(Form("Mixing value: %.3g [%.3g ; %.3g]",BestDelta,mindeltaerr,maxdeltaerr));
        cout << Form("Mixing value: %.3g [%.3g ; %.3g]",BestDelta,mindeltaerr,maxdeltaerr) << endl;
        ok = true;
    }
    else {
        gbash_color->WarningMessage("Experimental point not on the curve, uncertainties estimated using the Chi2+1 values");
    }
    if(!ok) {
        gbash_color->WarningMessage("could not find any value corresponding to Chi2min+1");
        fMixingLabel->SetText("Mixing value: ?");
    }

    delete ftest;

    fAngCorrPads[1]->Modified();
    fAngCorrPads[1]->Update();
    fAngCorrPads[3]->Modified();
    fAngCorrPads[3]->Update();
}

void CXAngCorrPlayer::FitMixing2D()
{
    double ExpA2 = fExpAks[0][1]->GetNumber();
    double ExpA4 = fExpAks[1][1]->GetNumber();

    while(TGraph *g = GetElement<TGraph>(fAngCorrPads[1],"A2A4BestChi2Marker")) delete g;

    delete fChi2Func2D;
    fChi2Func2D = new TF2(Form("Chi2_2D_%s",fMainWindow->GetCanvas()->GetName()), this, &CXAngCorrPlayer::EvalChi2, -90,90,-90,90, 11, "CXAngCorrPlayer", "EvalChi2");

    fAngCorrPads[3]->Clear();

    int TwoJ1 = fNESpins[0]->GetIntNumber();
    int TwoJ2 = fNESpins[1]->GetIntNumber();
    int TwoJ3 = fNESpins[2]->GetIntNumber();

    fChi2Func2D->SetParameters(2,
                               TwoJ1,TwoJ2,TwoJ3,
                               0.,
                               ExpA2,fExpAks[0][0]->GetNumber(),fExpAks[0][2]->GetNumber(),
                               ExpA4,fExpAks[1][0]->GetNumber(),fExpAks[1][2]->GetNumber()
                               );
    fChi2Func2D->SetNpx(250);
    fChi2Func2D->SetNpy(250);
    fChi2Func2D->GetXaxis()->SetTitle("ATan(#delta_{12}) (deg)");
    fChi2Func2D->GetYaxis()->SetTitle("ATan(#delta_{23}) (deg)");
    fChi2Func2D->GetZaxis()->SetTitle("#Chi^{2}/ndf");
    fChi2Func2D->GetXaxis()->SetTitleSize(0.05);
    fChi2Func2D->GetXaxis()->SetLabelSize(0.05);
    fChi2Func2D->GetYaxis()->SetTitleSize(0.05);
    fChi2Func2D->GetYaxis()->SetLabelSize(0.05);
    fChi2Func2D->GetYaxis()->SetTitleOffset(1.05);
    fChi2Func2D->GetXaxis()->SetTitleOffset(1.05);
    fChi2Func2D->GetXaxis()->CenterTitle();
    fChi2Func2D->GetYaxis()->CenterTitle();

    double Chi2min = fChi2Func2D->GetMinimum();
    double atandelta1min, atandelta2min;
    fChi2Func2D->GetMinimumXY(atandelta1min,atandelta2min);
    double delta1min = tan(atandelta1min*TMath::DegToRad());
    double delta2min = tan(atandelta2min*TMath::DegToRad());

    fAngCorrPads[3]->cd();

    // Double_t contours[1] = {Chi2min+1};
    // Chi2Func2D->SetContour(1, contours);
    // Chi2Func2D->Draw("CONT Z LIST");
    // fAngCorrPads[3]->Update();

    // TObjArray *conts = (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
    // TList* contLevel = dynamic_cast<TList*>(conts->Last());

    // delete fChi2Func2D_errband;
    // fChi2Func2D_errband = dynamic_cast<TGraph*>(contLevel->Last()->Clone());
    // fChi2Func2D_errband->SetName(((TString)Chi2Func2D->GetName()).ReplaceAll("Chi2_2D_","Chi2_2DErrBand_"));
    // fChi2Func2D_errband->SetLineColor(kRed);
    // fChi2Func2D_errband->SetLineWidth(3);

    // delete Chi2Func2D;

    fAngCorrPads[3]->cd();
    fChi2Func2D->Draw("col");
    fAngCorrPads[3]->Update();

    // auto getx = [this] (double x) -> double {
    //     double width = 180.;
    //     double widthcurrent = this->GetPad(3)->GetUxmax()-this->GetPad(3)->GetUxmin();
    //     return this->GetPad(3)->GetUxmin() + (x + 90)*widthcurrent/width;
    // };
    // auto gety = [this] (double x) -> double {
    //     double width = 180.;
    //     double widthcurrent = this->GetPad(3)->GetUymax()-this->GetPad(3)->GetUymin();
    //     return this->GetPad(3)->GetUymin() + (x + 90)*widthcurrent/width;
    // };
    // // ça marche pas... quand on zoom ça re change la valeur du range du pad

    // for(int i=0 ; i<fChi2Func2D_errband->GetN() ; i++) {
    //     fChi2Func2D_errband->SetPoint(i,fChi2Func2D_errband->GetPointX(i),fChi2Func2D_errband->GetPointY(i));
    // }
    // fChi2Func2D_errband->Draw("C");
    // fChi2Func2D_errband->SetBit(TObject::kCannotPick);

    delete fChi2Func2D_errband;
    fChi2Func2D_errband = (TF2*)fChi2Func2D->Clone(((TString)fChi2Func2D->GetName()).ReplaceAll("Chi2_2D_","Chi2_2DErrBand_"));

    Double_t contours[1] = {Chi2min+1};
    fChi2Func2D_errband->SetContour(1, contours);
    fChi2Func2D_errband->Draw("same");
    fChi2Func2D_errband->SetBit(TObject::kCannotPick);

    TGraph *gr = new TGraph;
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kMagenta);
    gr->AddPoint(atandelta1min,atandelta2min);
    gr->Draw("p");
    gr->SetBit(TObject::kCannotPick);

    TLatex *text = new TLatex(atandelta1min,atandelta2min-5,Form("#delta_{1}=%.3g, #delta_{2}=%.3g, #Chi^{2}=%.1f ",delta1min,delta2min,Chi2min));
    text->SetTextColor(kMagenta);
    text->SetTextSize(0.05);
    text->SetTextFont(132);
    text->SetTextAlign(23);
    text->SetTextAngle(0);
    text->Draw();
    text->SetBit(TObject::kCannotPick);

    fAngCorrPads[3]->SetLogy(0);
    fAngCorrPads[3]->SetLogz(1);
    fAngCorrPads[3]->Modified();
    fAngCorrPads[3]->Update();
    fAngCorrPads[3]->GetFrame()->SetBit(TObject::kCannotPick);

    vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,delta1min,delta2min);
    Float_t _A2 = 0.;
    Float_t _A4 = 0.;
    if(Akks.size()>=1) _A2 = Akks.at(0);
    if(Akks.size()>=2) _A4 = Akks.at(1);

    gr = new TGraph;
    gr->SetName("A2A4BestChi2Marker");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kMagenta);
    gr->AddPoint(_A2,_A4);

    fAngCorrPads[1]->cd();
    gr->Draw("p");
    gr->SetBit(TObject::kCannotPick);

    gbash_color->InfoMessage("2D Mixing result:");
    fMixingLabel->SetText(Form("Mixing values: delta12 = %.3g, delta23 = %.3g",delta1min,delta2min));
    cout << Form("Mixing values: delta12 = %.3g, delta23 = %.3g",delta1min,delta2min) << endl;

    fAngCorrPads[1]->Modified();
    fAngCorrPads[1]->Update();
}

void CXAngCorrPlayer::UpdateData()
{
    double ExpA2 = fExpAks[0][1]->GetNumber();
    double ExpA2errlow = fExpAks[0][0]->GetNumber();
    double ExpA2errhigh = fExpAks[0][2]->GetNumber();

    double ExpA4 = fExpAks[1][1]->GetNumber();
    double ExpA4errlow = fExpAks[1][0]->GetNumber();
    double ExpA4errhigh = fExpAks[1][2]->GetNumber();

    GetCurrentInstance();

    delete fA2A4ExpMarker;
    if(ExpA2==0. && ExpA4==0. && ExpA2errlow==0. && ExpA2errhigh==0. && ExpA4errlow==0. && ExpA4errhigh==0. ) return;
    if(!fA2A4MixingGraph) return;

    fA2A4ExpMarker = new TGraphAsymmErrors;
    fA2A4ExpMarker->SetName("A2A4ExpMarker");
    fA2A4ExpMarker->SetMarkerStyle(20);
    fA2A4ExpMarker->SetMarkerColor(kRed);
    fA2A4ExpMarker->SetFillColor(kRed);
    fA2A4ExpMarker->SetFillColorAlpha(kRed,0.3);
    fA2A4ExpMarker->SetFillStyle(1001);

    fA2A4ExpMarker->SetPoint(0,ExpA2,ExpA4);
    fA2A4ExpMarker->SetPointEXlow(0,ExpA2-fExpAks[0][0]->GetNumber());
    fA2A4ExpMarker->SetPointEXhigh(0,fExpAks[0][2]->GetNumber()-ExpA2);
    fA2A4ExpMarker->SetPointEYlow(0,ExpA4-fExpAks[1][0]->GetNumber());
    fA2A4ExpMarker->SetPointEYhigh(0,fExpAks[1][2]->GetNumber()-ExpA4);

    double minx,maxx,miny,maxy;
    minx = *min_element(fA2A4MixingGraph->GetX(), fA2A4MixingGraph->GetX()+fA2A4MixingGraph->GetN());
    miny = *min_element(fA2A4MixingGraph->GetY(), fA2A4MixingGraph->GetY()+fA2A4MixingGraph->GetN());
    maxx = *max_element(fA2A4MixingGraph->GetX(), fA2A4MixingGraph->GetX()+fA2A4MixingGraph->GetN());
    maxy = *max_element(fA2A4MixingGraph->GetY(), fA2A4MixingGraph->GetY()+fA2A4MixingGraph->GetN());

    minx = min(minx,fExpAks[0][0]->GetNumber());
    maxx = max(maxx,fExpAks[0][2]->GetNumber());
    miny = min(miny,fExpAks[1][0]->GetNumber());
    maxy = max(maxy,fExpAks[1][2]->GetNumber());

    double deltaX = (maxx-minx)*0.1;
    double deltaY = (maxy-miny)*0.1;

    TH1 *frame = GetElement<TH1>(fAngCorrPads[1],"hframe");
    frame->GetYaxis()->SetRangeUser(miny-deltaY,maxy+deltaY);
    frame->GetXaxis()->SetRangeUser(minx-deltaX,maxx+deltaX);

    fAngCorrPads[1]->cd();
    fA2A4ExpMarker->Draw("2");
    fA2A4ExpMarker->Draw("p");

    fAngCorrPads[1]->Modified();
    fAngCorrPads[1]->Update();
    fA2A4ExpMarker->SetBit(TObject::kCannotPick);
}

void CXAngCorrPlayer::UpdateTheory()
{
    int TwoJ1 = fNESpins[0]->GetIntNumber();
    int TwoJ2 = fNESpins[1]->GetIntNumber();
    int TwoJ3 = fNESpins[2]->GetIntNumber();

    double mix12 = fNEMixing[0]->GetNumber();
    double mix23 = fNEMixing[1]->GetNumber();

    auto Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,mix12,mix23);

    TString label = "Ak values: ";
    for(int i=0 ; i<Akks.size() ; i++) {
        label += Form("A%d = %.6g  ",2+i*2,Akks.at(i));
    }
    fAksLabel->SetText(label);

    GetCurrentInstance();

    if(fAngCorrPads[0] == nullptr) return;

    array<Float_t,2> SavedRange{};
    SavedRange.at(0) = fAngCorrPads[0]->GetUymin();
    SavedRange.at(1) = fAngCorrPads[0]->GetUymax();

    if(fTheoreticalDistribution==nullptr) {
        SavedRange.at(0) = 0.5;
        SavedRange.at(1) = 1.5;
    }

    delete fTheoreticalDistribution;
    fTheoreticalDistribution = new TF1(Form("%s_Theo",fMainWindow->GetCanvas()->GetName()), this, &CXAngCorrPlayer::TheoreticalAngCorrFunction, 0, 180, 7, "CXAngCorrPlayer", "TheoreticalAngCorrFunction");
    fTheoreticalDistribution->SetParameters(1,TwoJ1,TwoJ2,TwoJ3,mix12,mix23,1);

    fTheoreticalDistribution->SetNpx(5000);
    fTheoreticalDistribution->SetLineColor(kRed);
    fTheoreticalDistribution->GetXaxis()->SetTitle("#theta (degree)");
    fTheoreticalDistribution->GetYaxis()->SetTitle("W(#theta)");
    fTheoreticalDistribution->GetYaxis()->SetTitleOffset(1.10);
    fTheoreticalDistribution->GetXaxis()->SetTitleOffset(1.05);
    fTheoreticalDistribution->GetXaxis()->SetTitleSize(0.05);
    fTheoreticalDistribution->GetXaxis()->SetLabelSize(0.05);
    fTheoreticalDistribution->GetYaxis()->SetTitleSize(0.05);
    fTheoreticalDistribution->GetYaxis()->SetLabelSize(0.05);
    fTheoreticalDistribution->GetXaxis()->CenterTitle();
    fTheoreticalDistribution->GetYaxis()->CenterTitle();

    fTheoreticalDistribution->GetYaxis()->SetRangeUser(SavedRange.at(0),SavedRange.at(1));

    fAngCorrPads[0]->cd();
    fTheoreticalDistribution->Draw();
    fAngCorrPads[0]->Modified();
    fAngCorrPads[0]->Update();
    fAngCorrPads[0]->GetFrame()->SetBit(TObject::kCannotPick);

    if(fA2A4MixingTheoMarker) {
        Float_t _A2 = 0.;
        Float_t _A4 = 0.;
        if(Akks.size()>=1) _A2 = Akks.at(0);
        if(Akks.size()>=2) _A4 = Akks.at(1);
        fA2A4MixingTheoMarker->SetPoint(0,_A2,_A4);
        fAngCorrPads[1]->Modified();
        fAngCorrPads[1]->Update();
    }

    UpdateData();
}

// Thesis Marcelo Barbosa, p 40
// https://cds.cern.ch/record/1641449/files/CERN-THESIS-2010-294.pdf?subformat=pdfa
// Fk(LL′IiI)
double CXAngCorrPlayer::Fk(int twoL1, int twoL1p, int twoIi, int twoI, int k)
{
    double tot = 0;

#ifdef HAS_MATHMORE
    tot = pow(-1.,(twoIi+twoI-2)/2)*
          sqrt((twoL1+1)*(twoL1p+1)*(twoI+1)*(2*k+1))*
          ROOT::Math::wigner_3j(twoL1,twoL1p,2*k,2,-2,0)*          // wigner_3j takes 2ji as input
          ROOT::Math::wigner_6j(twoL1,twoL1p,2*k,twoI,twoI,twoIi); // wigner_6j takes 2ji as input

#endif
    return  tot;
}

vector<double> CXAngCorrPlayer::Eval_Ak(int TwoJ1, int TwoJ2, int TwoJ3, double _mix12, double _mix23)
{
    vector<double> Akks;

    //Determine multipolarity of gammas
    //So first find smallest possible L1 L2
    int twoL1  = TMath::Max(abs(TwoJ1-TwoJ2),2); // no monopole
    int twoL1p = twoL1+2;

    int twoL2  = TMath::Max(abs(TwoJ3-TwoJ2),2); // no monopole
    int twoL2p = twoL2+2;

    int kmax = TMath::Min(TwoJ2, TMath::Min(twoL1p,twoL2p));

    for(int k=2; k<=kmax; k+=2) {
        double Ak1 = 1./(1.+_mix12*_mix12)*(Fk(twoL1,twoL1,TwoJ1,TwoJ2,k) - 2.*_mix12*Fk(twoL1,twoL1p,TwoJ1,TwoJ2,k) + _mix12*_mix12*Fk(twoL1p,twoL1p,TwoJ1,TwoJ2,k));
        double Ak2 = 1./(1.+_mix23*_mix23)*(Fk(twoL2,twoL2,TwoJ3,TwoJ2,k) + 2.*_mix23*Fk(twoL2,twoL2p,TwoJ3,TwoJ2,k) + _mix23*_mix23*Fk(twoL2p,twoL2p,TwoJ3,TwoJ2,k));
        double Akk = Ak1*Ak2;
        Akks.push_back(Akk);
    }

    return Akks;
}

double CXAngCorrPlayer::ExpAngCorrFunction(double *x, double *p)
{
    double func = 0.;

#ifdef HAS_MATHMORE

    bool deg = (TMath::Nint(p[5])==1);

    double costheta = cos(x[0]*TMath::DegToRad());
    if(!deg) costheta = x[0];

    double A0  = p[0];
    double A2  = p[1];
    double A4  = p[2];

    double Q2  = p[3];
    double Q4  = p[4];

    func = A0*(1+A2*Q2*ROOT::Math::legendre(2,costheta) + A4*Q4*ROOT::Math::legendre(4,costheta));
#endif

    return func;
}

double CXAngCorrPlayer::TheoreticalAngCorrFunction(double *x, double *p)
{
    double scale = p[0];
    double tot=1.;

#ifdef HAS_MATHMORE
    int TwoJ1 = TMath::Nint(p[1]);
    int TwoJ2 = TMath::Nint(p[2]);
    int TwoJ3 = TMath::Nint(p[3]);

    double mix12 = p[4];
    double mix23 = p[5];

    bool deg = (TMath::Nint(p[6])==1);

    double costheta = cos(x[0]*TMath::DegToRad());
    if(!deg) costheta = x[0];

    vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,mix12,mix23);
    int kmax = Akks.size()*2;
    int iak=0;
    for(int k=2; k<=kmax; k+=2, iak++) {
        tot+=Akks.at(iak)*ROOT::Math::legendre(k,costheta);
    }
#endif

    return scale*tot;
}

double CXAngCorrPlayer::EvalChi2(double *x, double *p)
{
    // From Urban_2013_J_Inst_8_P03014
    int mode = TMath::Nint(p[0]);

    bool evaluncertainty = (mode>=10);
    if(evaluncertainty) mode = mode-10;

    int TwoJ1 = TMath::Nint(p[1]);
    int TwoJ2 = TMath::Nint(p[2]);
    int TwoJ3 = TMath::Nint(p[3]);

    double mix12, mix23;

    if(mode==0) {
        mix12 = tan(x[0]*TMath::DegToRad());
        mix23 = p[4];
    }
    else if(mode==1) {
        mix23 = tan(x[0]*TMath::DegToRad());
        mix12 = p[4];
    }
    else {
        mix12 = tan(x[0]*TMath::DegToRad());
        mix23 = tan(x[1]*TMath::DegToRad());
    }

    vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,mix12,mix23);
    Float_t _A2 = 0.;
    Float_t _A4 = 0.;
    if(Akks.size()>=1) _A2 = Akks.at(0);
    if(Akks.size()>=2) _A4 = Akks.at(1);

    double ExpA2 = p[5];
    double ExpA2_err = 0.5*(p[7]-p[6]);

    double ExpA4 = p[8];
    double ExpA4_err = 0.5*(p[10]-p[9]);

    if(ExpA2_err==0.) ExpA2_err=1.;
    if(ExpA4_err==0.) ExpA4_err=1.;

    double Chi2_A2 = (_A2-ExpA2)/ExpA2_err;
    double Chi2_A4 = (_A4-ExpA4)/ExpA4_err;

    double Chi2 = Chi2_A2*Chi2_A2+Chi2_A4*Chi2_A4;

    if(evaluncertainty) {
        bool A2ok = ((_A2 >= p[6]) && (_A2 <= p[7]));
        bool A4ok = ((_A4 >= p[9]) && (_A4 <= p[10]));

        if(A2ok && A4ok) return Chi2;
        else return 0.;
    }

    return Chi2;
}

double CXAngCorrPlayer::EvalChi2Deriv(double *x,double */*parameters*/)
{
    if(!fChi2Func) return 0.;
    return fChi2Func->Derivative(x[0]);
}

double CXAngCorrPlayer::EvalChi2Deriv2(double *x,double */*parameters*/)
{
    if(!fChi2Func) return 0.;
    return fChi2Func->Derivative2(x[0]);
}

void CXAngCorrPlayer::PlotMixingEvaluation()
{
    StopMixingEvaluation();

    fMixingEvalInProcess = true;
    fStopMixingEval = false;

    int TwoJ1 = fNESpins[0]->GetIntNumber();
    int TwoJ2 = fNESpins[1]->GetIntNumber();
    int TwoJ3 = fNESpins[2]->GetIntNumber();

    GetCurrentInstance();

    if(fAngCorrPads[1] == nullptr) {
        fMixingEvalInProcess = false;
        return;
    }

    bool TwoDMixing = (fFixMixing[0]->GetState()==kButtonUp && fFixMixing[1]->GetState()==kButtonUp);

    delete fA2A4MixingGraph;
    fA2A4MixingGraph = new TGraph;
    fA2A4MixingGraph->SetName("A2A4MixingGraph");
    fA2A4MixingGraph->SetMarkerStyle(20);
    fA2A4MixingGraph->SetLineColor(kGreen);

    if(TwoDMixing) {
        fA2A4MixingGraph->SetMarkerStyle(1);
        fA2A4MixingGraph->SetLineStyle(kDotted);
        fA2A4MixingGraph->SetLineWidth(1);
        fA2A4MixingGraph->SetMarkerColorAlpha(kBlack,0.3);
    }
    else {
        fA2A4MixingGraph->SetMarkerSize(0.5);
        fA2A4MixingGraph->SetLineStyle(kSolid);
        fA2A4MixingGraph->SetLineWidth(1);
    }

    double deltapmin=-INFINITY;
    double deltapmax=INFINITY;
    int deltanb=5000;

    double atandeltamin1 = atan(deltapmin);
    double atandeltamax1 = atan(deltapmax);
    double atandeltamin2 = atan(deltapmin);
    double atandeltamax2 = atan(deltapmax);

    double deltapatandelta=(atandeltamax1-atandeltamin1)/(deltanb+1.);
    double atandelta2=atandeltamin2;

    if(fFixMixing[1]->GetState()==kButtonDown) {
        atandelta2 = TMath::ATan(fNEMixing[1]->GetNumber());
        atandeltamax2 = atandelta2;
    }

    int npoints = fNEMixingPoints->GetIntNumber();
    if(TwoDMixing) npoints = fNEMixingPoints->GetIntNumber()*fNEMixingPoints->GetIntNumber();
    fProgressBar->Reset();
    fProgressBar->SetMax(100);

    int step1 = ((atandeltamax2-atandelta2)/deltapatandelta+1)/fNEMixingPoints->GetIntNumber();
    if(step1==0) step1=1;
    int i=0;
    while(atandelta2<=atandeltamax2) {
        if(fStopMixingEval) break;
        double atandelta1=atandeltamin1;
        atandeltamax1 = atan(deltapmax);
        if(fFixMixing[0]->GetState()==kButtonDown) {
            atandelta1 = TMath::ATan(fNEMixing[0]->GetNumber());
            atandeltamax1 = atandelta1;
        }
        int step2 = ((atandeltamax1-atandelta1)/deltapatandelta+1)/fNEMixingPoints->GetIntNumber();
        if(step2==0) step2=1;
        int j=0;
        while(atandelta1<=atandeltamax1) {
            if(fStopMixingEval) break;
            if(((i%step1)==0) && ((j%step2)==0)) {
                vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,tan(atandelta1),tan(atandelta2));
                Float_t _A2 = 0.;
                Float_t _A4 = 0.;
                if(Akks.size()>=1) _A2 = Akks.at(0);
                if(Akks.size()>=2) _A4 = Akks.at(1);
                fA2A4MixingGraph->SetPoint(fA2A4MixingGraph->GetN(),_A2,_A4);

                if(((int)(((double)fA2A4MixingGraph->GetN())/((double)npoints)*100.)) != ((int)(((double)fA2A4MixingGraph->GetN()-1)/((double)npoints)*100.))) {
                    fProgressBar->SetPosition(((double)fA2A4MixingGraph->GetN())/((double)npoints)*100.);
                    gSystem->ProcessEvents();
                }
            }
            j++;
            atandelta1 += deltapatandelta;
        }
        atandelta2 += deltapatandelta;
        i++;
    }

    double minx,maxx,miny,maxy;
    minx = *min_element(fA2A4MixingGraph->GetX(), fA2A4MixingGraph->GetX()+fA2A4MixingGraph->GetN());
    miny = *min_element(fA2A4MixingGraph->GetY(), fA2A4MixingGraph->GetY()+fA2A4MixingGraph->GetN());
    maxx = *max_element(fA2A4MixingGraph->GetX(), fA2A4MixingGraph->GetX()+fA2A4MixingGraph->GetN());
    maxy = *max_element(fA2A4MixingGraph->GetY(), fA2A4MixingGraph->GetY()+fA2A4MixingGraph->GetN());

    double deltaX = (maxx-minx)*0.1;
    double deltaY = (maxy-miny)*0.1;

    fAngCorrPads[1]->Clear();

    TH1 *frame = fAngCorrPads[1]->DrawFrame(-2,-2,2,2);
    frame->GetXaxis()->SetTitle("A2");
    frame->GetXaxis()->CenterTitle();
    frame->GetYaxis()->SetTitle("A4");
    frame->GetYaxis()->CenterTitle();
    frame->GetXaxis()->SetTitleSize(0.05);
    frame->GetXaxis()->SetLabelSize(0.05);
    frame->GetYaxis()->SetTitleSize(0.05);
    frame->GetYaxis()->SetLabelSize(0.05);
    frame->GetYaxis()->SetTitleOffset(1.10);
    frame->GetXaxis()->SetTitleOffset(1.05);

    frame->GetYaxis()->SetRangeUser(miny-deltaY,maxy+deltaY);
    frame->GetXaxis()->SetRangeUser(minx-deltaX,maxx+deltaX);

    double mix12 = fNEMixing[0]->GetNumber();
    double mix23 = fNEMixing[1]->GetNumber();

    vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,mix12,mix23);
    Float_t _A2 = 0.;
    Float_t _A4 = 0.;
    if(Akks.size()>=1) _A2 = Akks.at(0);
    if(Akks.size()>=2) _A4 = Akks.at(1);

    fAngCorrPads[1]->cd();
    if(TwoDMixing) fA2A4MixingGraph->Draw("pl");
    else fA2A4MixingGraph->Draw("pl");

    delete fA2A4MixingTheoMarker;
    fA2A4MixingTheoMarker = new TGraph;
    fA2A4MixingTheoMarker->SetName("A2A4MixingTheoMarker");
    fA2A4MixingTheoMarker->SetMarkerStyle(20);
    fA2A4MixingTheoMarker->SetMarkerColor(kGreen);
    fA2A4MixingTheoMarker->AddPoint(_A2,_A4);
    fA2A4MixingTheoMarker->Draw("p");

    fAngCorrPads[1]->Modified();
    fAngCorrPads[1]->Update();
    fAngCorrPads[1]->GetFrame()->SetBit(TObject::kCannotPick);
    fA2A4MixingTheoMarker->SetBit(TObject::kCannotPick);
    fA2A4MixingGraph->SetBit(TObject::kCannotPick);

    fMixingEvalInProcess = false;

    UpdateData();
}

void CXAngCorrPlayer::SaveCorrectionFactors()
{
    TString WSName="";
    CXDialogBox *diag = new CXDialogBox(this->GetMainFrame(),"Save in workspace:");
    diag->Add("Workspace name",WSName);
    diag->Popup();

    if(WSName=="") {
        glog << tkn::info << "Efficiency saving aborted" << tkn::do_endl;
        return;
    }

    CXWorkspace *workspace = fMainWindow->GetWSManager()->GetWorkspace(WSName);

    if(!workspace) {
        glog << tkn::error << "No Workspace named: " << WSName << tkn::do_endl;
        return;
    }

    workspace->SetAngCorrQis(fExpQks[0][1]->GetNumber(),fExpQks[0][0]->GetNumber(),fExpQks[0][2]->GetNumber(),
                             fExpQks[1][1]->GetNumber(),fExpQks[1][0]->GetNumber(),fExpQks[1][2]->GetNumber());
}

void CXAngCorrPlayer::SetQis(double _Q2, double _Q2low, double _Q2high, double _Q4, double _Q4low, double _Q4high)
{
    fExpQks[0][1]->SetNumber(_Q2),fExpQks[0][0]->SetNumber(_Q2low),fExpQks[0][2]->SetNumber(_Q2high);
    fExpQks[1][1]->SetNumber(_Q4),fExpQks[1][0]->SetNumber(_Q4low),fExpQks[1][2]->SetNumber(_Q4high);
}

void CXAngCorrPlayer::PlotTheoryOnDist()
{
    GetCurrentInstance();

    if(!fAngularDistributionFunction) return;

    int TwoJ1 = fNESpins[0]->GetIntNumber();
    int TwoJ2 = fNESpins[1]->GetIntNumber();
    int TwoJ3 = fNESpins[2]->GetIntNumber();

    double mix12 = fNEMixing[0]->GetNumber();
    double mix23 = fNEMixing[1]->GetNumber();

    double _Q2 = fExpQks[0][1]->GetNumber();
    double _Q4 = fExpQks[1][1]->GetNumber();

    vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,mix12,mix23);
    Float_t _A2 = 0.;
    Float_t _A4 = 0.;
    if(Akks.size()>=1) _A2 = Akks.at(0);
    if(Akks.size()>=2) _A4 = Akks.at(1);

    delete fTheoreticalDistributionOnExp;
    double min=0;
    double max=180;
    int deg=1;
    if(fAnglesButtons[1]->GetState()==kButtonDown) {
        min=-1;
        max=1;
        deg=0;
    }

    fTheoreticalDistributionOnExp = new TF1(Form("%s_TheoOnExp",fMainWindow->GetCanvas()->GetName()), this, &CXAngCorrPlayer::ExpAngCorrFunction, min, max, 6, "CXAngCorrPlayer", "ExpAngCorrFunction");
    fTheoreticalDistributionOnExp->SetParameters(fAngularDistributionFunction->GetParameter(0),_A2,_A4,_Q2,_Q4,deg);

    fTheoreticalDistributionOnExp->SetNpx(5000);
    fTheoreticalDistributionOnExp->SetLineColor(kRed);

    fAngCorrPads[2]->cd();
    fTheoreticalDistributionOnExp->Draw("same");
    fAngCorrPads[2]->Modified();
    fAngCorrPads[2]->Update();
    fAngCorrPads[2]->GetFrame()->SetBit(TObject::kCannotPick);
}

ClassImp(CXAngCorrPlayer)
