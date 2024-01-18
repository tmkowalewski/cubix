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

#include "CXFitEfficiency.h"

#include <iostream>

#include "TGNumberEntry.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TROOT.h"
#include "TObjArray.h"
#include "TF1.h"
#include "TMath.h"
#include "TSystemDirectory.h"
#include "TFitResult.h"
#include "TMath.h"
#include "Math/MinimizerOptions.h"
#include "TVirtualFitter.h"

#include "CXBashColor.h"
#include "CXMainWindow.h"
#include "CXGammaSource.h"
#include "CXDialogBox.h"
#include "CXFitFunctions.h"

using namespace std;

CXFitEfficiency::CXFitEfficiency(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{

    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Sources", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fGroupFrame->AddFrame(new TGLabel(fGroupFrame,"Input data"), new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 5, 0));
    fSources = new TGTextEntry(fGroupFrame, "152Eu");
    fSources->SetToolTipText("List of sources (files in $CUBIX_SYS/dataBase/Sources)");
    fSources->Connect("TextChanged(const char *)", "CXFitEfficiency", this, "UpdateText()");
    fSources->Connect("ReturnPressed()", "CXFitEfficiency", this, "UpdateSources()");
    fGroupFrame->AddFrame(fSources,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,0,0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Energy: From "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fSourceEnergyRangeMin = new TGNumberEntry(fHorizontalFrame, 0, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fSourceEnergyRangeMin->Connect("ValueSet(Long_t)", "CXFitEfficiency", this, "UpdateSources()");
    fHorizontalFrame->AddFrame(fSourceEnergyRangeMin,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "To "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fSourceEnergyRangeMax = new TGNumberEntry(fHorizontalFrame, 10000, 5, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fSourceEnergyRangeMax,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fSourceEnergyRangeMax->Connect("ValueSet(Long_t)", "CXFitEfficiency", this, "UpdateSources()");
    TGTextButton *Apply = new TGTextButton(fHorizontalFrame, "Apply");
    Apply->Connect("Clicked()", "CXFitEfficiency", this, "UpdateSources()");
    fHorizontalFrame->AddFrame(Apply,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Intensity: From "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,2,5,0,0));
    fSourceIntensityRangeMin = new TGNumberEntry(fHorizontalFrame, 1, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fSourceIntensityRangeMin->Connect("ValueSet(Long_t)", "CXFitEfficiency", this, "UpdateSources()");
    fHorizontalFrame->AddFrame(fSourceIntensityRangeMin,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "To "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fSourceIntensityRangeMax = new TGNumberEntry(fHorizontalFrame, 100, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fSourceIntensityRangeMax,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fSourceIntensityRangeMax->Connect("ValueSet(Long_t)", "CXFitEfficiency", this, "UpdateSources()");
    Apply = new TGTextButton(fHorizontalFrame, "Apply");
    Apply->Connect("Clicked()", "CXFitEfficiency", this, "UpdateSources()");
    fHorizontalFrame->AddFrame(Apply,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *ShowSources = new TGTextButton(fHorizontalFrame, "Show available sources");
    ShowSources->Connect("Clicked()", "CXFitEfficiency", this, "ShowSources()");
    fHorizontalFrame->AddFrame(ShowSources,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,50,50,10,10));

    fGroupFrame = new TGGroupFrame(MotherFrame, "Fit properties", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Verbose level"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,20,0,0));
    fVerboseLevel = new TGNumberEntry(fHorizontalFrame, 1, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELLimitMinMax,0,3);
    fHorizontalFrame->AddFrame(fVerboseLevel,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,5,5));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "FWHM"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,40,0,0));
    fFWHMSPEntry = new TGNumberEntry(fHorizontalFrame, 15, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fFWHMSPEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Amplitude"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,15,10,0,0));
    fThresholdSPEntry = new TGNumberEntry(fHorizontalFrame, 5, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fThresholdSPEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Range:   From "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,5,0,0));
    fRangeMin = new TGNumberEntry(fHorizontalFrame, 0, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fRangeMin,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "To "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,15,5,0,0));
    fRangeMax = new TGNumberEntry(fHorizontalFrame, 32000, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fRangeMax,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    TGTextButton *DoCurrentRange = new TGTextButton(fHorizontalFrame, "Current");
    DoCurrentRange->Connect("Clicked()", "CXFitEfficiency", this, "GetCurrentRange()");
    fHorizontalFrame->AddFrame(DoCurrentRange,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Left tail"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,20,0,0));
    fLeftTail = new TGCheckButton(fHorizontalFrame);
    fLeftTail->SetState(kButtonDown);
    fHorizontalFrame->AddFrame(fLeftTail,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Right tail"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,15,20,0,0));
    fRightTail = new TGCheckButton(fHorizontalFrame);
    fRightTail->SetState(kButtonUp);
    fHorizontalFrame->AddFrame(fRightTail,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Use 2nd-derivative search"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,20,0,0));
    f2DSearch = new TGCheckButton(fHorizontalFrame);
    fHorizontalFrame->AddFrame(f2DSearch,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fGroupFrame = new TGGroupFrame(MotherFrame, "Efficiency fit", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *DoEfficiencyCurve = new TGTextButton(fHorizontalFrame, "Build efficiency curve");
    DoEfficiencyCurve->SetTextColor(CXred);
    DoEfficiencyCurve->Connect("Clicked()", "CXFitEfficiency", this, "BuildEfficiencyCurve()");
    fHorizontalFrame->AddFrame(DoEfficiencyCurve,new TGLayoutHints(kLHintsCenterY,5,5,0,0));

    TGTextButton *DoEfficiencyAutoFit = new TGTextButton(fHorizontalFrame, "  Auto fit  ");
    DoEfficiencyAutoFit->SetTextColor(CXred);
    DoEfficiencyAutoFit->Connect("Clicked()", "CXFitEfficiency", this, "AutoFit()");
    DoEfficiencyAutoFit->SetToolTipText("Fit process proposed by radware: First fit high Energy (fix: A, B, C, G), then low energy (release A,B) and finally the link (release G)");
    fHorizontalFrame->AddFrame(DoEfficiencyAutoFit,new TGLayoutHints(kLHintsCenterY,5,5,0,0));

    TGTextButton *Save = new TGTextButton(fHorizontalFrame, "Save");
    Save->SetTextColor(CXred);
    Save->Connect("Clicked()", "CXFitEfficiency", this, "Save()");
    fHorizontalFrame->AddFrame(Save,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,10,10,10,10));

    TGLabel *label;
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, "Normalize to ref:"),new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 5, 0, 0));
    label->SetTextColor(CXblue);
    fNormalized = new TGCheckButton(fHorizontalFrame, "", 0);
    fNormalized->Connect("Clicked()", "CXFitEfficiency", this, "HandleMyButton()");
    fNormalized->SetState(kButtonDown);
    fHorizontalFrame->AddFrame(fNormalized,new TGLayoutHints(kLHintsTop | kLHintsLeft,10,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,10,5));

    TString Names[7] = {"A","B","C","D","E","F","G"};
    double mins[7]   = {0.,0.,0.,0.,-2.,-1.,1.};
    double values[7] = {7,0.7,0.,5.,-0.9,0.01,11};
    double maxs[7]   = {100.,2.,1.,50.,0.,1.,30.};

    for(int i=0 ; i<7 ; i++) {
        fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
        fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, Form("%s:",Names[i].Data())),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
        label->SetTextColor(CXblue);
        fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "fixed"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
        fFixFitPar[i] = new TGCheckButton(fHorizontalFrame, "", 0);
        fFixFitPar[i]->Connect("Clicked()", "CXFitEfficiency", this, "HandleMyButton()");
        fHorizontalFrame->AddFrame(fFixFitPar[i],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,5,0,0));

        fNE_FitPars[i][0] = new TGNumberEntry(fHorizontalFrame, mins[i], 5,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);
        fHorizontalFrame->AddFrame(fNE_FitPars[i][0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));

        fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));

        fNE_FitPars[i][1] = new TGNumberEntry(fHorizontalFrame, values[i], 5,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);
        fHorizontalFrame->AddFrame(fNE_FitPars[i][1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));

        fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, " <"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));

        fNE_FitPars[i][2] = new TGNumberEntry(fHorizontalFrame, maxs[i], 5,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber ,TGNumberFormat::kNELNoLimits);
        fHorizontalFrame->AddFrame(fNE_FitPars[i][2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));

        fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,0,5));

        if(i==1) fFixFitPar[i]->SetState(kButtonDown);
        if(i==2) fFixFitPar[i]->SetState(kButtonDown);
        if(i==6) fFixFitPar[i]->SetState(kButtonDown);
    }

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *DoFit = new TGTextButton(fHorizontalFrame, "Fit");
    DoFit->SetTextColor(CXred);
    DoFit->Connect("Clicked()", "CXFitEfficiency", this, "DoFit()");
    fHorizontalFrame->AddFrame(DoFit,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));
    TGTextButton *DoReset = new TGTextButton(fHorizontalFrame, "Reset params");
    DoReset->SetTextColor(CXred);
    DoReset->Connect("Clicked()", "CXFitEfficiency", this, "ResetParams()");
    fHorizontalFrame->AddFrame(DoReset,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,20,20,10,10));

    HandleMyButton();

    fListOfObjects = new TList;
    fListOfObjects->SetOwner();

    fRecalEnergy = new CXRecalEnergy;
}

