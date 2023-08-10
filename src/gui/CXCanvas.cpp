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

#include <unistd.h>

#include "CXCanvas.h"
#include "TROOT.h"
#include "TBox.h"
#include "TAxis.h"
#include "TContextMenu.h"
#include "TGWindow.h"
#include "KeySymbols.h"
#include "TMath.h"
#include "TProfile.h"
#include "TF1.h"
#include "TGMsgBox.h"
#include "CXHistoManipulator.h"
#include "CXGateBox.h"
#include "TPadPainter.h"
#include "TFrame.h"
#include "TCanvasImp.h"
#include "TSystem.h"
#include "TGFileDialog.h"

#include <Riostream.h>

#include <iostream>
#include <fstream>

#include "CXMainWindow.h"
#include "CXRadCubeTH1Proj.h"
#include "TVirtualX.h"

using namespace std;

TObject* gCopyObject = nullptr;
TString gDrawOptions = "";

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>CXCanvas</h2>
TCanvas with mouse-controlled dynamic zoom and pan & scan.<br>

<img alt="CXCanvas" src="http://indra.in2p3.fr/KaliVedaDoc/images/CXCanvas.png"><br><br>

Dynamic zoom: left-click to draw rectangle around region of interest, release mouse button to zoom in<br>
Zoom in: mouse wheel up<br>
Zoom out: mouse wheel down<br>
Pan: hold down mouse wheel (centre button) and move
<!-- */
// --> END_HTML
// KEYBOARD SHORTCUTS:
//<ctrl> e   show editor
//<ctrl> f   start fit panel (TH1)
//<ctrl> g   set/unset grid on X and Y axes
//<ctrl> i   show shortcuts infos
//<ctrl> l   set/unset log scale on Y axis (TH1) or Z axis (TH2)
//<ctrl> n   normalize drawn histogram to its integral
//<ctrl> p x   draw profile X (TH2)
//<ctrl> p y   draw profile Y (TH2)
//<ctrl> s   save canvas as
//<ctrl> u   update canvas
//<ctrl> v   set/unset 'vener' mode (TH2)
//<ctrl> w   set/unset 'Age Of Empire' mode (TH2)
//<ctrl> +   set minimum +1 (TH2)
//<ctrl> -   set minimum -1 (TH2)
//      F9   set/unset log scale on X axis
//     F10   set/unset log scale on X axis
//     F11   set/unset log scale on X axis
//     F12   unzoom
//  Arrows   move on histogram or axis
////////////////////////////////////////////////////////////////////////////////

#include "TGClient.h"

//________________________________________________________________
CXCanvas::CXCanvas()
{
    fAgeOfEmpireMode = false;
    fVenerMode   = false;
    fHasDisabledClasses = false;
    fDisabledClasses = "";
    fFreezed = kFALSE;
    fPPressed = kFALSE;
    fJPressed = kFALSE;
    fSelectedHisto = nullptr;
    ScaleFact = 0.1;
    InitInfos();

    // Default constructor
}

//________________________________________________________________
CXCanvas::~CXCanvas()
{
    gCopyObject = nullptr;
    // Destructor

    gClient->Disconnect("ProcessedEvent(Event_t *, Window_t)",this,"EventProcessed(Event_t *, Window_t)");
}

//________________________________________________________________
CXCanvas::CXCanvas(const char* name, const char* title, Int_t ww, Int_t wh, Bool_t): TCanvas(name, title, ww, wh)
{
    fAgeOfEmpireMode = false;
    fVenerMode   = false;
    fHasDisabledClasses = false;
    fDisabledClasses = "";
    fFreezed = kFALSE;
    fPPressed = kFALSE;
    fJPressed = kFALSE;
    fSelectedHisto = nullptr;
    ScaleFact = 0.1;
    InitInfos();

    gClient->Connect("ProcessedEvent(Event_t *, Window_t)","CXCanvas",this,"EventProcessed(Event_t *, Window_t)");

}

//________________________________________________________________
CXCanvas::CXCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid): TCanvas(name, ww, wh, winid)
{
    fAgeOfEmpireMode = false;
    fVenerMode   = false;
    fHasDisabledClasses = false;
    fHasDisabledObject = false;
    fDisabledClasses = "";
    fFreezed = kFALSE;
    fPPressed = kFALSE;
    fJPressed = kFALSE;
    fSelectedHisto = nullptr;
    ScaleFact = 0.1;
    InitInfos();

    gClient->Connect("ProcessedEvent(Event_t *, Window_t)","CXCanvas",this,"EventProcessed(Event_t *, Window_t)");
}

void CXCanvas::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

//________________________________________________________________
bool CXCanvas::IsLogz()
{
    return fLogz;
}

//________________________________________________________________
bool CXCanvas::IsLogy()
{
    return fLogy;
}

//________________________________________________________________
bool CXCanvas::IsLogx()
{
    return fLogx;
}

//______________________________________________________________________________
void CXCanvas::RunAutoExec()
{
    // Execute the list of TExecs in the current pad.

    if (!TestBit(kAutoExec)) return;
    if (!gPad) return;
    dynamic_cast<TPad*>(gPad)->AutoExec();
}

//______________________________________________________________________________
void CXCanvas::DisableClass(const char* className)
{
    fHasDisabledClasses = true;
    fDisabledClasses += className;

}

//______________________________________________________________________________
void CXCanvas::ResetDisabledClass()
{
    fHasDisabledClasses = false;
    fDisabledClasses = "";
}

//______________________________________________________________________________
void CXCanvas::ResetDisabledObject()
{
    fHasDisabledObject = true;
    fDisabledObjects.Clear();
}

//______________________________________________________________________________
void CXCanvas::DisableObject(TObject* obj)
{
    if (!obj) return;
    fHasDisabledObject = true;
    fDisabledObjects.AddLast(obj);
}

//______________________________________________________________________________
void CXCanvas::DrawEventStatus(Int_t event, Int_t px, Int_t py, TObject* selected)
{
    // Report name and title of primitive below the cursor.
    //
    //    This function is called when the option "Event Status"
    //    in the canvas menu "Options" is selected.

    const Int_t kTMAX=256;
    static char atext[kTMAX];

    if (!TestBit(kShowEventStatus) || !selected) return;

    if (!fCanvasImp) return; //this may happen when closing a TAttCanvas

    TVirtualPad* savepad;
    savepad = gPad;
    gPad = GetSelectedPad();

    fCanvasImp->SetStatusText(selected->GetTitle(),0);
    fCanvasImp->SetStatusText(selected->GetName(),1);
    if (event == kKeyPress)
        snprintf(atext, kTMAX, "%c", (char) px);
    else
        snprintf(atext, kTMAX, "%d,%d", px, py);
    fCanvasImp->SetStatusText(atext,2);
    fCanvasImp->SetStatusText(selected->GetObjectInfo(px,py),3);
    gPad = savepad;
}

