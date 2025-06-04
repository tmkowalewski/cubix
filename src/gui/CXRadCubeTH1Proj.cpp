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

#include "CXRadCubeTH1Proj.h"

#include "TH2.h"
#include "TCanvas.h"
#include "KeySymbols.h"
#include "TPadPainter.h"
#include "TColor.h"
#include "TROOT.h"
#include "TFrame.h"
#include "TError.h"
#include "TContextMenu.h"
#include "TRootEmbeddedCanvas.h"
#include "TVirtualX.h"
#include "TSystem.h"
#include "TGFileDialog.h"
#include "TF1.h"

#include "CXMainWindow.h"
#include "CXRad2DPlayer.h"
#include "CXRadCubePlayer.h"
#include "CXGateBox.h"
#include "CXBashColor.h"

using namespace std;

CXRadCubeTH1Proj::CXRadCubeTH1Proj(CXRadReader *radreader, int _nprojs) : TH1D(*radreader->GetTotalProjection())
{
    fListOfGates = new TList;
    fListOfGates->SetOwner();
    fListOfGates->SetName("ListOfGates");

    fNProjs = _nprojs;
    for(int i=0 ; i<fNProjs ; i++) {
        fProjection.push_back(nullptr);
        fProjPad.push_back(nullptr);
    }

    fRadReader = new CXRadReader;

    Int_t Status=0;

    // Read Cube
    Status += fRadReader->ReadCube(radreader->GetCubeFileName());
    // Read LUT
    Status += fRadReader->ReadTabFile(radreader->GetLUTFileName());
    // Read calibrations
    Status += fRadReader->ReadCalibs(radreader->GetECalFileName(), radreader->GetEffFileName(), radreader->GetCompFactor());
    // Read Tot proj
    Status += fRadReader->ReadTotProj(radreader->GetTotalProjFileName());
    // Read Background
    Status += fRadReader->ReadBackground(radreader->GetBackgroundFileName());
    // Read 2D projection
    Status += fRadReader->Read2DProj(radreader->Get2DProjFileName());

    if(Status>0){
        gbash_color->WarningMessage("Errors occured in reading the cube, ignored");
    }
    else{
        fRadReader->BuildHistos();
    }

    GetXaxis()->SetTitle("Energy (keV)");
    GetXaxis()->SetTitleSize(0.05);
    GetXaxis()->SetTitleOffset(0.8);
    GetXaxis()->SetTitleFont(132);
    GetXaxis()->SetLabelFont(132);

    GetYaxis()->SetTitle(Form("Counts/%g keV",GetBinWidth(1)));
    GetYaxis()->SetTitleSize(0.05);
    GetYaxis()->SetTitleOffset(0.5);
    GetYaxis()->SetTitleFont(132);
    GetYaxis()->SetLabelFont(132);

    SetName(Form("%s_TotProj",radreader->GetName().Data()));
    SetTitle(Form("%s TotProj",radreader->GetName().Data()));

    fMode = "Cube";

    SetStats();
}

CXRadCubeTH1Proj::CXRadCubeTH1Proj(TH2 *hist, int _nprojs) : TH1D(*hist->ProjectionX("CXRadCubeTH1Proj_tmp"))
{
    SetName(Form("%s_TotProj",hist->GetName()));
    fListOfGates = new TList;
    fListOfGates->SetOwner();
    fListOfGates->SetName("ListOfGates");

    fNProjs = _nprojs;
    for(int i=0 ; i<fNProjs ; i++) {
        fProjection.push_back(nullptr);
        fProjPad.push_back(nullptr);
    }

    fRadReader = new CXRadReader;

    // Read 2D projection
    fRadReader->ReadGG(hist);

    delete fGGHist;
    fGGHist = dynamic_cast<TH2 *>(hist->Clone());

    Reset();
    for(int i=0 ; i<=GetNbinsX() ; i++) {
        SetBinContent(i,fRadReader->GetTotalProjection()->GetBinContent(i));
    }
    GetXaxis()->SetTitle("Energy (keV)");
    GetXaxis()->SetTitleSize(0.05);
    GetXaxis()->SetTitleOffset(0.8);
    GetXaxis()->SetTitleFont(132);
    GetXaxis()->SetLabelFont(132);

    GetYaxis()->SetTitle(Form("Counts/%g keV",GetBinWidth(1)));
    GetYaxis()->SetTitleSize(0.05);
    GetYaxis()->SetTitleOffset(0.5);
    GetYaxis()->SetTitleFont(132);
    GetYaxis()->SetLabelFont(132);

    fMode = "2D";

    SetStats();
}