void CXFitEfficiency::HandleMyButton()
{
    for(int i=0 ; i<7 ; i++) {
        if(fFixFitPar[i]->GetState() == kButtonDown) {
            fNE_FitPars[i][0]->SetState(false);
            fNE_FitPars[i][2]->SetState(false);
        }
        else{
            fNE_FitPars[i][0]->SetState(true);
            fNE_FitPars[i][2]->SetState(true);
        }
    }
}

CXFitEfficiency::~CXFitEfficiency() = default;

void CXFitEfficiency::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXFitEfficiency::UpdateText()
{
    fSources->SetTextColor(CXred);
}

void CXFitEfficiency::ShowSources()
{
    TString Path = Form("%s/databases/Sources",getenv("CUBIX_SYS"));
    TSystemDirectory dir(Path,Path);

    if(dir.GetListOfFiles() == nullptr) {
        gbash_color->WarningMessage("source folder is empty");
        return;
    }
    TList *ListOfFiles = dir.GetListOfFiles();
    ListOfFiles->Sort();
    gbash_color->InfoMessage("Sources availbale (including intensities):");
    for(auto file: *ListOfFiles) {
        TString filename = file->GetName();
        if(filename.EndsWith(".sou")) cout << filename.Copy().ReplaceAll(".sou","") << " ";
    }
    cout << endl;
}

