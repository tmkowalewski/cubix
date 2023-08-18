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

#include "CXGuiENSDFPlayer.h"

#include <iostream>
#include <iomanip>

#include "TGLabel.h"
#include "TGButton.h"
#include "TMath.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"
#include "TObjArray.h"
#include "TGComboBox.h"

#include "CXMainWindow.h"
#include "CXENSDFLevelSchemePlayer.h"
#include "CXBashColor.h"

#include "tkmanager.h"

using namespace std;

CXGuiENSDFPlayer::CXGuiENSDFPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    fLSPlayer = new CXENSDFLevelSchemePlayer("LS Player","LS Player");

    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Level scheme player", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fGroupFrame->AddFrame(new TGLabel(fGroupFrame,"List of nuclei"), new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 5, 0));
    fListOfNucleus = new TGTextEntry(fGroupFrame, "");
    fListOfNucleus->SetToolTipText("List of nuclei for which the gamma rays will be shown, press enter to validate manual entry");
    fListOfNucleus->Connect("TextChanged(const char *)", "CXGuiENSDFPlayer", this, "ManuallyAddNucleus()");
    fListOfNucleus->Connect("ReturnPressed()", "CXGuiENSDFPlayer", this, "CheckListOfNuclei()");
    fGroupFrame->AddFrame(fListOfNucleus,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,0,0));

    fGroupFrame->AddFrame(new TGLabel(fGroupFrame,"ENSDF data-set"), new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 5, 0));
    fDataSetMode = new TGComboBox(fGroupFrame);
    fDataSetMode->Resize(200,20);
    fDataSetMode->Connect("Selected(Int_t)", "CXGuiENSDFPlayer", this, "UpdateDataSet()");
    fGroupFrame->AddFrame(fDataSetMode, new TGLayoutHints( kLHintsCenterY  | kLHintsExpandX ,-10,-10,0,0));

    TGCompositeFrame * fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Text size: "),new TGLayoutHints(kLHintsCenterY , 0, 0, 0, 0));
    fHorizontalFrame->AddFrame(fTextSize = new TGNumberEntry(fHorizontalFrame,0.03, 4,0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELLimitMinMax, 0.01 ,0.1), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));
    fTextSize->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");

    fHorizontalFrame->AddFrame(fFullGammaTitle = new TGCheckButton(fHorizontalFrame, "Full title", 86), new TGLayoutHints(kLHintsTop | kLHintsRight,5,0,0,0));
    fFullGammaTitle->Connect("Clicked()", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 10, 3));

    fGroupFrame = new TGGroupFrame(MotherFrame, "Parameters", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 10, 0));

    /// Branching ratio
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fUseBranchingRatio = new TGCheckButton(fHorizontalFrame, "", 0), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,1,1,0,0));
    fUseBranchingRatio->Connect("Clicked()", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");
    fBranchingRatio[0] = new TGNumberEntry(fHorizontalFrame, 0, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELLimitMinMax, 0. ,100.);
    fHorizontalFrame->AddFrame(fBranchingRatio[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " < Branching  ratio < "),new TGLayoutHints(kLHintsCenterY , 0, 0, 0, 0));

    fBranchingRatio[1] = new TGNumberEntry(fHorizontalFrame, 100, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELLimitMinMax, 0. ,100.);
    fHorizontalFrame->AddFrame(fBranchingRatio[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));

    fBranchingRatio[0]->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");
    fBranchingRatio[1]->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,0,0,5,0));

    /// ELevel
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fUseELevels = new TGCheckButton(fHorizontalFrame, "", 0), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,1,1,0,0));
    fUseELevels->Connect("Clicked()", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");
    fELevel[0] = new TGNumberEntry(fHorizontalFrame, 0, 6,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fELevel[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " < Level energy < "),new TGLayoutHints(kLHintsCenterY, 7, 8, 0, 0));

    fELevel[1] = new TGNumberEntry(fHorizontalFrame, 10000, 6,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fELevel[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));

    fELevel[0]->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");
    fELevel[1]->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,0,0,5,0));

    /// LifeTime
    ///
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fUseLifeTime = new TGCheckButton(fHorizontalFrame, "", 0), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,1,1,0,0));
    fUseLifeTime->Connect("Clicked()", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");

    for(int i=0 ; i<2 ; i++){
        fLifeTimeScale[i] = new TGComboBox(fHorizontalFrame);
        fLifeTimeScale[i]->Resize(40,fELevel[0]->GetHeight());
        fLifeTimeScale[i]->AddEntry("s",0);
        fLifeTimeScale[i]->AddEntry("ms",3);
        fLifeTimeScale[i]->AddEntry("us",6);
        fLifeTimeScale[i]->AddEntry("ns",9);
        fLifeTimeScale[i]->AddEntry("ps",12);
        fLifeTimeScale[i]->AddEntry("fs",15);
        if(i==0) fLifeTimeScale[i]->Select(15);
        if(i==1) fLifeTimeScale[i]->Select(0);
        fLifeTimeScale[i]->Connect("Selected(Int_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");
    }


    fLifeTime[0] = new TGNumberEntry(fHorizontalFrame, 0, 6,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fLifeTime[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));
    fHorizontalFrame->AddFrame(fLifeTimeScale[0], new TGLayoutHints( kLHintsCenterY ,0,1,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " < T < "),new TGLayoutHints(kLHintsCenterY, 40, 40, 0, 0));

    fLifeTime[1] = new TGNumberEntry(fHorizontalFrame, 1, 3,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fLifeTime[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));
    fHorizontalFrame->AddFrame(fLifeTimeScale[1], new TGLayoutHints( kLHintsCenterY ,0,1,0,0));

    fLifeTime[0]->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");
    fLifeTime[1]->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,0,0,5,0));

    /// Spins
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fUseSpins = new TGCheckButton(fHorizontalFrame, "", 0), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,1,1,0,0));
    fUseSpins->Connect("Clicked()", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");
    fSpins[0] = new TGNumberEntry(fHorizontalFrame, 0, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fSpins[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " < Spin < "),new TGLayoutHints(kLHintsCenterY, 29, 32, 0, 0));

    fSpins[1] = new TGNumberEntry(fHorizontalFrame, 100, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fSpins[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));

    fSpins[0]->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");
    fSpins[1]->Connect("ValueSet(Long_t)", "CXGuiENSDFPlayer", this, "UpdateGammaRays()");

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,0,0,5,0));

    ///IsYrast
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fYrastButton = new TGCheckButton(fHorizontalFrame, "Only Yrast states", 87), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,0,0,5,0));
    fYrastButton->Connect("Clicked()","CXGuiENSDFPlayer", this, "UpdateGammaRays()");

    UpdateGammaRays();
}

