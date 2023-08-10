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

#include "CXGateBox.h"

#include <iostream>

#include "TMath.h"
#include "TROOT.h"
#include "TH1.h"
#include "TVirtualX.h"
#include "TVirtualPad.h"

using namespace std;

CXGateBox::CXGateBox(Float_t E, Float_t width, TVirtualPad *pad) :
    TBox(),
    fWidth(width),
    fCentroid(E),
    fAssociatedPad(pad)
{
    if(pad != nullptr)
    {
        Float_t X1 = TMath::Min(fCentroid-fWidth,fCentroid+fWidth);
        Float_t X2 = TMath::Max(fCentroid-fWidth,fCentroid+fWidth);

        fAssociatedHist = GetHisto();

        if(fAssociatedHist) {

            Int_t BinX1 = fAssociatedHist->FindBin(X1);
            Int_t BinX2 = fAssociatedHist->FindBin(X2);

            if(X1<fAssociatedHist->GetXaxis()->GetBinCenter(BinX1))
                X1=fAssociatedHist->GetXaxis()->GetBinLowEdge(BinX1);
            else
                X1=fAssociatedHist->GetXaxis()->GetBinUpEdge(BinX1);

            if(X2<fAssociatedHist->GetXaxis()->GetBinCenter(BinX2))
                X2=fAssociatedHist->GetXaxis()->GetBinLowEdge(BinX2);
            else
                X2=fAssociatedHist->GetXaxis()->GetBinUpEdge(BinX2);
        }
        fWidth = (X2-X1)/2.;
        fCentroid = X1 + fWidth;

        SetX1(X1);
        SetX2(X2);

        if(fAssociatedPad->GetLogy() == false){
            SetY1(fAssociatedPad->GetUymin());
            SetY2(fAssociatedPad->GetUymax());
        }
        else{
            SetY1(TMath::Power(10,fAssociatedPad->GetUymin()));
            SetY2(TMath::Power(10,fAssociatedPad->GetUymax()));
        }
    }
    fTitle = Form("%.1f_w%.1f",fCentroid,fWidth);

    SetFillColor(kRed-6);
    SetFillStyle(kFDotted1);
}

void CXGateBox::SetBGD()
{
    fIsBGD   = true;
    fIsGate1 = false;
    fIsGate2 = false;

    SetFillColor(kBlue-6);
    UpdateBox();
}

void CXGateBox::SetGate1()
{
    fIsBGD   = false;
    fIsGate1 = true;
    fIsGate2 = false;

    SetFillColor(kRed-6);
    UpdateBox();
}

void CXGateBox::SetGate2()
{
    fIsBGD   = false;
    fIsGate1 = false;
    fIsGate2 = true;

    SetFillColor(kGreen-6);
    UpdateBox();
}

void CXGateBox::Update()
{
    fWidth = (TMath::Max(GetX1(),GetX2()) - TMath::Min(GetX1(),GetX2()))/2.;
    fCentroid = TMath::Min(GetX1(),GetX2()) + fWidth;

    fAssociatedHist = GetHisto();

    if(fAssociatedHist && fIsButtonReleased) {

        Float_t X1 = TMath::Min(fCentroid-fWidth,fCentroid+fWidth);
        Float_t X2 = TMath::Max(fCentroid-fWidth,fCentroid+fWidth);

        Int_t BinX1 = fAssociatedHist->FindBin(X1);
        Int_t BinX2 = fAssociatedHist->FindBin(X2);

        if(X1<fAssociatedHist->GetXaxis()->GetBinCenter(BinX1))
            X1=fAssociatedHist->GetXaxis()->GetBinLowEdge(BinX1);
        else
            X1=fAssociatedHist->GetXaxis()->GetBinUpEdge(BinX1);

        if(X2<fAssociatedHist->GetXaxis()->GetBinCenter(BinX2))
            X2=fAssociatedHist->GetXaxis()->GetBinLowEdge(BinX2);
        else
            X2=fAssociatedHist->GetXaxis()->GetBinUpEdge(BinX2);

        SetX1(X1);
        SetX2(X2);

        fWidth = (X2-X1)/2.;
        fCentroid = X1 + fWidth;
    }

    if(fAssociatedPad->GetLogy() == false){
        SetY1(fAssociatedPad->GetUymin());
        SetY2(fAssociatedPad->GetUymax());
    }
    else{
        SetY1(TMath::Power(10,fAssociatedPad->GetUymin()));
        SetY2(TMath::Power(10,fAssociatedPad->GetUymax()));
    }

    fTitle = Form("%.1f_w%.1f",fCentroid,fWidth);
}