CXRadCubeTH1Proj::CXRadCubeTH1Proj(): TH1D::TH1D()
{
}

CXRadCubeTH1Proj::~CXRadCubeTH1Proj()
{
    fMainWindow->GetROOTCanvas()->GetClient()->Disconnect("ProcessedEvent(Event_t *, Window_t)", this, "EventProcessed(Event_t*)");
}

void CXRadCubeTH1Proj::SetCubePlayer(CXRadCubePlayer *p)
{
    fRadCubePlayer = p;
}

void CXRadCubeTH1Proj::Set2DPlayer(CXRad2DPlayer *p)
{
    fRad2DPlayer = p;
}

void CXRadCubeTH1Proj::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXRadCubeTH1Proj::SetProjPad(int _ipad, TPad *_pad)
{
    fProjPad.at(_ipad) = _pad;
}

void CXRadCubeTH1Proj::SetCurrentPad(TPad *pad)
{
    fCurrentPad = pad;
    fCurrentPad->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "CXRadCubeTH1Proj", this, "HandleMovement(Int_t,Int_t,Int_t, TObject*)");
    fCurrentPad->Connect("RangeChanged()", "CXRadCubeTH1Proj", this, "UpdateGates()");
    fCurrentPad->Connect("RangeAxisChanged()", "CXRadCubeTH1Proj", this, "UpdateGates()");
    fCurrentPad->Connect("Resized()", "CXRadCubeTH1Proj", this, "UpdateGates()");
    fCurrentPad->Connect("UnZoomed()", "CXRadCubeTH1Proj", this, "UpdateGates()");
}