CXGuiENSDFPlayer::~CXGuiENSDFPlayer()
{
    delete fLSPlayer;
}

void CXGuiENSDFPlayer::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
    fLSPlayer->SetMainWindow(fMainWindow);
    fLSPlayer->SetGuiLSPlayer(this);
}

void CXGuiENSDFPlayer::CheckListOfNuclei()
{
    gErrorIgnoreLevel = kFatal;

    fNucleiAreKnown = true;

    TString tmp = fListOfNucleus->GetText();
    tmp.ReplaceAll(",", " ");

    TObjArray *arr = tmp.Tokenize(" ");

    fNumberOfNuclei = 0;

    for(int i=0 ; i<arr->GetEntries() ; i++) {
        if(!gmanager->known_nucleus(arr->At(i)->GetName()))
            fNucleiAreKnown = false;
        else
            fNumberOfNuclei++;
    }
    delete arr;

    if(fNucleiAreKnown) {
        fListOfNucleus->SetTextColor(CXblack);
        if(!tmp.EndsWith(" "))
            tmp.Append(" ");
    }

    fCurrentDataSet = "ADOPTED LEVELS, GAMMAS";
    fSelectedEntry = 0;

    UpdateGammaRays();

    gErrorIgnoreLevel = kPrint;
}

void CXGuiENSDFPlayer::UpdateGammaRays()
{
    if(fMainWindow == nullptr) return;

    fLSPlayer->CleanArrows();

    if(!fNucleiAreKnown) {
        gbash_color->WarningMessage("at least one of the nuclei is not known ");
    }

    fDataSetMode->RemoveAll();

    gPad = fMainWindow->GetSelectedPad();

    if(gPad == nullptr) {
        gPad = fMainWindow->GetCanvas()->cd();
        fMainWindow->SetSelectedPad(gPad);
    }

    fBranchingRatio[0]->SetState(fUseBranchingRatio->GetState());fBranchingRatio[1]->SetState(fUseBranchingRatio->GetState());
    fELevel[0]->SetState(fUseELevels->GetState());fELevel[1]->SetState(fUseELevels->GetState());
    fLifeTime[0]->SetState(fUseLifeTime->GetState());fLifeTime[1]->SetState(fUseLifeTime->GetState());
    fLifeTimeScale[0]->SetEnabled(fUseLifeTime->GetState());fLifeTimeScale[1]->SetEnabled(fUseLifeTime->GetState());
    fSpins[0]->SetState(fUseSpins->GetState());fSpins[1]->SetState(fUseSpins->GetState());

    fYrastMode = fYrastButton->GetState();

    if(fLSPlayer->CanReplot() && fMainWindow && fMainWindow->GetSelectedPad()) {
        fMainWindow->GetCanvas()->ResetDisabledClass();
        fSelectedLevelScheme = fLSPlayer->DrawArrows(fListOfNucleus->GetText(),fMainWindow->GetHisto(), fCurrentDataSet);

        if(fSelectedLevelScheme) {
            if(fNumberOfNuclei == 1) {
                int i=0;
                for(auto &dataset : fSelectedLevelScheme->get_datasets() ) {
                    fDataSetMode->AddEntry(dataset.second->get_name().data(),i);
                    i++;
                }
            }
            else {
                fDataSetMode->AddEntry(fSelectedLevelScheme->get_dataset()->get_name().data(),0);
            }
            fNoUpdateDataSet = true;
            fDataSetMode->Select(fSelectedEntry);
            fNoUpdateDataSet = false;
        }
        fMainWindow->GetCanvas()->DisableClass("TBox");
    }

    fMainWindow->RefreshPads();
}