void CXFitEfficiency::UpdateSources()
{
    fEnergies.clear();
    fIntensities.clear();
    fERef = 0.;

    TString tmp = fSources->GetText();
    tmp.ReplaceAll(",", " ");

    TObjArray *arr = tmp.Tokenize(" ");
    if(arr->GetEntries()==0) {
        delete arr;
        return;
    }

    fSources->SetTextColor(CXblack);

    gbash_color->InfoMessage("Energies used for calibration: ");

    bool err=false;

    for(int i=0 ; i<arr->GetEntries() ; i++) {

        TString SourceName =  arr->At(i)->GetName();
        CXGammaSource source(SourceName);
        if(source.is_known()) {
            if(source.is_source()) gbash_color->InfoMessage(Form("Loading source: %s",SourceName.Data()));
            else gbash_color->InfoMessage(Form("Loading calibration dataset: %s",SourceName.Data()));
            for(auto &dec: source.get_decays()) {
                if(dec.energy.get_value()<fSourceEnergyRangeMin->GetNumber() || dec.energy.get_value()>fSourceEnergyRangeMax->GetNumber()) continue;
                if(source.is_source() && (dec.intensity.get_value()<fSourceIntensityRangeMin->GetNumber() || dec.intensity.get_value()>fSourceIntensityRangeMax->GetNumber())) continue;
                fEnergies.push_back({dec.energy.get_value(), dec.energy.get_error()});
                fIntensities.push_back({dec.energy.get_value(), dec.energy.get_error(),dec.intensity.get_value(), dec.intensity.get_error()});
                if(dec.is_ref) fERef = dec.energy.get_value();
                if(source.is_source()) cout << left << " --> E: " << setw(9) << dec.energy.get_value() << "(" << setw(6) << dec.energy.get_error() << ") keV ; I: " << setw(6) << dec.intensity.get_value() << "(" << setw(6) << dec.intensity.get_error() << ")" << endl;
                else cout << left << " --> E: " << setw(9) << dec.energy.get_value() << " keV" << endl;
            }
        }
        else if(((TString)arr->At(i)->GetName()) == "-ener" && i<arr->GetEntries()-1) {
            TString manualvalue = ((TString)arr->At(i+1)->GetName());
            if(manualvalue.IsFloat()) {
                fEnergies.push_back({manualvalue.Atof(),0.});
                cout << "Value: " << arr->At(i+1)->GetName() << " (keV) manually added." << endl;
                i++;
                continue;
            }
            else {
                gbash_color->WarningMessage(" Error in manually added value");
                fSources->SetTextColor(CXred);
                continue;
            }
        }
        else if(((TString)arr->At(i)->GetName()) == "-remove" && i<arr->GetEntries()-1) {
            TString manualvalue = ((TString)arr->At(i+1)->GetName());
            if(manualvalue.IsFloat()) {
                double val = manualvalue.Atof();

                // remove the clostest value if diff lower than 1 keV
                int index=0;
                double closest = fEnergies.at(index).first;
                double diff = abs(closest-val);
                for(int itest=0 ; itest<fEnergies.size() ; itest++) {
                    double tmpdiff = abs(fEnergies.at(itest).first-val);
                    if(tmpdiff<diff) {
                        closest = fEnergies.at(itest).first;
                        diff = tmpdiff;
                        index=itest;
                    }
                }
                if(diff<1.) {
                    fEnergies.erase(fEnergies.begin() + index);
                    for (auto it = fIntensities.begin(); it != fIntensities.end(); it++) {
                        if((*it).at(0) == closest) {
                            fIntensities.erase(it);
                            break;
                        }
                    }
                    i++;
                    gbash_color->InfoMessage(Form("Value: %g (keV) manually removed.",closest));
                }
                else {
                    gbash_color->WarningMessage(Form("Error in manually removed energy, no transition found at less than one keV from: %s",arr->At(i+1)->GetName()));
                    err=true;
                    continue;
                }
            }
            else {
                gbash_color->WarningMessage("Error in manually removed energy: not a float value");
                err=true;
                continue;
            }
        }
        else if(((TString)arr->At(i)->GetName()) == "-ref" && i<arr->GetEntries()-1) {
            TString refvalue = ((TString)arr->At(i+1)->GetName());
            if(refvalue.IsFloat()) {
                fERef = refvalue.Atof();
                i++;
                continue;
            }
            else {
                gbash_color->WarningMessage("Error in manually added reference value: not a float value");
                err=true;
                continue;
            }
        }
        else {
            gbash_color->WarningMessage(SourceName + " not found ");
            err=true;
            continue;
        }
    }
    delete arr;

    if(fERef>0.) {
        // define the ref peak at the closest one to the given value
        double closest = fEnergies.front().first;
        double diff = abs(closest-fERef);
        for(auto &e: fEnergies) {
            double tmpdiff = abs(e.first-fERef);
            if(tmpdiff<diff) {
                closest = e.first;
                diff = tmpdiff;
            }
        }
        if(diff<1.) {
            fERef = closest;
            gbash_color->InfoMessage(Form("Value: %g (keV) manually used as reference.",fERef));
        }
        else {
            err = true;
            gbash_color->WarningMessage(Form("Error in manually refence value, no transition found at less than one keV from: %g",fERef));
            fERef=0.;
        }
    }

    if(err) {
        fSources->SetTextColor(CXred);
        return;
    }

    std::sort(fEnergies.begin(), fEnergies.end(), [](const std::pair<double, double>& a, const std::pair<double, double>& b) {
        return a.first < b.first;
    });

    if(fERef==0.) fERef = fEnergies.back().first;

    if(fEnergies.size() && fERef>0.) {
        gbash_color->InfoMessage(Form("Reference energy for printouts: %.3f keV",fERef));
    }

    cout<<endl;
}

