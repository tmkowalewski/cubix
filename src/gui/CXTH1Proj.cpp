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

#include "CXTH1Proj.h"

#include <array>

#include "TH2.h"
#include "TCanvas.h"
#include "KeySymbols.h"
#include "TPadPainter.h"
#include "TColor.h"
#include "TROOT.h"
#include "TError.h"
#include "TContextMenu.h"
#include "TVirtualX.h"
#include "TF1.h"
#include "TFrame.h"

#include "CXMainWindow.h"
#include "CXHist2DPlayer.h"
#include "CXGateBox.h"
#include "CXBashColor.h"

using namespace std;

CXTH1Proj::CXTH1Proj(const TH1D &hist, int _nprojs): TH1D::TH1D(hist)
{
    GetXaxis()->SetTitle("Energy (keV) [X axis]");
    GetXaxis()->SetTitleSize(0.05);
    GetXaxis()->SetTitleOffset(0.8);
    GetXaxis()->SetTitleFont(132);
    GetXaxis()->SetLabelFont(132);

    GetYaxis()->SetTitle(Form("Counts/%g keV",GetBinWidth(1)));
    GetYaxis()->SetTitleSize(0.05);
    GetYaxis()->SetTitleOffset(0.5);
    GetYaxis()->SetTitleFont(132);
    GetYaxis()->SetLabelFont(132);

    fListOfGates = new TList;
    fListOfGates->SetOwner();
    fListOfGates->SetName("ListOfGates");

    fNProjs = _nprojs;
    for(int i=0 ; i<fNProjs ; i++) {
        fProjection.push_back(nullptr);
        fProjPad.push_back(nullptr);
    }

    SetStats();
}

CXTH1Proj::CXTH1Proj()
{
    fListOfGates = new TList;
    fListOfGates->SetOwner();
    fListOfGates->SetName("ListOfGates");

    SetStats();
}

CXTH1Proj::~CXTH1Proj()
{
    delete fListOfGates;
}

void CXTH1Proj::UpdateProjection(Int_t Axis)
{
    if(fGGHist->GetXaxis()->GetNbins() != fGGHist->GetYaxis()->GetNbins()) {
        gbash_color->WarningMessage("X and Y axis needs to have the same number of bins for GG projections");
        //        return;
    }
    if(Axis == 0) {
        Reset();
        GetXaxis()->SetTitle("Energy (keV) [X axis]");
        TH1 *projtmp = fGGHist->ProjectionX();
        SetBins(projtmp->GetXaxis()->GetNbins(),projtmp->GetXaxis()->GetXmin(),projtmp->GetXaxis()->GetXmax());
        for(int i=0 ; i<projtmp->GetNbinsX() ; i++)
            SetBinContent(i,projtmp->GetBinContent(i));
        delete projtmp;
    }
    else if(Axis == 1) {
        Reset();
        GetXaxis()->SetTitle("Energy (keV) [Y axis]");
        TH1 *projtmp = fGGHist->ProjectionY();
        SetBins(projtmp->GetXaxis()->GetNbins(),projtmp->GetXaxis()->GetXmin(),projtmp->GetXaxis()->GetXmax());
        for(int i=0 ; i<projtmp->GetNbinsX() ; i++)
            SetBinContent(i,projtmp->GetBinContent(i));
        delete projtmp;
    }

    fProjectionAxis = Axis;

    fCurrentPad->Modified();
    fCurrentPad->Update();
}

void CXTH1Proj::SetPlayer(CXHist2DPlayer *p)
{
    f2DPlayer = p;
}

void CXTH1Proj::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXTH1Proj::SetTH2(TH2 *hist)
{
    delete fGGHist;
    fGGHist = dynamic_cast<TH2*>(hist->Clone());
}

void CXTH1Proj::SetProjPad(int _ipad, TPad *_pad)
{
    fProjPad.at(_ipad) = _pad;
}

