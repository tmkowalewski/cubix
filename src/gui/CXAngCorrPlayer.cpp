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

#include <iostream>
#include <iomanip>

#include "TGNumberEntry.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGSlider.h"
#include "TFrame.h"
#include "TROOT.h"
#include "TObjArray.h"
#include "TF1.h"
#include "TMath.h"
#include "TSystemDirectory.h"
#include "TRandom3.h"
#include "Math/MinimizerOptions.h"
#include "TFitResult.h"
#include "TVirtualFitter.h"
#include "TGraphErrors.h"
#include "TMarker.h"

#include "CXBashColor.h"
#include "CXMainWindow.h"
#include "CXFitFunctions.h"

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

    fGroupFrame = new TGGroupFrame(MotherFrame, "Fit distribution", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));


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
    fSlider[0]->SetRange(-1000,1000);
    fSlider[0]->SetPosition(0);
    fHorizontalFrame->AddFrame(fSlider[0], new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    fNEMixing[0] = new TGNumberEntry(fHorizontalFrame, 0., 5, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax,-1,1);
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
    fSlider[1]->SetRange(-1000,1000);
    fSlider[1]->SetPosition(0);
    fHorizontalFrame->AddFrame(fSlider[1], new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,5,0,0));

    fNEMixing[1] = new TGNumberEntry(fHorizontalFrame, 0., 5, 1, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax,-1,1);
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
    fFixMixing[0]->SetState(kButtonUp);
    fHorizontalFrame->AddFrame(fFixMixing[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft ,1,3,0,0));

    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Fix mix 2/3: "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,10,5,0,0));
    fFixMixing[1] = new TGCheckButton(fHorizontalFrame);
    fFixMixing[1]->SetState(kButtonDown);
    fHorizontalFrame->AddFrame(fFixMixing[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft ,1,3,0,0));

    button = new TGTextButton(fHorizontalFrame, "  Plot  ");
    button->Connect("Clicked()", "CXAngCorrPlayer", this, "PlotMixingEvaluation()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,5,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

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

    fTheoreticalDistribution = GetElement<TF1>(fAngCorrPads[0]);
    fA2A4MixingGraph = GetElement<TGraph>(fAngCorrPads[1],"A2A4MixingGraph");
    fA2A4MixingTheoMarker = GetElement<TGraph>(fAngCorrPads[1],"A2A4MixingTheoMarker");
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
    if (frm->IsA()->InheritsFrom(TGSlider::Class()) /*&& !fFreezemixing*/) {
        TGSlider *sl = (TGSlider*) frm;
        id = sl->WidgetId();
        double value = _pos/1000.;
        fNEMixing[id]->SetNumber(value);
    }
    if (frm->IsA()->InheritsFrom(TGNumberEntry::Class())) {
        TGNumberEntry *ne = (TGNumberEntry*) frm;
        id = ne->WidgetId();
        int position = TMath::Nint(ne->GetNumber()*1000.);
        fSlider[id]->SetPosition(position);
    }
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
    fTheoreticalDistribution = new TF1(Form("%s_Theo",fMainWindow->GetCanvas()->GetName()), this, &CXAngCorrPlayer::TheoreticalAngCorrFunction, 0, 180, 6, "CXAngCorrPlayer", "TheoreticalAngCorrFunction");
    fTheoreticalDistribution->SetParameters(1,TwoJ1,TwoJ2,TwoJ3,mix12,mix23);

    fTheoreticalDistribution->SetNpx(5000);
    fTheoreticalDistribution->SetLineColor(kRed);
    fTheoreticalDistribution->GetXaxis()->SetTitle("#theta (degree)");
    fTheoreticalDistribution->GetYaxis()->SetTitle("W(#theta)");
    fTheoreticalDistribution->GetYaxis()->SetTitleOffset(0.80);
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
    }
}

// Thesis Marcelo Barbosa, p 40
// https://cds.cern.ch/record/1641449/files/CERN-THESIS-2010-294.pdf?subformat=pdfa
// Fk(LL′IiI)
double CXAngCorrPlayer::Fk(int twoL1, int twoL1p, int twoIi, int twoI, int k)
{
    double tot = pow(-1.,(twoIi+twoI-2)/2)*
                 sqrt((twoL1+1)*(twoL1p+1)*(twoI+1)*(2*k+1))*
                 ROOT::Math::wigner_3j(twoL1,twoL1p,2*k,2,-2,0)*          // wigner_3j takes 2ji as input
                 ROOT::Math::wigner_6j(twoL1,twoL1p,2*k,twoI,twoI,twoIi); // wigner_6j takes 2ji as input

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

double CXAngCorrPlayer::TheoreticalAngCorrFunction(double *x, double *p)
{
    double scale = p[0];

    int TwoJ1 = TMath::Nint(p[1]);
    int TwoJ2 = TMath::Nint(p[2]);
    int TwoJ3 = TMath::Nint(p[3]);

    double mix12 = p[4];
    double mix23 = p[5];

    vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,mix12,mix23);
    int kmax = Akks.size()*2;
    double tot=1;
    int iak=0;
    for(int k=2; k<=kmax; k+=2, iak++) {
        double arg = cos(*x*TMath::DegToRad());
        tot+=Akks.at(iak)*ROOT::Math::legendre(k,arg);
    }
    return scale*tot;
}

