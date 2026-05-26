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

#include "CXFit.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "TMath.h"
#include "TF1.h"
#include "TH1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TGNumberEntry.h"
#include "TGButton.h"
#include "Math/MinimizerOptions.h"

#include "CXHist1DPlayer.h"
#include "CXArrow.h"
#include "CXMainWindow.h"
#include "CXWSManager.h"

#include "CXBashColor.h"

using namespace std;

CXFit::CXFit(TH1 *hist, TVirtualPad *pad, CXHist1DPlayer *player, CXWorkspace *_workspace)
    : fPlayer(player), fPad(pad), fHistogram(hist), fWorkspace(_workspace) {
    fListOfArrows = new TList();
    fListOfArrows->SetOwner();

    fListOfPeaks = new TList();
    fListOfPeaks->SetOwner();
}

CXFit::CXFit(const CXFit &other): TObject(other), fEnergies(other.fEnergies), fBackgd(other.fBackgd), fFixedMean(other.fFixedMean), fBindFWHM(other.fBindFWHM)
{
    fListOfArrows = new TList();
    if (other.fListOfArrows) {
        for (TObject *obj : *(other.fListOfArrows)) {
            CXArrow *arr = dynamic_cast<CXArrow*>(obj->Clone());
            arr->SetFit(this);
            fListOfArrows->Add(arr);
        }
    }
    fListOfPeaks = new TList();
    fListOfPeaks->SetOwner();
}

CXFit& CXFit::operator=(const CXFit &other) {
    if (this == &other) return *this;  // Handle self-assignment

    TObject::operator=(other);
    fPad = other.fPad;
    fPlayer = other.fPlayer;
    fWorkspace = other.fWorkspace;

    fEnergies = other.fEnergies;
    fBackgd = other.fBackgd;
    fFixedMean = other.fFixedMean;
    fBindFWHM = other.fBindFWHM;

    // Delete existing objects before assigning new ones
    delete fHistogram;
    delete fFitFunction;
    delete fBackFunction;
    delete fResidue;
    delete fListOfArrows;
    delete fListOfPeaks;

    fHistogram = nullptr;
    fFitFunction = nullptr;
    fBackFunction = nullptr;
    fResidue = nullptr;

    fListOfArrows = new TList();
    if (other.fListOfArrows) {
        for (TObject *obj : *(other.fListOfArrows)) {
            fListOfArrows->Add(obj->Clone());
        }
    }

    fListOfPeaks = new TList();
    if (other.fListOfPeaks) {
        for (TObject *obj : *(other.fListOfPeaks)) {
            fListOfPeaks->Add(obj->Clone());
        }
    }

    fsavedStream.str();

    return *this;
}

TObject* CXFit::Clone(const char* newname) const {
    return new CXFit(*this);
}

CXFit::~CXFit()
{
    if(fPad) Clear(fPad);

    delete fListOfArrows;

    delete fFitFunction;
    delete fBackFunction;
    delete fResidue;
    delete fListOfPeaks;

    if(fPlayer) fPlayer->GetMainWindow()->RefreshPads();
}

void CXFit::UpdateFit(TH1 *hist, TVirtualPad *pad, CXHist1DPlayer *player, CXWorkspace *_workspace)
{
    fHistogram = hist;
    fPad = pad;
    fPlayer = player;

    fWorkspace = _workspace;

    DrawArrows();
    Update();
}


void CXFit::AddArrow(Double_t Energy)
{
    Int_t Bin = fHistogram->FindBin(Energy);
    Double_t Value = fHistogram->GetBinContent(Bin);

    if(fListOfArrows->GetEntries()==1) {
        for(int i=fHistogram->FindBin(Energy)-2 ; i<=fHistogram->FindBin(Energy)+2 ; i++){
            if(i>0 && fHistogram->GetBinContent(i)>Value){
                Value = fHistogram->GetBinContent(i);
                Bin = i;
            }
        }
    }

    Energy = fHistogram->GetBinCenter(Bin);
    Double_t MaxGlob = fHistogram->GetMaximum();
    auto *arrow = new CXArrow(this,Energy,(Value + MaxGlob/100.) ,(Value +MaxGlob/15.),0.01,0.03,"<|");
    arrow->SetAngle(30);
    arrow->SetLineWidth(2);
    arrow->Draw();

    fListOfArrows->Add(arrow);
    Update();
}