void CXTH1Proj::SetCurrentPad(TPad *pad)
{
    fCurrentPad = pad;
    fCurrentPad->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "CXTH1Proj", this, "HandleMovement(Int_t,Int_t,Int_t, TObject*)");
    fCurrentPad->Connect("RangeChanged()", "CXTH1Proj", this, "UpdateGates()");
    fCurrentPad->Connect("RangeAxisChanged()", "CXTH1Proj", this, "UpdateGates()");
    fCurrentPad->Connect("Resized()", "CXTH1Proj", this, "UpdateGates()");
    fCurrentPad->Connect("UnZoomed()", "CXTH1Proj", this, "UpdateGates()");
}


void CXTH1Proj::AddBackgd1(double Mean, double Width)
{
    if(Mean==0 && ((CXGateBox*)fListOfGates->Last()) && ((CXGateBox*)fListOfGates->Last())->IsGate1()) {
        Mean = ((CXGateBox*)fListOfGates->Last())->GetCentroid() + 4*((CXGateBox*)fListOfGates->Last())->GetWidth();
    }
    if(Width==0.) {
        if(f2DPlayer->UseFWHM()) {
            Mean = gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX());
            Width = fMainWindow->GetWSManager()->GetActiveWorkspace()->fFWHMFunction->Eval(Mean);
            Width *= f2DPlayer->GetFWHMBckFraction();
        }
        else if(((CXGateBox*)fListOfGates->Last())) {
            Width = ((CXGateBox*)fListOfGates->Last())->GetWidth();
        }
    }

    fAddNewBacground = false;

    if(Mean==0. || Width == 0.) return;

    auto *box = new CXGateBox(Mean,Width,fCurrentPad);
    box->SetBGD();
    fListOfGates->Add(box);

    fCurrentPad->cd();
    box->Draw();
}

void CXTH1Proj::AddGate1(double Mean, double Width)
{
    if(Width==0.) {
        if(f2DPlayer->UseFWHM()) {
            if(Mean==0) return;
            Width = fMainWindow->GetWSManager()->GetActiveWorkspace()->fFWHMFunction->Eval(Mean);
            Width *= f2DPlayer->GetFWHMGateFraction();
        }
        else {
            for(int i=(fListOfGates->GetEntries()-1) ; i>=0 ; i--) {
                CXGateBox *box = dynamic_cast<CXGateBox*>(fListOfGates->At(i));
                if(box->IsGate1()) {
                    Width = box->GetWidth();
                    break;
                }
            }
        }
    }

    fAddNewGate = false;

    if(Mean==0. || Width == 0.) return;

    auto *box = new CXGateBox(Mean,Width,fCurrentPad);
    box->SetGate1();
    fListOfGates->Add(box);

    fCurrentPad->cd();
    box->Draw();
}

