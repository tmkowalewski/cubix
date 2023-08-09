#include "CXBgdFit.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "TMath.h"
#include "TF1.h"
#include "TH1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TPad.h"
#include "TGNumberEntry.h"
#include "TGButton.h"
#include "Fit/Fitter.h"

#include "CXHist1DPlayer.h"
#include "CXArrow.h"
#include "CXMainWindow.h"
#include "CXBashColor.h"

using namespace std;

CXBgdFit::CXBgdFit(TH1 *hist, TVirtualPad *pad, CXHist1DPlayer *player) : TObject()
{
    fHistogram = hist;
    fPad = pad;
    fPlayer = player;

    fListOfArrows = new TList;
    fListOfArrows->SetOwner();
}

CXBgdFit::~CXBgdFit()
{
    Clear(fPad);

    delete fListOfArrows;

    delete fBackFunction;

    fPlayer->GetMainWindow()->RefreshPads();
}

void CXBgdFit::AddArrow(Double_t Energy)
{
    Int_t Bin = fHistogram->FindBin(Energy);
    Double_t Value = fHistogram->GetBinContent(Bin);

    for(int i=fHistogram->FindBin(Energy)-2 ; i<=fHistogram->FindBin(Energy)+2 ; i++){
        if(i>0 && fHistogram->GetBinContent(i)>Value){
            Value = fHistogram->GetBinContent(i);
            Bin = i;
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

void CXBgdFit::RemoveArrow(CXArrow *arrow)
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

void CXBgdFit::Update()
{
    fBackgd.clear();

    fListOfArrows->Sort();
    TList back,Ener;

    if(!fPlayer->DoNewBgdFit && (fListOfArrows->GetEntries()%2)==1) {
        Clear(fPad);
        gbash_color->WarningMessage("Sets of two ranges are needed for a bgd fit --> command ignored");
        return;
    }

    for(int i=0 ; i<fListOfArrows->GetEntries() ; i++) {
        auto *arr = dynamic_cast<CXArrow*>(fListOfArrows->At(i));
        back.Add(arr);
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

    fPlayer->GetMainWindow()->RefreshPads();
}

void CXBgdFit::Clear(TVirtualPad *pad)
{
    fPlayer->EndFit();

    if(fPad==nullptr) {
        gbash_color->WarningMessage("No selected pad, ignored");
        return;
    }
    if(pad == nullptr) pad = fPad;

    TList *list = pad->GetListOfPrimitives();

    list->Remove(fBackFunction);

    for(int i=0 ; i<fListOfArrows->GetEntries() ; i++)
        list->Remove(fListOfArrows->At(i));

    fPlayer->RemoveBgdFit(this);
    fPlayer->GetMainWindow()->RefreshPads();
}

void CXBgdFit::Fit()
{
    if(fListOfArrows->GetEntries()<4 || (fListOfArrows->GetEntries()%2) !=0) {
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

    ROOT::Math::MinimizerOptions::SetDefaultMinimizer(fPlayer->GetMinimizer(),fPlayer->GetAlgorithm());
    ROOT::Math::MinimizerOptions::SetDefaultTolerance(fPlayer->GetTolerance());
    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(fPlayer->GetPrintLevel());

    Int_t NPars = 3;

    delete fBackFunction;
    fBackFunction = new TF1("MyFit", this, &CXBgdFit::FuncBackground, fBackgd.front(), fBackgd.back(), NPars, "CXBgdFit", "FuncBackground");

    fBackFunction->SetParName(0, "BkgConst");
    fBackFunction->SetParName(1, "BkgSlope");
    fBackFunction->SetParName(2, "BkgExp");

    fBackFunction->SetNpx(1000);
    fBackFunction->SetLineColor(kRed);

    // Copy only the ranges contains within the arrows
    TH1 *HistoToFit = dynamic_cast<TH1*>(fHistogram->Clone());
    HistoToFit->Reset();

    for(size_t i=0; i<fBackgd.size() ; i+=2) {
        Int_t binmin = fHistogram->GetXaxis()->FindBin(fBackgd.at(i));
        Int_t binmax = fHistogram->GetXaxis()->FindBin(fBackgd.at(i+1));

        for(int ibin=binmin ; ibin<=binmax ; ibin++) {
            HistoToFit->SetBinContent(ibin,fHistogram->GetBinContent(ibin));
            HistoToFit->SetBinError(ibin,fHistogram->GetBinError(ibin));
        }
    }

    HistoToFit->GetXaxis()->SetRangeUser(fBackgd.front(), fBackgd.back());

    //Calc Bckd
    fBackFunction->SetParameter(0, HistoToFit->GetBinContent(HistoToFit->FindBin(fBackgd.front())));
    fBackFunction->SetParLimits(0, 0.,HistoToFit->GetMaximum());

    fBackFunction->SetParameter(1, 0);
    fBackFunction->SetParLimits(1, -50., 0.);
    fBackFunction->SetParameter(2, 0.);
//    fBackFunction->SetParLimits(2, -1., 0.);

    if(!fPlayer->fUseBgdPol1)
        fBackFunction->FixParameter(1,0);
    if(!fPlayer->fUseBgdExp)
        fBackFunction->FixParameter(2,0);

    TString FitOpt = "R0S";
    if(fPlayer->GetPrintLevel()>0) FitOpt +="V";
    TFitResultPtr r = HistoToFit->Fit(fBackFunction,FitOpt.Data(),FitOpt.Data());
    ostringstream text;

    cout<<r<<endl;
    if(r==-1) {
        gbash_color->WarningMessage("Oups... Error in fitting histogram");
        return;
    }

    text << "Fit results :";
    cout<<text.str()<<endl;fPlayer->PrintInListBox(text.str(),kPrint);text.str("");
    text << "Status: ";
    if(r->Status()==0)
        text << " Successeful" << endl;
    else
        text << " Failed" << endl;
    cout<<text.str();
    if(r->Status()==0)
        fPlayer->PrintInListBox(text.str(),kPrint);
    else
        fPlayer->PrintInListBox(text.str(),kError);
    text.str("");

    Float_t Area = fHistogram->Integral(fHistogram->GetXaxis()->FindBin(fBackgd.front()),fHistogram->GetXaxis()->FindBin(fBackgd.back()));
    Float_t BgdArea = fBackFunction->Integral(fBackgd.front(),fBackgd.back(),1e-6);
    Float_t CorrArea = Area-BgdArea;

    text<<left<<setw(11)<<"Integral"<<": "<<setprecision(7)<<setw(10)<<Area<<" ("<<setprecision(7)<<setw(10)<<sqrt(Area)<<")";
    cout<<text.str()<<endl;
    fPlayer->PrintInListBox(text.str(),kInfo);
    text.str("");

    text<<left<<setw(11)<<"Bgd Area"<<": "<<setprecision(7)<<setw(10)<<BgdArea<<" ("<<setprecision(7)<<setw(10)<<sqrt(BgdArea)<<")";
    cout<<text.str()<<endl;
    fPlayer->PrintInListBox(text.str(),kInfo);
    text.str("");

    text<<left<<setw(11)<<"Peak Area"<<": "<<setprecision(7)<<setw(10)<<CorrArea<<" ("<<setprecision(7)<<setw(10)<<sqrt(CorrArea)<<")";
    cout<<text.str()<<endl;
    fPlayer->PrintInListBox(text.str(),kInfo);
    text.str("");

    fBackFunction->Draw("same");

    fPlayer->GetMainWindow()->RefreshPads();

    delete HistoToFit;
}

Double_t CXBgdFit::FuncBackground(Double_t*xx,Double_t*pp)
{
    Double_t x   = xx[0];


    Double_t Back_const = pp[0];
    Double_t Back_slope = pp[1];
    Double_t Back_Exp = pp[2];

    Double_t f_tot = (Back_const + (x-fBackgd.front())*Back_slope)*exp((x-fBackgd.front())*Back_Exp);

    return f_tot;
}