void CXFit::RemoveArrow(CXArrow *arrow)
{
    if(arrow == nullptr) {
        fListOfArrows->RemoveLast();
        fPad->GetListOfPrimitives()->RemoveLast();
    }
    else {
        fListOfArrows->Remove(arrow);
        fPad->GetListOfPrimitives()->Remove(arrow);
    }

    Update();
}

void CXFit::DrawArrows()
{
    for(int i=0 ; i<fListOfArrows->GetEntries() ; i++) {
        auto *arr = dynamic_cast<CXArrow*>(fListOfArrows->At(i));
        arr->Draw();
    }
}

void CXFit::Update()
{
    fEnergies.clear();
    fBackgd.clear();
    fFixedMean.clear();

    fListOfArrows->Sort();
    TList back,Ener;

    if(!fPlayer->DoNewFit && fListOfArrows->GetEntries()<3) {
        Clear(fPad);
        gbash_color->WarningMessage("At least one peak and two background are needed for a fit --> command ignored");
        return;
    }

    for(int i=0 ; i<fListOfArrows->GetEntries() ; i++) {
        auto *arr = dynamic_cast<CXArrow*>(fListOfArrows->At(i));
        if(i==0) back.Add(arr);
        else if(i==fListOfArrows->GetEntries()-1) back.Add(arr);
        else Ener.Add(arr);
    }

    for(int i=0 ; i<back.GetEntries() ; i++) {
        auto *arr = dynamic_cast<CXArrow*>(back.At(i));
        Double_t E = arr->GetEnergy();
        Double_t MaxGlob = fHistogram->GetMaximum();
        Double_t Value = fHistogram->GetBinContent(fHistogram->FindBin(E));
        arr->Set(E,(Value + MaxGlob/100.), (Value+MaxGlob/15.));
        arr->SetLineColor(kBlue);
        arr->SetFillColor(kBlue);
        fBackgd.push_back(E);
    }
    for(int i=0 ; i<Ener.GetEntries() ; i++) {
        auto *arr = dynamic_cast<CXArrow*>(Ener.At(i));
        Double_t E = arr->GetEnergy();
        Double_t MaxGlob = fHistogram->GetMaximum();
        Double_t Value = fHistogram->GetBinContent(fHistogram->FindBin(E));
        arr->Set(E,(Value + MaxGlob/100.), (Value+MaxGlob/15.));
        arr->SetLineColor(kRed);
        arr->SetFillColor(kRed);
        fEnergies.push_back(E);
        fFixedMean.push_back(arr->GetMeanFixed());
    }

    fPlayer->GetMainWindow()->RefreshPads();
}

void CXFit::Clear(TVirtualPad *pad)
{
    if(fPlayer) fPlayer->EndFit();

    if(fPad==nullptr) {
        gbash_color->WarningMessage("No selected pad, ignored");
        return;
    }
    if(pad == nullptr) pad = fPad;

    TList *list = pad->GetListOfPrimitives();

    list->Remove(fFitFunction);
    list->Remove(fBackFunction);
    list->Remove(fResidue);


    for(int i=0 ; i<fListOfPeaks->GetEntries() ; i++)
        list->Remove(fListOfPeaks->At(i));

    for(int i=0 ; i<fListOfArrows->GetEntries() ; i++)
        list->Remove(fListOfArrows->At(i));

    fPlayer->RemoveFit(this);

    fPlayer->GetMainWindow()->RefreshPads();
}

