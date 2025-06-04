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

#include "CXArrow.h"

#include "TLatex.h"
#include "TBox.h"
#include "TH1.h"
#include "TVirtualPad.h"
#include "TFrame.h"

#include "CXFit.h"
#include "CXBgdFit.h"

CXArrow::CXArrow(CXFit *fit, Double_t E, Double_t y1, Double_t y2, Float_t arrowsize, Float_t textsize, Option_t *option)
    : TArrow(E, y1, E, y2, arrowsize, option), fFit(fit), fTextSize(textsize) {
}

CXArrow::CXArrow(CXBgdFit *fit, Double_t E, Double_t y1, Double_t y2, Float_t arrowsize, Float_t textsize, Option_t *option)
    : TArrow(E, y1, E, y2, arrowsize, option), fBgdFit(fit), fTextSize(textsize) {
}

CXArrow::CXArrow(const CXArrow &other) : TArrow(other), fTextSize(other.fTextSize), fMeanFixed(other.fMeanFixed), fBindFWHM(other.fBindFWHM) {
    fFit = other.fFit;
    fBgdFit = other.fBgdFit;

    fLatex = other.fLatex ? new TLatex(*other.fLatex) : nullptr;
    fBox = other.fBox ? new CXArrowBox(*other.fBox) : nullptr;
}

CXArrow::~CXArrow() {
    delete fLatex;
    delete fBox;
}

CXArrow &CXArrow::operator=(const CXArrow &other) {
    if (this == &other) return *this;

    TArrow::operator=(other);

    fFit = other.fFit;
    fBgdFit = other.fBgdFit;
    fTextSize = other.fTextSize;
    fMeanFixed = other.fMeanFixed;
    fBindFWHM = other.fBindFWHM;

    delete fLatex;
    delete fBox;

    fLatex = other.fLatex ? new TLatex(*other.fLatex) : nullptr;
    fBox = other.fBox ? new CXArrowBox(*other.fBox) : nullptr;

    return *this;
}

TObject* CXArrow::Clone(const char* newname) const {
    return new CXArrow(*this);
}

void CXArrow::SetEnergy(Float_t E)
{
    SetX1(E);
    SetX2(E);
    if(fFit) fFit->Update();
    if(fBgdFit) fBgdFit->Update();
}

void CXArrow::Set(Double_t X, Double_t Y1, Double_t Y2)
{
    fX1=X;
    fX2=X ;
    fY1=Y1;
    fY2=Y2;
}

void CXArrow::RemoveArrow()
{
    if(fFit) fFit->RemoveArrow(this);
    if(fBgdFit) fBgdFit->RemoveArrow(this);
}

void CXArrow::RemoveFit()
{
    delete fFit;
    delete fBgdFit;
}

Int_t CXArrow::Compare(const TObject *obj) const
{
    if(obj->InheritsFrom(CXArrow::Class())) {
        auto *arr = dynamic_cast<const CXArrow*>(obj);
        return (GetEnergy() > arr->GetEnergy()) ? 1 : -1;
    }

    return 0;
}

void CXArrow::SetText(TH1 *hist, const TString &text, const TString &tooltip)
{
    Int_t XMinbin = hist->GetXaxis()->GetFirst();
    Int_t XMaxbin = hist->GetXaxis()->GetLast();
    Int_t XMin = hist->GetBinLowEdge(XMinbin);
    Int_t XMax = hist->GetBinLowEdge(XMaxbin);

    delete fBox;

    fBox = new CXArrowBox(this);
    fBox->SetX1(fX1-(XMax-XMin)/300.);
    fBox->SetX2(fX1+(XMax-XMin)/300.);
    fBox->SetY1(fY1);
    fBox->SetY2(fY2);

    fBox->SetFillStyle(0);
    fBox->SetFillColor(0);
    fBox->SetLineWidth(0);
    fBox->SetLineColor(0);
    fBox->SetLineStyle(3);

    Double_t height = gPad->GetFrame()->GetY2() - gPad->GetFrame()->GetY1();
    delete fLatex;
    fLatex = new TLatex(fX1,fY2+fArrowSize*height,text);
    fLatex->SetTextAngle(90);
    fLatex->SetTextFont(132);
    fLatex->SetTextSize(fTextSize);
    fLatex->SetTextColor(hist->GetLineColor());
    fLatex->SetTextAlign(12);
    fLatex->SetBit(TObject::kCannotPick);
    fLatex->Draw();

    fBox->SetToolTipText(tooltip.Data(),500);
    fBox->Draw();

    Float_t MaxGlob = hist->GetMaximum();
    Float_t Y = fLatex->GetY()-(gPad->AbsPixeltoY(fLatex->GetBBox().fY+fLatex->GetBBox().fWidth)-gPad->AbsPixeltoY(fLatex->GetBBox().fY));
    if(gPad->GetUymax()< Y)
        hist->GetYaxis()->SetRangeUser(gPad->GetUymin(),Y+MaxGlob/50.);
}

void CXArrow::ClearPad(TVirtualPad *pad, bool refresh)
{
    if(pad==nullptr) pad = gPad;

    pad->GetListOfPrimitives()->Remove(fLatex);
    pad->GetListOfPrimitives()->Remove(fBox);
    pad->GetListOfPrimitives()->Remove(this);

    if(refresh) {
        pad->Modified();
        pad->Update();
    }
}


void CXArrow::Paint(Option_t *option)
{
    TArrow::Paint(option);
}

void CXArrow::SetMeanFixed(Bool_t on)
{
    fMeanFixed=on;
    fFit->Update();
}

void CXArrow::SetBindFWHM(Bool_t on) {
    if (fBindFWHM == on) return;
    fBindFWHM = on;
    if(fFit) fFit->BindFWHM(on);
}

ClassImp(CXArrow);