//______________________________________________________________________________
void CXCanvas::HandleInput(EEventType event, Int_t px, Int_t py)
{
    // Handle Input Events.
    //
    //  Handle input events, like button up/down in current canvas.

    if (fFreezed) return;

    TPad*    pad;
    TPad*    prevSelPad = (TPad*) fSelectedPad;
    TObject* prevSelObj = fSelected;

    fPadSave = dynamic_cast<TPad*>(gPad);
    cd();        // make sure this canvas is the current canvas

    fEvent  = event;
    fEventX = px;
    fEventY = py;

    Int_t sign = 0;
    Bool_t sendOrder = true;

    if (fHasDisabledClasses && fSelected) {
        if (fDisabledClasses.Contains(fSelected->ClassName())) sendOrder = false;
    }

    if (fHasDisabledObject && fSelected) {
        if (fDisabledObjects.Contains(fSelected)) sendOrder = false;
    }

    switch (event) {

    case kMouseMotion:
        // highlight object tracked over
        pad = Pick(px, py, prevSelObj);
        if (!pad) return;

        EnterLeave(prevSelPad, prevSelObj);

        gPad = pad;   // don't use cd() we will use the current
        // canvas via the GetCanvas member and not via
        // gPad->GetCanvas

        if (sendOrder) fSelected->ExecuteEvent(event, px, py);

        RunAutoExec();

        if (fAgeOfEmpireMode && (fSelected->InheritsFrom("TH2"))) {
            TH2* TheHisto = dynamic_cast<TH2*>(FindHisto()); //fSelected;

            Double_t size = 0.4 - 0.35 * fVenerMode;

            Double_t XCenter = (fCanvas->GetUxmax()+fCanvas->GetUxmin())*0.5;
            Double_t YCenter = (fCanvas->GetUymax()+fCanvas->GetUymin())*0.5;

            Double_t Width = fCanvas->GetUxmax()-fCanvas->GetUxmin();
            Double_t Height = fCanvas->GetUymax()-fCanvas->GetUymin();

            Double_t ppx = AbsPixeltoX(px);
            Double_t ppy = AbsPixeltoY(py);

            Double_t ddXX = XCenter - ppx;
            Double_t ddYY = YCenter - ppy;

            Double_t distX = TMath::Abs(ddXX) / Width;
            Double_t distY = TMath::Abs(ddYY) / Height;

            if (distX >= 0.5 || distY>=0.5) return;
            if ((distX <= size) && (distY <= size)) return;


            Int_t dX = 0;
            Int_t dY = 0;

            TAxis* ax = TheHisto->GetXaxis();
            Int_t X0 = ax->GetFirst();
            Int_t X1 = ax->GetLast();
            px = ax->FindBin(ppx);

            Double_t ddX   = (X1 + X0) * 0.5 - px;

            TAxis* ay = TheHisto->GetYaxis();
            Int_t Y0 = ay->GetFirst();
            Int_t Y1 = ay->GetLast();
            py = ay->FindBin(ppy);

            Double_t ddY   = (Y1 + Y0) * 0.5 - py;

            dX = TMath::Nint(ddX * (0.05 + 0.05 * fVenerMode));
            dY = TMath::Nint(ddY * (0.05 + 0.05 * fVenerMode));

            if (TMath::Abs(dX) < 1) dX = TMath::Sign(1., ddX);
            if (TMath::Abs(dY) < 1) dY = TMath::Sign(1., ddY);

            Bool_t up = false;
            if (distX > size && (X0 - dX > 0) && (X1 - dX < TheHisto->GetNbinsX())) {
                TheHisto->GetXaxis()->SetRange(X0 - dX, X1 - dX);
                up = true;
            }
            if (distY > size && (Y0 - dY > 0) && (Y1 - dY < TheHisto->GetNbinsY())) {
                TheHisto->GetYaxis()->SetRange(Y0 - dY, Y1 - dY);
                up = true;
            }
            if (up) {
                Modified();
                Update();
                usleep(100000);
                gSystem->ProcessEvents();
                HandleInput(event, fCanvas->GetEventX(), fCanvas->GetEventY());
            }
        }

        break;

    case kMouseEnter:
        // mouse enters canvas
        if (!fDoubleBuffer) FeedbackMode(kTRUE);
        break;

    case kMouseLeave: {
        // force popdown of tooltips
        TObject* sobj = fSelected;
        TPad*    spad = fSelectedPad;
        fSelected     = nullptr;
        fSelectedPad  = nullptr;
        EnterLeave(prevSelPad, prevSelObj);
        fSelected     = sobj;
        fSelectedPad  = spad;
        if (!fDoubleBuffer) FeedbackMode(kFALSE);
        break;
    }
    case kButton1Double:{
        auto *padsave = gROOT->GetSelectedPad();
        gPad = padsave;
        auto *hist = FindHisto();
        if(fMainWindow->IsShiftOn() && hist) {
            if(!fMainWindow->IsCtrlOn()) {
                hist->GetXaxis()->UnZoom();
                hist->GetYaxis()->UnZoom();

                gPad->Update();
                gPad->Modified();
                gROOT->SetSelectedPad(padsave);
            }
            else {
                TObject *obj = nullptr;
                TH1 *hist = nullptr;
                TIter    next(fPrimitives);
                while ((obj = next())) {
                    if (obj->InheritsFrom(TPad::Class())) {
                        hist = FindHisto(dynamic_cast<TVirtualPad*>(obj));
                        if(hist){
                            hist->GetXaxis()->UnZoom();
                            hist->GetYaxis()->UnZoom();
                            dynamic_cast<TVirtualPad*>(obj)->Update();
                            dynamic_cast<TVirtualPad*>(obj)->Modified();
                        }
                    }
                }
            }
            moved1D = false;
            if(fZoomBox) {
                delete fZoomBox;
                fZoomBox = nullptr;
            }
            button1double=true;
            break;
        }
    }
    case kButton1Shift:
    case kButton1Down: {
        gROOT->SetSelectedPad(fSelectedPad);
        auto *padsave = gROOT->GetSelectedPad();

        // find pad in which input occured
        pad = Pick(px, py, prevSelObj);
        if (!pad) return;

        gROOT->SetSelectedPad(padsave);
        fSelectedPad = (TPad*) padsave;
        gPad = fSelectedPad;   // don't use cd() because we won't draw in pad
        // we will only use its coordinate system

        if (fSelected) {
            FeedbackMode(kTRUE);   // to draw in rubberband mode
            fSelected->ExecuteEvent(event, px, py);

            RunAutoExec();
        }
    }
    break;
    case kButton1Motion:
    case kButton1ShiftMotion: {
        if (fSelected && !moved1D) {

            gPad = fSelectedPad;

            if (sendOrder) fSelected->ExecuteEvent(event, px, py);
            gVirtualX->Update();

            if (!fSelected->InheritsFrom(TAxis::Class())) {
                Bool_t resize = kFALSE;
                if (fSelected->InheritsFrom(TBox::Class()))
                    resize = dynamic_cast<TBox*>(fSelected)->IsBeingResized();
                if (fSelected->InheritsFrom(TVirtualPad::Class()))
                    resize = dynamic_cast<TVirtualPad*>(fSelected)->IsBeingResized();

                if ((!resize && TestBit(kMoveOpaque)) || (resize && TestBit(kResizeOpaque))) {
                    gPad = fPadSave;
                    Update();
                    FeedbackMode(kTRUE);
                }
            }

            RunAutoExec();
        }
        auto *hist = FindHisto();
        if (fMainWindow->IsShiftOn() && fSelected && hist && hist->GetDimension() == 1) {
            auto *padsave = gROOT->GetSelectedPad();
            gPad = padsave;
            if(!moved1D) {
                moved1D = true;

                oldx = gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX());
                oldy = gPad->AbsPixeltoY(gPad->GetCanvas()->GetEventY());
                xinit = oldx;
                yinit = oldy;
                dynamic_cast<TPadPainter*>(gPad->GetPainter())->DrawBox(xinit,yinit,xinit,yinit, TVirtualPadPainter::kHollow);

                fZoomBox = new TBox(xinit, yinit, xinit, yinit);
                Int_t ci = TColor::GetColor("#7d7dff");
                TColor *zoomcolor = gROOT->GetColor(ci);
                if (!SupportAlpha() || !zoomcolor)
                    fZoomBox->SetFillStyle(3002);
                else
                    zoomcolor->SetAlpha(0.5);

                fZoomBox->SetFillColor(ci);
                fZoomBox->Draw();

                gPad->Modified();
                gPad->Update();
                gROOT->SetSelectedPad(padsave);
                gVirtualX->SetLineWidth(1);
            }
            if(moved1D) {
                gPad = fSelectedPad;

                gPad->GetPainter()->SetFillStyle(kFDotted1);
                gPad->GetPainter()->SetFillColor(kBlue);

                dynamic_cast<TPadPainter*>(gPad->GetPainter())->DrawBox(xinit,yinit,oldx,oldy, TVirtualPadPainter::kHollow);
                oldx = gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX());
                oldy = gPad->AbsPixeltoY(gPad->GetCanvas()->GetEventY());

                if (fZoomBox){
                    fZoomBox->SetX1(xinit);
                    fZoomBox->SetY1(yinit);
                    fZoomBox->SetX2(oldx);
                    fZoomBox->SetY2(oldy);
                }

                gPad->Modified();
                gPad->Update();
                gROOT->SetSelectedPad(fSelectedPad);
            }
        }
        break;
    }
    case kButton1Up: {
        if (fSelected) {
            gPad = fSelectedPad;

            if (sendOrder) fSelected->ExecuteEvent(event, px, py);

            RunAutoExec();

            if (fPadSave)
                gPad = fPadSave;
            else {
                gPad     = this;
                fPadSave = this;
            }

            auto *hist = FindHisto();
            if(hist && hist->GetDimension()==1 && !button1double) {
                if(!moved1D && fMainWindow->IsShiftOn()) {
                    TAxis* ax =  nullptr;
                    if(!fMainWindow->IsCtrlOn()) {
                        ax = FindHisto()->GetXaxis();
                        ax->SetRangeUser(ax->GetBinLowEdge(ax->GetFirst()), gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX()));

                        gPad->Update();
                        gPad->Modified();
                        gROOT->SetSelectedPad(fSelectedPad);
                    }
                    else {
                        TObject *obj = nullptr;
                        TH1 *hist = nullptr;
                        TIter    next(fPrimitives);
                        while ((obj = next())) {
                            if (obj->InheritsFrom(TPad::Class())) {
                                hist = FindHisto(dynamic_cast<TVirtualPad*>(obj));
                                if(hist){
                                    ax = hist->GetXaxis();
                                    ax->SetRangeUser(ax->GetBinLowEdge(ax->GetFirst()), gPad->AbsPixeltoX(gPad->GetCanvas()->GetEventX()));
                                    dynamic_cast<TVirtualPad*>(obj)->Update();
                                    dynamic_cast<TVirtualPad*>(obj)->Modified();
                                }
                            }
                        }
                    }
                }
                if(moved1D) {
                    xmin = xinit;
                    xmax = oldx;

                    Double_t toto = 0;
                    if (xmax < xmin) {
                        toto = xmax;
                        xmax = xmin;
                        xmin = toto;
                    }

                    Double_t ratio1 = (xmin - GetUxmin()) / (GetUxmax() - GetUxmin());
                    Double_t ratio2 = (xmax - GetUxmin()) / (GetUxmax() - GetUxmin());

                    if((ratio2 - ratio1 > 0.05)) {
                        TAxis* ax =  nullptr;

                        if(!fMainWindow->IsCtrlOn()) {
                            ax = FindHisto()->GetXaxis();
                            ax->SetRangeUser(xmin, xmax);

                            gPad->Update();
                            gPad->Modified();
                            gROOT->SetSelectedPad(fSelectedPad);
                        }
                        else {
                            TObject *obj = nullptr;
                            TH1 *hist = nullptr;
                            TIter    next(fPrimitives);
                            while ((obj = next())) {
                                if (obj->InheritsFrom(TPad::Class())) {
                                    hist = FindHisto(dynamic_cast<TVirtualPad*>(obj));
                                    if(hist){
                                        ax = hist->GetXaxis();
                                        ax->SetRangeUser(xmin, xmax);
                                        dynamic_cast<TVirtualPad*>(obj)->Update();
                                        dynamic_cast<TVirtualPad*>(obj)->Modified();
                                    }
                                }
                            }
                        }
                    }

                    moved1D = false;

                    xmax = xmin = ymax = ymin = 0.;

                    delete fZoomBox;
                    fZoomBox = nullptr;
                }
            }
            button1double=false;

            Update();    // before calling update make sure gPad is reset
            gROOT->SetSelectedPad(fSelectedPad);
        }
        break;
    }
    case kButton2Down:
        // find pad in which input occured
        pad = Pick(px, py, prevSelObj);
        if (!pad) return;

        gPad = pad;   // don't use cd() because we won't draw in pad
        // we will only use its coordinate system

        FeedbackMode(kTRUE);

        if (!fSelected->InheritsFrom("TH1")) fSelected->Pop();             // pop object to foreground
        pad->cd();                                 // and make its pad the current pad

        if (fSelected->InheritsFrom("TH2") && !fSelected->InheritsFrom("TH3")) {
            // implement pan & scan
            X0 = px;
            Y0 = py;  // u clikd here
            theXaxis = dynamic_cast<TH2*>(FindHisto())->GetXaxis();
            theYaxis = dynamic_cast<TH2*>(FindHisto())->GetYaxis();
            NXbins = theXaxis->GetNbins();  // maximum bin number in X
            NYbins = theYaxis->GetNbins();  // maximum bin number in Y
            Xf1 = Xfirst0 = theXaxis->GetFirst(); // initial displayed bin range in X
            Xl1 = Xlast0 = theXaxis->GetLast();
            Yf1 = Yfirst0 = theYaxis->GetFirst(); // initial displayed bin range in Y
            Yl1 = Ylast0 = theYaxis->GetLast();
            // size of axes in pixels
            Int_t pixelWidthX = gPad->XtoAbsPixel(gPad->GetUxmax()) - gPad->XtoAbsPixel(gPad->GetUxmin());
            Int_t pixelWidthY = gPad->YtoAbsPixel(gPad->GetUymax()) - gPad->YtoAbsPixel(gPad->GetUymin());
            // sizes of bins in pixels
            NdisXbins = Xlast0 - Xfirst0 + 1;
            NdisYbins = Ylast0 - Yfirst0 + 1;
            XbinPixel = pixelWidthX / (1.0 * NdisXbins);
            YbinPixel = pixelWidthY / (1.0 * NdisYbins);
        }
        if (gDebug)
            printf("Current Pad: %s / %s\n", pad->GetName(), pad->GetTitle());

        // loop over all canvases to make sure that only one pad is highlighted
        {
            TIter next(gROOT->GetListOfCanvases());
            TCanvas* tc;
            while ((tc = dynamic_cast<TCanvas*>(next())))
                tc->Update();
        }

        /*if (pad->GetGLDevice() != -1 && fSelected)
          fSelected->ExecuteEvent(event, px, py);*/

        break;   // don't want fPadSave->cd() to be executed at the end

    case kButton2Motion:
        //was empty!
        if (fSelected && fSelected->InheritsFrom("TH2") && !fSelected->InheritsFrom("TH3")) {
            // implement pan & scan
            Int_t dX = px - X0; // how far have i moved ?
            Int_t dY = py - Y0;
            Int_t dXbins = dX / XbinPixel;
            Int_t dYbins = dY / YbinPixel;
            Bool_t changed = kFALSE;
            Int_t newXfirst = Xfirst0 - dXbins;
            Int_t newXlast;
            if (newXfirst < 1) {
                newXfirst = 1;
                newXlast = NdisXbins;
            } else {
                newXlast = Xlast0 - dXbins;
                if (newXlast > NXbins) {
                    newXlast = NXbins;
                    newXfirst = newXlast - NdisXbins + 1;
                }
            }
            if (newXfirst != Xf1) {
                Xf1 = newXfirst;
                Xl1 = newXlast;
                theXaxis->SetRange(Xf1, Xl1);
                changed = kTRUE;
            }
            Int_t newYfirst = Yfirst0 - dYbins;
            Int_t newYlast;
            if (newYfirst < 1) {
                newYfirst = 1;
                newYlast = NdisYbins;
            } else {
                newYlast = Ylast0 - dYbins;
                if (newYlast > NYbins) {
                    newYlast = NYbins;
                    newYfirst = newYlast - NdisYbins + 1;
                }
            }
            if (newYfirst != Yf1) {
                Yf1 = newYfirst;
                Yl1 = newYlast;
                theYaxis->SetRange(Yf1, Yl1);
                changed = kTRUE;
            }
            if (changed) {
                Modified();
                Update();
            }
        }

        /* fall through */

    case kButton2Up:
        if (fSelected) {
            gPad = fSelectedPad;

            if (sendOrder) fSelected->ExecuteEvent(event, px, py);
            RunAutoExec();
        }
        break;

    case kButton2Double:
        break;

    case kButton3Down:
        // popup context menu
        pad = Pick(px, py, prevSelObj);
        if (!pad) return;

        if (!fDoubleBuffer) FeedbackMode(kFALSE);

        if (fContextMenu && !fSelected->TestBit(kNoContextMenu) && !pad->TestBit(kNoContextMenu) && !TestBit(kNoContextMenu)) {
            if (sendOrder) fContextMenu->Popup(px, py, fSelected, this, pad);
            else fSelected->ExecuteEvent(event, px, py);
        }

        break;

    case kButton3Motion:
        break;

    case kButton3Up:
        if (!fDoubleBuffer) FeedbackMode(kTRUE);
        break;

    case kButton3Double:
        break;

    case kKeyDown:
        //       Info("HandleInput","Key down: %d %d",px,py);
        break;

    case kKeyUp:
        //       Info("HandleInput","Key up: %d %d",px,py);
        break;
    case kArrowKeyRelease:

        event = kKeyPress;

        if(px<fLastX)
            py = kKey_Left;
        else if(px>fLastX)
            py = kKey_Right;
        else if(py<fLastY)
            py = kKey_Up;
        else if(py>fLastY)
            py = kKey_Down;

        /* fall through */

    case kKeyPress:
        if (!fSelectedPad || !fSelected) return;
        gPad = fSelectedPad;   // don't use cd() because we won't draw in pad
        // we will only use its coordinate system
        fSelected->ExecuteEvent(event, px, py);

        HandleKey(px, py);
        RunAutoExec();

        break;
    case kWheelUp:
    case kWheelDown:
        pad = Pick(px, py, prevSelObj);
        if (!pad) return;

        sign = (event == kWheelUp ? 1 : -1);

        if( fMainWindow->IsShiftOn() && fSelected && fSelected->InheritsFrom(TH1::Class())) {
            fSelectedHisto = (TH1*)fSelected;
            if(fSelectedHisto->GetDimension()==1) {
                Float_t Scale = 1.+sign*ScaleFact;
                fSelectedHisto->Scale(Scale);
                Modified();
                Update();
                break;
            }
            fSelectedHisto = nullptr;
        }

        gPad = pad;
        if (fSelected && fSelected->InheritsFrom(TAxis::Class())) fSelected->ExecuteEvent(event, px, py);
        else if (fSelected && fSelected->InheritsFrom(TH2::Class())) DynamicZoom(sign, px, py);

        RunAutoExec();

        break;
    default:
        break;
    }

    //    if (fPadSave && event != kButton2Down)
    //        fPadSave->cd();

    if (event != kMouseLeave) { // signal was already emitted for this event
        ProcessedEvent(event, px, py, fSelected);  // emit signal
        DrawEventStatus(event, px, py, fSelected);
    }

    if(event == kMouseMotion || event == kArrowKeyRelease)
    {
        fLastX = px;
        fLastY = py;
    }

    fLastEvent = event;
}