void CXGateBox::UpdateBox()
{
    SetX1(fCentroid-fWidth);
    SetX2(fCentroid+fWidth);

    if(fAssociatedPad->GetLogy() == false){
        SetY1(fAssociatedPad->GetUymin());
        SetY2(fAssociatedPad->GetUymax());
    }
    else{
        SetY1(TMath::Power(10,fAssociatedPad->GetUymin()));
        SetY2(TMath::Power(10,fAssociatedPad->GetUymax()));
    }

    fTitle = Form("%.1f_w%.1f",fCentroid,fWidth);

    fAssociatedPad->Modified();
    fAssociatedPad->Update();
}

TBox *CXGateBox::GetBox()
{
    TBox *b = new TBox;
    b->SetX1(GetX1());
    b->SetX2(GetX2());
    b->SetY1(GetY1());
    b->SetY2(GetY2());

    b->SetFillColor(GetFillColor());
    b->SetFillStyle(GetFillStyle());
    b->SetLineColor(GetLineColor());
    b->SetLineStyle(GetLineStyle());
    b->SetLineWidth(GetLineWidth());

    return b;
}

Int_t CXGateBox::DistancetoPrimitive(Int_t px, Int_t py)
{
    Int_t pxl, pyl, pxt, pyt;
    Int_t px1, py1, px2, py2;

    px1 = gPad->XtoAbsPixel(fX1);
    py1 = gPad->YtoAbsPixel(fY1);
    px2 = gPad->XtoAbsPixel(fX2);
    py2 = gPad->YtoAbsPixel(fY2);

    if(gPad->GetLogx()){
        px1 = gPad->XtoAbsPixel(TMath::Log10(GetX1()));
        px2 = gPad->XtoAbsPixel(TMath::Log10(GetX2()));
    }
    if(gPad->GetLogy()){
        py1 = gPad->YtoAbsPixel(TMath::Log10(GetY1()));
        py2 = gPad->YtoAbsPixel(TMath::Log10(GetY2()));
    }

    if (px1 < px2) {pxl = px1; pxt = px2;}
    else           {pxl = px2; pxt = px1;}
    if (py1 < py2) {pyl = py1; pyt = py2;}
    else           {pyl = py2; pyt = py1;}

    // Are we inside the box?
    if (GetFillStyle()) {
        if ( (px >= pxl && px <= pxt) && (py >= pyl && py <= pyt) ) return 0;
        else return 9999;
    }

    // Are we on the edges?
    Int_t dxl = TMath::Abs(px - pxl);
    if (py < pyl) dxl += pyl - py;
    if (py > pyt) dxl += py - pyt;
    Int_t dxt = TMath::Abs(px - pxt);
    if (py < pyl) dxt += pyl - py;
    if (py > pyt) dxt += py - pyt;
    Int_t dyl = TMath::Abs(py - pyl);
    if (px < pxl) dyl += pxl - px;
    if (px > pxt) dyl += px - pxt;
    Int_t dyt = TMath::Abs(py - pyt);
    if (px < pxl) dyt += pxl - px;
    if (px > pxt) dyt += px - pxt;

    Int_t distance = dxl;
    if (dxt < distance) distance = dxt;
    if (dyl < distance) distance = dyl;
    if (dyt < distance) distance = dyt;

    return distance - Int_t(0.5*fLineWidth);
}