void CXAngCorrPlayer::PlotMixingEvaluation()
{
    int TwoJ1 = fNESpins[0]->GetIntNumber();
    int TwoJ2 = fNESpins[1]->GetIntNumber();
    int TwoJ3 = fNESpins[2]->GetIntNumber();

    GetCurrentInstance();

    if(fAngCorrPads[1] == nullptr) return;

    fAngCorrPads[1]->Clear();

    TH1 *frame = fAngCorrPads[1]->DrawFrame(-1,-1,1,1);
    frame->GetXaxis()->SetTitle("A2");
    frame->GetXaxis()->CenterTitle();
    frame->GetYaxis()->SetTitle("A4");
    frame->GetYaxis()->CenterTitle();
    frame->GetXaxis()->SetTitleSize(0.05);
    frame->GetXaxis()->SetLabelSize(0.05);
    frame->GetYaxis()->SetTitleSize(0.05);
    frame->GetYaxis()->SetLabelSize(0.05);

    delete fA2A4MixingGraph;
    fA2A4MixingGraph = new TGraph;
    fA2A4MixingGraph->SetName("A2A4MixingGraph");
    fA2A4MixingGraph->SetMarkerStyle(20);
    fA2A4MixingGraph->SetMarkerSize(0.5);
    fA2A4MixingGraph->SetLineColor(kGreen);
    fA2A4MixingGraph->SetLineStyle(kDashed);
    fA2A4MixingGraph->SetLineWidth(2);

    double deltapmin=-INFINITY;
    double deltapmax=INFINITY;
    int deltanb=5000;

    double atandeltamin1 = atan(deltapmin);
    double atandeltamax1 = atan(deltapmax);
    double atandeltamin2 = atan(deltapmin);
    double atandeltamax2 = atan(deltapmax);

    double deltapatandelta=(atandeltamax1-atandeltamin1)/(deltanb+1.);
    double atandelta1=atandeltamin1;

    if(fFixMixing[0]->GetState()==kButtonDown) {
        atandelta1 = fNEMixing[0]->GetNumber();
        atandeltamax1 = atandelta1;
    }

    int j=0;
    while(atandelta1<=atandeltamax1) {
        double atandelta2=atandeltamin2;
        if(fFixMixing[1]->GetState()==kButtonDown) {
            atandelta2 = fNEMixing[1]->GetNumber();
            atandeltamax2 = atandelta2;
        }
        while(atandelta2<=atandeltamax2) {

            vector<double> Akks = Eval_Ak(TwoJ1,TwoJ2,TwoJ3,tan(atandelta1),tan(atandelta2));

            Float_t _A2 = 0.;
            Float_t _A4 = 0.;

            if(Akks.size()>=1) _A2 = Akks.at(0);
            if(Akks.size()>=2) _A4 = Akks.at(1);

            if(j%200==0) fA2A4MixingGraph->SetPoint(fA2A4MixingGraph->GetN(),_A2,_A4);
            j++;
            atandelta2 += deltapatandelta;
        }
        atandelta1 += deltapatandelta;
    }

    double minx,maxx,miny,maxy;
    minx = *min_element(fA2A4MixingGraph->GetX(), fA2A4MixingGraph->GetX()+fA2A4MixingGraph->GetN());
    miny = *min_element(fA2A4MixingGraph->GetY(), fA2A4MixingGraph->GetY()+fA2A4MixingGraph->GetN());
    maxx = *max_element(fA2A4MixingGraph->GetX(), fA2A4MixingGraph->GetX()+fA2A4MixingGraph->GetN());
    maxy = *max_element(fA2A4MixingGraph->GetY(), fA2A4MixingGraph->GetY()+fA2A4MixingGraph->GetN());

    double deltaX = (maxx-minx)*0.1;
    double deltaY = (maxy-miny)*0.1;

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
    fA2A4MixingGraph->Draw("p");

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
}

ClassImp(CXAngCorrPlayer)