//________________________________________________________________
void CXCanvas::ZoomSelected(TH2* TheHisto)
{
    if (!TheHisto) return;
    TAxis* ax = TheHisto->GetXaxis();

    Double_t ratio1 = (xmin - GetUxmin()) / (GetUxmax() - GetUxmin());
    Double_t ratio2 = (xmax - GetUxmin()) / (GetUxmax() - GetUxmin());
    if ((ratio2 - ratio1 > 0.05)) ax->SetRangeUser(xmin, xmax);

    ax = TheHisto->GetYaxis();

    ratio1 = (ymin - GetUymin()) / (GetUymax() - GetUymin());
    ratio2 = (ymax - GetUymin()) / (GetUymax() - GetUymin());
    if ((ratio2 - ratio1 > 0.05)) ax->SetRangeUser(ymin, ymax);

    xmax = xmin = ymax = ymin = 0.;
}

//________________________________________________________________
void CXCanvas::DynamicZoomTH1(Int_t Sign, Int_t px, Int_t)
{
    // Zoom in or out of histogram with mouse wheel

    if (!fSelected) return;
    TH1* TheHisto = (TH1*) FindHisto();//fSelected;

    Double_t percent = 0.15 - Sign * 0.05;

    Int_t dX = 0;

    px = AbsPixeltoX(px);

    TAxis* ax = TheHisto->GetXaxis();
    Int_t NbinsXtmp = ax->GetNbins();
    Int_t X0tmp = ax->GetFirst();
    Int_t X1tmp = ax->GetLast();
    Int_t step = TMath::Min(TMath::Max(1, (Int_t)(percent * (X1tmp - X0tmp))), NbinsXtmp / 2);
    step *= Sign;
    X0tmp = TMath::Min(TMath::Max(X0tmp + step, 1), X1tmp - step);
    X1tmp = TMath::Max(TMath::Min(X1tmp - step, NbinsXtmp), X0tmp);
    if (X0tmp >= X1tmp) X0tmp = X1tmp - 1;
    if (Sign > 0) dX = (Int_t)(X0tmp + (X1tmp - X0tmp) * 0.5 - ax->FindBin(px));
    if ((X0tmp - dX) < 0) ax->SetRange(0, X1tmp - X0tmp);
    else if ((X1tmp - dX) > ax->GetNbins()) ax->SetRange(ax->GetNbins() - (X1tmp - X0tmp), ax->GetNbins());
    else ax->SetRange(X0tmp - dX, X1tmp - dX);


    Modified();
    Update();
}