void CXTH1Proj::Project(Bool_t FixRange, int _rebin_value)
{
    if(!fprojok) return;
    fprojok = false;

    if( fCurrentProjPad >= fProjPad.size() ) {
        gbash_color->WarningMessage(Form("Trying to access to proj pad number %d while size is %ld",fCurrentProjPad, fProjPad.size() ) );
        return;
    }

    array<Float_t,2> SavedRange{};
    if( FixRange ){
        if(fMainWindow->GetHisto(fProjPad.at(fCurrentProjPad))){
            SavedRange.at(0) = fProjPad.at(fCurrentProjPad)->GetUxmin();
            SavedRange.at(1) = fProjPad.at(fCurrentProjPad)->GetUxmax();
        }
    }

    delete fProjection.at(fCurrentProjPad);
    fProjection.at(fCurrentProjPad) = nullptr;

    Float_t TotalGateWidth = 0;
    Float_t TotalBGDWidth = 0;

    TString ProjName;
    if(fProjectionAxis==0) ProjName = Form("%s_ProjY",fGGHist->GetName());
    else ProjName = Form("%s_ProjX",fGGHist->GetName());

    TString NameGates  = "";
    TString NameBGD  = "";

    for(int i=0 ; i<fListOfGates->GetEntries() ; i++) {
        auto* box =  dynamic_cast<CXGateBox*>(fListOfGates->At(i));

        if(box->IsBGD()){
            TotalBGDWidth += box->GetWidth();
            NameBGD += Form("_B%.1f(%.1f)",box->GetCentroid(),box->GetWidth());
        }
        else if(box->IsGate2()){
            gbash_color->WarningMessage("Cube gates not possible in GG mode, ignores");
        }
        else{
            TotalGateWidth += box->GetWidth();
            NameGates += Form("_E%.1f(%.1f)",box->GetCentroid(),box->GetWidth());
        }
    }

    ProjName += NameGates + NameBGD;

    gErrorIgnoreLevel = kError;

    if(fProjectionAxis==0) fProjection.at(fCurrentProjPad) = fGGHist->ProjectionY("tempproj");
    else fProjection.at(fCurrentProjPad) = fGGHist->ProjectionX("tempproj");

    fProjection.at(fCurrentProjPad)->SetStats();
    fProjection.at(fCurrentProjPad)->Reset();
    if(_rebin_value>1) fProjection.at(fCurrentProjPad)->Rebin(_rebin_value);
    GetYaxis()->SetTitle(Form("Counts/%g keV",fProjection.at(fCurrentProjPad)->GetBinWidth(1)));

    fProjection.at(fCurrentProjPad)->SetName(ProjName.Data());
    fProjection.at(fCurrentProjPad)->SetTitle(ProjName.Data());

    Float_t WidthRef = fProjection.at(fCurrentProjPad)->GetBinWidth(1);

    TString XAxis = GetXaxis()->GetTitle();
    if(XAxis.Contains("[X"))
        XAxis.ReplaceAll("[X","[Y");
    else if(XAxis.Contains("[Y"))
        XAxis.ReplaceAll("[Y","[X");

    fProjection.at(fCurrentProjPad)->GetXaxis()->SetTitle(XAxis);
    fProjection.at(fCurrentProjPad)->GetXaxis()->SetTitleSize(GetXaxis()->GetTitleSize());
    fProjection.at(fCurrentProjPad)->GetXaxis()->SetTitleOffset(GetXaxis()->GetTitleOffset());
    fProjection.at(fCurrentProjPad)->GetXaxis()->SetTitleFont(GetXaxis()->GetTitleFont());
    fProjection.at(fCurrentProjPad)->GetXaxis()->SetLabelFont(GetXaxis()->GetLabelFont());

    fProjection.at(fCurrentProjPad)->GetYaxis()->SetTitle(GetYaxis()->GetTitle());
    fProjection.at(fCurrentProjPad)->GetYaxis()->SetTitleSize(GetYaxis()->GetTitleSize());
    fProjection.at(fCurrentProjPad)->GetYaxis()->SetTitleOffset(GetYaxis()->GetTitleOffset());
    fProjection.at(fCurrentProjPad)->GetYaxis()->SetTitleFont(GetYaxis()->GetTitleFont());
    fProjection.at(fCurrentProjPad)->GetYaxis()->SetLabelFont(GetYaxis()->GetLabelFont());

    for(int i=0 ; i<fListOfGates->GetEntries() ; i++) {
        auto* box =  dynamic_cast<CXGateBox*>(fListOfGates->At(i));

        auto X1 = box->GetX1();
        auto X2 = box->GetX2();

        int BinX1, BinX2;

        if(fProjectionAxis==0) {
            BinX1 = fGGHist->GetXaxis()->FindBin(X1);
            BinX2 = fGGHist->GetXaxis()->FindBin(X2);
        }
        else {
            BinX1 = fGGHist->GetYaxis()->FindBin(X1);
            BinX2 = fGGHist->GetYaxis()->FindBin(X2);
        }

        Float_t Weight = 1.;
        if(box->IsBGD())
            Weight = -TotalGateWidth/TotalBGDWidth;
        if(box->IsGate2())
            continue;

        for(int ibin=BinX1 ; ibin<BinX2 ; ibin++) {
            TH1D *temp = nullptr;
            if(fProjectionAxis==0) temp = fGGHist->ProjectionY("tmp",ibin,ibin);
            else temp = fGGHist->ProjectionX("tmp",ibin,ibin);

            if(_rebin_value>1) temp->Rebin(_rebin_value);

            temp->Scale(fProjection.at(fCurrentProjPad)->GetBinWidth(ibin)/WidthRef);
            fProjection.at(fCurrentProjPad)->Add(temp,Weight);
            delete temp;
        }
    }

    gbash_color->InfoMessage(Form("Integral of projection: %g",fProjection.at(fCurrentProjPad)->Integral()));

    fProjPad.at(fCurrentProjPad)->cd();
    fProjection.at(fCurrentProjPad)->Draw("hist");
    if(FixRange) fProjection.at(fCurrentProjPad)->GetXaxis()->SetRangeUser(SavedRange.at(0),SavedRange.at(1));
    gErrorIgnoreLevel = kPrint;

    fProjPad.at(fCurrentProjPad)->Update();
    fProjPad.at(fCurrentProjPad)->Modified();

    fProjPad.at(fCurrentProjPad)->SetBit(TPad::kCannotMove);
    fProjPad.at(fCurrentProjPad)->GetFrame()->SetBit(TObject::kCannotPick);

    fprojok=true;
}