void CXGateBox::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
    if (!gPad) return;
    if (!gPad->IsEditable() && event != kMouseEnter) return;

    if (TestBit(kCannotMove)) return;

    const Int_t kMaxDiff = 7;
    const Int_t kMinSize = 20;

    static Int_t px1, px2, py1, py2, pxl, pyl, pxt, pyt, pxold, pyold;
    static Int_t px1p, px2p, py1p, py2p, pxlp, pylp, pxtp, pytp;
    static Double_t oldX1, oldY1, oldX2, oldY2;
    static Bool_t pA, pB, pC, pD, pTop, pL, pR, pBot, pINSIDE;
    Int_t  wx, wy;
    TVirtualPad  *parent = gPad;
    Bool_t opaque  = gPad->OpaqueMoving();
    Bool_t ropaque = gPad->OpaqueResizing();

    HideToolTip(event);

    switch (event) {

    case kButton1Double:
        px1 = -1; //used by kButton1Up
        break;

    case kArrowKeyPress:
    case kButton1Down:

        fIsButtonReleased = false;

        oldX1 = fX1;
        oldY1 = fY1;
        oldX2 = fX2;
        oldY2 = fY2;
        gVirtualX->SetLineColor(-1);
        TAttLine::Modify();  //Change line attributes only if necessary
        if (GetFillColor())
            gVirtualX->SetLineColor(GetFillColor());
        else
            gVirtualX->SetLineColor(1);
        gVirtualX->SetLineWidth(2);

        /* fall through */

    case kMouseMotion:

        px1 = gPad->XtoAbsPixel(GetX1());
        py1 = gPad->YtoAbsPixel(GetY1());
        px2 = gPad->XtoAbsPixel(GetX2());
        py2 = gPad->YtoAbsPixel(GetY2());

        if(gPad->GetLogx()){
            px1 = gPad->XtoAbsPixel(TMath::Log10(GetX1()));
            px2 = gPad->XtoAbsPixel(TMath::Log10(GetX2()));
        }
        if(gPad->GetLogy()){
            py1 = gPad->YtoAbsPixel(TMath::Log10(GetY1()));
            py2 = gPad->YtoAbsPixel(TMath::Log10(GetY2()));
        }

        if (px1 < px2) {
            pxl = px1;
            pxt = px2;
        } else {
            pxl = px2;
            pxt = px1;
        }
        if (py1 < py2) {
            pyl = py1;
            pyt = py2;
        } else {
            pyl = py2;
            pyt = py1;
        }

        px1p = parent->XtoAbsPixel(parent->GetX1()) + parent->GetBorderSize();
        py1p = parent->YtoAbsPixel(parent->GetY1()) - parent->GetBorderSize();
        px2p = parent->XtoAbsPixel(parent->GetX2()) - parent->GetBorderSize();
        py2p = parent->YtoAbsPixel(parent->GetY2()) + parent->GetBorderSize();

        if (px1p < px2p) {
            pxlp = px1p;
            pxtp = px2p;
        } else {
            pxlp = px2p;
            pxtp = px1p;
        }
        if (py1p < py2p) {
            pylp = py1p;
            pytp = py2p;
        } else {
            pylp = py2p;
            pytp = py1p;
        }

        pA = pB = pC = pD = pTop = pL = pR = pBot = pINSIDE = kFALSE;

        // case pA
        if (TMath::Abs(px - pxl) <= kMaxDiff && TMath::Abs(py - pyl) <= kMaxDiff) {
            pxold = pxl; pyold = pyl; pA = kTRUE;
            gPad->SetCursor(kTopLeft);
        }
        // case pB
        if (TMath::Abs(px - pxt) <= kMaxDiff && TMath::Abs(py - pyl) <= kMaxDiff) {
            pxold = pxt; pyold = pyl; pB = kTRUE;
            gPad->SetCursor(kTopRight);
        }
        // case pC
        if (TMath::Abs(px - pxt) <= kMaxDiff && TMath::Abs(py - pyt) <= kMaxDiff) {
            pxold = pxt; pyold = pyt; pC = kTRUE;
            gPad->SetCursor(kBottomRight);
        }
        // case pD
        if (TMath::Abs(px - pxl) <= kMaxDiff && TMath::Abs(py - pyt) <= kMaxDiff) {
            pxold = pxl; pyold = pyt; pD = kTRUE;
            gPad->SetCursor(kBottomLeft);
        }

        if ((px > pxl+kMaxDiff && px < pxt-kMaxDiff) &&
                TMath::Abs(py - pyl) < kMaxDiff) {             // top edge
            pxold = pxl; pyold = pyl; pTop = kTRUE;
            gPad->SetCursor(kTopSide);
        }

        if ((px > pxl+kMaxDiff && px < pxt-kMaxDiff) &&
                TMath::Abs(py - pyt) < kMaxDiff) {             // bottom edge
            pxold = pxt; pyold = pyt; pBot = kTRUE;
            gPad->SetCursor(kBottomSide);
        }

        if ((py > pyl+kMaxDiff && py < pyt-kMaxDiff) &&
                TMath::Abs(px - pxl) < kMaxDiff) {             // left edge
            pxold = pxl; pyold = pyl; pL = kTRUE;
            gPad->SetCursor(kLeftSide);
        }

        if ((py > pyl+kMaxDiff && py < pyt-kMaxDiff) &&
                TMath::Abs(px - pxt) < kMaxDiff) {             // right edge
            pxold = pxt; pyold = pyt; pR = kTRUE;
            gPad->SetCursor(kRightSide);
        }

        if ((px > pxl+kMaxDiff && px < pxt-kMaxDiff) &&
                (py > pyl+kMaxDiff && py < pyt-kMaxDiff)) {    // inside box
            pxold = px; pyold = py; pINSIDE = kTRUE;
            if (event == kButton1Down)
                gPad->SetCursor(kMove);
            else
                gPad->SetCursor(kCross);
        }

        fResizing = kFALSE;
        if (pA || pB || pC || pD || pTop || pL || pR || pBot)
            fResizing = kTRUE;

        if (!pA && !pB && !pC && !pD && !pTop && !pL && !pR && !pBot && !pINSIDE)
            gPad->SetCursor(kCross);

        break;

    case kArrowKeyRelease:
    case kButton1Motion:

        wx = wy = 0;

        if (pA) {
            if (!ropaque) gVirtualX->DrawBox(pxold, pyt, pxt, pyold, TVirtualX::kHollow);  // draw the old box
            if (px > pxt-kMinSize) { px = pxt-kMinSize; wx = px; }
            if (py > pyt-kMinSize) { py = pyt-kMinSize; wy = py; }
            if (px < pxlp) { px = pxlp; wx = px; }
            if (py < pylp) { py = pylp; wy = py; }
            if (!ropaque) gVirtualX->DrawBox(px   , pyt, pxt, py,    TVirtualX::kHollow);  // draw the new box
        }
        if (pB) {
            if (!ropaque) gVirtualX->DrawBox(pxl  , pyt, pxold, pyold, TVirtualX::kHollow);
            if (px < pxl+kMinSize) { px = pxl+kMinSize; wx = px; }
            if (py > pyt-kMinSize) { py = pyt-kMinSize; wy = py; }
            if (px > pxtp) { px = pxtp; wx = px; }
            if (py < pylp) { py = pylp; wy = py; }
            if (!ropaque) gVirtualX->DrawBox(pxl  , pyt, px ,  py,    TVirtualX::kHollow);
        }
        if (pC) {
            if (!ropaque) gVirtualX->DrawBox(pxl  , pyl, pxold, pyold, TVirtualX::kHollow);
            if (px < pxl+kMinSize) { px = pxl+kMinSize; wx = px; }
            if (py < pyl+kMinSize) { py = pyl+kMinSize; wy = py; }
            if (px > pxtp) { px = pxtp; wx = px; }
            if (py > pytp) { py = pytp; wy = py; }
            if (!ropaque) gVirtualX->DrawBox(pxl  , pyl, px ,   py,    TVirtualX::kHollow);
        }
        if (pD) {
            if (!ropaque) gVirtualX->DrawBox(pxold, pyold, pxt, pyl, TVirtualX::kHollow);
            if (px > pxt-kMinSize) { px = pxt-kMinSize; wx = px; }
            if (py < pyl+kMinSize) { py = pyl+kMinSize; wy = py; }
            if (px < pxlp) { px = pxlp; wx = px; }
            if (py > pytp) { py = pytp; wy = py; }
            if (!ropaque) gVirtualX->DrawBox(px   , py ,   pxt, pyl, TVirtualX::kHollow);
        }
        if (pTop) {
            if (!ropaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);
            py2 += py - pyold;
            if (py2 > py1-kMinSize) { py2 = py1-kMinSize; wy = py2; }
            if (py2 < py2p) { py2 = py2p; wy = py2; }
            if (!ropaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);
        }
        if (pBot) {
            if (!ropaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);
            py1 += py - pyold;
            if (py1 < py2+kMinSize) { py1 = py2+kMinSize; wy = py1; }
            if (py1 > py1p) { py1 = py1p; wy = py1; }
            if (!ropaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);
        }
        if (pL) {
            if (!ropaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);
            px1 += px - pxold;
            if (px1 > px2-kMinSize) { px1 = px2-kMinSize; wx = px1; }
            if (px1 < px1p) { px1 = px1p; wx = px1; }
            if (!ropaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);
        }
        if (pR) {
            if (!ropaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);
            px2 += px - pxold;
            if (px2 < px1+kMinSize) { px2 = px1+kMinSize; wx = px2; }
            if (px2 > px2p) { px2 = px2p; wx = px2; }
            if (!ropaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);
        }
        if (pINSIDE) {
            if (!opaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);  // draw the old box
            Int_t dx = px - pxold;
            Int_t dy = py - pyold;
            px1 += dx; py1 += dy; px2 += dx; py2 += dy;
            if (px1 < px1p) { dx = px1p - px1; px1 += dx; px2 += dx; wx = px+dx; }
            if (px2 > px2p) { dx = px2 - px2p; px1 -= dx; px2 -= dx; wx = px-dx; }
            if (py1 > py1p) { dy = py1 - py1p; py1 -= dy; py2 -= dy; wy = py-dy; }
            if (py2 < py2p) { dy = py2p - py2; py1 += dy; py2 += dy; wy = py+dy; }
            if (!opaque) gVirtualX->DrawBox(px1, py1, px2, py2, TVirtualX::kHollow);  // draw the new box
        }

        if (wx || wy) {
            if (wx) px = wx;
            if (wy) py = wy;
            gVirtualX->Warp(px, py);
        }

        pxold = px;
        pyold = py;


        if ((pINSIDE && opaque) || (fResizing && ropaque)) {
            if (pA) {
                fX1 = gPad->AbsPixeltoX(pxold);
                fY1 = gPad->AbsPixeltoY(pyt);
                fX2 = gPad->AbsPixeltoX(pxt);
                fY2 = gPad->AbsPixeltoY(pyold);
            }
            if (pB) {
                fX1 = gPad->AbsPixeltoX(pxl);
                fY1 = gPad->AbsPixeltoY(pyt);
                fX2 = gPad->AbsPixeltoX(pxold);
                fY2 = gPad->AbsPixeltoY(pyold);
            }
            if (pC) {
                fX1 = gPad->AbsPixeltoX(pxl);
                fY1 = gPad->AbsPixeltoY(pyold);
                fX2 = gPad->AbsPixeltoX(pxold);
                fY2 = gPad->AbsPixeltoY(pyl);
            }
            if (pD) {
                fX1 = gPad->AbsPixeltoX(pxold);
                fY1 = gPad->AbsPixeltoY(pyold);
                fX2 = gPad->AbsPixeltoX(pxt);
                fY2 = gPad->AbsPixeltoY(pyl);
            }
            if (pTop || pBot || pL || pR || pINSIDE) {
                fX1 = gPad->AbsPixeltoX(px1);
                fY1 = gPad->AbsPixeltoY(py1);
                fX2 = gPad->AbsPixeltoX(px2);
                fY2 = gPad->AbsPixeltoY(py2);
            }

            if(gPad->GetLogx()){
                fX1 = TMath::Power(10,fX1);
                fX2 = TMath::Power(10,fX2);
            }
            if(gPad->GetLogy()){
                fY1 = TMath::Power(10,fY1);
                fY2 = TMath::Power(10,fY2);
            }

            if (pINSIDE) gPad->ShowGuidelines(this, event, 'i', true);
            if (pTop) gPad->ShowGuidelines(this, event, 't', true);
            if (pBot) gPad->ShowGuidelines(this, event, 'b', true);
            if (pL) gPad->ShowGuidelines(this, event, 'l', true);
            if (pR) gPad->ShowGuidelines(this, event, 'r', true);
            if (pA) gPad->ShowGuidelines(this, event, '1', true);
            if (pB) gPad->ShowGuidelines(this, event, '2', true);
            if (pC) gPad->ShowGuidelines(this, event, '3', true);
            if (pD) gPad->ShowGuidelines(this, event, '4', true);
            gPad->Modified(kTRUE);
        }

        break;

    case kButton1Up:
        fIsButtonReleased = true;

        if (gROOT->IsEscaped()) {
            gROOT->SetEscape(kFALSE);
            if (opaque) {
                this->SetX1(oldX1);
                this->SetY1(oldY1);
                this->SetX2(oldX2);
                this->SetY2(oldY2);
                gPad->Modified(kTRUE);
                gPad->Update();
            }
            break;
        }

        if (opaque || ropaque) {
            gPad->ShowGuidelines(this, event);
        } else {
            if (px1 < 0 ) break;
            if (pA) {
                fX1 = gPad->AbsPixeltoX(pxold);
                fY1 = gPad->AbsPixeltoY(pyt);
                fX2 = gPad->AbsPixeltoX(pxt);
                fY2 = gPad->AbsPixeltoY(pyold);
            }
            if (pB) {
                fX1 = gPad->AbsPixeltoX(pxl);
                fY1 = gPad->AbsPixeltoY(pyt);
                fX2 = gPad->AbsPixeltoX(pxold);
                fY2 = gPad->AbsPixeltoY(pyold);
            }
            if (pC) {
                fX1 = gPad->AbsPixeltoX(pxl);
                fY1 = gPad->AbsPixeltoY(pyold);
                fX2 = gPad->AbsPixeltoX(pxold);
                fY2 = gPad->AbsPixeltoY(pyl);
            }
            if (pD) {
                fX1 = gPad->AbsPixeltoX(pxold);
                fY1 = gPad->AbsPixeltoY(pyold);
                fX2 = gPad->AbsPixeltoX(pxt);
                fY2 = gPad->AbsPixeltoY(pyl);
            }
            if (pTop || pBot || pL || pR || pINSIDE) {
                fX1 = gPad->AbsPixeltoX(px1);
                fY1 = gPad->AbsPixeltoY(py1);
                fX2 = gPad->AbsPixeltoX(px2);
                fY2 = gPad->AbsPixeltoY(py2);
            }

            if(gPad->GetLogx()){
                fX1 = TMath::Power(10,fX1);
                fX2 = TMath::Power(10,fX2);
            }
            if(gPad->GetLogy()){
                fY1 = TMath::Power(10,fY1);
                fY2 = TMath::Power(10,fY2);
            }

            if (pINSIDE) {
                // if it was not a pad that was moved then it must have been
                // a box or something like that so we have to redraw the pad
                if (parent == gPad) gPad->Modified(kTRUE);
            }
        }

        if (pA || pB || pC || pD || pTop || pL || pR || pBot) gPad->Modified(kTRUE);

        if (!opaque) {
            gVirtualX->SetLineColor(-1);
            gVirtualX->SetLineWidth(-1);
        }

        break;

    case kButton1Locate:

        ExecuteEvent(kButton1Down, px, py);

        while (1) {
            px = py = 0;
            event = gVirtualX->RequestLocator(1, 1, px, py);

            ExecuteEvent(kButton1Motion, px, py);

            if (event != -1) {                     // button is released
                ExecuteEvent(kButton1Up, px, py);
                return;
            }
        }
    }
}

