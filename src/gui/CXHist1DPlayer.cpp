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

#include "CXHist1DPlayer.h"
// #include "cubix_config.h"

#include <iostream>

#include "TGButton.h"
#include "TGLabel.h"
#include "TSpectrum.h"
#include "TROOT.h"
#include "TGListBox.h"
#include "TGClient.h"
#include "TSystem.h"
#include "TError.h"
#include "TMath.h"

// #if (OS_TYPE == OS_LINUX)
#include "TGResourcePool.h"
// #endif

#include "CXMainWindow.h"
#include "CXArrow.h"
#include "CXFit.h"
#include "CXBgdFit.h"
#include "CXBashColor.h"
#include "CXWSManager.h"
#include "CXDialogBox.h"

using namespace std;

CXHist1DPlayer::CXHist1DPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    /// Search Peak
    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Peak search", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));


    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Sigma"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,10,0,0));
    fSigmaSPEntry = new TGNumberEntry(fHorizontalFrame, 2, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fSigmaSPEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Threshold"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fThresholdSPEntry = new TGNumberEntry(fHorizontalFrame, 0.05, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELLimitMinMax,0.001,0.999);
    fHorizontalFrame->AddFrame(fThresholdSPEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Size"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fTextSize = new TGNumberEntry(fHorizontalFrame,0.03, 4,0, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELLimitMinMax, 0.01 ,0.1);
    fHorizontalFrame->AddFrame(fTextSize,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *SearchButton = new TGTextButton(fHorizontalFrame, "Search");
    SearchButton->SetTextColor(CXred);
    SearchButton->Connect("Clicked()", "CXHist1DPlayer", this, "PeakSearch()");
    fHorizontalFrame->AddFrame(SearchButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));

    TGTextButton *ClearButton = new TGTextButton(fHorizontalFrame, "Clear");
    ClearButton->Connect("Clicked()", "CXHist1DPlayer", this, "PeakSearchClear()");
    ClearButton->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(ClearButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    /// Minimizer

    fGroupFrame = new TGGroupFrame(MotherFrame, "Minimizer", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Minimizer"), new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,10,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Algorithm"), new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fMinimizer = new TGComboBox(fHorizontalFrame);
    fMinimizer->Resize(200,20);
    fMinimizer->Connect("Selected(Int_t)", "CXHist1DPlayer", this, "UpdateMinimizer()");
    fHorizontalFrame->AddFrame(fMinimizer, new TGLayoutHints( kLHintsCenterY  | kLHintsExpandX ,10,10,0,0));
    fAlgorithm = new TGComboBox(fHorizontalFrame);
    fAlgorithm->Resize(200,20);
    fHorizontalFrame->AddFrame(fAlgorithm, new TGLayoutHints( kLHintsCenterY  | kLHintsExpandX ,10,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fMinimizer->AddEntry("Minuit",0);
    fMinimizer->Select(0);
    TString Lib = "libMinuit2";
    if(gSystem->FindDynamicLibrary(Lib)) {
        fMinimizer->AddEntry("Minuit2",1);
        fMinimizer->Select(1);
    }
    fMinimizer->AddEntry("Fumili",2);
    Lib = "libMathMore";
    if(gSystem->FindDynamicLibrary(Lib)) {
        fMinimizer->AddEntry("GSLMultiMin",3);
        fMinimizer->AddEntry("GSLMultiFit",4);
        fMinimizer->AddEntry("GSLSimAn",5);
    }
    Lib = "libGenetic";
    if(gSystem->FindDynamicLibrary(Lib))
        fMinimizer->AddEntry("Genetic",6);

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Tolerance"), new TGLayoutHints(kLHintsBottom | kLHintsLeft,10,10,0,0));
    fTolenrance = new TGNumberEntry(fHorizontalFrame, 0.01, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fTolenrance,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,10,10,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Print Level"), new TGLayoutHints(kLHintsBottom | kLHintsLeft,10,10,0,0));
    fPrintLevel = new TGNumberEntry(fHorizontalFrame, 0, 4, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fPrintLevel,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,10,10,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Fit options:"), new TGLayoutHints(kLHintsBottom | kLHintsLeft,10,10,0,0));
    fFitOptions = new TGTextEntry(fHorizontalFrame, "I");
    fHorizontalFrame->AddFrame(fFitOptions,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX,10,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));


    /// Background Fit
    fGroupFrame = new TGGroupFrame(MotherFrame, "Background Fit", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *NewBgdFit = new TGTextButton(fHorizontalFrame, "Set");
    NewBgdFit->SetTextColor(CXred);
    NewBgdFit->Connect("Clicked()", "CXHist1DPlayer", this, "NewBgdFit()");
    fHorizontalFrame->AddFrame(NewBgdFit,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));

    TGTextButton *DoBgdFit = new TGTextButton(fHorizontalFrame, "Fit");
    DoBgdFit->Connect("Clicked()", "CXHist1DPlayer", this, "DoBgdFit()");
    DoBgdFit->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(DoBgdFit,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));

    TGTextButton *ClearBgdFit = new TGTextButton(fHorizontalFrame, "Clear");
    ClearBgdFit->SetTextColor(CXred);
    ClearBgdFit->Connect("Clicked()", "CXHist1DPlayer", this, "ClearFits()");
    fHorizontalFrame->AddFrame(ClearBgdFit,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,10));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fBckPol1_Bgd = new TGCheckButton(fHorizontalFrame, "Pol1", 0);
    fBckPol1_Bgd->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fBckPol1_Bgd->SetState((EButtonState)fUseBgdPol1);
    fHorizontalFrame->AddFrame(fBckPol1_Bgd,new TGLayoutHints(kLHintsTop | kLHintsLeft,5,5,0,0));
    fBckExp_Bgd = new TGCheckButton(fHorizontalFrame, "Exp", 0);
    fBckExp_Bgd->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fBckExp_Bgd->SetState((EButtonState)fUseBgdExp);
    fHorizontalFrame->AddFrame(fBckExp_Bgd,new TGLayoutHints(kLHintsTop | kLHintsLeft,5,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,0,5));

    /// Fit Peak
    fGroupFrame = new TGGroupFrame(MotherFrame, "Peak Fit", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *NewFit = new TGTextButton(fHorizontalFrame, "Set");
    NewFit->SetTextColor(CXred);
    NewFit->Connect("Clicked()", "CXHist1DPlayer", this, "NewFit()");
    fHorizontalFrame->AddFrame(NewFit,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));

    TGTextButton *DoFit = new TGTextButton(fHorizontalFrame, "Fit");
    DoFit->Connect("Clicked()", "CXHist1DPlayer", this, "DoFit()");
    DoFit->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(DoFit,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));

    TGTextButton *ClearFit = new TGTextButton(fHorizontalFrame, "Clear");
    ClearFit->SetTextColor(CXred);
    ClearFit->Connect("Clicked()", "CXHist1DPlayer", this, "ClearFits()");
    fHorizontalFrame->AddFrame(ClearFit,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,10));

    TGLabel *label;

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, "Mean:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 0, 0));
    label->SetTextColor(CXblue);
    fFixMean = new TGCheckButton(fHorizontalFrame, "Fixed", 0);
    fFixMean->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fHorizontalFrame->AddFrame(fFixMean,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));

    fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, "Amplitude:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 20, 5, 0, 0));
    label->SetTextColor(CXblue);
    fFixAmpli = new TGCheckButton(fHorizontalFrame, "Fixed", 0);
    fFixAmpli->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fHorizontalFrame->AddFrame(fFixAmpli,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,15,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,0,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, "Background:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 0, 0));
    label->SetTextColor(CXblue);
    fBckStep = new TGCheckButton(fHorizontalFrame, "Step", 0);
    fBckStep->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fBckStep->SetState((EButtonState)fUseStep);
    fHorizontalFrame->AddFrame(fBckStep,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fBckPol1 = new TGCheckButton(fHorizontalFrame, "Pol1", 0);
    fBckPol1->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fBckPol1->SetState((EButtonState)fUsePol1);
    fHorizontalFrame->AddFrame(fBckPol1,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,5,0,0));
    fBckExp = new TGCheckButton(fHorizontalFrame, "Exp", 0);
    fBckExp->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fBckExp->SetState((EButtonState)fUseExp);
    fHorizontalFrame->AddFrame(fBckExp,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, "FWHM:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 0, 0));
    label->SetTextColor(CXblue);
    fFixFWHM = new TGCheckButton(fHorizontalFrame, "Fixed", 0);
    fFixFWHM->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fHorizontalFrame->AddFrame(fFixFWHM,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));

    fFWHMFromWS = new TGCheckButton(fHorizontalFrame, "From WS", 0);
    fFWHMFromWS->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fHorizontalFrame->AddFrame(fFWHMFromWS,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Sigma"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fFWHMSigma = new TGNumberEntry(fHorizontalFrame, 2., 5,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fFWHMSigma,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,5,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));


    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Value: "),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_FWHM[1] = new TGNumberEntry(fHorizontalFrame, (Double_t)2, 5,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_FWHM[1],new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX,5,5,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fNE_FWHM[0] = new TGNumberEntry(fHorizontalFrame, (Double_t)5, 5,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_FWHM[0],new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX,5,5,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fNE_FWHM[2] = new TGNumberEntry(fHorizontalFrame, (Double_t)10, 5,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_FWHM[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,5,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,0,5));


    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, "Left tail:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 0, 0));
    label->SetTextColor(CXblue);
    fUseLT = new TGCheckButton(fHorizontalFrame, "Used", 0);
    fUseLT->SetState(kButtonDown);
    fUseLT->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fHorizontalFrame->AddFrame(fUseLT,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fFixLT = new TGCheckButton(fHorizontalFrame, "Fixed", 0);
    fFixLT->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fHorizontalFrame->AddFrame(fFixLT,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Value: "),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_LT[1] = new TGNumberEntry(fHorizontalFrame, (Double_t)-5, 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELLimitMinMax, (Double_t)-5 ,(Double_t)-0.1);
    fHorizontalFrame->AddFrame(fNE_LT[1],new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX,5,5,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fNE_LT[0] = new TGNumberEntry(fHorizontalFrame, (Double_t)-2, 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELLimitMinMax, (Double_t)-5 ,(Double_t)-0.1);
    fHorizontalFrame->AddFrame(fNE_LT[0],new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX,5,5,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fNE_LT[2] = new TGNumberEntry(fHorizontalFrame, (Double_t)-0.1, 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELLimitMinMax, (Double_t)-5 ,(Double_t)-0.1);
    fHorizontalFrame->AddFrame(fNE_LT[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,5,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,0,5));


    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, "Right tail:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 0, 0));
    label->SetTextColor(CXblue);
    fUseRT = new TGCheckButton(fHorizontalFrame, "Used", 0);
    fUseRT->SetState(kButtonUp);
    fUseRT->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fHorizontalFrame->AddFrame(fUseRT,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fFixRT = new TGCheckButton(fHorizontalFrame, "Fixed", 0);
    fFixRT->Connect("Clicked()", "CXHist1DPlayer", this, "HandleMyButton()");
    fHorizontalFrame->AddFrame(fFixRT,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Value: "),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_RT[1] = new TGNumberEntry(fHorizontalFrame, (Double_t)0.1, 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELLimitMinMax, (Double_t)0.1 ,(Double_t)5);
    fHorizontalFrame->AddFrame(fNE_RT[1],new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX,5,5,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fNE_RT[0] = new TGNumberEntry(fHorizontalFrame, (Double_t)2, 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELLimitMinMax, (Double_t)0.1 ,(Double_t)5);
    fHorizontalFrame->AddFrame(fNE_RT[0],new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX,5,5,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fNE_RT[2] = new TGNumberEntry(fHorizontalFrame, (Double_t)5, 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELLimitMinMax, (Double_t)0.1 ,(Double_t)5);
    fHorizontalFrame->AddFrame(fNE_RT[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,5,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,0,5));


    fGroupFrame = new TGGroupFrame(MotherFrame, "Fit results", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY | kLHintsExpandX, 3, 3, 0, 0));


    fFitResultsBox = new TGListBox(fGroupFrame);
    fFitResultsBox->GetContainer()->RemoveInput(kButtonReleaseMask | kButtonMotionMask);
    fGroupFrame->AddFrame(fFitResultsBox, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,-10,-10,0,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *Save = new TGTextButton(fHorizontalFrame, "Save fit result");
    Save->SetTextColor(CXred);
    Save->Connect("Clicked()", "CXHist1DPlayer", this, "SaveFit()");
    fHorizontalFrame->AddFrame(Save,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,20,20,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    HandleMyButton();

    fListOfFitObjects = new TList;
    fListOfFitObjects->SetOwner();

    fListOfFitObjectsSaved = new TList;
    fListOfFitObjectsSaved->SetOwner();

    fListOfBgdFitObjects = new TList;
    fListOfBgdFitObjects->SetOwner();
}

CXHist1DPlayer::~CXHist1DPlayer()
{
}

void CXHist1DPlayer::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXHist1DPlayer::PeakSearchClear()
{
    TVirtualPad *pad = fMainWindow->GetSelectedPad();
    gPad = pad;

    if(pad==nullptr) {
        cout<<"No selected pad, ignored"<<endl;
        return;
    }

    for(int i=0 ; i<pad->GetListOfPrimitives()->GetEntries() ; i++) {
        TObject *o = pad->GetListOfPrimitives()->At(i);
        if(o->InheritsFrom(CXArrow::Class())) {
            ((CXArrow*)o)->ClearPad(pad,false);
            i--;
        }
    }

    fMainWindow->RefreshPads();
}

void CXHist1DPlayer::PeakSearch()
{
    //    PeakSearchClear();

    gPad = fMainWindow->GetSelectedPad();

    fCurrentHist = fMainWindow->GetHisto();

    if(fCurrentHist==nullptr || fCurrentHist->InheritsFrom("TH2")) {
        cout<<"No 1D histogram found, ignored"<<endl;
        return;
    }

    TSpectrum *tspec = new TSpectrum;
    tspec->Search(fCurrentHist,fSigmaSPEntry->GetNumber(),"goff",fThresholdSPEntry->GetNumber());

    Int_t NPeaks = tspec->GetNPeaks();

    for(int i=0 ; i<NPeaks ; i++) {
        Float_t Energy = tspec->GetPositionX()[i];
        Float_t Value = tspec->GetPositionY()[i];
        Float_t MaxGlob = fCurrentHist->GetMaximum();

        bool found = false;
        for(int i=0 ; i<gPad->GetListOfPrimitives()->GetEntries() ; i++) {
            TObject *o = gPad->GetListOfPrimitives()->At(i);
            if(o->InheritsFrom(CXArrow::Class())) {
                CXArrow *arr = ((CXArrow*)o);
                if(TMath::Abs(arr->GetEnergy()-Energy)<0.1) found = true;
            }
        }

        if(found) continue;

        CXArrow *GammaArrow = new CXArrow((CXFit*)nullptr, Energy,(Value + MaxGlob/100.) ,(Value +MaxGlob/15.),0.01,fTextSize->GetNumber(),"<|");

        GammaArrow->SetAngle(40);
        GammaArrow->SetLineColor(fCurrentHist->GetLineColor());
        GammaArrow->SetFillColor(fCurrentHist->GetLineColor());
        GammaArrow->SetLineWidth(1);

        GammaArrow->SetText(fCurrentHist,Form("%.2f",Energy),"tooltip");
        GammaArrow->Draw();
        GammaArrow->SetBit(TObject::kCannotPick);
    }

    fMainWindow->RefreshPads();

    delete tspec;
}

void CXHist1DPlayer::NewBgdFit()
{
    fCurrentHist = fMainWindow->GetHisto();
    if(fCurrentHist==nullptr || fCurrentHist->InheritsFrom(TH2::Class())) {
        cout<<"No 1D istogram found, ignored"<<endl;
        return;
    }

    DoNewBgdFit = true;

    fMainWindow->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "CXHist1DPlayer", this, "HandleMouse(Int_t,Int_t,Int_t, TObject*)");

    fCurrentBgdFit = new CXBgdFit(fCurrentHist,fMainWindow->GetSelectedPad(),this, fMainWindow->GetWSManager()->GetActiveWorkspace());
    fListOfBgdFitObjects->Add(fCurrentBgdFit);
}

void CXHist1DPlayer::NewFit()
{
    fCurrentHist = fMainWindow->GetHisto();
    if(fCurrentHist==nullptr || fCurrentHist->InheritsFrom(TH2::Class())) {
        cout<<"No 1D istogram found, ignored"<<endl;
        return;
    }

    DoNewFit = true;

    fMainWindow->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "CXHist1DPlayer", this, "HandleMouse(Int_t,Int_t,Int_t, TObject*)");

    fCurrentFit = new CXFit(fCurrentHist,fMainWindow->GetSelectedPad(),this, fMainWindow->GetWSManager()->GetActiveWorkspace());
    fListOfFitObjects->Add(fCurrentFit);
}

void CXHist1DPlayer::LastFit()
{
    ClearFits();

    fCurrentHist = fMainWindow->GetHisto(fMainWindow->GetSelectedPad());

    if(fCurrentHist==nullptr || fCurrentHist->InheritsFrom(TH2::Class())) {
        cout<<"No 1D istogram found, ignored"<<endl;
        return;
    }
    for(int i=0 ; i<fListOfFitObjectsSaved->GetEntries() ; i++) {
        CXFit *fit = (CXFit*)fListOfFitObjectsSaved->At(i)->Clone();
        fit->UpdateFit(fCurrentHist,fMainWindow->GetSelectedPad(),this, fMainWindow->GetWSManager()->GetActiveWorkspace());
        fListOfFitObjects->Add(fit);
    }
    cout << "updated fit on: " << fCurrentHist->GetName() << endl;
    DoFit();
}

void CXHist1DPlayer::RemoveFit(CXFit *fit)
{
    fListOfFitObjects->Remove(fit);
}

void CXHist1DPlayer::RemoveBgdFit(CXBgdFit *fit)
{
    fListOfBgdFitObjects->Remove(fit);
}

void CXHist1DPlayer::HandleMouse(Int_t EventType,Int_t EventX,Int_t EventY, TObject* selected)
{
    TPad *pad = dynamic_cast<TPad*>(gROOT->GetSelectedPad());

    if(pad==nullptr)
        return;

    if(EventType == kButton1Up && selected && selected->InheritsFrom("CXArrow")) {
        CXArrow *arr = dynamic_cast<CXArrow*>(selected);
        if(arr->GetFit()) arr->GetFit()->Update();
        if(arr->GetBgdFit()) arr->GetBgdFit()->Update();
    }

    if(DoNewFit && fCurrentFit) {
        if(EventType == kButton1Up && (!selected || !selected->InheritsFrom("CXArrow"))) {
            if(     pad->AbsPixeltoX(EventX)>gPad->GetUxmin() &&
                pad->AbsPixeltoX(EventX)<gPad->GetUxmax() &&
                pad->AbsPixeltoY(EventY)>gPad->GetUymin() &&
                pad->AbsPixeltoY(EventY)<gPad->GetUymax()
                ) {
                Float_t E = pad->AbsPixeltoX(EventX);
                fCurrentFit->AddArrow(E);
            }
        }
        if(EventType == kButton1Double) {
            DoNewFit = false;
            fCurrentFit->Fit();
            fListOfFitObjectsSaved->Add(fCurrentFit->Clone());
            fCurrentFit = nullptr;
        }
    }

    if(DoNewBgdFit && fCurrentBgdFit) {
        if(EventType == kButton1Up && (!selected || !selected->InheritsFrom("CXArrow"))) {
            if(     pad->AbsPixeltoX(EventX)>gPad->GetUxmin() &&
                pad->AbsPixeltoX(EventX)<gPad->GetUxmax() &&
                pad->AbsPixeltoY(EventY)>gPad->GetUymin() &&
                pad->AbsPixeltoY(EventY)<gPad->GetUymax()
                ) {
                Float_t E = pad->AbsPixeltoX(EventX);
                fCurrentBgdFit->AddArrow(E);
            }
        }
        if(EventType == kButton1Double) {
            DoNewBgdFit = false;
            fCurrentBgdFit->Fit();
            fCurrentBgdFit = nullptr;
        }
    }
}

void CXHist1DPlayer::ClearFits()
{
    fFitResultsBox->RemoveAll();

    TVirtualPad *pad = fMainWindow->GetSelectedPad();

    if(pad==nullptr) {
        cout<<"No selected pad, ignored"<<endl;
        return;
    }

    for(int i=0 ; i<fListOfFitObjects->GetEntries() ; i++) {
        CXFit *fit = (CXFit*)fListOfFitObjects->At(i);
        if(pad == fit->GetPad()) {
            fit->Clear(pad);
            fListOfFitObjects->Remove(fit);
            i--;
        }
    }
    for(int i=0 ; i<fListOfBgdFitObjects->GetEntries() ; i++) {
        CXBgdFit *fit = (CXBgdFit*)fListOfBgdFitObjects->At(i);
        if(pad == fit->GetPad()) {
            fit->Clear(pad);
            fListOfBgdFitObjects->Remove(fit);
            i--;
        }
    }

    DoNewFit = false;
    fCurrentFit=nullptr;
    DoNewBgdFit = false;
    fCurrentBgdFit=nullptr;

    fMainWindow->RefreshPads();
}

void CXHist1DPlayer::DoFit()
{
    fListOfFitObjectsSaved->Clear();

    fFitResultsBox->RemoveAll();

    TVirtualPad *pad = fMainWindow->GetSelectedPad();

    if(pad==nullptr) {
        cout<<"No selected pad, ignored"<<endl;
        return;
    }

    for(int i=0 ; i<fListOfFitObjects->GetEntries() ; i++) {
        CXFit *fit = (CXFit*)fListOfFitObjects->At(i);
        if(pad == fit->GetPad()) {
            fListOfFitObjectsSaved->Add(fit->Clone());
            fit->Fit();
        }
    }

    fMainWindow->RefreshPads();

    DoNewFit=false;
    fCurrentFit=nullptr;
}

void CXHist1DPlayer::DoBgdFit()
{
    fFitResultsBox->RemoveAll();

    for(int i=0 ; i<fListOfBgdFitObjects->GetEntries() ; i++) {
        CXBgdFit *fit = (CXBgdFit*)fListOfBgdFitObjects->At(i);
        fit->Fit();
    }

    fMainWindow->RefreshPads();

    DoNewBgdFit=false;
    fCurrentBgdFit=nullptr;
}


void CXHist1DPlayer::HandleMyButton()
{

    fUseBgdPol1 = (bool)fBckPol1_Bgd->GetState();
    fUseBgdExp = (bool)fBckExp_Bgd->GetState();

    fUseStep = (bool)fBckStep->GetState();
    fUsePol1 = (bool)fBckPol1->GetState();
    fUseExp = (bool)fBckExp->GetState();

    fUseFWHM = (bool)fFWHMFromWS->GetState();

    if(fUseLT->GetState() == kButtonUp) {
        fFixLT->SetState(kButtonDisabled);

        fNE_LT[0]->SetState(false);
        fNE_LT[1]->SetState(false);
        fNE_LT[2]->SetState(false);
    }
    else {
        fFixLT->IsDown() ? fFixLT->SetState(kButtonDown) : fFixLT->SetState(kButtonUp);

        fNE_LT[0]->SetState(true);
        fNE_LT[1]->SetState(true);
        fNE_LT[2]->SetState(true);
    }
    if(fUseLT->GetState() == kButtonDown && fFixLT->GetState() == kButtonDown) {
        fNE_LT[0]->SetState(true);
        fNE_LT[1]->SetState(false);
        fNE_LT[2]->SetState(false);
    }
    if(fUseLT->GetState() == kButtonDown && fFixLT->GetState() == kButtonUp) {
        fNE_LT[0]->SetState(true);
        fNE_LT[1]->SetState(true);
        fNE_LT[2]->SetState(true);
    }

    if(fUseRT->GetState() == kButtonUp) {
        fFixRT->SetState(kButtonDisabled);

        fNE_RT[0]->SetState(false);
        fNE_RT[1]->SetState(false);
        fNE_RT[2]->SetState(false);
    }
    else {
        fFixRT->IsDown() ? fFixRT->SetState(kButtonDown) : fFixRT->SetState(kButtonUp);

        fNE_RT[0]->SetState(true);
        fNE_RT[1]->SetState(true);
        fNE_RT[2]->SetState(true);
    }
    if(fUseRT->GetState() == kButtonDown && fFixRT->GetState() == kButtonDown) {
        fNE_RT[0]->SetState(true);
        fNE_RT[1]->SetState(false);
        fNE_RT[2]->SetState(false);
    }
    if(fUseRT->GetState() == kButtonDown && fFixRT->GetState() == kButtonUp) {
        fNE_RT[0]->SetState(true);
        fNE_RT[1]->SetState(true);
        fNE_RT[2]->SetState(true);
    }
    
    if(fUseFWHM) {
        auto WS = fMainWindow->GetWSManager()->GetActiveWorkspace();
        if(!WS) {
            gbash_color->ErrorMessage("No workspace loaded");
            fUseFWHM = false;
        }
        else {
            auto func = fMainWindow->GetWSManager()->GetActiveWorkspace()->fFWHMFunction;
            auto err = fMainWindow->GetWSManager()->GetActiveWorkspace()->fFWHMErrors;
            if(!func) {
                gbash_color->ErrorMessage(Form("No FWHM function defined in current workspace: %s", fMainWindow->GetWSManager()->GetActiveWSName().Data()));
                fUseFWHM = false;
            }
            else if(!err && (fFixFWHM->GetState() == kButtonUp)) {
                gbash_color->ErrorMessage(Form("No FWHM error band defined in current workspace: %s", fMainWindow->GetWSManager()->GetActiveWSName().Data()));
                fUseFWHM = false;
            }
        }
        if(!fUseFWHM) fFWHMFromWS->SetState(kButtonUp);
    }

    fFWHMSigma->SetState(false);
    if(fUseFWHM) {
        if(fFixFWHM->GetState() == kButtonDown) fFWHMSigma->SetState(false);
        else fFWHMSigma->SetState(true);

        fNE_FWHM[0]->SetState(false);
        fNE_FWHM[1]->SetState(false);
        fNE_FWHM[2]->SetState(false);
    }
    else {
        if((fFixFWHM->GetState() == kButtonDown) || fUseFWHM) {
            fNE_FWHM[0]->SetState(true);
            fNE_FWHM[1]->SetState(false);
            fNE_FWHM[2]->SetState(false);
        }
        if((fFixFWHM->GetState() == kButtonUp) && !fUseFWHM) {
            fNE_FWHM[0]->SetState(true);
            fNE_FWHM[1]->SetState(true);
            fNE_FWHM[2]->SetState(true);
        }
    }

}

void CXHist1DPlayer::PrintInListBox(TString mess, Int_t Type)
{
    // #if (OS_TYPE == OS_LINUX)
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

    TGTextLBEntry *entry = new TGTextLBEntry(fFitResultsBox->GetContainer(), new TGString(mess), fFitResultsBox->GetNumberOfEntries()+1, uGC->GetGC(), ufont->GetFontStruct());
    // #else
    //     TGTextLBEntry *entry = new TGTextLBEntry(fFitResultsBox->GetContainer(), new TGString(mess), fFitResultsBox->GetNumberOfEntries()+1);
    // #endif

    if(Type == kError)
        entry->SetBackgroundColor((Pixel_t)0xff0000);
    else if(Type == kInfo)
        entry->SetBackgroundColor((Pixel_t)0x87a7d2);
    else if(Type == kWarning)
        entry->SetBackgroundColor((Pixel_t)0xdfdf44);
    else if(Type == kPrint)
        entry->SetBackgroundColor((Pixel_t)0x90f269);

    fFitResultsBox->AddEntry((TGLBEntry *)entry, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX));
    fFitResultsBox->Layout();
}

void CXHist1DPlayer::UpdateMinimizer()
{
    fAlgorithm->RemoveAll();

    if(fMinimizer->GetSelected() == 0) {
        fAlgorithm->AddEntry("Migrad",0);
        fAlgorithm->AddEntry("Simplex",1);
        fAlgorithm->AddEntry("Minimize",2);
        fAlgorithm->AddEntry("MigradImproved",3);
        fAlgorithm->AddEntry("Scan",4);
        fAlgorithm->AddEntry("Seek",5);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 1) {
        fAlgorithm->AddEntry("Migrad",0);
        fAlgorithm->AddEntry("Simplex",1);
        fAlgorithm->AddEntry("Minimize",2);
        fAlgorithm->AddEntry("Scan",3);
        fAlgorithm->AddEntry("Fumili",4);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 2) {
        fAlgorithm->AddEntry("Fumili",0);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 3) {
        fAlgorithm->AddEntry("BFGS2",0);
        fAlgorithm->AddEntry("BFGS",1);
        fAlgorithm->AddEntry("ConjugateFR",2);
        fAlgorithm->AddEntry("ConjugatePR",3);
        fAlgorithm->AddEntry("SteepestDescent",4);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 4) {
        fAlgorithm->AddEntry("GSLMultiFit",0);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 5) {
        fAlgorithm->AddEntry("GSLSimAn",0);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 6) {
        fAlgorithm->AddEntry("Genetic",0);
        fAlgorithm->Select(0);
    }
}

void CXHist1DPlayer::SaveFit()
{
    if(!fMainWindow->GetWSManager()->GetActiveWorkspace()) {
        gbash_color->ErrorMessage("A workspace needs to be defined to save fit results");
        return;
    }

    TString name = "";
    TString overwrite="0";
    CXDialogBox *diag = new CXDialogBox(this->GetMainFrame(),"Save fit results");
    diag->Add("File name (without extension)",name);
    diag->Add("Overwrite",overwrite);
    diag->Popup();

    TString FolderName = Form("%s/%s/FitResults",fMainWindow->GetWSManager()->GetWSDirectory().Data(),fMainWindow->GetWSManager()->GetActiveWSName().Data());
    gSystem->mkdir(FolderName,true);
    TString FileName = Form("%s/%s.fit",FolderName.Data(),name.Data());

    bool ow = (overwrite=="1");

    if(!ow && !gSystem->AccessPathName(FileName)) {
        gbash_color->WarningMessage("File already existing ==> ignored");
        return;
    }

    ofstream file(FileName);

    TString StringToBeSaved;
    for(int i=0 ; i<fListOfFitObjects->GetEntries() ; i++) {
        CXFit *fit = (CXFit*)fListOfFitObjects->At(i);
        file << fit->Save();
    }

    file.close();

    gbash_color->InfoMessage(Form("Fit saved in: %s",FileName.Data()));
}

ClassImp(CXHist1DPlayer)
