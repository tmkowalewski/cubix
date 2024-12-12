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

#include "CXHist1DCalib.h"

#include <iostream>
#include <iomanip>

#include "TGNumberEntry.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TROOT.h"
#include "TObjArray.h"
#include "TF1.h"
#include "TMath.h"
#include "TSystemDirectory.h"
#include "TRandom3.h"
#include "Math/MinimizerOptions.h"
#include "TFitResult.h"
#include "TVirtualFitter.h"

#include "CXBashColor.h"
#include "CXMainWindow.h"
#include "CXGammaSource.h"
#include "CXDialogBox.h"
#include "CXFitFunctions.h"

using namespace std;

CXHist1DCalib::CXHist1DCalib(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{

    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Sources", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fGroupFrame->AddFrame(new TGLabel(fGroupFrame,"Input data"), new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 5, 0));
    fSources = new TGTextEntry(fGroupFrame, "");
    fSources->SetToolTipText("List of sources (files in $CUBIX_SYS/dataBase/Sources)");
    fSources->Connect("TextChanged(const char *)", "CXHist1DCalib", this, "UpdateText()");
    fSources->Connect("ReturnPressed()", "CXHist1DCalib", this, "UpdateSources()");
    fGroupFrame->AddFrame(fSources,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,0,0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Energy: From "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fSourceEnergyRangeMin = new TGNumberEntry(fHorizontalFrame, 0, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fSourceEnergyRangeMin->Connect("ValueSet(Long_t)", "CXHist1DCalib", this, "UpdateSources()");
    fHorizontalFrame->AddFrame(fSourceEnergyRangeMin,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "To "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fSourceEnergyRangeMax = new TGNumberEntry(fHorizontalFrame, 10000, 5, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fSourceEnergyRangeMax,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fSourceEnergyRangeMax->Connect("ValueSet(Long_t)", "CXHist1DCalib", this, "UpdateSources()");
    TGTextButton *button = new TGTextButton(fHorizontalFrame, "Apply");
    button->Connect("Clicked()", "CXHist1DCalib", this, "UpdateSources()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Intensity: From "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,2,5,0,0));
    fSourceIntensityRangeMin = new TGNumberEntry(fHorizontalFrame, 1, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fSourceIntensityRangeMin->Connect("ValueSet(Long_t)", "CXHist1DCalib", this, "UpdateSources()");
    fHorizontalFrame->AddFrame(fSourceIntensityRangeMin,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "To "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fSourceIntensityRangeMax = new TGNumberEntry(fHorizontalFrame, 100, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fSourceIntensityRangeMax,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fSourceIntensityRangeMax->Connect("ValueSet(Long_t)", "CXHist1DCalib", this, "UpdateSources()");
    button = new TGTextButton(fHorizontalFrame, "Apply");
    button->Connect("Clicked()", "CXHist1DCalib", this, "UpdateSources()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    button = new TGTextButton(fHorizontalFrame, "Show available sources");
    button->Connect("Clicked()", "CXHist1DCalib", this, "ShowSources()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
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
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Calibration order"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,10,0,0));
    fCalibOrder = new TGNumberEntry(fHorizontalFrame, 1, 5, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fCalibOrder,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "No offset"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,10,0,0));
    fNoOffset = new TGCheckButton(fHorizontalFrame);
    fNoOffset->SetState(kButtonUp);
    fHorizontalFrame->AddFrame(fNoOffset,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
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
    button = new TGTextButton(fHorizontalFrame, "Current");
    button->Connect("Clicked()", "CXHist1DCalib", this, "GetCurrentRange()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,3,0,0));
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

    fGroupFrame = new TGGroupFrame(MotherFrame, "Energy calibration", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    button = new TGTextButton(fHorizontalFrame, "Calibrate");
    button->SetTextColor(CXred);
    button->Connect("Clicked()", "CXHist1DCalib", this, "Calibrate()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    button = new TGTextButton(fHorizontalFrame, "Apply");
    button->SetTextColor(CXred);
    button->Connect("Clicked()", "CXHist1DCalib", this, "ApplyCalibration()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    button = new TGTextButton(fHorizontalFrame, "Save");
    button->SetTextColor(CXred);
    button->Connect("Clicked()", "CXHist1DCalib", this, "SaveECal()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,10,10,10,10));

    fGroupFrame = new TGGroupFrame(MotherFrame, "FWHM fit", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    button = new TGTextButton(fHorizontalFrame, "FWHM Calib");
    button->SetTextColor(CXred);
    button->Connect("Clicked()", "CXHist1DCalib", this, "FWHMCalib()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    button = new TGTextButton(fHorizontalFrame, "Save");
    button->SetTextColor(CXred);
    button->Connect("Clicked()", "CXHist1DCalib", this, "SaveFWHM()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,20,20,10,10));

    fListOfObjects = new TList;
    fListOfObjects->SetOwner();

    fRecalEnergy = new CXRecalEnergy;
}

CXHist1DCalib::~CXHist1DCalib() = default;

void CXHist1DCalib::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXHist1DCalib::UpdateText()
{
    fSources->SetTextColor(CXred);
}

void CXHist1DCalib::ShowSources()
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
    gbash_color->InfoMessage("Calibration dataset availbale (only energies):");
    for(auto file: *ListOfFiles) {
        TString filename = file->GetName();
        if(filename.EndsWith(".cal")) cout << filename.Copy().ReplaceAll(".cal","") << " ";
    }
    cout << endl;
}

void CXHist1DCalib::UpdateSources()
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

    if(fVerboseLevel->GetNumber()!=0) gbash_color->InfoMessage("Energies used for calibration: ");

    bool err=false;

    for(int i=0 ; i<arr->GetEntries() ; i++) {

        TString SourceName =  arr->At(i)->GetName();
        CXGammaSource source(SourceName);
        if(source.is_known()) {
            if(fVerboseLevel->GetNumber()!=0) {
                if(source.is_source()) gbash_color->InfoMessage(Form("Loading source: %s",SourceName.Data()));
                else gbash_color->InfoMessage(Form("Loading calibration dataset: %s",SourceName.Data()));
            }
            for(auto &dec: source.get_decays()) {
                if(dec.energy.get_value()<fSourceEnergyRangeMin->GetNumber() || dec.energy.get_value()>fSourceEnergyRangeMax->GetNumber()) continue;
                if(source.is_source() && (dec.intensity.get_value()<fSourceIntensityRangeMin->GetNumber() || dec.intensity.get_value()>fSourceIntensityRangeMax->GetNumber())) continue;
                fEnergies.push_back({dec.energy.get_value(), dec.energy.get_error()});
                fIntensities.push_back({dec.energy.get_value(), dec.energy.get_error(),dec.intensity.get_value(), dec.intensity.get_error()});
                if(dec.is_ref) fERef = dec.energy.get_value();
                if(fVerboseLevel->GetNumber()!=0) {
                    if(source.is_source()) cout << left << " --> E: " << setw(9) << dec.energy.get_value() << "(" << setw(6) << dec.energy.get_error() << ") keV ; I: " << setw(6) << dec.intensity.get_value() << "(" << setw(6) << dec.intensity.get_error() << ")" << endl;
                    else cout << left << " --> E: " << setw(9) << dec.energy.get_value() << " keV" << endl;
                }
            }
        }
        else if(((TString)arr->At(i)->GetName()) == "-ener" && i<arr->GetEntries()-1) {
            TString manualvalue = ((TString)arr->At(i+1)->GetName());
            if(manualvalue.IsFloat()) {
                fEnergies.push_back({manualvalue.Atof(),0.});
                if(fVerboseLevel->GetNumber()!=0) cout << "Value: " << arr->At(i+1)->GetName() << " (keV) manually added." << endl;
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
            if(fVerboseLevel->GetNumber()!=0) gbash_color->InfoMessage(Form("Value: %g (keV) manually used as reference.",fERef));
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

    if(fEnergies.size() && fERef>0. && fVerboseLevel->GetNumber()!=0) {
        gbash_color->InfoMessage(Form("Reference energy for printouts: %.3f keV",fERef));
    }

    if(fVerboseLevel->GetNumber()!=0) cout<<endl;
}

void CXHist1DCalib::CleanCalib()
{
    fListOfObjects->Clear();
}

void CXHist1DCalib::GetCurrentRange()
{
    TH1 *hist = fMainWindow->GetCanvas()->FindHisto();

    if(hist == nullptr || hist->GetDimension()>1) {
        gbash_color->WarningMessage("No 1D histogram in the current pad, ignored ");
        return;
    }

    fRangeMin->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetFirst()));
    fRangeMax->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetLast()));
}

bool CXHist1DCalib::CheckFitProperties(TH1 *hist)
{
    UpdateSources();

    if(hist == nullptr || hist->GetDimension()>1) {
        gbash_color->WarningMessage("No 1D histogram in the current pad, ignored ");
        return false;
    }
    if(fEnergies.size()==0) {
        gbash_color->WarningMessage("No source defined, ignored ");
        return false;
    }

    if(fRangeMin->GetNumber()<hist->GetXaxis()->GetBinLowEdge(1))
        fRangeMin->SetNumber(hist->GetXaxis()->GetBinLowEdge(1));
    if(fRangeMax->GetNumber()>hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()))
        fRangeMax->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()));

    fRecalEnergy->Reset();
    fRecalEnergy->SetDataFromHistTH1(hist,0);

    for (auto ie : fEnergies)
        fRecalEnergy->AddPeak(ie.first, ie.second);

    if(fERef>0.) fRecalEnergy->SetRefPeak(fERef);

    //    fRecalEnergy->SetGain(1.);                          // scaling factor for the slope [1]
    //    fRecalEnergy->SetChannelOffset(0);                  // channel offset to subtract to the position of the peaks [0]
    fRecalEnergy->SetVerbosityLevel(fVerboseLevel->GetNumber()-1);                 // verbosity -1=noprint 0=fit_details, 1=calib_details, 2=more_calib_details [-1]

    fRecalEnergy->SetFitPlynomialOrder(fCalibOrder->GetNumber());
    fRecalEnergy->SetNoOffset(fNoOffset->GetState());

    fRecalEnergy->UseLeftTail(fLeftTail->GetState());
    fRecalEnergy->UseRightTail(fRightTail->GetState());

    if(f2DSearch->GetState() == kButtonUp)
        fRecalEnergy->UseFirstDerivativeSearch();
    else
        fRecalEnergy->UseSecondDerivativeSearch();

    //fRecalEnergy->UseSecondDerivativeSearch();        // use the 2nd-derivative search
    fRecalEnergy->SetGlobalChannelLimits(fRangeMin->GetNumber(),fRangeMax->GetNumber());      // limit the search to this range in channels
    fRecalEnergy->SetGlobalPeaksLimits(fFWHMSPEntry->GetNumber(),fThresholdSPEntry->GetNumber());   // default fwhm and minmum amplitude for the peaksearch [15 5]

    return true;
}

void CXHist1DCalib::Calibrate2D(TH2 *hist)
{
    if(hist->GetNbinsY()>100) {
        gbash_color->WarningMessage("Too many channels on Y axis for an auto calibration of all Y bins");
        return;
    }
    if(hist->GetYaxis()->GetBinWidth(1)!=1) {
        gbash_color->WarningMessage("Auto TH2 calib can only be done on histogram with Y bins width = 1");
        return;
    }

    delete fCalib2DGraph;
    fCalib2DGraph = new TGraph;
    fCalib2DGraph->SetNameTitle("Calibrate2D","Calibrate2D");
    fCalib2DGraph->SetMarkerStyle(20);
    fCalib2DGraph->GetXaxis()->SetTitle("Channel id");
    fCalib2DGraph->GetYaxis()->SetTitle("FWHM (keV)");

    for(int i=1 ; i<=hist->GetNbinsY() ; i++) {

        TH1 *proj = hist->ProjectionX("_px",i,i);

        bool ok = CheckFitProperties(proj);

        if(!ok) {
            gbash_color->WarningMessage("No histogram found for energy calibration, ignored");
            return;
        }

        if(!fEnergies.size()) {
            gbash_color->WarningMessage("No source with energies defined, ignored");
            return;
        }

        fRecalEnergy->StartCalib();
        fCalib2DGraph->SetPoint(fCalib2DGraph->GetN(),i-1,fRecalEnergy->f_ref_fw05);

        if(fVerboseLevel->GetNumber()==0) {
            int prec = cout.precision();
            cout<< left << scientific << setprecision(6);
            cout<< Form("%s_%d: ",hist->GetName(),i-1);
            if(fRecalEnergy->fCalibFunction) {
                cout << setw(14) << fRecalEnergy->fCalibFunction->GetParameter(1);
                for(int i=1 ; i<=fRecalEnergy->fCalibOrder ; i++) {
                    cout << setw(14) << fRecalEnergy->fCalibFunction->GetParameter(i+1);
                }
            }
            else {
                cout << setw(14) << 0.;
                for(int i=1 ; i<=fRecalEnergy->fCalibOrder ; i++) {
                    cout << setw(14) << 0.;
                }
                cout << " => Fit failed";
            }
            cout<<endl;
            cout.precision(prec);
            cout << fixed;
        }
    }

    if(fCalib2DCanvas && fCalib2DCanvas->GetCanvasImp()) {
        fCalib2DCanvas->cd();
    }
    else {
        if(fCalib2DCanvas) {
            for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
                if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("Calibration 2D Results")) {
                    gROOT->GetListOfCanvases()->RemoveAt(i);
                    i--;
                }
            }
        }
        fCalib2DCanvas = new TCanvas("Calibration2DResults","Calibration 2D Results");
    }
    fCalib2DGraph->Draw("ape");

    gErrorIgnoreLevel=kFatal;
    fCalib2DCanvas->Update();
    fCalib2DCanvas->Modified();
    gErrorIgnoreLevel=kPrint;

    fCalib2DCanvas->RaiseWindow();

    fMainWindow->GetCanvas()->cd();
}

void CXHist1DCalib::Calibrate()
{
    CleanCalib();

    TH1 *hist = fMainWindow->GetCanvas()->FindHisto(fMainWindow->GetCanvas());

    if(hist->GetDimension()==2) {
        Calibrate2D(dynamic_cast<TH2*>(hist));
        return;
    }

    bool ok = CheckFitProperties(hist);

    if(!ok) {
        gbash_color->WarningMessage("No histogram found for energy calibration, ignored");
        return;
    }

    if(!fEnergies.size()) {
        gbash_color->WarningMessage("No source with energies defined, ignored");
        return;
    }

    fRecalEnergy->StartCalib();

    vector < Fitted > FitResults = fRecalEnergy->GetFitResults();

    if(fVerboseLevel->GetNumber()==0) {
        int prec = cout.precision();
        cout<< left << scientific << setprecision(6);
        cout<< hist->GetName()<<": ";
        if(fRecalEnergy->fCalibFunction) {
            cout << setw(14) << fRecalEnergy->fCalibFunction->GetParameter(1);
            for(int i=1 ; i<=fRecalEnergy->fCalibOrder ; i++) cout << setw(14) << fRecalEnergy->fCalibFunction->GetParameter(i+1);
        }
        else {
            cout << setw(14) << 0.;
            for(int i=1 ; i<=fRecalEnergy->fCalibOrder ; i++) {
                cout << setw(14) << 0.;
            }
            cout << " => Fit failed";
        }
        cout<<endl;
        cout.precision(prec);
        cout << fixed;
    }

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

    if(FitResults.size()>1 && fRecalEnergy->fCalibFunction) {
        if(fCalibCanvas && fCalibCanvas->GetCanvasImp()) {
            fCalibCanvas->cd();
        }
        else {
            if(fCalibCanvas) {
                for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
                    if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("Calibration Results")) {
                        gROOT->GetListOfCanvases()->RemoveAt(i);
                        i--;
                    }
                }
            }
            fCalibCanvas = new TCanvas("CalibrationResults","Calibration Results");
            fCalibCanvas->Divide(1,2,0.0001,0.0001);
        }
        fCalibCanvas->cd(1);

        fRecalEnergy->fCalibGraph->Draw("ape");
        fRecalEnergy->fCalibFunction->Draw("same");
        if(fRecalEnergy->fCalibConfidenceIntervall) fRecalEnergy->fCalibConfidenceIntervall->Draw("e3 same");

        fCalibCanvas->cd(2);
        fRecalEnergy->fResidueGraph->Draw("ape");

        gErrorIgnoreLevel=kFatal;
        fCalibCanvas->Update();
        fCalibCanvas->Modified();
        gErrorIgnoreLevel=kPrint;

        fCalibCanvas->RaiseWindow();
    }

    fMainWindow->GetCanvas()->cd();
}


void CXHist1DCalib::BuildFWHMGraph()
{
    CleanCalib();

    TH1 *hist = fMainWindow->GetCanvas()->FindHisto(fMainWindow->GetCanvas());

    bool ok = CheckFitProperties(hist);

    if(!ok) {
        gbash_color->WarningMessage("No histogram found for energy calibration, ignored");
        return;
    }

    if(!fEnergies.size()) {
        gbash_color->WarningMessage("No source with energies defined, ignored");
        return;
    }

    fRecalEnergy->StartCalib();

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
        if(fFWHMCanvas && fFWHMCanvas->GetCanvasImp()) fFWHMCanvas->cd();
        else {
            if(fFWHMCanvas) {
                for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
                    if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("FWHM fit")) {
                        gROOT->GetListOfCanvases()->RemoveAt(i);
                        i--;
                    }
                }
            }
            fFWHMCanvas = new TCanvas("FWHM","FWHM fit");
        }
        fRecalEnergy->fFWHMGraph->Draw("ape");

        fFWHMCanvas->Update();
        fFWHMCanvas->Modified();
        fFWHMCanvas->RaiseWindow();
    }

    fMainWindow->GetCanvas()->cd();
}

void CXHist1DCalib::FWHMCalib()
{
    fFWHMGraph = fMainWindow->GetGraph();
    if(!fFWHMGraph) {
        BuildFWHMGraph();
        if(fFWHMCanvas && fFWHMCanvas->GetCanvasImp())
            fFWHMGraph = fMainWindow->GetGraph(fFWHMCanvas,1);
    }
    if(fFWHMGraph == nullptr || fFWHMGraph->GetN()<2) {
        gbash_color->WarningMessage("FWHM graph not existing of not containing enough points");
        return;
    }
    double xmin,xmax;
    if(!fFWHMFunction) {
        xmin = 0.;
        xmax = fFWHMGraph->GetX()[fFWHMGraph->GetN()-1]*1.5;
        delete fFWHMFunction;
        fFWHMFunction = new TF1("FWHMFunc",&CXFitFunctions::FWHMFunction,xmin,xmax,3);
        fFWHMFunction->SetLineColor(kBlue);
        fFWHMFunction->SetNpx(5000);
        fFWHMFunction->SetParNames("F","G","H");
        fFWHMFunction->SetParameters(2.,1.,0.);
        fFWHMFunction->SetParLimits(0,0,100);
        fFWHMFunction->SetParLimits(1,0,10);
        fFWHMFunction->SetParLimits(2,0,5);
    }
    xmin = 0.;
    xmax = fFWHMGraph->GetX()[fFWHMGraph->GetN()-1]*1.5;
    fFWHMFunction->SetRange(xmin,xmax);

    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Migrad");
    ROOT::Math::MinimizerOptions::SetDefaultMaxIterations(1000000);
    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(1000000);

    TFitResultPtr r = fFWHMGraph->Fit(fFWHMFunction,"S0R");

    if(!r->IsValid()) {
        gbash_color->WarningMessage("Warning: Fit failed");
        delete gROOT->FindObject("FWHMConfidence95");
        fFWHMConfidenceIntervall = nullptr;
    }
    else {
        delete gROOT->FindObjectAny("FWHMConfidence95");
        fFWHMConfidenceIntervall = new TH1D("FWHMConfidence95","FWHM 0.95 confidence band", 10000, 0, 10000);
        (TVirtualFitter::GetFitter())->GetConfidenceIntervals(fFWHMConfidenceIntervall);
        fFWHMConfidenceIntervall->SetLineWidth(0);
        fFWHMConfidenceIntervall->SetFillColor(kBlue);
        if(gPad->GetCanvas()->SupportAlpha()) {
            fFWHMConfidenceIntervall->SetFillColorAlpha(kBlue,0.1);
            fFWHMConfidenceIntervall->SetFillStyle(1001);
        }
        else {
            fFWHMConfidenceIntervall->SetFillStyle(3003);
        }
        fFWHMConfidenceIntervall->SetStats(false);
        fFWHMConfidenceIntervall->SetDirectory(nullptr);
        fFWHMConfidenceIntervall->GetXaxis()->SetTitle(fFWHMGraph->GetXaxis()->GetTitle());
        fFWHMConfidenceIntervall->GetYaxis()->SetTitle(fFWHMGraph->GetYaxis()->GetTitle());
    }

    if(fFWHMCanvas && fFWHMCanvas->GetCanvasImp()) fFWHMCanvas->cd();
    else {
        if(fFWHMCanvas) {
            for(int i=0 ; i<gROOT->GetListOfCanvases()->GetEntries() ; i++) {
                if(((TString)gROOT->GetListOfCanvases()->At(i)->GetTitle()).EqualTo("FWHM fit")) {
                    gROOT->GetListOfCanvases()->RemoveAt(i);
                    i--;
                }
            }
        }
        fFWHMCanvas = new TCanvas("FWHM","FWHM fit");
    }
    fFWHMGraph->Draw("ape");

    if(fFWHMConfidenceIntervall) fFWHMConfidenceIntervall->Draw("e3 same");
    fFWHMFunction->Draw("same");

    fFWHMCanvas->Update();
    fFWHMCanvas->Modified();
    fFWHMCanvas->RaiseWindow();
}


void CXHist1DCalib::ApplyCalibration(TH1 *_hist, TF1 *_func)
{
    if(!_hist) fMainWindow->GetCanvas()->cd();

    if(!_hist) _hist = fMainWindow->GetCanvas()->FindHisto(fMainWindow->GetCanvas());
    if(!_hist || _hist->GetDimension()>1) {
        gbash_color->WarningMessage("No 1D histogram found to apply calibration");
        return;
    }
    if(!_func) _func = fRecalEnergy->fCalibFunction;
    if(!_func) {
        gbash_color->WarningMessage("No calibration processed, first use: Energy Calibration");
        return;
    }

    auto xmin = 0.;
    auto xmax = (int)_func->Eval(_hist->GetXaxis()->GetXmax());
    int nbins = ((int)(_hist->GetNbinsX()/xmax+0.5))*xmax;

    TH1D *calibrated_histo = (TH1D*) _hist->Clone();
    calibrated_histo->Reset();
    calibrated_histo->SetBins(nbins,xmin,xmax);
    calibrated_histo->SetNameTitle(Form("%s_calib",_hist->GetName()),Form("%s calibrated",_hist->GetTitle()));
    calibrated_histo->GetXaxis()->SetTitle("Energy (keV)");

    TRandom3 *r = new TRandom3(0);
    for(int i=1 ; i<=nbins ; i++) {
        auto N = _hist->GetBinContent(i);
        auto width = _hist->GetBinWidth(i);
        auto val = _hist->GetBinLowEdge(i);
        for(Long64_t j=0 ; j<N ; j++) {
            auto cal = _func->Eval(val+r->Rndm()*width);
            calibrated_histo->Fill(cal);
        }
    }

    calibrated_histo->Draw(_hist->GetDrawOption());
    gPad->Modified();
    gPad->Update();
}

void CXHist1DCalib::CloseCanvas() {
    cout << "close canvas "<< endl;
}

double CXHist1DCalib::DinoFct(double*xx,double*pp)
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

TF1 *CXHist1DCalib::GetDinoFct(TString Name,double min, double max, int Npar)
{
    TF1 *f = new TF1(Name,this,&CXHist1DCalib::DinoFct,min,max,Npar,"CXHist1DCalib","DinoFct");

    return f;
}

void CXHist1DCalib::SaveECal()
{
    TString WSName="";
    CXDialogBox *diag = new CXDialogBox(this->GetMainFrame(),"Save in workspace:");
    diag->Add("Workspace name",WSName);
    diag->Popup();

    if(WSName=="") {
        glog << info << "Energy calibration saving aborted" << do_endl;
        return;
    }

    CXWorkspace *workspace = fMainWindow->GetWSManager()->GetWorkspace(WSName);

    if(!workspace) {
        glog << error << "No Workspace named: " << WSName << do_endl;
        return;
    }

    workspace->SetCalibration(fRecalEnergy->fCalibGraph,fRecalEnergy->fCalibFunction,fRecalEnergy->fCalibConfidenceIntervall,fRecalEnergy->fResidueGraph);
}

void CXHist1DCalib::SaveFWHM()
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

    workspace->SetFWHM(fFWHMGraph,fFWHMFunction,fFWHMConfidenceIntervall);
}

ClassImp(CXHist1DCalib)