void CXFitEfficiency::CleanEfficiency()
{
    fListOfObjects->Clear();
}

void CXFitEfficiency::GetCurrentRange()
{
    TH1 *hist = fMainWindow->GetCanvas()->FindHisto();

    if(hist == nullptr || hist->GetDimension()>1) {
        gbash_color->WarningMessage("No 1D histogram in the current pad, ignored ");
        return;
    }

    fRangeMin->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetFirst()));
    fRangeMax->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetLast()));
}

TH1 *CXFitEfficiency::CheckFitProperties()
{
    UpdateSources();

    TH1 *hist = fMainWindow->GetCanvas()->FindHisto(fMainWindow->GetCanvas());

    if(hist == nullptr || hist->GetDimension()>1) {
        gbash_color->WarningMessage("No 1D histogram in the current pad, ignored ");
        return nullptr;
    }
    if(fEnergies.size()==0) {
        gbash_color->WarningMessage("No source defined, ignored ");
        return nullptr;
    }

    if(fRangeMin->GetNumber()<hist->GetXaxis()->GetBinLowEdge(1))
        fRangeMin->SetNumber(hist->GetXaxis()->GetBinLowEdge(1));
    if(fRangeMax->GetNumber()>hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()))
        fRangeMax->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()));

    fRecalEnergy->Reset();
    fRecalEnergy->SetDataFromHistTH1(hist,0);

    for (auto ie : fEnergies)
        fRecalEnergy->AddPeak(ie.first, ie.second);
    for (auto ie : fIntensities)
        fRecalEnergy->AddEfficiencyPeak(ie);

    if(fERef>0.) fRecalEnergy->SetRefPeak(fERef);

    //    fRecalEnergy->SetGain(1.);                          // scaling factor for the slope [1]
    //    fRecalEnergy->SetChannelOffset(0);                  // channel offset to subtract to the position of the peaks [0]
    fRecalEnergy->SetVerbosityLevel(fVerboseLevel->GetNumber()-1);                 // verbosity -1=noprint 0=fit_details, 1=calib_details, 2=more_calib_details [-1]

    fRecalEnergy->UseLeftTail(fLeftTail->GetState());
    fRecalEnergy->UseRightTail(fRightTail->GetState());

    if(f2DSearch->GetState() == kButtonUp)
        fRecalEnergy->UseFirstDerivativeSearch();
    else
        fRecalEnergy->UseSecondDerivativeSearch();

    //fRecalEnergy->UseSecondDerivativeSearch();        // use the 2nd-derivative search
    fRecalEnergy->SetGlobalChannelLimits(fRangeMin->GetNumber(),fRangeMax->GetNumber());      // limit the search to this range in channels
    fRecalEnergy->SetGlobalPeaksLimits(fFWHMSPEntry->GetNumber(),fThresholdSPEntry->GetNumber());   // default fwhm and minmum amplitude for the peaksearch [15 5]

    fRecalEnergy->DoEfficiencyScale(false);
    if(fNormalized->GetState() == kButtonDown) fRecalEnergy->DoEfficiencyScale(true);

    return hist;
}

