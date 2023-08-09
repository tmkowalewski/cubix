#include "CXRadCubeTH1Proj.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

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

#include "CXMainWindow.h"
#include "CXRad2DPlayer.h"
#include "CXRadCubePlayer.h"
#include "CXGateBox.h"
#include "CXBashColor.h"

using namespace std;

CXRadCubeTH1Proj::CXRadCubeTH1Proj(CXRadReader *radreader) : TH1D(*radreader->GetTotalProjection())
{
    fListOfGates = new TList;
    fListOfGates->SetOwner();
    fListOfGates->SetName("ListOfGates");

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

CXRadCubeTH1Proj::CXRadCubeTH1Proj(TH2 *hist) : TH1D(*hist->ProjectionX("CXRadCubeTH1Proj_tmp"))
{
    SetName(Form("%s_TotProj",hist->GetName()));
    fListOfGates = new TList;
    fListOfGates->SetOwner();
    fListOfGates->SetName("ListOfGates");

    fRadReader = new CXRadReader;

    // Read 2D projection
    fRadReader->ReadGG(hist);

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

void CXRadCubeTH1Proj::SetProjPad(TPad *pad)
{
    fProjPad = pad;
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
    CXGateBox *box = new CXGateBox(Mean,Width,fCurrentPad);

    box->SetGate1();
    fListOfGates->Add(box);

    fCurrentPad->cd();
    box->Draw();

    fAddNewGate1 = false;
}

void CXRadCubeTH1Proj::AddGate2(Float_t Mean, Float_t Width)
{
    CXGateBox *box = new CXGateBox(Mean,Width,fCurrentPad);

    box->SetGate2();
    fListOfGates->Add(box);

    box->Draw();

    fAddNewGate2 = false;
}

void CXRadCubeTH1Proj::Project(Bool_t FixRange,Bool_t BGSubtract, Int_t Rebin)
{
    Float_t SavedRange[2];
    if(!fMainWindow->GetHisto(fProjPad)) FixRange = false;
    if( FixRange ){
        SavedRange[0] = fProjPad->GetUxmin();
        SavedRange[1] = fProjPad->GetUxmax();
    }

    vector< pair<double, double> > gates1; //{make_pair(205.5,214.5)};
    vector< pair<double, double> > gates2; //{make_pair(345.5,355.5)};

    for(int i=0 ; i<fListOfGates->GetEntries() ; i++)
    {
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

    TH1 *FinalProj = fRadReader->Project(gates1,gates2,BGSubtract);
    if(FinalProj == nullptr) return;

    FinalProj->GetXaxis()->SetTitle(GetXaxis()->GetTitle());
    FinalProj->GetXaxis()->SetTitleSize(GetXaxis()->GetTitleSize());
    FinalProj->GetXaxis()->SetTitleOffset(GetXaxis()->GetTitleOffset());
    FinalProj->GetXaxis()->SetTitleFont(GetXaxis()->GetTitleFont());
    FinalProj->GetXaxis()->SetLabelFont(GetXaxis()->GetLabelFont());

    FinalProj->GetYaxis()->SetTitle(GetYaxis()->GetTitle());
    FinalProj->GetYaxis()->SetTitleSize(GetYaxis()->GetTitleSize());
    FinalProj->GetYaxis()->SetTitleOffset(GetYaxis()->GetTitleOffset());
    FinalProj->GetYaxis()->SetTitleFont(GetYaxis()->GetTitleFont());
    FinalProj->GetYaxis()->SetLabelFont(GetYaxis()->GetLabelFont());

    if(Rebin>1) {
        FinalProj->Rebin(Rebin);
        FinalProj->SetYTitle(Form("Counts (%g keV/bin)",FinalProj->GetBinWidth(1)));
    }

    fProjPad->cd();
    FinalProj->Draw("hist");

    if(FixRange)
        FinalProj->GetXaxis()->SetRangeUser(SavedRange[0],SavedRange[1]);
    else
        FinalProj->GetXaxis()->UnZoom();

    gErrorIgnoreLevel = kPrint; // to recover warnings

    gbash_color->InfoMessage(Form("Integral of projection: %g",FinalProj->Integral()));

    fProjPad->Update();
    fProjPad->Modified();

    fProjPad->SetBit(TPad::kCannotMove);
    fProjPad->GetFrame()->SetBit(TObject::kCannotPick);
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
        if( KeySym == kKey_l ) {
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
        if(!fAddNewGate1 && !fAddNewGate2)
            break;

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