void CXFit::Fit()
{
    if(fListOfArrows->GetEntries()<3) {
        return;
    }

    if(fPad==nullptr) {
        gbash_color->WarningMessage("No selected pad, ignored");
        return;
    }
    fPad->cd();

    if(fPlayer == nullptr) {
        gbash_color->WarningMessage("1DPlayer not defined, ignored");
        return;
    }

    if(fHistogram==nullptr || fHistogram->InheritsFrom("TH2")) {
        gbash_color->WarningMessage("No 1D histogram found, ignored");
        return;
    }

    fsavedStream.str("");

    ROOT::Math::MinimizerOptions::SetDefaultMinimizer(fPlayer->GetMinimizer(),fPlayer->GetAlgorithm());
    ROOT::Math::MinimizerOptions::SetDefaultTolerance(fPlayer->GetTolerance());
    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(fPlayer->GetPrintLevel());

    //    fPlayer->GetFitResultsBox()->RemoveAll();

    Double_t DefFWHM = fPlayer->fNE_FWHM[0]->GetNumber();
    Double_t DefFWHM_min = fPlayer->fNE_FWHM[1]->GetNumber();
    Double_t DefFWHM_max = fPlayer->fNE_FWHM[2]->GetNumber();

    Double_t LeftTailVal = fPlayer->fNE_LT[0]->GetNumber();;
    Double_t LeftTailValMin = fPlayer->fNE_LT[1]->GetNumber();;
    Double_t LeftTailValMax = fPlayer->fNE_LT[2]->GetNumber();;

    Double_t RightTailVal = fPlayer->fNE_RT[0]->GetNumber();;
    Double_t RightTailValMin = fPlayer->fNE_RT[1]->GetNumber();;
    Double_t RightTailValMax = fPlayer->fNE_RT[2]->GetNumber();;

    Double_t StepVal = 0.01;
    Double_t StepValMin = 0.;
    Double_t StepValMax = 1.;

    Int_t NPars = 4+6*fEnergies.size();

    delete fFitFunction;
    fFitFunction = new TF1("MyFit", [this](double* x, double* p) -> double { return this->DoubleTailedStepedGaussian(x, p); }, fBackgd[0], fBackgd[1], NPars);

    fFitFunction->SetParName(0, "NumberOfPeaks");
    fFitFunction->SetParName(1, "BkgConst");
    fFitFunction->SetParName(2, "BkgSlope");
    fFitFunction->SetParName(3, "BkgExp");

    for(auto i=0U ; i<fEnergies.size() ; i++) {
        fFitFunction->SetParName(4+i*6+0, Form("Height_%d",i));
        fFitFunction->SetParName(4+i*6+1, Form("Position_%d",i));
        fFitFunction->SetParName(4+i*6+2, Form("FWHM_%d",i));
        fFitFunction->SetParName(4+i*6+3, Form("LeftTail_%d",i));
        fFitFunction->SetParName(4+i*6+4, Form("RightTail_%d",i));
        fFitFunction->SetParName(4+i*6+5, Form("AmplitudeStep_%d",i));
    }

    fFitFunction->SetNpx(1000);
    fFitFunction->SetLineColor(kRed);

    fFitFunction->FixParameter(0, fEnergies.size()); // 1 peak

    Double_t x,y;
    x = fPad->GetUxmin();
    y = fPad->GetUxmax();
    fHistogram->GetXaxis()->SetRangeUser(fBackgd[0],fBackgd[1]);

    //Calc Bckd
    fFitFunction->SetParameter(1, fHistogram->GetBinContent(fHistogram->FindBin(fBackgd[0])));
    fFitFunction->SetParLimits(1, 0.,fHistogram->GetMaximum());

    fFitFunction->SetParameter(2, 0);
    fFitFunction->SetParLimits(2, -50., 0.);
    fFitFunction->SetParameter(3, 0.);
    //    fFitFunction->SetParLimits(3, -50., 0.);

    if(!fPlayer->fUsePol1)
        fFitFunction->FixParameter(2,0);
    if(!fPlayer->fUseExp)
        fFitFunction->FixParameter(3,0);

    for(auto i=0U ; i<fEnergies.size() ; i++) {
        //Position
        fFitFunction->SetParameter(4+i*6+1, fEnergies[i]);
        fFitFunction->SetParLimits(4+i*6+1, fEnergies[i]-DefFWHM, fEnergies[i]+DefFWHM);
        if((fPlayer->fFixMean->GetState() == kButtonDown) || fFixedMean.at(i))
            fFitFunction->FixParameter(4+i*6+1,fEnergies[i]);

        //FWHM
        if(fBindFWHM && i>0) fFitFunction->FixParameter(4+i*6+2,0.);
        else if(fPlayer->fUseFWHM && fWorkspace && fWorkspace->fFWHMFunction && fWorkspace->fFWHMErrors) {
            double FWHM = fWorkspace->fFWHMFunction->Eval(fEnergies[i]);
            if(fPlayer->fFixFWHM->GetState() == kButtonDown) {
                fFitFunction->FixParameter(4+i*6+2,FWHM);
            }
            else {
                double error = fWorkspace->fFWHMErrors->GetBinError(fWorkspace->fFWHMErrors->FindBin(fEnergies[i]));
                double sigma = fPlayer->fFWHMSigma->GetNumber();
                fFitFunction->SetParameter(4+i*6+2, FWHM);
                fFitFunction->SetParLimits(4+i*6+2, std::max(0.,FWHM-error*sigma) , FWHM+error*sigma);
            }
        }
        else {
            fFitFunction->SetParameter(4+i*6+2, DefFWHM);
            fFitFunction->SetParLimits(4+i*6+2, DefFWHM_min, DefFWHM_max);
            if(fPlayer->fFixFWHM->GetState() == kButtonDown)
                fFitFunction->FixParameter(4+i*6+2,DefFWHM);
        }

        //Height
        // find max in E+-FWHM
        int bin1 = fHistogram->FindBin(fEnergies[i]-fFitFunction->GetParameters()[4+0*6+2]);
        int bin2 = fHistogram->FindBin(fEnergies[i]+fFitFunction->GetParameters()[4+0*6+2]);
        double max = fHistogram->GetBinContent(bin1);

        for (int i = bin1 + 1; i <= bin2; ++i) {
            if (fHistogram->GetBinContent(i) > max) {
                max = fHistogram->GetBinContent(i);
            }
        }

        fFitFunction->SetParameter(4+i*6+0, max - (fHistogram->GetBinContent(fHistogram->FindBin(fBackgd[0]))+fHistogram->GetBinContent(fHistogram->FindBin(fBackgd[1])))*0.5 );
        fFitFunction->SetParLimits(4+i*6+0, 0, fFitFunction->GetParameters()[4+i*6+0]*2);

        //LeftTail
        fFitFunction->SetParameter(4+i*6+3, LeftTailVal);
        fFitFunction->SetParLimits(4+i*6+3, LeftTailValMin, LeftTailValMax);
        if(fPlayer->fUseLT->GetState() == kButtonUp)
            fFitFunction->FixParameter(4+i*6+3,-5);
        else if(fPlayer->fFixLT->GetState() == kButtonDown)
            fFitFunction->FixParameter(4+i*6+3,LeftTailVal);

        //RightTail
        fFitFunction->SetParameter(4+i*6+4, RightTailVal);
        fFitFunction->SetParLimits(4+i*6+4, RightTailValMin, RightTailValMax);
        if(fPlayer->fUseRT->GetState() == kButtonUp)
            fFitFunction->FixParameter(4+i*6+4,5);
        else if(fPlayer->fFixRT->GetState() == kButtonDown)
            fFitFunction->FixParameter(4+i*6+4,RightTailVal);

        //AmplitudeStep
        fFitFunction->SetParameter(4+i*6+5, StepVal);
        fFitFunction->SetParLimits(4+i*6+5, StepValMin, StepValMax);
        if(!fPlayer->fUseStep)
            fFitFunction->FixParameter(4+i*6+5,0);
    }

    fHistogram->GetXaxis()->SetRangeUser(x,y);

    TString FitOpt = "R0S";
    if(fPlayer->GetPrintLevel()>0) FitOpt +="V";
    FitOpt += fPlayer->fFitOptions->GetText();
    TFitResultPtr r = fHistogram->Fit(fFitFunction,FitOpt.Data());

    if(fPlayer->fFixAmpli->GetState() == kButtonDown) {

        //Extract Background
        delete fBackFunction;
        fBackFunction = new TF1("Background", [this](double* x, double* p) -> double { return this->StepedBackground(x, p); }, fBackgd[0], fBackgd[1], NPars);
        fBackFunction->SetParameters(fFitFunction->GetParameters());

        for(auto i=0U ; i<fEnergies.size() ; i++) {
            Double_t Ampli = fHistogram->GetBinContent(fHistogram->FindBin(fEnergies[i]))- fBackFunction->Eval(fEnergies[i]);
            fFitFunction->FixParameter(4+i*6+0,Ampli);
        }

        r = fHistogram->Fit(fFitFunction,FitOpt.Data());
    }

    if(fBindFWHM) {
        for(auto i=1U ; i<fEnergies.size() ; i++) {
            fFitFunction->FixParameter(4+i*6+2, fFitFunction->GetParameter(4+0*6+2));
        }
        r = fHistogram->Fit(fFitFunction,FitOpt.Data());
    }

    if(r.Get() == nullptr) {
        gbash_color->WarningMessage("Oups... Error in fitting histogram");
        return;
    }

    //Extract Background
    delete fBackFunction;
    fBackFunction = new TF1("Background", [this](double* x, double* p) -> double { return this->StepedBackground(x, p); }, fBackgd[0], fBackgd[1], NPars);
    fBackFunction->SetParameters(fFitFunction->GetParameters());
    fBackFunction->SetNpx(1000);

    fBackFunction->SetLineColor(kBlue);
    fBackFunction->Draw("same");

    //Extract Residue
    delete fResidue;
    fResidue = new TF1("Residue", [this](double* x, double* p) -> double { return this->Residue(x, p); }, fBackgd[0], fBackgd[1], NPars);
    fResidue->SetParameters(fFitFunction->GetParameters());
    fResidue->SetNpx(1000);

    fResidue->SetLineWidth(1);
    fResidue->SetLineColor(kBlack);
    fResidue->Draw("same");

    ostringstream text;

    text << "Fit results :";
    cout<<text.str()<<endl;fPlayer->PrintInListBox(text.str(),kPrint); fsavedStream << text.str() << endl; text.str("");
    text << "Status: ";
    if(r->IsValid())
        text << " Successeful" << endl;
    else
        text << " Failed" << endl;
    cout<<text.str();fsavedStream << text.str();
    if(r->IsValid())
        fPlayer->PrintInListBox(text.str(),kPrint);
    else
        fPlayer->PrintInListBox(text.str(),kError);
    text.str("");

    text << "Chi2  = "<< r->Chi2();
    cout<<text.str()<<endl;fPlayer->PrintInListBox(text.str(),kInfo); fsavedStream << text.str() << endl; text.str("");
    text << "Ndf   = "<< r->Ndf();
    cout<<text.str()<<endl;fPlayer->PrintInListBox(text.str(),kInfo); fsavedStream << text.str() << endl; text.str("");
    text << "P val = "<< r->Prob();
    cout<<text.str()<<endl;fPlayer->PrintInListBox(text.str(),kInfo); fsavedStream << text.str() << endl; text.str("");

    fListOfPeaks->Clear();

    for(auto i=0U ; i< fEnergies.size() ; i++) {
        text<<"Peak "<<i<<":";
        cout<<text.str()<<endl;fPlayer->PrintInListBox(text.str(),kPrint); fsavedStream << text.str() << endl; text.str("");

        TF1 *peak = new TF1(Form("Peak%d",i), [this](double* x, double* p) -> double { return this->PeakFunction(x, p); }, fBackgd[0], fBackgd[1], NPars);
        peak->SetParameters(r->GetParams());
        peak->SetParErrors(r->GetErrors());
        peak->SetParameter(1,1);//with backgroud
        peak->SetParameter(0,i);
        peak->SetNpx(1000);

        peak->SetLineColor(kGreen);
        peak->SetLineStyle(kDashed);
        peak->Draw("same");

        fListOfPeaks->Add(peak);

        // old way to calculate the area error -> Integral from TF1 and ∆N=2*sqrt(N)
        // Double_t Area     = (peak->Integral(fBackgd[0],fBackgd[1],1e-6)-fBackFunction->Integral(fBackgd[0],fBackgd[1],1e-6))/fHistogram->GetBinWidth(1);
        // Double_t AreaErr  = 2*sqrt(Area);

        Double_t Mean     = peak->GetParameter(4+i*6+1);
        Double_t MeanErr  = peak->GetParError(4+i*6+1);
        Double_t FWHM     = peak->GetParameter(4+i*6+2);
        Double_t FWHMErr  = peak->GetParError(4+i*6+2);
        //        Double_t LeftT    = TMath::Abs(peak->GetParameter(4+i*6+3));
        //        Double_t LeftTErr = peak->GetParError(4+i*6+3);
        //        Double_t Right    = peak->GetParameter(4+i*6+4);
        //        Double_t RightErr = peak->GetParError(4+i*6+4);


        // calculation of the integrat and its uncertainty from the analytical analysis of the function (inspired from Dino method in RecalEnergy code)
        const double  sqrt2pi = sqrt(8.*atan(1.));
        double area_tail = 0;
        if(fPlayer->fUseLT->GetState() == kButtonDown) {
            double L = TMath::Abs(peak->GetParameter(4+i*6+3));
            double a = exp(-0.5*L*L)/L;
            double b = sqrt2pi/2*std::erf(L/sqrt(2.));
            area_tail += a + b;
        }
        else {
            area_tail += sqrt2pi/2;
        }
        if(fPlayer->fUseRT->GetState() == kButtonDown) {
            double R = TMath::Abs(peak->GetParameter(4+i*6+4));
            double a = exp(-0.5*R*R)/R;
            double b = sqrt2pi/2*std::erf(R/sqrt(2.));
            area_tail += a + b;
        }
        else {
            area_tail += sqrt2pi/2;
        }
        double area_gauss = peak->GetParameter(4+i*6+0) * peak->GetParameter(4+i*6+2)*1./sqrt(8.*log(2.)) / fHistogram->GetBinWidth(1);
        double Area_calc = area_tail*area_gauss;

        double Area_peak_calcErr = area_gauss*sqrt(
                             pow(peak->GetParError(4+i*6+0)/peak->GetParameter(4+i*6+0),2.) +
                             pow(peak->GetParError(4+i*6+2)/peak->GetParameter(4+i*6+2),2.));

        Area_peak_calcErr *= Area_calc/area_gauss; // error with tail only rescaled to the proportion of the peak area relatively to the gaus area

        Double_t Area_eff = 0.;
        Double_t Area_eff_err = 0.;
        if(fWorkspace && fWorkspace->fEfficiencyFunction) {
            double eff = fWorkspace->fEfficiencyFunction->Eval(Mean);
            Area_eff = Area_calc / eff;
            double error = 0.;
            if(fWorkspace->fEfficiencyErrors) error = fWorkspace->fEfficiencyErrors->GetBinError(fWorkspace->fEfficiencyErrors->FindBin(Mean));
            Area_eff_err = Area_eff * sqrt(Area_peak_calcErr*Area_peak_calcErr/(Area_calc*Area_calc) + error*error/(eff*eff));
        }

        peak->SetParameter(1,0);//without backgroud
        Double_t Max      = peak->GetParameter(4+i*6+0);
        Double_t MaxErr   = peak->GetParError(4+i*6+0);

        Double_t FWHM_L     = peak->GetX(Max/2,fBackgd[0],Mean,1e-6);
        Double_t FWHM_L_err = peak->GetX((Max-MaxErr)/2,fBackgd[0],Mean,1e-6);

        Double_t F01_L     = Mean-peak->GetX(Max/10.,fBackgd[0],Mean,1e-6);
        Double_t F01_R     = peak->GetX(Max/10.,Mean,fBackgd[1],1e-6)-Mean;

        Double_t FWHM_R     = peak->GetX(Max/2,Mean,fBackgd[1],1e-6);
        Double_t FWHM_R_err = peak->GetX((Max-MaxErr)/2,Mean, fBackgd[1],1e-6);


        Double_t LeftTailParam = F01_L/(FWHM*0.5);
        Double_t RightTailParam = F01_R/(FWHM*0.5);

        Double_t FWHM_Real     = FWHM_R-FWHM_L;
        Double_t FWHM_Real_err = (FWHM_R_err-FWHM_L_err)-FWHM_Real;
        peak->SetParameter(1,1);//with backgroud

        text<<left<<setw(11)<<"Mean"<<": "<<setprecision(7)<<setw(10)<<Mean<<" ("<<setprecision(7)<<setw(10)<<MeanErr<<")";
        cout<<text.str()<<endl;fsavedStream << text.str() << endl;
        fPlayer->PrintInListBox(text.str(),kInfo);
        text.str("");

        text<<left<<setw(11)<<"Amplitude"<<": "<<setprecision(7)<<setw(10)<<Max<<" ("<<setprecision(7)<<setw(10)<<MaxErr<<")";
        cout<<text.str()<<endl;fsavedStream << text.str() << endl;
        fPlayer->PrintInListBox(text.str(),kInfo);
        text.str("");

        text<<left<<setw(11)<<"FWHM (gaus)"<<": "<<setprecision(7)<<setw(10)<<FWHM<<" ("<<setprecision(7)<<setw(10)<<FWHMErr<<")";
        cout<<text.str()<<endl;fsavedStream << text.str() << endl;
        fPlayer->PrintInListBox(text.str(),kInfo);
        text.str("");

        text<<left<<setw(11)<<"FWHM (real)"<<": "<<setprecision(7)<<setw(10)<<FWHM_Real<<" ("<<setprecision(7)<<setw(10)<<FWHM_Real_err<<")";
        cout<<text.str()<<endl;fsavedStream << text.str() << endl;
        fPlayer->PrintInListBox(text.str(),kInfo);
        text.str("");

        text<<left<<setw(11)<<"L Tail"<<": "<<setprecision(7)<<setw(10)<<LeftTailParam;
        cout<<text.str()<<endl;fsavedStream << text.str() << endl;
        fPlayer->PrintInListBox(text.str(),kInfo);
        text.str("");

        text<<left<<setw(11)<<"R Tail"<<": "<<setprecision(7)<<setw(10)<<RightTailParam;
        cout<<text.str()<<endl;fsavedStream << text.str() << endl;
        fPlayer->PrintInListBox(text.str(),kInfo);
        text.str("");

        text<<left<<setw(11)<<"Area"<<": "<<setprecision(7)<<setw(10)<<Area_calc<<" ("<<setprecision(7)<<setw(10)<<Area_peak_calcErr<<")";
        cout<<text.str()<<endl;fsavedStream << text.str() << endl;
        fPlayer->PrintInListBox(text.str(),kInfo);
        text.str("");

        if(Area_eff>0.) {
            text<<left<<setw(11)<<Form("%s area",fWorkspace->GetName())<<": "<<setprecision(7)<<setw(10)<<Area_eff<<" ("<<setprecision(7)<<setw(10)<<Area_eff_err<<")";
            cout<<text.str()<<endl;fsavedStream << text.str() << endl;
            fPlayer->PrintInListBox(text.str(),kInfo);
            text.str("");
        }
    }

    fsavedStream << endl;

    fFitFunction->Draw("same");

    fPlayer->GetMainWindow()->RefreshPads();
}