void CXFitEfficiency::BuildEfficiencyCurve()
{
    CleanEfficiency();

    TH1 *hist = CheckFitProperties();

    if(hist == nullptr) {
        gbash_color->WarningMessage("No histogram found in the current canvas");
        return;
    }

    if(!fIntensities.size()) {
        gbash_color->WarningMessage("No source with intensities defined, ignored");
        return;
    }

    fRecalEnergy->FitEfficiency();

    vector < Fitted > FitResults = fRecalEnergy->GetFitResults();

    double xmin=-1;
    double xmax=-1;

    int NGoodPeak=0;

    fMainWindow->GetCanvas()->cd();

    // remove bad peaks
    for(size_t i=0 ; i<FitResults.size() ; i++) {
        Fitted FitRes = FitResults[i];
        if(!FitRes.good) {
            FitResults.erase(FitResults.begin()+i);
            i--;
        }
    }
    for(size_t i=0 ; i<FitResults.size() ; i++) {
        Fitted FitRes = FitResults[i];
    }
    // recalculate NSubPeaks
    for(size_t i=0 ; i<FitResults.size() ;) {
        Fitted FitRes = FitResults[i];
        int nsub=0;
        for(int j=i ; j<FitResults.size() ; j++) {
            Fitted FitRes2 = FitResults[j];
            if(FitRes2.BgTo>FitRes.BgTo) break;
            nsub++;
        }
        for(int j=0 ; j<nsub ; j++) FitResults[i+j].NSubPeaks = nsub;
        i+=nsub;
    }

    for(size_t i=0 ; i<FitResults.size() ; i++) {
        Fitted FitRes = FitResults[i];

        int NSubPeaks = FitRes.NSubPeaks;

        TF1 *f = GetDinoFct(Form("Peak%d_%.1f",NGoodPeak,FitRes.eref),FitRes.BgFrom/fRecalEnergy->hGain,FitRes.BgTo/fRecalEnergy->hGain,5+6*NSubPeaks);
        f->SetNpx(10000);

        f->SetParameter(0,NSubPeaks);
        f->SetParameter(1,FitRes.BgFrom/fRecalEnergy->hGain);
        f->SetParameter(2,FitRes.BgTo/fRecalEnergy->hGain);
        f->SetParameter(3,FitRes.BgdOff);
        f->SetParameter(4,FitRes.BgdSlope);

        if(xmin == -1) xmin = FitRes.BgFrom/fRecalEnergy->hGain;
        xmax = FitRes.BgTo/fRecalEnergy->hGain;

        int peakid=0;
        for(int j=0 ; j<NSubPeaks ; j++) {

            FitRes = FitResults[i+j];

            f->SetParameter(5+peakid*6+0,FitRes.ampli);
            f->SetParameter(5+peakid*6+1,FitRes.posi/fRecalEnergy->hGain);
            f->SetParameter(5+peakid*6+2,FitRes.fwhm/fRecalEnergy->hGain);
            f->SetParameter(5+peakid*6+3,FitRes.Lambda);
            f->SetParameter(5+peakid*6+4,FitRes.Rho);
            f->SetParameter(5+peakid*6+5,FitRes.S);

            if(fLeftTail->GetState()==kButtonUp)
                f->SetParameter(5+peakid*6+3,-50);
            if(fRightTail->GetState()==kButtonUp)
                f->SetParameter(5+peakid*6+4,50);

            peakid++;
        }
        f->Draw("same");
        fListOfObjects->Add(f);
        i += NSubPeaks-1;
    }

    if(xmin!=-1) hist->GetXaxis()->SetRangeUser(xmin-(xmax-xmin)*0.1,xmax+(xmax-xmin)*0.1);

    fMainWindow->RefreshPads();

    if(FitResults.size()>1) {
        if(fEfficiencyCanvas && fEfficiencyCanvas->GetCanvasImp()) {
            fEfficiencyCanvas->cd();
        }
        else {
            if(fEfficiencyCanvas) {
                for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
                    if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("Efficiency fit")) {
                        gROOT->GetListOfCanvases()->RemoveAt(i);
                        i--;
                    }
                }
            }
            fEfficiencyCanvas = new TCanvas("Efficiency","Efficiency fit");
        }
        fRecalEnergy->fEfficiencyGraph->Draw("ape");

        fEfficiencyCanvas->Update();
        fEfficiencyCanvas->Modified();
        fEfficiencyCanvas->RaiseWindow();
    }

    fMainWindow->GetCanvas()->cd();
}