void CXRadCubeTH1Proj::AddGate1(Float_t Mean, Float_t Width)
{
    if(Width==0.) {
        if(( (fRadCubePlayer && fRadCubePlayer->UseFWHM()) || (fRad2DPlayer && fRad2DPlayer->UseFWHM()) )) {
            if(Mean==0) return;
            Width = fMainWindow->GetWSManager()->GetActiveWorkspace()->fFWHMFunction->Eval(Mean);
            if(fRadCubePlayer) Width *= fRadCubePlayer->GetFWHMGateFraction();
            else Width *= fRad2DPlayer->GetFWHMGateFraction();
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

    fAddNewGate1 = false;

    if(Mean==0. || Width == 0.) return;

    CXGateBox *box = new CXGateBox(Mean,Width,fCurrentPad);

    box->SetGate1();
    fListOfGates->Add(box);

    fCurrentPad->cd();
    box->Draw();
}

void CXRadCubeTH1Proj::AddGate2(Float_t Mean, Float_t Width)
{
    if(Width==0.) {
        if(( (fRadCubePlayer && fRadCubePlayer->UseFWHM()) || (fRad2DPlayer && fRad2DPlayer->UseFWHM()) )) {
            if(Mean==0) return;
            Width = fMainWindow->GetWSManager()->GetActiveWorkspace()->fFWHMFunction->Eval(Mean);
            if(fRadCubePlayer) Width *= fRadCubePlayer->GetFWHMGateFraction();
            else Width *= fRad2DPlayer->GetFWHMGateFraction();
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

    fAddNewGate2 = false;

    if(Mean==0. || Width == 0.) return;

    CXGateBox *box = new CXGateBox(Mean,Width,fCurrentPad);

    box->SetGate2();
    fListOfGates->Add(box);

    box->Draw();
}

void CXRadCubeTH1Proj::Project(Bool_t FixRange,Bool_t BGSubtract, Int_t Rebin)
{
    if(!fprojok) return;
    fprojok = false;

    if( fCurrentProjPad >= fProjPad.size() ) {
        gbash_color->WarningMessage(Form("Trying to access to proj pad number %d while size is %ld",fCurrentProjPad, fProjPad.size() ) );
        return;
    }

    Float_t SavedRange[2];
    if(!fMainWindow->GetHisto(fProjPad.at(fCurrentProjPad))) FixRange = false;
    if( FixRange ) {
        SavedRange[0] = fProjPad.at(fCurrentProjPad)->GetUxmin();
        SavedRange[1] = fProjPad.at(fCurrentProjPad)->GetUxmax();
    }

    vector< pair<float, float> > gates1; //{make_pair(205.5,214.5)};
    vector< pair<float, float> > gates2; //{make_pair(345.5,355.5)};

    for(int i=0 ; i<fListOfGates->GetEntries() ; i++) {
        CXGateBox* box =  static_cast<CXGateBox*>(fListOfGates->At(i));

        if(box->IsBGD()) {
            gbash_color->WarningMessage("Background gates not used in this mode, ignored ");
            continue;
        }
        if(box->IsGate2() && fMode == "2D") {
            gbash_color->WarningMessage("Cube projections not available in this mode, ignored ");
            continue;
        }

        pair<double, double> apair = make_pair(box->GetCentroid()-box->GetWidth(),box->GetCentroid()+box->GetWidth());

        if(box->IsGate1())
            gates1.push_back(apair);
        else if(box->IsGate2() && fMode == "Cube")
            gates2.push_back(apair);
    }

    gErrorIgnoreLevel = kError; // To avoid warnings

    delete fProjection.at(fCurrentProjPad);
    fProjection.at(fCurrentProjPad) = (TH1*) fRadReader->Project(gates1,gates2,BGSubtract)->Clone();
    if(fProjection.at(fCurrentProjPad) == nullptr) return;

    fProjection.at(fCurrentProjPad)->GetXaxis()->SetTitle(GetXaxis()->GetTitle());
    fProjection.at(fCurrentProjPad)->GetXaxis()->SetTitleSize(GetXaxis()->GetTitleSize());
    fProjection.at(fCurrentProjPad)->GetXaxis()->SetTitleOffset(GetXaxis()->GetTitleOffset());
    fProjection.at(fCurrentProjPad)->GetXaxis()->SetTitleFont(GetXaxis()->GetTitleFont());
    fProjection.at(fCurrentProjPad)->GetXaxis()->SetLabelFont(GetXaxis()->GetLabelFont());

    fProjection.at(fCurrentProjPad)->GetYaxis()->SetTitle(GetYaxis()->GetTitle());
    fProjection.at(fCurrentProjPad)->GetYaxis()->SetTitleSize(GetYaxis()->GetTitleSize());
    fProjection.at(fCurrentProjPad)->GetYaxis()->SetTitleOffset(GetYaxis()->GetTitleOffset());
    fProjection.at(fCurrentProjPad)->GetYaxis()->SetTitleFont(GetYaxis()->GetTitleFont());
    fProjection.at(fCurrentProjPad)->GetYaxis()->SetLabelFont(GetYaxis()->GetLabelFont());

    if(Rebin>1) {
        fProjection.at(fCurrentProjPad)->Rebin(Rebin);
        fProjection.at(fCurrentProjPad)->SetYTitle(Form("Counts (%g keV/bin)",fProjection.at(fCurrentProjPad)->GetBinWidth(1)));
    }

    fProjPad.at(fCurrentProjPad)->cd();
    fProjection.at(fCurrentProjPad)->Draw("hist");

    if(FixRange)
        fProjection.at(fCurrentProjPad)->GetXaxis()->SetRangeUser(SavedRange[0],SavedRange[1]);
    else
        fProjection.at(fCurrentProjPad)->GetXaxis()->UnZoom();

    gErrorIgnoreLevel = kPrint; // to recover warnings

    gbash_color->InfoMessage(Form("Integral of projection: %g",fProjection.at(fCurrentProjPad)->Integral()));

    fProjPad.at(fCurrentProjPad)->Update();
    fProjPad.at(fCurrentProjPad)->Modified();

    fProjPad.at(fCurrentProjPad)->SetBit(TPad::kCannotMove);
    fProjPad.at(fCurrentProjPad)->GetFrame()->SetBit(TObject::kCannotPick);

    fprojok=true;
}

void CXRadCubeTH1Proj::ClearGates(){
    fListOfGates->Clear();
    UpdateGates();
}

void CXRadCubeTH1Proj::RemoveGate(CXGateBox *box){
    fCurrentPad->GetListOfPrimitives()->Remove(box);
    fListOfGates->Remove(box);
    UpdateGates();
}

void CXRadCubeTH1Proj::SetBackground(Float_t LenghtFactor, Float_t Width, Float_t FWHM_0, Float_t FWHM_1, Float_t FWHM_2)
{
    fLenghtFactor = LenghtFactor;
    fWidth        = Width;
    fFWHM_0       = FWHM_0;
    fFWHM_1       = FWHM_1;
    fFWHM_2       = FWHM_2;

    fRadReader->ReEvalBackground(fCurrentPad->GetUxmin(), fCurrentPad->GetUxmax(), fLenghtFactor,fWidth,fFWHM_0,fFWHM_1,fFWHM_2);

    fCurrentPad->Update();
    fCurrentPad->Modified();
}

void CXRadCubeTH1Proj::SaveBackground()
{
    const char* SaveAsTypes[] = {
        "Radware format", "*.spe",
        "All files"     , "*",
        0,              0
    };

    TString workdir = gSystem->WorkingDirectory();
    static TString dir(".");
    static Int_t typeidx = 0;
    static Bool_t overwr = kFALSE;
    TGFileInfo fi;
    fi.fFileTypes   = SaveAsTypes;
    fi.fIniDir      = StrDup(dir);
    fi.fFileTypeIdx = typeidx;
    fi.fOverwrite   = overwr;
    new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), kFDSave, &fi);
    gSystem->ChangeDirectory(workdir.Data());
    if (!fi.fFilename) return;
    TString fn = fi.fFilename;
    dir     = fi.fIniDir;
    typeidx = fi.fFileTypeIdx;
    overwr  = fi.fOverwrite;

    if(! fn.EndsWith(".spe") ) {
        gbash_color->WarningMessage(".spe extension automatically added :");
        fn.Append(".spe");
    }

    fRadReader->SaveBackground(fn);
}

void CXRadCubeTH1Proj::HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected)
{
    if(gROOT->GetSelectedPad()) {
        TString PadName = gROOT->GetSelectedPad()->GetName();
        TObjArray *arr = PadName.Tokenize("_");
        int padnum = atoi(arr->Last()->GetName());
        if(padnum>1) {
            fCurrentProjPad = padnum-2;
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

        EKeySym KeySym = (EKeySym)EventY;

        if(fLastSym == kKey_g && KeySym == kKey_g)  {
            fAddNewGate1 = false;
            TMethod* m = Class()->GetMethod("AddGate1", "500,2");
            if (m != 0x0 && gPad->GetCanvas()->GetContextMenu() != 0x0)
                gPad->GetCanvas()->GetContextMenu()->Action(this, m);
        }
        else if( KeySym == kKey_g )
            fAddNewGate1 = true;
        if(fLastSym == kKey_G && KeySym == kKey_G)  {
            fAddNewGate2 = false;
            TMethod* m = Class()->GetMethod("AddGate2", "500,2");
            if (m != 0x0 && gPad->GetCanvas()->GetContextMenu() != 0x0)
                gPad->GetCanvas()->GetContextMenu()->Action(this, m);
        }
        else if( KeySym == kKey_G )
            fAddNewGate2 = true;
        if( KeySym == kKey_c && fMainWindow->GetSelectedPad() == fCurrentPad)
            ClearGates();
        if( KeySym == kKey_p ){
            if(fRadCubePlayer)
                fRadCubePlayer->Project();
            else if(fRad2DPlayer)
                fRad2DPlayer->Project();
        }
        if( KeySym == kKey_d  && selected && selected->InheritsFrom("CXGateBox"))
            RemoveGate(((CXGateBox*)selected));
        if((EKeySym)KeySym==kKey_g && (EKeySym)fLastSym==kKey_l) {
            if(fRadCubePlayer)
                fRadCubePlayer->ApplyLastGates();
            else if(fRad2DPlayer)
                fRad2DPlayer->ApplyLastGates();
        }

        UpdateGates();

        fLastSym = KeySym;

        break;
    }
    case kButton1Down:{
        fLastSym = kAnyKey;

        if((fAddNewGate1 || fAddNewGate2) && ( (fRadCubePlayer && fRadCubePlayer->UseFWHM()) || (fRad2DPlayer && fRad2DPlayer->UseFWHM()) ) ) {
            double x = gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX());
            if(fAddNewGate1) AddGate1(x);
            else AddGate2(x);
        }

        if(!fAddNewGate1 && !fAddNewGate2)
            break;

        if(selected && selected->InheritsFrom("TAxis"))
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

        ((TPadPainter*)gPad->GetPainter())->DrawBox(xinit,yinit,xinit,yinit, TVirtualPadPainter::kHollow);

        fGateVirtualBox = new TBox(xinit, yinit, xinit, yinit);
        Int_t ci;
        if(fAddNewGate2)
            ci = TColor::GetColor("#66cc66");
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

        fLastSym = kAnyKey;

        break;
    }
    case kButton1Motion:{
        fLastSym = kAnyKey;

        if(!fAddNewGate1 && !fAddNewGate2) {
            CXGateBox*box = dynamic_cast<CXGateBox*>(selected);
            if(( (fRadCubePlayer && fRadCubePlayer->UseDynamicProjection()) || (fRad2DPlayer && fRad2DPlayer->UseDynamicProjection()) ) && box) {
                if(fMainWindow->GetHisto(fProjPad.at(fCurrentProjPad))) {
                    if(fRadCubePlayer)
                        fRadCubePlayer->Project();
                    else if(fRad2DPlayer)
                        fRad2DPlayer->Project();
                }
            }
            else break;
        }

        if(moved==false)
            break;

        gPad = fCurrentPad;    // don't use cd() because we won't draw in pad

        gPad->GetPainter()->SetFillStyle(kFDotted1);
        gPad->GetPainter()->SetFillColor(kRed);

        ((TPadPainter*)gPad->GetPainter())->DrawBox(xinit,yinit,oldx,oldy, TVirtualPadPainter::kHollow);
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
        if(!fAddNewGate1 && !fAddNewGate2){
            UpdateGates();
            break;
        }

        if(moved){
            gPad = fCurrentPad;

            Float_t Mean = (xmax+xmin)/2;
            Float_t Width = (xmax-xmin)/2.;

            if(fAddNewGate1)
                AddGate1(Mean,Width);
            if(fAddNewGate2)
                AddGate2(Mean,Width);

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

void CXRadCubeTH1Proj::UpdateGates()
{
    fCurrentPad->Modified();
    fCurrentPad->Update();

    for(int i=0 ; i<fListOfGates->GetEntries() ; i++){
        CXGateBox* box =  static_cast<CXGateBox*>(fListOfGates->At(i));
        box->Update();
    }

    fCurrentPad->Modified();
    fCurrentPad->Update();
}

ClassImp(CXRadCubeTH1Proj)