//________________________________________________________________
void CXCanvas::DynamicZoom(Int_t Sign, Int_t px, Int_t py)
{
    // Zoom in or out of histogram with mouse wheel

    //    Info("DynamicZoom","px=%d py=%d",px,py);

    if (!fSelected) return;
    TH2* TheHisto = dynamic_cast<TH2*>(FindHisto());//fSelected;

    Double_t percent = 0.15 - Sign * 0.05;

    Int_t dX = 0;
    Int_t dY = 0;

    Double_t ppx = AbsPixeltoX(px);
    Double_t ppy = AbsPixeltoY(py);

    TAxis* ax = TheHisto->GetXaxis();
    Int_t NbinsXtmp = ax->GetNbins();
    Int_t X0tmp = ax->GetFirst();
    Int_t X1tmp = ax->GetLast();
    Int_t step = TMath::Min(TMath::Max(1, (Int_t)(percent * (X1tmp - X0tmp))), NbinsXtmp / 2);
    step *= Sign;
    X0tmp = TMath::Min(TMath::Max(X0tmp + step, 1), X1tmp - step);
    X1tmp = TMath::Max(TMath::Min(X1tmp - step, NbinsXtmp), X0tmp);
    if (X0tmp >= X1tmp) X0tmp = X1tmp - 1;
    if (Sign > 0) dX = (Int_t)(X0tmp + (X1tmp - X0tmp) * 0.5 - ax->FindBin(ppx));
    if ((X0tmp - dX) < 0) ax->SetRange(0, X1tmp - X0tmp);
    else if ((X1tmp - dX) > ax->GetNbins()) ax->SetRange(ax->GetNbins() - (X1tmp - X0tmp), ax->GetNbins());
    else ax->SetRange(X0tmp - dX, X1tmp - dX);

    ax = TheHisto->GetYaxis();
    Int_t NbinsYtmp = ax->GetNbins();
    Int_t Y0tmp = ax->GetFirst();
    Int_t Y1tmp = ax->GetLast();
    step = TMath::Min(TMath::Max(1, (Int_t)(percent * (Y1tmp - Y0tmp))), NbinsYtmp / 2);
    step *= Sign;
    Y0tmp = TMath::Min(TMath::Max(Y0tmp + step, 1), Y1tmp - step);
    Y1tmp = TMath::Max(TMath::Min(Y1tmp - step, NbinsYtmp), Y0tmp);
    if (Y0tmp >= Y1tmp) Y0tmp = Y1tmp - 1;
    if (Sign > 0) dY = (Int_t)(Y0tmp + (Y1tmp - Y0tmp) * 0.5 - ax->FindBin(ppy));
    if ((Y0tmp - dY) < 0) ax->SetRange(0, Y1tmp - Y0tmp);
    else if ((Y1tmp - dY) > ax->GetNbins()) ax->SetRange(ax->GetNbins() - (Y1tmp - Y0tmp), ax->GetNbins());
    else ax->SetRange(Y0tmp - dY, Y1tmp - dY);

    Modified();
    Update();
}

