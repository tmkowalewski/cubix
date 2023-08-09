#include "CXTH1Proj.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <array>

#include "TH2.h"
#include "TCanvas.h"
#include "KeySymbols.h"
#include "TPadPainter.h"
#include "TColor.h"
#include "TROOT.h"
#include "TFrame.h"
#include "TError.h"
#include "TContextMenu.h"
#include "TVirtualX.h"

#include "CXMainWindow.h"
#include "CXHist2DPlayer.h"
#include "CXGateBox.h"
#include "CXBashColor.h"

using namespace std;

CXTH1Proj::CXTH1Proj(const TH1D &hist): TH1D::TH1D(hist)
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
        GetXaxis()->SetTitle("Energy (keV) [X axis]");
        TH1 *projtmp = fGGHist->ProjectionX();
        SetBins(fGGHist->GetXaxis()->GetNbins(),fGGHist->GetXaxis()->GetXmin(),fGGHist->GetXaxis()->GetXmax());
        for(int i=0 ; i<projtmp->GetNbinsX()+2 ; i++)
            SetBinContent(i,projtmp->GetBinContent(i));
        delete projtmp;
    }
    else if(Axis == 1) {
        GetXaxis()->SetTitle("Energy (keV) [Y axis]");
        SetBins(fGGHist->GetYaxis()->GetNbins(),fGGHist->GetYaxis()->GetXmin(),fGGHist->GetYaxis()->GetXmax());
        TH1 *projtmp = fGGHist->ProjectionY();
        for(int i=0 ; i<projtmp->GetNbinsX()+2 ; i++)
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

void CXTH1Proj::SetProjPad(TPad *pad)
{
    fProjPad = pad;
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


void CXTH1Proj::AddBackgd(Float_t Mean, Float_t Width)
{
    if(Mean==0 && Width==0 && ((CXGateBox*)fListOfGates->Last()) && ((CXGateBox*)fListOfGates->Last())->IsGate1()) {
        Mean = ((CXGateBox*)fListOfGates->Last())->GetCentroid() + 4*((CXGateBox*)fListOfGates->Last())->GetWidth();
        Width = ((CXGateBox*)fListOfGates->Last())->GetWidth();
    }
    auto *box = new CXGateBox(Mean,Width,fCurrentPad);
    box->SetBGD();
    fListOfGates->Add(box);

    fCurrentPad->cd();
    box->Draw();

    fAddNewBacground = false;
}

void CXTH1Proj::AddGate1(Float_t Mean, Float_t Width)
{
    auto *box = new CXGateBox(Mean,Width,fCurrentPad);
    box->SetGate1();
    fListOfGates->Add(box);

    fCurrentPad->cd();
    box->Draw();

    fAddNewGate = false;
}

void CXTH1Proj::Project(Bool_t FixRange, int _rebin_value) {

    array<Float_t,2> SavedRange{};
    if( FixRange ){
        if(fMainWindow->GetHisto(fProjPad)){
            SavedRange.at(0) = fProjPad->GetUxmin();
            SavedRange.at(1) = fProjPad->GetUxmax();
        }
    }

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

    if(gROOT->FindObject(ProjName.Data()))
        delete gROOT->FindObject(ProjName.Data());

    gErrorIgnoreLevel = kError;
    TH1D *FinalProj = nullptr;
    if(fProjectionAxis==0) FinalProj = fGGHist->ProjectionY("tempproj");
    else FinalProj = fGGHist->ProjectionX("tempproj");

    FinalProj->SetStats();
    FinalProj->Reset();
    if(_rebin_value>1) FinalProj->Rebin(_rebin_value);
    GetYaxis()->SetTitle(Form("Counts/%g keV",FinalProj->GetBinWidth(1)));

    FinalProj->SetName(ProjName.Data());
    FinalProj->SetTitle(ProjName.Data());

    Float_t WidthRef = FinalProj->GetBinWidth(1);

    TString XAxis = GetXaxis()->GetTitle();
    if(XAxis.Contains("[X"))
        XAxis.ReplaceAll("[X","[Y");
    else if(XAxis.Contains("[Y"))
        XAxis.ReplaceAll("[Y","[X");

    FinalProj->GetXaxis()->SetTitle(XAxis);
    FinalProj->GetXaxis()->SetTitleSize(GetXaxis()->GetTitleSize());
    FinalProj->GetXaxis()->SetTitleOffset(GetXaxis()->GetTitleOffset());
    FinalProj->GetXaxis()->SetTitleFont(GetXaxis()->GetTitleFont());
    FinalProj->GetXaxis()->SetLabelFont(GetXaxis()->GetLabelFont());

    FinalProj->GetYaxis()->SetTitle(GetYaxis()->GetTitle());
    FinalProj->GetYaxis()->SetTitleSize(GetYaxis()->GetTitleSize());
    FinalProj->GetYaxis()->SetTitleOffset(GetYaxis()->GetTitleOffset());
    FinalProj->GetYaxis()->SetTitleFont(GetYaxis()->GetTitleFont());
    FinalProj->GetYaxis()->SetLabelFont(GetYaxis()->GetLabelFont());

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

            temp->Scale(FinalProj->GetBinWidth(ibin)/WidthRef);
            FinalProj->Add(temp,Weight);
            delete temp;
        }
    }

    gbash_color->InfoMessage(Form("Integral of projection: %g",FinalProj->Integral()));

    fProjPad->cd();
    FinalProj->Draw("hist");
    if(FixRange)
        FinalProj->GetXaxis()->SetRangeUser(SavedRange.at(0),SavedRange.at(1));
    gErrorIgnoreLevel = kPrint;

    fProjPad->Update();
    fProjPad->Modified();

    fProjPad->SetBit(TPad::kCannotMove);
    fProjPad->GetFrame()->SetBit(TObject::kCannotPick);
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
    //    cout<<EventType<<" "<<EventX<<" "<<EventY<<" "<<selected<<endl;

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
            TMethod* m = Class()->GetMethod("AddBackgd", "500,2");
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
        if(!fAddNewGate && !fAddNewBacground)
            break;

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
                AddBackgd(Mean,Width);

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