Double_t CXFit::DoubleTailedStepedGaussian(Double_t*xx,Double_t*pp)
{
    Double_t x   = xx[0];

    int    NSubPeaks = (int)pp[0]; //Number of subpeaks in the peak range

    Double_t f_tot = 0.;

    Int_t Npar = 6;

    Double_t Back_const = pp[1];
    Double_t Back_slope = pp[2];
    Double_t Back_Exp = pp[3];

    f_tot += (Back_const + (x-fBackgd[0])*Back_slope)*exp((x-fBackgd[0])*Back_Exp);

    for(int i=0 ; i<NSubPeaks ; i++) {
        Double_t Ampli     = pp[4+i*Npar+0];
        Double_t Mean      = pp[4+i*Npar+1];
        Double_t Sigma     = pp[4+i*Npar+2]*1./sqrt(8.*log(2.));
        if(fBindFWHM) Sigma     = pp[4+0*Npar+2]*1./sqrt(8.*log(2.));
        Double_t Lambda    = pp[4+i*Npar+3];
        Double_t Rho       = pp[4+i*Npar+4];
        Double_t S         = pp[4+i*Npar+5];

        Double_t U         = (x-Mean)/Sigma;
        Double_t f_g       = Ampli*TMath::Exp(-U*U*0.5);
        Double_t f_lambda  = Ampli*TMath::Exp(-0.5*Lambda*(2.*U-Lambda));
        Double_t f_rho     = Ampli*TMath::Exp(-0.5*Rho*(2.*U-Rho));
        Double_t f_S       = Ampli*S*1./((1+TMath::Exp(U))*(1+TMath::Exp(U)));

        if(U<Lambda) f_tot += f_lambda;
        else if(U>Rho) f_tot += f_rho;
        else f_tot += f_g;

        f_tot += f_S;
    }

    return f_tot;
}