void CXFitEfficiency::InitFit()
{
    if(!fEfficiencyGraph || fEfficiencyGraph->GetN()<2) {
        gbash_color->WarningMessage("Efficiency graph not existing of not containing enough points");
        return;
    }
    double xmin = 0.;
    double xmax = fEfficiencyGraph->GetX()[fEfficiencyGraph->GetN()-1]*1.5;
    delete fEfficiencyFunction;
    fEfficiencyFunction = new TF1("EfficiencyFunc", &CXFitFunctions::EfficiencyFunc, xmin, xmax, 8);

    fEfficiencyFunction->SetLineColor(kBlue);
    fEfficiencyFunction->SetNpx(5000);
    fEfficiencyFunction->SetParNames("Scale","A","B","C","D","E","F","G");
}

void CXFitEfficiency::ResetParams()
{
    double default_params[7] = {7,0.7,0.,5.,-0.9,0.01,11};
    for(int i=0 ; i<7 ; i++) {
        fNE_FitPars[i][1]->SetNumber(default_params[i]);
    }
}

void CXFitEfficiency::DoFit()
{
    fEfficiencyGraph = nullptr;

    if(fEfficiencyCanvas && fEfficiencyCanvas->GetCanvasImp()) fEfficiencyGraph = fMainWindow->GetGraph(fEfficiencyCanvas,1);
    if(fEfficiencyGraph == nullptr) fEfficiencyGraph = fMainWindow->GetGraph();
    if(fEfficiencyGraph == nullptr) {
        gbash_color->WarningMessage("Efficiency graph not found");
        return;
    }
    if(fEfficiencyGraph->GetN()<2) {
        gbash_color->WarningMessage("Efficiency graph not containing enough points");
        return;
    }

    if(!fEfficiencyFunction) InitFit();
    if(!fEfficiencyFunction) return;

    double xmin = 0.;
    double xmax = fEfficiencyGraph->GetX()[fEfficiencyGraph->GetN()-1]*1.5;
    fEfficiencyFunction->SetRange(xmin,xmax);

    // double scalefact = 1.;
    // if(fNormalized->GetState() == kButtonDown) scalefact = 0.0011988228*(*max_element(fEfficiencyGraph->GetY(),fEfficiencyGraph->GetY()+fEfficiencyGraph->GetN()));  // 0.0011988228 is to scale the default parameters, to a max at 1

    double scalefact = 1.;
    if(fNormalized->GetState() == kButtonDown) {
        double max = *max_element(fEfficiencyGraph->GetY(),fEfficiencyGraph->GetY()+fEfficiencyGraph->GetN());
        scalefact = max/1000.;  // scale the default parameters, to a max at 1000 (like in radware's example)
    }

    fEfficiencyFunction->FixParameter(0,scalefact);

    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Migrad");
    ROOT::Math::MinimizerOptions::SetDefaultMaxIterations(1000000);
    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(1000000);

    fEfficiencyFunction->FixParameter(0,scalefact);

    for(int i=0 ; i<7 ; i++) {
        if(fFixFitPar[i]->GetState() == kButtonDown) {
            fEfficiencyFunction->FixParameter(i+1,fNE_FitPars[i][1]->GetNumber());
        }
        else {
            fEfficiencyFunction->ReleaseParameter(i+1);
            fEfficiencyFunction->SetParameter(i+1,fNE_FitPars[i][1]->GetNumber());
            fEfficiencyFunction->SetParLimits(i+1,fNE_FitPars[i][0]->GetNumber(),fNE_FitPars[i][2]->GetNumber());
        }
    }

    TFitResultPtr r = fEfficiencyGraph->Fit(fEfficiencyFunction,"S0R");

    for(int i=0 ; i<7 ; i++) {
        fNE_FitPars[i][1]->SetNumber(fEfficiencyFunction->GetParameter(i+1));
    }

    if(!r->IsValid()) {
        gbash_color->WarningMessage("Warning: Fit failed");
        delete gROOT->FindObject("EffConfidence95");
        fEfficiencyConfidenceIntervall = nullptr;
    }
    else {
        delete gROOT->FindObject("EffConfidence95");
        fEfficiencyConfidenceIntervall = new TH1D("EffConfidence95","Efficiency 0.95 confidence band", 10000, 0, 10000);
        (TVirtualFitter::GetFitter())->GetConfidenceIntervals(fEfficiencyConfidenceIntervall);
        fEfficiencyConfidenceIntervall->SetLineWidth(0);
        fEfficiencyConfidenceIntervall->SetFillColor(kBlue);
        fEfficiencyConfidenceIntervall->SetFillColorAlpha(kBlue,0.1);
        fEfficiencyConfidenceIntervall->SetFillStyle(1001);
        fEfficiencyConfidenceIntervall->SetStats(false);
        fEfficiencyConfidenceIntervall->SetDirectory(nullptr);
        fEfficiencyConfidenceIntervall->GetXaxis()->SetTitle(fEfficiencyGraph->GetXaxis()->GetTitle());
        fEfficiencyConfidenceIntervall->GetYaxis()->SetTitle(fEfficiencyGraph->GetYaxis()->GetTitle());
    }

    if(fEfficiencyCanvas && fEfficiencyCanvas->GetCanvasImp()) {
        fEfficiencyCanvas->cd();
    }
    else {
        if(fEfficiencyCanvas) {
            for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
                if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("Efficiency fit")) {
                    gROOT->GetListOfCanvases()->RemoveAt(i);
                    i--;
                }
            }
        }
        fEfficiencyCanvas = new TCanvas("Efficiency","Efficiency fit");
    }
    fEfficiencyGraph->Draw("ape");

    if(fEfficiencyConfidenceIntervall) fEfficiencyConfidenceIntervall->Draw("e3 same");
    fEfficiencyFunction->Draw("same");

    fEfficiencyCanvas->Update();
    fEfficiencyCanvas->Modified();
    fEfficiencyCanvas->RaiseWindow();
}