//________________________________________________________________
Bool_t CXCanvas::HandleKey(Int_t px, Int_t py)
{
    // Handle keys

    //        Info("HandleKey","key pressed : %d %d",px,py);

    TObject* obj = nullptr;
    TIter next(GetListOfPrimitives());

    if (!fEnabledShortcuts) return kTRUE;

    TH1 *CurrentHist = nullptr;
    TH1 *CurrentHist1D = nullptr;
    TH2 *CurrentHist2D = nullptr;
    TF1 *CurrentTF1 = nullptr;

    if (fSelected->InheritsFrom(TH2::Class())) fSelected = FindHisto();
    else if (fSelected->InheritsFrom(TFrame::Class())) {
        fSelected = FindHisto();
        if(fSelected == nullptr) fSelected = this;
    }

    if (fSelected == nullptr) return kTRUE;

    if(fSelected->InheritsFrom(TH2::Class())) CurrentHist2D = dynamic_cast<TH2*>(fSelected);
    else if(fSelected->InheritsFrom(TH1::Class())) CurrentHist1D = dynamic_cast<TH1*>(fSelected);

    if(fSelected->InheritsFrom(TH1::Class())) CurrentHist = dynamic_cast<TH1*>(fSelected);
    if(fSelected->InheritsFrom(TF1::Class())) CurrentTF1 = dynamic_cast<TF1*>(fSelected);

    bool CTRL = fMainWindow->IsCtrlOn();
    bool SHIFT = fMainWindow->IsShiftOn();


    if( CurrentHist1D && !fSelectedHisto && SHIFT ) {
        fSelectedHisto = dynamic_cast<TH1*>(fSelected);
    }

    //    cout << "My Canvas: CTRL:" << CTRL << endl;

    //    cout<<CTRL<<" "<<SHIFT<<endl;

    if(CTRL) {
        switch ((EKeySym)py) {

        case kKey_a:
            break;

        case kKey_b:
            break;

        case kKey_c:
            CopyObject(fSelected);
            break;

        case kKey_d:
            UndrawObject(fSelected);
            break;

        case kKey_e:
            GetCanvasImp()->ShowEditor(!GetCanvasImp()->HasEditor());
            break;

            //        case kKey_f:
            //            if (fSelected->InheritsFrom("TH1"))(dynamic_cast<TH1*>(fSelected))->FitPanel();
            //            break;

        case kKey_g:
            if (GetGridx() && GetGridy()) {
                SetGrid(0, 0);
                while ((obj = next())) {
                    if (obj->InheritsFrom(TPad::Class())) {
                        dynamic_cast<TPad*>(obj)->SetGrid(0, 0);
                    }
                }
            } else {
                SetGrid();
                while ((obj = next())) {
                    if (obj->InheritsFrom(TPad::Class())) {
                        dynamic_cast<TPad*>(obj)->SetGrid();
                    }
                }
            }
            Modified();
            Update();
            break;

        case kKey_i:
            ShowShortcutsInfos();
            break;

        case kKey_j:
            fJPressed = kTRUE;
            return kTRUE;
            break;

        case kKey_n:
            if (CurrentHist) {
                CurrentHist->Sumw2();
                CurrentHist->Scale(1. / CurrentHist->Integral(1,CurrentHist->GetNbinsX()));
            }
            Modified();
            Update();
            break;

        case kKey_m:
            if (CurrentHist) {
                CurrentHist->Scale(1. / CurrentHist->GetMaximum());
            }
            Modified();
            Update();
            break;

        case kKey_p:
            fPPressed = kTRUE;
            return kTRUE;
            break;

        case kKey_s:
            SaveCanvasAs();
            Modified();
            Update();
            break;

        case kKey_t:
            break;

        case kKey_u:
            Modified();
            Update();
            break;

        case kKey_v:
            Paste();
            break;

        case kKey_w:
            fAgeOfEmpireMode = !fAgeOfEmpireMode;
            break;

        case kKey_x:
            if (fPPressed && CurrentHist2D) ProfileX(CurrentHist2D);
            if (fJPressed && CurrentHist2D) ProjectionX(CurrentHist2D);
            if (!fPPressed && !fJPressed) {
                CutObject(fSelected);
            }
            break;

        case kKey_y:
            if (fPPressed && CurrentHist2D) ProfileY(CurrentHist2D);
            if (fJPressed && CurrentHist2D) ProjectionY(CurrentHist2D);
            break;
        case kKey_Space:
            break;

        default:
            fPPressed = kFALSE;
            fJPressed = kFALSE;
            return kTRUE;
        }
    }
    else {
        switch ((EKeySym)py) {
        case kKey_F1:
            break;

        case kKey_F2:
            break;

        case kKey_F3:
            break;

        case kKey_F4:
            break;

        case kKey_F5:
            break;

        case kKey_F6:
            break;

        case kKey_F7:
            break;

        case kKey_F8:
            break;

        case kKey_F9:
            gPad->SetLogx(!gPad->GetLogx());
            Modified();
            Update();
            break;

        case kKey_F10:
            gPad->SetLogy(!gPad->GetLogy());
            Modified();
            Update();
            break;

        case kKey_F11:
            gPad->SetLogz(!gPad->GetLogz());
            Modified();
            Update();
            break;

        case kKey_F12:
            if (fSelected->InheritsFrom(TAxis::Class())) {
                dynamic_cast<TAxis*>(fSelected)->UnZoom();
                Modified();
                Update();
            }
            else if (FindHisto() && (FindHisto()->InheritsFrom(TH1::Class())) ) {
                if(!fMainWindow->IsCtrlOn()){
                    FindHisto()->GetYaxis()->UnZoom();
                    FindHisto()->GetXaxis()->UnZoom();
                    gPad->Modified();
                    gPad->Update();
                }
                else{
                    TObject *obj = nullptr;
                    TH1 *hist = nullptr;
                    TIter    next(fPrimitives);
                    while ((obj = next())) {
                        if (obj->InheritsFrom(TPad::Class())) {
                            hist = FindHisto(dynamic_cast<TVirtualPad*>(obj));
                            if(hist){
                                hist->GetXaxis()->UnZoom();
                                hist->GetYaxis()->UnZoom();
                                dynamic_cast<TVirtualPad*>(obj)->Update();
                                dynamic_cast<TVirtualPad*>(obj)->Modified();
                            }
                        }
                    }
                }
            }
            break;

        case kKey_Left:
            if (fSelected->InheritsFrom(TAxis::Class()) && ((TString)fSelected->GetName()) == "xaxis")    MoveAxis(FindHisto()->GetXaxis(), -1);
            else if (fSelected->InheritsFrom(TH1::Class())) MoveAxis(FindHisto()->GetXaxis(), -1);
            break;

        case kKey_Down:
            if (fSelected->InheritsFrom(TAxis::Class()) && ((TString)fSelected->GetName()) == "yaxis")    MoveAxis(dynamic_cast<TAxis*>(fSelected), -1);
            else if (fSelected->InheritsFrom(TH1::Class())) MoveAxis(FindHisto()->GetYaxis(), -1);
            break;

        case kKey_Right:
            if (fSelected->InheritsFrom(TAxis::Class()) && ((TString)fSelected->GetName()) == "xaxis")    MoveAxis(dynamic_cast<TAxis*>(fSelected), 1);
            else if (fSelected->InheritsFrom(TH1::Class())) MoveAxis(FindHisto()->GetXaxis(), 1);
            break;

        case kKey_Up:
            if (fSelected->InheritsFrom(TAxis::Class()) && ((TString)fSelected->GetName()) == "yaxis")    MoveAxis(dynamic_cast<TAxis*>(fSelected), 1);
            else if (fSelected->InheritsFrom(TH1::Class())) MoveAxis(FindHisto()->GetYaxis(), 1);
            break;

        case kKey_C:
            SetCrosshair(!GetCrosshair());
            gPad->Modified();
            gPad->Update();
            break;

        case kKey_Plus:
            if (CurrentHist2D) {
                CurrentHist2D->SetMinimum(CurrentHist2D->GetMinimum() + 1);
                gPad->Modified();
                gPad->Update();
            } else if (CurrentTF1) {
                CurrentTF1->SetNpx(CurrentTF1->GetNpx() + 50);
                gPad->Modified();
                gPad->Update();
            } else if (CurrentHist1D) {
                TIter it(CurrentHist1D->GetListOfFunctions());
                while ((obj = it())) dynamic_cast<TF1*>(obj)->SetNpx(dynamic_cast<TF1*>(obj)->GetNpx() + 50);
                gPad->Modified();
                gPad->Update();
            }

            ScaleFact *=2;
            if(ScaleFact>0.99)
                ScaleFact = 0.99;

            break;

        case kKey_Minus:
            if (CurrentHist2D) {
                if (CurrentHist2D->GetMinimum() > 0) CurrentHist2D->SetMinimum(CurrentHist2D->GetMinimum() - 1);
                Modified();
                Update();
            } else if (CurrentTF1) {
                CurrentTF1->SetNpx(CurrentTF1->GetNpx() - 50);
                Modified();
                Update();
            } else if (CurrentHist1D) {
                TIter it(CurrentHist1D->GetListOfFunctions());
                while ((obj = it())) dynamic_cast<TF1*>(obj)->SetNpx(dynamic_cast<TF1*>(obj)->GetNpx() - 50);
                Modified();
                Update();
            }

            ScaleFact *=0.5;

            if(ScaleFact<0.01)
                ScaleFact = 0.01;

            break;

        default:
            ;
        }
    }

    fPPressed = kFALSE;
    fJPressed = kFALSE;
    return kTRUE;
}