void CXGuiENSDFPlayer::GetBranchingRatio(Int_t &min, Int_t &max)
{
    min = fBranchingRatio[0]->GetNumber();
    max = fBranchingRatio[1]->GetNumber();
}

void CXGuiENSDFPlayer::GetELevels(Float_t &min, Float_t &max)
{
    min = fELevel[0]->GetNumber();
    max = fELevel[1]->GetNumber();
}

void CXGuiENSDFPlayer::GetSpins(Int_t &min, Int_t &max)
{
    min = fSpins[0]->GetNumber();
    max = fSpins[1]->GetNumber();
}

void CXGuiENSDFPlayer::GetLifeTime(Float_t &min, Float_t &max)
{
    min = fLifeTime[0]->GetNumber()*TMath::Power(10,-fLifeTimeScale[0]->GetSelected());
    max = fLifeTime[1]->GetNumber()*TMath::Power(10,-fLifeTimeScale[1]->GetSelected());
}

void CXGuiENSDFPlayer::ManuallyAddNucleus()
{
    fListOfNucleus->SetTextColor(CXred);
}

void CXGuiENSDFPlayer::UpdateDataSet()
{
    if(fNoUpdateDataSet) return;
    fCurrentDataSet = fDataSetMode->GetSelectedEntry()->GetTitle();
    fSelectedEntry = fDataSetMode->GetSelected();
    UpdateGammaRays();
}

ClassImp(CXGuiENSDFPlayer)