void CXFitEfficiency::AutoFit()
{
    fEfficiencyGraph = nullptr;

    if(fEfficiencyCanvas && fEfficiencyCanvas->GetCanvasImp()) fEfficiencyGraph = fMainWindow->GetGraph(fEfficiencyCanvas,1);
    if(fEfficiencyGraph == nullptr) fEfficiencyGraph = fMainWindow->GetGraph();
    if(fEfficiencyGraph == nullptr) {
        gbash_color->WarningMessage("Efficiency graph not found");
        return;
    }
    if(fEfficiencyGraph->GetN()<2) {
        gbash_color->WarningMessage("Efficiency graph not containing enough points");
        return;
    }

    if(!fEfficiencyFunction) InitFit();
    if(!fEfficiencyFunction) return;

    double xmin = 0.;
    double xmax = fEfficiencyGraph->GetX()[fEfficiencyGraph->GetN()-1]*1.5;
    fEfficiencyFunction->SetRange(xmin,xmax);

    double scalefact = 1.;
    if(fNormalized->GetState() == kButtonDown) {
        double max = *max_element(fEfficiencyGraph->GetY(),fEfficiencyGraph->GetY()+fEfficiencyGraph->GetN());
        scalefact = max/1000.;  // scale the default parameters, to a max at 1000 (like in radware's example)
    }

    fEfficiencyFunction->FixParameter(0,scalefact);

    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Migrad");
    ROOT::Math::MinimizerOptions::SetDefaultMaxIterations(1000000);
    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(1000000);

    fEfficiencyFunction->SetParameter(0,scalefact);

    double default_params[7] = {7,0.7,0.,5.,-0.9,0.01,11};

    // 1 - Fit only high energy part:
    fEfficiencyFunction->FixParameter(1,default_params[0]);
    fEfficiencyFunction->FixParameter(2,default_params[1]);
    fEfficiencyFunction->FixParameter(3,default_params[2]);

    fEfficiencyFunction->SetParameter(4,default_params[3]);
    fEfficiencyFunction->SetParLimits(4,0,50);
    fEfficiencyFunction->SetParameter(5,default_params[4]);
    fEfficiencyFunction->SetParLimits(5,-2.,0.);
    fEfficiencyFunction->SetParameter(6,default_params[5]);
    fEfficiencyFunction->SetParLimits(6,-1.,1.);

    fEfficiencyFunction->FixParameter(7,default_params[6]);

    TFitResultPtr r = fEfficiencyGraph->Fit(fEfficiencyFunction,"S0R","",0,xmax);

    // 2 - Add low energy part:
    fEfficiencyFunction->ReleaseParameter(1);
    // fEfficiencyFunction->SetParameter(1,0.);
    fEfficiencyFunction->SetParLimits(1,-50,50);
    fEfficiencyFunction->ReleaseParameter(2);
    // fEfficiencyFunction->SetParameter(2,1.);
    fEfficiencyFunction->SetParLimits(2,0.,2.);

    r = fEfficiencyGraph->Fit(fEfficiencyFunction,"S0R");

    // 2 - Release link between low and high energy
    fEfficiencyFunction->ReleaseParameter(7);
    // fEfficiencyFunction->SetParameter(7,10.);
    fEfficiencyFunction->SetParLimits(7,1.,30.);

    r = fEfficiencyGraph->Fit(fEfficiencyFunction,"S0R");

    for(int i=0 ; i<7 ; i++) {
        fNE_FitPars[i][1]->SetNumber(fEfficiencyFunction->GetParameter(i+1));
    }

    delete gROOT->FindObject("EffConfidence95");
    fEfficiencyConfidenceIntervall = nullptr;

    if(!r->IsValid()) {
        gbash_color->WarningMessage("Warning: Fit failed");
    }
    else {
        fEfficiencyConfidenceIntervall = new TH1D("EffConfidence95","Efficiency 0.95 confidence band", 5000, 0, 10000);
        (TVirtualFitter::GetFitter())->GetConfidenceIntervals(fEfficiencyConfidenceIntervall);
        fEfficiencyConfidenceIntervall->SetLineWidth(0);
        fEfficiencyConfidenceIntervall->SetFillColor(kBlue);
        fEfficiencyConfidenceIntervall->SetFillColorAlpha(kBlue,0.1);
        fEfficiencyConfidenceIntervall->SetFillStyle(1001);
        fEfficiencyConfidenceIntervall->SetStats(false);
        fEfficiencyConfidenceIntervall->SetDirectory(nullptr);
    }

    if(fEfficiencyCanvas && fEfficiencyCanvas->GetCanvasImp()) {
        fEfficiencyCanvas->cd();
    }
    else {
        if(fEfficiencyCanvas) {
            for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
                if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("Efficiency fit")) {
                    gROOT->GetListOfCanvases()->RemoveAt(i);
                    i--;
                }
            }
        }
        fEfficiencyCanvas = new TCanvas("Efficiency","Efficiency fit");
    }
    fEfficiencyGraph->Draw("ape");

    if(fEfficiencyConfidenceIntervall) fEfficiencyConfidenceIntervall->Draw("e3 same");
    fEfficiencyFunction->Draw("same");

    fEfficiencyCanvas->Update();
    fEfficiencyCanvas->Modified();
    fEfficiencyCanvas->RaiseWindow();
}