void CXCanvas::CopyObject(TObject *obj)
{
    if(obj == nullptr) return;

    if (gCopyObject) {
        gCopyObject->Delete();
        gCopyObject = nullptr;
    }
    if (!obj->InheritsFrom(CXCanvas::Class())) {
        if(obj->InheritsFrom(CXRadCubeTH1Proj::Class()))
            obj = dynamic_cast<CXRadCubeTH1Proj*>(obj)->GetTotalProj();
        gCopyObject = obj->Clone();
        gDrawOptions = obj->GetDrawOption();
    }
}

void CXCanvas::UndrawObject(TObject *obj)
{
    if(obj == nullptr) return;
    if (obj->InheritsFrom(TF1::Class())) {
        TH1* hh = nullptr;
        if ((hh = FindHisto())) hh->GetListOfFunctions()->Remove(obj);
    } else GetListOfPrimitives()->Remove(obj);


    bool firsthist=true;
    for(int i=0 ; i<fPrimitives->GetEntries() ; i++){
        TObject *o = fPrimitives->At(i);
        if(o->InheritsFrom("TH1")){
            TH1 *h = dynamic_cast<TH1*>(o);
            if(((TString)h->GetDrawOption()).Contains("same") && firsthist)
                h->SetDrawOption(((TString)h->GetDrawOption()).ReplaceAll("same",""));
            firsthist=false;
        }
    }

    Modified();
    Update();
}

void CXCanvas::Paste()
{
    if (gCopyObject) {
        cd();
        Modified();
        Update();
        TString option = gDrawOptions;
        if (gCopyObject->InheritsFrom("TH1") && (FindHisto() || FindGraph() )) option += " same";
        if (gCopyObject->InheritsFrom("TGraph") && (FindHisto() || FindGraph() )) option += "a";

        gCopyObject->Draw(option.Data());
        Modified();
        Update();
        gPad->GetFrame()->SetBit(TObject::kCannotPick);
        gCopyObject = nullptr;
    }
}

void CXCanvas::CutObject(TObject *obj)
{
    CopyObject(obj);
    UndrawObject(obj);
}

void CXCanvas::MoveAxis(TAxis* ax, Int_t sign)
{
    Int_t nBins = ax->GetNbins();
    Int_t first = ax->GetFirst();
    Int_t last  = ax->GetLast();

    if(!FindHisto()->InheritsFrom("TH2"))
        ax->UnZoom();

    Int_t dX = (last - first) / 10;
    if (dX == 0) dX++;

    if ((last + 2 < nBins) && (sign > 0)) {
        ax->SetRange(first + dX, last + dX);
        Modified();
        Update();
    }
    if ((first - 2 >= 0) && (sign < 0)) {
        ax->SetRange(first - dX, last - dX);
        Modified();
        Update();
    }
}

void CXCanvas::AddShortcutsInfo(const char* cut, const char* desc)
{
    fShortCuts.AddLast(new TNamed(cut, desc));
}

void CXCanvas::ShowShortcutsInfos()
{
    std::cout << std::endl << std::endl;
    TNamed* info = nullptr;
    TIter it(&fShortCuts);
    while ((info = dynamic_cast<TNamed*>(it()))) {
        std::cout << Form("%20s", info->GetName()) << "   " << info->GetTitle() << std::endl;
    }
    std::cout << std::endl;
}

void CXCanvas::SetVenerMode(Int_t value)
{
    fVenerMode = value;
}

void CXCanvas::SetAgeOfEmpireMode(Int_t value)
{
    fAgeOfEmpireMode = value;
}