Double_t CXFit::StepedBackground(Double_t*xx,Double_t*pp)
{
    Double_t x   = xx[0];

    Double_t f_tot = 0.;

    int    NSubPeaks = (int)pp[0]; //Number of subpeaks in the peak range

    Double_t Back_const = pp[1];
    Double_t Back_slope = pp[2];
    Double_t Back_Exp = pp[3];

    Int_t Npar = 6;

    f_tot += (Back_const + (x-fBackgd[0])*Back_slope)*exp((x-fBackgd[0])*Back_Exp);

    for(int i=0 ; i<NSubPeaks ; i++) {
        Double_t Ampli = pp[4+i*Npar+0];
        Double_t Mean  = pp[4+i*Npar+1];
        Double_t Sigma = pp[4+i*Npar+2]*1./sqrt(8.*log(2.));
        Double_t S     = pp[4+i*Npar+5];

        f_tot += Ampli*S*1./((1+TMath::Exp((x-Mean)/Sigma))*(1+TMath::Exp((x-Mean)/Sigma)));
    }

    return f_tot;
}


double CXFit::PeakFunction(Double_t*xx,Double_t*pp)
{
    Double_t x   = xx[0];

    int    NSubPeaks = (int)pp[0]; //Number of subpeaks in the peak range

    Double_t f_tot = 0.;

    Int_t Npar = 6;

    Bool_t WithBackground = true;
    if(pp[1]==0)
        WithBackground = false;

    Double_t BackGround = fBackFunction->Eval(x);

    if(WithBackground)
        f_tot += BackGround;

    Double_t Ampli     = pp[4+NSubPeaks*Npar+0];
    Double_t Mean      = pp[4+NSubPeaks*Npar+1];
    Double_t Sigma     = pp[4+NSubPeaks*Npar+2]*1./sqrt(8.*log(2.));
    Double_t Lambda    = pp[4+NSubPeaks*Npar+3];
    Double_t Rho       = pp[4+NSubPeaks*Npar+4];
    Double_t S         = 0;

    Double_t U         = (x-Mean)/Sigma;
    Double_t f_g       = Ampli*TMath::Exp(-U*U*0.5);
    Double_t f_lambda  = Ampli*TMath::Exp(-0.5*Lambda*(2.*U-Lambda));
    Double_t f_rho     = Ampli*TMath::Exp(-0.5*Rho*(2.*U-Rho));
    Double_t f_S       = Ampli*S*1./((1+TMath::Exp(U))*(1+TMath::Exp(U)));

    if(U<Lambda) f_tot += f_lambda;
    else if(U>Rho) f_tot += f_rho;
    else f_tot += f_g;

    f_tot += f_S;

    return f_tot;
}

Double_t CXFit::Residue(Double_t*xx,Double_t*/*pp*/)
{
    Double_t x   = xx[0];

    return fFitFunction->Eval(fHistogram->GetBinCenter(fHistogram->FindBin(x))) - fHistogram->GetBinContent(fHistogram->FindBin(x));
}

TString CXFit::Save()
{
    return fsavedStream.str();
}

void CXFit::BindFWHM(Bool_t on)
{
    fBindFWHM = on;
    TIter next(fListOfArrows);
    CXArrow* arrow = nullptr;
    while ((arrow = (CXArrow*)next())) {
        arrow->SetBindFWHM(on);
    }
}