void CXTH1Proj::ClearGates(){
    fListOfGates->Clear();
    UpdateGates();
}

void CXTH1Proj::RemoveGate(CXGateBox *box){
    fCurrentPad->GetListOfPrimitives()->Remove(box);
    fListOfGates->Remove(box);
    UpdateGates();
}

void CXTH1Proj::HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected)
{
    // cout<<EventType<<" "<<EventX<<" "<<EventY<<" "<<selected<<endl;

    if(gROOT->GetSelectedPad()) {
        TString PadName = gROOT->GetSelectedPad()->GetName();
        if((PadName.BeginsWith("GxG") || PadName.BeginsWith("Rad")) && PadName.CountChar('_') == 2) {
            TObjArray *arr = PadName.Tokenize("_");
            int padnum = atoi(arr->Last()->GetName());
            if(padnum>1 && (padnum-2)<fProjPad.size() ) {
                fCurrentProjPad = padnum-2;
            }
        }
    }

    bool CTRL = (( EventX == EventY-96) || fMainWindow->IsCtrlOn());

    switch (EventType){
    case kMouseMotion: {
        fLastSym = kAnyKey;
        break;
    }
    case kKeyPress:{
        if(CTRL)
            break;

        auto KeySym = (EKeySym)EventY;

        if(fLastSym == kKey_g && KeySym == kKey_g)  {
            fAddNewGate = false;
            TMethod* m = Class()->GetMethod("AddGate1", "500,2");
            if (m != 0x0 && gPad->GetCanvas()->GetContextMenu() != 0x0)
                gPad->GetCanvas()->GetContextMenu()->Action(this, m);
        }
        else if( KeySym == kKey_g )
            fAddNewGate = true;
        if(fLastSym == kKey_b && KeySym == kKey_b)  {
            fAddNewBacground = false;
            TMethod* m = Class()->GetMethod("AddBackgd1", "500,2");
            if (m != 0x0 && gPad->GetCanvas()->GetContextMenu() != 0x0)
                gPad->GetCanvas()->GetContextMenu()->Action(this, m);
        }
        else if( KeySym == kKey_b )
            fAddNewBacground = true;
        if( KeySym == kKey_c && fMainWindow->GetSelectedPad() == fCurrentPad)
            ClearGates();
        if( KeySym == kKey_p )
            f2DPlayer->Project();
        if( KeySym == kKey_d  && selected && selected->InheritsFrom(CXGateBox::Class()))
            RemoveGate(dynamic_cast<CXGateBox*>(selected));
        if( KeySym == kKey_l )
            f2DPlayer->ApplyLastGate();

        UpdateGates();

        fLastSym = KeySym;

        break;
    }
    case kButton1Down:{
        fLastSym = kAnyKey;

        if((fAddNewGate || fAddNewBacground) && f2DPlayer->UseFWHM()) {
            double x = gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX());
            if(fAddNewGate) AddGate1(x);
            else AddBackgd1(x);
        }

        if(!fAddNewGate && !fAddNewBacground)
            break;

        if(selected && selected->InheritsFrom(TAxis::Class()))
            break;

        gPad = fCurrentPad;

        gVirtualX->SetLineWidth(1);

        gPad->GetCanvas()->FeedbackMode(kTRUE);   // to draw in rubberband mode

        oldx = gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX());
        oldy = gPad->AbsPixeltoY(gPad->GetCanvas()->GetEventY());
        xinit = oldx;
        yinit = oldy;

        xmin = xinit;
        xmax = xinit;

        dynamic_cast<TPadPainter*>(gPad->GetPainter())->DrawBox(xinit,yinit,xinit,yinit, TVirtualPadPainter::kHollow);

        fGateVirtualBox = new TBox(xinit, yinit, xinit, yinit);
        Int_t ci;
        if(fAddNewBacground)
            ci = TColor::GetColor("#7d7dff");
        else
            ci = TColor::GetColor("#ff7d7d");

        TColor *zoomcolor = gROOT->GetColor(ci);
        if (!TCanvas::SupportAlpha() || !zoomcolor)
            fGateVirtualBox->SetFillStyle(3002);
        else
            zoomcolor->SetAlpha(0.5);

        fGateVirtualBox->SetFillColor(ci);
        fGateVirtualBox->Draw();

        gPad->Update();
        gPad->Modified();

        moved = true;
        break;
    }
    case kButton1Motion:{
        fLastSym = kAnyKey;

        if(!fAddNewGate && !fAddNewBacground) {
            CXGateBox*box = dynamic_cast<CXGateBox*>(selected);
            if(f2DPlayer->UseDynamicProjection() && box) {
                if(fMainWindow->GetHisto(fProjPad.at(fCurrentProjPad))) {
                    f2DPlayer->Project();
                }
            }
            else break;
        }

        if(!moved) break;

        gPad = fCurrentPad;    // don't use cd() because we won't draw in pad

        gPad->GetPainter()->SetFillStyle(kFDotted1);
        gPad->GetPainter()->SetFillColor(kRed);

        dynamic_cast<TPadPainter*>(gPad->GetPainter())->DrawBox(xinit,yinit,oldx,oldy, TVirtualPadPainter::kHollow);
        oldx = gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX());
        oldy = gPad->AbsPixeltoY(gPad->GetCanvas()->GetEventY());

        if (fGateVirtualBox){
            fGateVirtualBox->SetX1(xinit);
            fGateVirtualBox->SetY1(yinit);
            fGateVirtualBox->SetX2(oldx);
            fGateVirtualBox->SetY2(oldy);
        }

        gPad->Modified();
        gPad->Update();

        xmax = oldx;
        ymax = oldy;

        xmin = xinit;
        ymin = yinit;

        Double_t toto = 0;
        if (xmax < xmin){
            toto = xmax;
            xmax = xmin;
            xmin = toto;
        }
        if (ymax < ymin){
            toto = ymax;
            ymax = ymin;
            ymin = toto;
        }

        break;
    }
    case kButton1Up:{
        if(!fAddNewGate && !fAddNewBacground){
            UpdateGates();
            break;
        }
        if(moved){
            gPad = fCurrentPad;

            Float_t Mean = (xmax+xmin)/2;
            Float_t Width = (xmax-xmin)/2.;

            if(fAddNewGate)
                AddGate1(Mean,Width);
            if(fAddNewBacground)
                AddBackgd1(Mean,Width);

            xmax = xmin = ymax = ymin = 0.;
        }

        moved = false;

        if (fGateVirtualBox) {
            fGateVirtualBox->Delete();
            fGateVirtualBox = nullptr;
        }

        UpdateGates();

        break;
    }
    default:{
        break;
    }
    }

}

void CXTH1Proj::UpdateGates()
{
    fCurrentPad->Modified();
    fCurrentPad->Update();

    for(int i=0 ; i<fListOfGates->GetEntries() ; i++){
        auto* box =  dynamic_cast<CXGateBox*>(fListOfGates->At(i));
        box->Update();
    }

    fCurrentPad->Modified();
    fCurrentPad->Update();
}

ClassImp(CXTH1Proj)