void CXCanvas::InitInfos()
{
    fEnabledShortcuts = 1;
    fSavedAs = "";

    AddShortcutsInfo("n", "new Canvas");
    AddShortcutsInfo("", "");
    AddShortcutsInfo("<ctrl> c", "copy the object under cursor");
    AddShortcutsInfo("<ctrl> x", "cut the object under cursor");
    AddShortcutsInfo("<ctrl> v", "paste the last object copied");
    AddShortcutsInfo("<ctrl> d", "undraw the object under cursor (object not deleted)");
    AddShortcutsInfo("<Maj>  S", "Add selected object to stored spectra");

    AddShortcutsInfo("","");
    AddShortcutsInfo("s", "Peak search");
    AddShortcutsInfo("f", "define a new gamma fit");
    AddShortcutsInfo("<ctrl> f", "Fit");
    AddShortcutsInfo("<ctrl> a", "Calibrate");
    AddShortcutsInfo("c", "clear the current Pad (arrows, fits...)");
    AddShortcutsInfo("","");
    AddShortcutsInfo("Gamma Gamma mode","");
    AddShortcutsInfo("g", "Add a gate");
    AddShortcutsInfo("b", "Add a background");
    AddShortcutsInfo("c", "Clear all gates and background");
    AddShortcutsInfo("d", "Remove the gate under the cursor");
    AddShortcutsInfo("p", "Do the projection");
    AddShortcutsInfo("","");
    AddShortcutsInfo("<ctrl> f", "start the standarf fit panel");
    AddShortcutsInfo("","");
    AddShortcutsInfo("<ctrl> g", "set/unset grid on X and Y axes");
    AddShortcutsInfo("<Maj>  C", "set/unset CrossHair (wheel click to measure distances)");
    AddShortcutsInfo("","");
    AddShortcutsInfo("<ctrl> n", "normalize the histogram in area");
    AddShortcutsInfo("<ctrl> m", "normalize the histogram to the maximum");
    AddShortcutsInfo("","");
    AddShortcutsInfo("<ctrl> j x", "draw projection X (TH2)");
    AddShortcutsInfo("<ctrl> j y", "draw projection Y (TH2)");
    AddShortcutsInfo("<ctrl> p x", "draw profile X (TH2)");
    AddShortcutsInfo("<ctrl> p y", "draw profile Y (TH2)");
    AddShortcutsInfo("","");
    AddShortcutsInfo("<ctrl> s", "save canvas as");
    AddShortcutsInfo("<ctrl> u", "update canvas");
    AddShortcutsInfo("","");
    AddShortcutsInfo("<ctrl> +", "set minimum +1 (TH2)");
    AddShortcutsInfo("<ctrl> -", "set minimum -1 (TH2)");
    AddShortcutsInfo("","");
    AddShortcutsInfo("<ctrl> w", "set/unset 'Age Of Empire' mode (TH2)");
    AddShortcutsInfo("","");
    AddShortcutsInfo("F9",  "set/unset log scale on X axis");
    AddShortcutsInfo("F10", "set/unset log scale on Y axis");
    AddShortcutsInfo("F11", "set/unset log scale on Z axis");
    AddShortcutsInfo("","");
    AddShortcutsInfo("Arrows", "move on histogram or axis");
    AddShortcutsInfo("F12", "unzoom");
    AddShortcutsInfo("","");
    AddShortcutsInfo("<ctrl> i", "show this shortcuts infos");
}

void CXCanvas::ProfileX(TH2* hh)
{
    TObject* pfx = nullptr;
    if ((pfx = FindObject(Form("%s_pfx", hh->GetName())))) pfx->Delete();
    hh->ProfileX("_pfx", 1, -1, "i,d,same");
    if ((pfx = FindObject(Form("%s_pfx", hh->GetName())))) dynamic_cast<TProfile*>(pfx)->SetLineColor(kBlack);
    Modified();
    Update();
}

void CXCanvas::ProfileY(TH2* hh)
{
    TObject* obj = nullptr;
    if ((obj = gROOT->FindObject(Form("%s_pfy", hh->GetName())))) obj->Delete();
    TProfile* pfy = hh->ProfileY("_pfy", 1, -1, "i");
    TGraphErrors* gr = gHistoManipulator->MakeGraphFrom(pfy);
    pfy->Delete();
    TGraph* gg = gHistoManipulator->PermuteAxis(gr);
    gr->Delete();
    gg->SetName(Form("%s_pfy", hh->GetName()));
    gg->SetLineColor(kBlack);
    gg->Draw("PEZ");
    Modified();
    Update();
}

void CXCanvas::SaveCanvasAs()
{
    TCanvas *c = fCanvas;

    if (strcmp("", fSavedAs) != 0) {
        Int_t ret_val;
        TString file = fSavedAs.Data();
        file.ReplaceAll(gSystem->DirName(fSavedAs.Data()), "");
        file.ReplaceAll("/", "");
        new TGMsgBox(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), "File name exist",
                     Form("File name '%s' already exists, OK to owerwrite it?", file.Data()),
                     kMBIconExclamation, kMBOk | kMBCancel, &ret_val);

        if (ret_val & kMBOk) {
            if(fSavedAs.EndsWith(".root"))
            {
                c = GetTCanvas();
                c->SaveAs(fSavedAs);
                delete c;
            }
            else
                c->SaveAs(fSavedAs);
            return;
        }
    }

    const char* SaveAsTypes[] = {
        "PDF",          "*.pdf",
        //        "PostScript",   "*.ps",
        "Encapsulated PostScript", "*.eps",
        //        "SVG",          "*.svg",
        //        "TeX",          "*.tex",
        //        "GIF",          "*.gif",
        "ROOT macros",  "*.C",
        "ROOT files",   "*.root",
        //        "XML",          "*.xml",
        "PNG",          "*.png",
        "XPM",          "*.xpm",
        "JPEG",         "*.jpg",
        //        "TIFF",         "*.tiff",
        //        "XCF",          "*.xcf",
        //        "All files",    "*",
        nullptr,              nullptr
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

    if(fn.EndsWith(".root"))
    {
        c = GetTCanvas();
        c->SaveAs(fn);
        delete c;
    }
    else
        c->SaveAs(fn);

    fSavedAs = fn;

    fPadSave = dynamic_cast<TPad*>(fCanvas->cd());
}