void CXFitEfficiency::FitEfficiencyCurve()
{
    if(fEfficiencyFunction == nullptr) InitFit();
}

void CXFitEfficiency::CloseCanvas() {
    cout << "close canvas "<< endl;
}

double CXFitEfficiency::DinoFct(double*xx,double*pp)
{
    double x   = xx[0];

    int    NSubPeaks = (int)pp[0]; //Number of subpeaks in the peak range
    double BgFrom    = pp[1]; //First Channel for the Bg estimation
    double BgTo      = pp[2]; //Last Channel for the Bg estimation
    double BgdOff    = pp[3]; //Bg offset
    double BgdSlope  = pp[4]; //Bg slope

    double f_tot = 0.;

    if(x<BgFrom || x>BgTo) return 0.;
    else {
        double BGd = BgdSlope*(x-BgFrom) + BgdOff;
        f_tot += BGd;
    }

    //    cout<<NSubPeaks<<" "<<BgFrom<<" "<<BgTo<<" "<<BgdOff<<" "<<BgdSlope<<endl;
    for(int i=0 ; i<NSubPeaks ; i++) {
        double Ampli     = pp[5+i*6+0];
        double Mean      = pp[5+i*6+1];
        double Sigma     = pp[5+i*6+2]*1./sqrt(8.*log(2.));;
        double Lambda    = pp[5+i*6+3];
        double Rho       = pp[5+i*6+4];
        double S         = pp[5+i*6+5];

        //        cout<<Ampli<<" "<<Mean<<" "<<Sigma<<" "<<Lambda<<" "<<Rho<<" "<<S<<endl;

        double U         = (x-Mean)/Sigma;
        double f_g       = Ampli*TMath::Exp(-U*U*0.5);
        double f_lambda  = Ampli*TMath::Exp(-0.5*Lambda*(2.*U-Lambda));
        double f_rho     = Ampli*TMath::Exp(-0.5*Rho*(2.*U-Rho));
        double f_S       = Ampli*S*1./((1+TMath::Exp(U))*(1+TMath::Exp(U)));

        if(U<Lambda) f_tot += f_lambda;
        else if(U>Rho) f_tot += f_rho;
        else f_tot += f_g;

        f_tot += f_S;
    }

    return f_tot;
}

///******************************************************************************************///

TF1 *CXFitEfficiency::GetDinoFct(TString Name,double min, double max, int Npar)
{
    TF1 *f = new TF1(Name,this,&CXFitEfficiency::DinoFct,min,max,Npar,"CXFitEfficiency","DinoFct");

    return f;
}

void CXFitEfficiency::Save()
{
    TString WSName="";
    CXDialogBox *diag = new CXDialogBox(this->GetMainFrame(),"Save in workspace:");
    diag->Add("Workspace name",WSName);
    diag->Popup();

    if(WSName=="") {
        glog << info << "Efficiency saving aborted" << do_endl;
        return;
    }

    CXWorkspace *workspace = fMainWindow->GetWSManager()->GetWorkspace(WSName);

    if(!workspace) {
        glog << error << "No Workspace named: " << WSName << do_endl;
        return;
    }

    workspace->SetEfficiency(fEfficiencyGraph,fEfficiencyFunction, fEfficiencyConfidenceIntervall);
}

ClassImp(CXFitEfficiency)