TObject* CXGateBox::Clone(const char* newname) const
{
    CXGateBox* obj = new CXGateBox(fCentroid,fWidth,fAssociatedPad);
    if(newname)
        obj->SetName(newname);
    obj->SetTitle(fTitle);
    if(fIsBGD)
        obj->SetBGD();
    if(fIsGate1)
        obj->SetGate1();
    if(fIsGate2)
        obj->SetGate2();

    return obj;
}

void CXGateBox::ls(Option_t *option) const
{
    cout<<"Centroid: "<<fCentroid<<" ; Width: "<<fWidth<<" ; Type: ";
    if(fIsBGD) cout << "BG"<<endl;
    else if(fIsGate1) cout << "Gate 1"<<endl;
    else if(fIsGate2) cout << "Gate 2"<<endl;
    else cout << "undefined" <<endl;
}

TH1 *CXGateBox::GetHisto()
{
    TH1 *hist = nullptr;

    for(int i=0 ; i<fAssociatedPad->GetListOfPrimitives()->GetEntries() ; i++)
    {
        TObject *o = fAssociatedPad->GetListOfPrimitives()->At(i);

        if(o->InheritsFrom("TH1"))
        {
            hist = dynamic_cast<TH1*>(o);
            return hist;
        }
    }

    return nullptr;
}
