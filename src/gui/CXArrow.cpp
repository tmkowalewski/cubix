#include "CXArrow.h"

#include "TLatex.h"
#include "TBox.h"
#include "TH1.h"
#include "TVirtualPad.h"
#include "TFrame.h"

#include "CXFit.h"
#include "CXBgdFit.h"

CXArrow::CXArrow(CXFit *fit, Double_t E,Double_t y1 ,Double_t y2,Float_t arrowsize, Float_t textsize, Option_t *option) : TArrow(E, y1, E, y2, arrowsize, option)
{
    fFit = fit;
    fTextSize = textsize;
}

CXArrow::CXArrow(CXBgdFit *fit, Double_t E,Double_t y1 ,Double_t y2,Float_t arrowsize, Float_t textsize, Option_t *option) : TArrow(E, y1, E, y2, arrowsize, option)
{
    fBgdFit = fit;
    fTextSize = textsize;
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


ClassImp(CXArrow);