void CXCanvas::SaveHistToAsciiFile()
{
    TH1 *hist = FindHisto();

    if(hist == nullptr) {
        cout<<"No selected histogram, click on the histogram you want to save"<<endl;
        return;
    }


    const char* SaveAsTypes[] = {
        "ASCII",          "*.dat",
        nullptr,          nullptr
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

    if(!hist->InheritsFrom("TH2"))
    {
        ofstream outfile(fn);
        outfile<<"# "<<hist->GetName()<<endl;
        outfile<<"# X axis : "<<hist->GetXaxis()->GetTitle()<<endl;
        outfile<<"# Y axis : "<<hist->GetYaxis()->GetTitle()<<endl;
        outfile<<"# N Bins : "<<hist->GetXaxis()->GetNbins()<<endl;
        outfile<<"# X value at the center of the bins"<<endl;
        for(int ibin = 1 ; ibin<=hist->GetXaxis()->GetNbins() ; ibin++)
        {
            outfile<<hist->GetBinCenter(ibin)<<" "<<hist->GetBinContent(ibin)<<endl;
        }

        outfile.close();
    }
    else
    {
        TH2 *Hist2D = dynamic_cast<TH2*>(hist);

        ofstream outfile(fn);
        outfile<<"# "<<hist->GetName()<<endl;
        outfile<<"# X axis : "<<Hist2D->GetXaxis()->GetTitle()<<endl;
        outfile<<"# Y axis : "<<Hist2D->GetYaxis()->GetTitle()<<endl;
        outfile<<"# N BinsX : "<<Hist2D->GetXaxis()->GetNbins()<<endl;
        outfile<<"# N BinsY : "<<Hist2D->GetYaxis()->GetNbins()<<endl;

        outfile<<"# Value at the center of the bins"<<endl;
        for(int ibinx = 1 ; ibinx<=Hist2D->GetXaxis()->GetNbins() ; ibinx++)
        {
            for(int ibiny = 1 ; ibiny<=Hist2D->GetYaxis()->GetNbins() ; ibiny++)
            {
                outfile<<Hist2D->GetXaxis()->GetBinCenter(ibinx)<<" "<<Hist2D->GetYaxis()->GetBinCenter(ibiny)<<" "<<Hist2D->GetBinContent(ibinx,ibiny)<<endl;
            }

        }

        outfile.close();
    }
}


TCanvas *CXCanvas::GetTCanvas()
{
    gROOT->SetBatch(kTRUE);
    auto *c = new TCanvas;
    gROOT->SetBatch(kFALSE);

    for(int i=0 ; i<fPrimitives->GetEntries() ; i++) {
        if(fPrimitives->At(i)->InheritsFrom("CXGateBox")) {
            auto *b = dynamic_cast<CXGateBox*>(fPrimitives->At(i));
            c->GetListOfPrimitives()->Add(b->GetBox());
        }
        else {
            c->GetListOfPrimitives()->Add(fPrimitives->At(i));
        }
    }

    c->SetTopMargin(GetTopMargin());
    c->SetBottomMargin(GetBottomMargin());
    c->SetLeftMargin(GetLeftMargin());
    c->SetRightMargin(GetRightMargin());

    c->SetName(GetName());
    c->SetTitle(GetTitle());
    c->SetWindowSize(GetWw(),GetWh());

    c->Modified();
    c->Update();

    return c;
}

TH1* CXCanvas::FindHisto(TVirtualPad *pad)
{
    TObject* hh = nullptr;

    TVirtualPad *apad = pad;
    if(apad == nullptr) apad = gROOT->GetSelectedPad();

    if(apad == nullptr)
        return nullptr;

    TIter it(apad->GetListOfPrimitives());
    while ((hh = (TObject*)it())) {
        if ((hh->InheritsFrom(TH1::Class()))&& ((TString)hh->GetName()) != "hframe") {
            //            cout << "FindHisto: Pad="<<apad->GetName()<<" "<<"hist: "<<hh->GetName()<<endl;
            return dynamic_cast<TH1*>(hh);
        }
    }
    //    cout << "FindHisto: Pad="<<apad->GetName()<<" "<<"hist: NULL"<<endl;

    return nullptr;
}

TGraph* CXCanvas::FindGraph()
{
    TObject* hh = nullptr;
    TIter it(gROOT->GetSelectedPad()->GetListOfPrimitives());
    while ((hh = (TObject*)it())) {
        if (hh->InheritsFrom("TGraph") && ((TString)hh->GetName()) != "hframe") return dynamic_cast<TGraph*>(hh);
    }

    return nullptr;
}

Bool_t CXCanvas::ExpandFunctionRange()
{
    Bool_t up = kFALSE;
    TH1* hh = FindHisto();
    if (!hh) return up;
    TObject* obj = nullptr;
    TIter it(hh->GetListOfFunctions());
    while ((obj = it())) {
        dynamic_cast<TF1*>(obj)->SetRange(hh->GetXaxis()->GetXmin(), hh->GetXaxis()->GetXmax());
        up = kTRUE;
    }
    return up;
}

void CXCanvas::SetEnabledShortcuts(Int_t value)
{
    fEnabledShortcuts = value;
}

void CXCanvas::ProjectionX(TH2* hh)
{
    TString pname = Form("%s_px", hh->GetName());
    Int_t ip = 1;
    while (gROOT->FindObject(pname.Data())) {
        pname = Form("%s_px%d", hh->GetName(), ip);
        ip++;
    }

    TH1* px = hh->ProjectionX(pname.Data());
    if (!px) return;
    Double_t minY = (hh->GetYaxis()->GetXmin());
    Double_t maxY = (hh->GetYaxis()->GetXmax());
    Double_t dY = (maxY - minY) * 0.8;

    Double_t maxH = px->GetBinContent(px->GetMaximumBin());

    TGraph* gg = nullptr;
    if ((gg = dynamic_cast<TGraph*>(gROOT->FindObject(Form("%s_gjx", hh->GetName()))))) gg->Delete();

    gg = new TGraph;
    for (int i = 0; i < px->GetNbinsX(); i++) {
        gg->SetPoint(i, px->GetBinCenter(i), minY + px->GetBinContent(i)*dY / maxH);
    }

    gg->SetName(Form("%s_gjx", hh->GetName()));
    gg->SetTitle(Form("%s_gjx", hh->GetName()));
    gg->SetLineColor(kBlack);
    gg->SetMarkerColor(kBlack);
    gg->SetMarkerStyle(8);
    gg->Draw("PL");

    Modified();
    Update();
}

void CXCanvas::ProjectionY(TH2* hh)
{
    TString pname = Form("%s_py", hh->GetName());
    Int_t ip = 1;

    while (gROOT->FindObject(pname.Data())) {
        pname = Form("%s_py%d", hh->GetName(), ip);
        ip++;
    }

    TH1* py = hh->ProjectionY(pname.Data());
    if (!py) return;
    Double_t minY = (hh->GetXaxis()->GetXmin());
    Double_t maxY = (hh->GetXaxis()->GetXmax());
    Double_t dY = (maxY - minY) * 0.8;

    Double_t maxH = py->GetBinContent(py->GetMaximumBin());

    TGraph* gg = nullptr;
    if ((gg = dynamic_cast<TGraph*>(gROOT->FindObject(Form("%s_gjy", hh->GetName()))))) gg->Delete();

    gg = new TGraph;
    for (int i = 0; i < py->GetNbinsX(); i++) {
        gg->SetPoint(i, minY + py->GetBinContent(i)*dY / maxH, py->GetBinCenter(i));
    }

    gg->SetName(Form("%s_gjy", hh->GetName()));
    gg->SetTitle(Form("%s_gjy", hh->GetName()));
    gg->SetLineColor(kBlack);
    gg->SetMarkerColor(kBlack);
    gg->SetMarkerStyle(8);
    gg->Draw("PL");

    Modified();
    Update();
}

void CXCanvas::EventProcessed(Event_t *ev, Window_t)
{
    TString evType;
    switch (ev->fType) {
    case kMapNotify:
        evType.Form("kMapNotify");
        break;
    case kUnmapNotify:
        evType.Form("kUnmapNotify");
        break;
    case kDestroyNotify:
        evType.Form("kDestroyNotify");
        break;
    case kExpose:
        evType.Form("kExpose");
        break;
    case kConfigureNotify:
        evType.Form("kConfigureNotify");
        break;
    case kGKeyPress:
        evType.Form("kGKeyPress");
        break;
    case kKeyRelease:
        if(fSelectedHisto) fSelectedHisto = nullptr;
        evType.Form("kKeyRelease");
        break;
    case kFocusIn:
        evType.Form("kFocusIn");
        break;
    case kFocusOut:
        evType.Form("kFocusOut");
        break;
    case kButtonPress:
        evType.Form("kButtonPress");
        break;
    case kButtonDoubleClick:
        evType.Form("kButtonDoubleClick");
        break;
    case kButtonRelease:
        evType.Form("kButtonRelease");
        break;
    case kEnterNotify:
        evType.Form("kEnterNotify");
        break;
    case kLeaveNotify:
        evType.Form("kLeaveNotify");
        break;
    case kMotionNotify:
        evType.Form("kMotionNotify");
        break;
    case kClientMessage:
        evType.Form("kClientMessage");
        break;
    case kSelectionNotify:
        evType.Form("kSelectionNotify");
        break;
    case kSelectionRequest:
        evType.Form("kSelectionRequest");
        break;
    case kSelectionClear:
        evType.Form("kSelectionClear");
        break;
    case kColormapNotify:
        evType.Form("kColormapNotify");
        break;
    case kOtherEvent:
        evType.Form("kOtherEvent");
        break;
    default:
        evType.Form("Unknown");
        break;
    }
}
ClassImp(CXCanvas)






