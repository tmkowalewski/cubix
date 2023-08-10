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

#include "CXENSDFLevelSchemePlayer.h"

#include <iostream>
#include <iomanip>
#include <vector>

#include "TArrow.h"
#include "TLatex.h"
#include "TPad.h"
#include "TBox.h"
#include "KeySymbols.h"
#include "TObjArray.h"
#include "TMath.h"

#include "CXCanvas.h"
#include "CXMainWindow.h"
#include "CXGuiENSDFPlayer.h"
#include "CXBashColor.h"

#include "tkmanager.h"

using namespace std;

typedef std::vector<Float_t> Float_vec_t;

CXENSDFLevelSchemePlayer::CXENSDFLevelSchemePlayer(const char* name, const char *title) :
    TNamed(name,title),
    fCanReplot(true)
{
    Color_t c[12] = {kBlue-6, kCyan-6, kGreen-6, kYellow-6 , kRed-6, kMagenta-6, kAzure, kTeal, kSpring, kOrange, kPink, kViolet};
    for(int i=0 ; i<48 ; i++) fColorWheel[i] = c[i%12] - (int)i/12;
    fCurrentColor = fColorWheel[0];

    fListOfArrows = new TList;
    fListOfArrows->SetOwner();

    fListOfLatex = new TList;
    fListOfLatex->SetOwner();

    fListOfBoxes = new TList;
    fListOfBoxes->SetOwner();

    fListOfCXArrows = new TList;
    fListOfCXArrows->SetOwner();
}

CXENSDFLevelSchemePlayer::~CXENSDFLevelSchemePlayer()
{
}

void CXENSDFLevelSchemePlayer::ConnectCanvas()
{
    fMainWindow->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)","CXENSDFLevelSchemePlayer", this, "ProcessedEventLevelScheme(Int_t, Int_t, Int_t, TObject*)");
}

void CXENSDFLevelSchemePlayer::CleanArrows()
{
    gPad = fMainWindow->GetSelectedPad();

    if(gPad == nullptr) return;

    for(int i=0 ; i<fListOfArrows->GetEntries() ; i++) {
        if(gPad->GetListOfPrimitives()->FindObject(fListOfArrows->At(i))) {
            gPad->GetListOfPrimitives()->Remove(fListOfArrows->At(i));
            fListOfArrows->RemoveAt(i);
            gPad->GetListOfPrimitives()->Remove(fListOfLatex->At(i));
            fListOfLatex->RemoveAt(i);
            gPad->GetListOfPrimitives()->Remove(fListOfBoxes->At(i));
            fListOfBoxes->RemoveAt(i);
            i--;
        }
    }
}

shared_ptr<tklevel_scheme> CXENSDFLevelSchemePlayer::DrawArrows(TString ListOfNuclei,TH1 *h, TString DataSet)
{
    shared_ptr<tklevel_scheme> RefLevel = nullptr;

    if(fMainWindow->GetCanvas() == nullptr)
        gbash_color->InfoMessage("Oops, current canvas not found...");

    gPad = fMainWindow->GetSelectedPad();

    ConnectCanvas();

    fCanReplot = false;

    fGuiLSPlayer->GetBranchingRatio(fMinStrenght,fMaxStrenght);
    fGuiLSPlayer->GetELevels(fMinELevel,fMaxELevel);
    fGuiLSPlayer->GetSpins(fMinSpin,fMaxSpin);
    fGuiLSPlayer->GetLifeTime(fMinLifeTime,fMaxLifeTime);

    fCurrentHist = h;
    bool dodraw = (fCurrentHist!=nullptr);

    if(dodraw) {
        fCurrentHist->GetYaxis()->UnZoom();
        gPad->Update();
        gPad->Modified();
    }

    Int_t IColor = 0;

    fCurrentColor = fColorWheel[IColor];

    fLevelDraw = false;

    TObjArray *arr = ListOfNuclei.Tokenize(" ");
    for(int i=0 ; i<arr->GetEntries() ; i++) {
        TString Nuc = arr->At(i)->GetName();

        fCurrentColor = fColorWheel[IColor];
        IColor++;

        auto levtest = DrawArrowsForNuc(Nuc,DataSet);
        if(!RefLevel) RefLevel = levtest;
    }
    delete arr;

    if(fLevelDraw) {
        if(fGuiLSPlayer->IsFullTitleMode())
            fCurrentHist->SetMaximum(fCurrentHist->GetMaximum()*1.45);
        else
            fCurrentHist->SetMaximum(fCurrentHist->GetMaximum()*1.25);

        gPad->Update();
        gPad->Modified();

        Float_t YMax = 0;

        Float_t PadMax = gPad->GetUymax();
        if(gPad->GetLogy())
            PadMax = TMath::Power(10,gPad->GetUymax());

        while(YMax>PadMax || YMax==0) {
            for(int i=0 ; i<fListOfLatex->GetEntries() ; i++) {
                TLatex *l = (TLatex*)fListOfLatex->At(i);
                if(gPad->GetListOfPrimitives()->FindObject(l) == nullptr) continue;
                Float_t Y = l->GetY()-(gPad->AbsPixeltoY(l->GetBBox().fY+l->GetBBox().fWidth)-gPad->AbsPixeltoY(l->GetBBox().fY));
                if(Y>YMax)
                    YMax = Y;
            }

            if(YMax>0.) fCurrentHist->SetMaximum(YMax*1.01);

            gPad->Update();
            gPad->Modified();

            for(int i=0 ; i<fListOfLatex->GetEntries() ; i++)
            {
                TLatex *l = (TLatex*)fListOfLatex->At(i);
                if(gPad->GetListOfPrimitives()->FindObject(l) == nullptr) continue;
                Float_t Y = l->GetY()-(gPad->AbsPixeltoY(l->GetBBox().fY+l->GetBBox().fWidth)-gPad->AbsPixeltoY(l->GetBBox().fY));
                if(Y>YMax)
                    YMax = Y;
            }

            PadMax = gPad->GetUymax();
            if(gPad->GetLogy())
                PadMax = TMath::Power(10,gPad->GetUymax());
        }

        if(YMax<fCurrentHist->GetBinContent(fCurrentHist->GetMaximumBin()))
            fCurrentHist->GetYaxis()->UnZoom();

        gPad->Update();
        gPad->Modified();
    }

    fCanReplot = true;

    return RefLevel;
}

shared_ptr<tklevel_scheme> CXENSDFLevelSchemePlayer::DrawArrowsForNuc(TString NucName, TString DataSet, bool do_draw)
{
    if(!gmanager->known_nucleus(NucName.Data())) return nullptr;
    tknucleus nuc(NucName);

    auto levscheme = nuc.get_level_scheme();
    if(!levscheme) return nullptr;
    levscheme->select_dataset(DataSet.Data());

    auto decays = levscheme->get_decays<tkgammadecay>();
    for(const auto &dec: decays) {
        if(do_draw) DrawArrow(fCurrentHist,dec,nuc);
    }

    return levscheme;
}

void CXENSDFLevelSchemePlayer::DrawArrow(TH1 *hist, const std::shared_ptr<tkn::tkgammadecay> gamma, tknucleus &Nuc)
{
    if(hist == nullptr) return;

    auto level_from = gamma->get_level_from();
    auto level_to = gamma->get_level_to();

    double Energy = gamma->get_energy();

    if(Energy < fCurrentHist->GetBinLowEdge(fCurrentHist->GetXaxis()->GetFirst()) || Energy > fCurrentHist->GetBinLowEdge(fCurrentHist->GetXaxis()->GetLast())) return;

    double Strengh = gamma->get_relative_intensity();

    if( fGuiLSPlayer->UseBranchingRatio() ) {
        if(isnan(Strengh)) return;
        if((Strengh<fMinStrenght || Strengh>fMaxStrenght)) return;
    }

    double ELevI = level_from->get_energy();
    double ELevF = level_to->get_energy();

    if(fGuiLSPlayer->UseELevels() && (ELevI<fMinELevel || ELevI>fMaxELevel)) return;

    double spinI = level_from->get_spin_parity()->get_spin().get_value();

    if(fGuiLSPlayer->UseSpins() && (spinI<fMinSpin || spinI>fMaxSpin)) return;

    if(fGuiLSPlayer->GetYrastMode() && !level_from->is_yrast(true)) return;

    double LifeTime = level_from->get_lifetime();

    if(fGuiLSPlayer->UseLifeTime()) {
        if(isnan(LifeTime)) return;
        if((LifeTime<fMinLifeTime || LifeTime>fMaxLifeTime)) return;
    }

    Float_t MaxGlob = hist->GetMaximum();

    Int_t XMinbin = hist->GetXaxis()->GetFirst();
    Int_t XMaxbin = hist->GetXaxis()->GetLast();

    Int_t XMin = hist->GetBinLowEdge(XMinbin);
    Int_t XMax = hist->GetBinLowEdge(XMaxbin);

    hist->GetXaxis()->SetRangeUser(Energy-3,Energy+3);
    Float_t MaxLoc = hist->GetMaximum();
    hist->GetXaxis()->SetRange(XMinbin, XMaxbin);

    Int_t A = Nuc.get_a();

    TArrow *GammaArrow = new TArrow(Energy,(MaxLoc + MaxGlob/100.) ,Energy,(MaxLoc +MaxGlob/10.),0.005,"<|");

    GammaArrow->SetAngle(40);
    GammaArrow->SetLineColor(fCurrentColor);
    GammaArrow->SetFillColor(fCurrentColor);
    GammaArrow->SetLineWidth(1);

    GammaArrow->SetAngle(40);
    GammaArrow->SetArrowSize(0.01);
    GammaArrow->SetLineWidth(1);

    GammaArrow->Draw();
    GammaArrow->SetBit(TObject::kCannotPick);

    fListOfArrows->Add(GammaArrow);

    TBox *box; box = new TBox();
    box->SetX1(Energy-(XMax-XMin)/300.);
    box->SetX2(Energy+(XMax-XMin)/300.);
    box->SetY1(MaxLoc +1.2*MaxGlob/10.);
    box->SetY2(MaxLoc +2.6*MaxGlob/10.);
    if(fGuiLSPlayer->IsFullTitleMode())
        box->SetY2(MaxLoc +4.0*MaxGlob/10.);
    box->SetFillStyle(0);
    box->SetFillColor(0);
    box->SetLineWidth(0);
    box->SetLineColor(0);
    box->SetLineStyle(3);

    TString GammaTitle=Form("^{%d}%s : %0.1f",A,Nuc.get_element_symbol().data(),Energy);

    TString ToolTip;

    ToolTip.Append(Form("%s : %0.1f keV",Nuc.get_symbol().data(),Energy));

    ///Bug sur la multipolarite pour le moment

    TString Multipolority = gamma->get_multipolarity();

    if(Multipolority != "") ToolTip.Append(Form(" %s",Multipolority.Data()));

    double Mixing = gamma->get_mixing_ratio();
    if(Multipolority != "" && !isnan(Mixing)) ToolTip.Append(Form("(%.1f)",Mixing));

    ToolTip.Append(" ==> ");

    Bool_t AdvancedTitle = fGuiLSPlayer->IsFullTitleMode();

    TString SIL = level_from->get_spin_parity_str();
    TString ILE = Form("%.1f",ELevI);

    ToolTip.Append(Form("%s (%s keV)",SIL.Data(),ILE.Data()));
    if(AdvancedTitle) GammaTitle.Append(Form(" : %s",SIL.Data()));

    ToolTip.Append(" --> ");
    if(AdvancedTitle) GammaTitle.Append("#rightarrow ");

    TString SFL = level_to->get_spin_parity_str();
    TString FLE = Form("%.1f",ELevF);

    ToolTip.Append(Form("%s (%s keV)",SFL.Data(),FLE.Data()));
    if(AdvancedTitle) GammaTitle.Append(Form("%s",SFL.Data()));

    if(!isnan(Strengh)) ToolTip.Append(Form(" ; I=%.1f",Strengh));

    if(!isnan(LifeTime)) {
        TString LifeTimeText = Form(" ; T1/2 = %s",level_from->get_lifetime_str().data());
        ToolTip.Append(LifeTimeText);
        if(AdvancedTitle) GammaTitle.Append(LifeTimeText);
    }

    TLatex *Latex = new TLatex(Energy,MaxLoc +1.2*MaxGlob/10.,GammaTitle);
    Latex->SetTextAngle(90);
    Latex->SetTextFont(132);
    Latex->SetTextSize(fGuiLSPlayer->GetTextSize());
    Latex->SetTextColor(fCurrentColor);
    Latex->SetBit(TObject::kCannotPick);
    Latex->Draw();
    fListOfLatex->Add(Latex);

    box->SetToolTipText(ToolTip.Data(),500);
    box->Draw();
    fListOfBoxes->Add(box);

    fLevelDraw = true;
}

void CXENSDFLevelSchemePlayer::PlotLevelScheme(TString NucName, TString Type, TString DataSet)
{
    gbash_color->WarningMessage("Need to be implemented");

//    LevelScheme *lev = nullptr;

//    if(Type=="ENSDF")
//        lev = ImportENSDFData(NucName,true,DataSet,false);
//    else
//        lev = ImportExpData(NucName);

//    if(lev == nullptr) {
//        cout<<Form("%s Level scheme for %s not found",Type.Data(), NucName.Data())<<endl;
//        return;
//    }

//    fRangeXMin = 1e6;
//    fRangeXMax = -1e6;

//    if(fGuiLSPlayer->GetYrastMode())
//        CalcYrast(lev);

//    fGuiLSPlayer->GetBranchingRatio(fMinStrenght,fMaxStrenght);
//    fGuiLSPlayer->GetELevels(fMinELevel,fMaxELevel);
//    fGuiLSPlayer->GetSpins(fMinSpin,fMaxSpin);
//    fGuiLSPlayer->GetLifeTime(fMinLifeTime,fMaxLifeTime);

//    TCanvas *ctest = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("GLSPlayer");
//    if(ctest != 0x0) ctest->Close();
//    ctest = new TCanvas("GLSPlayer","GLSPlayer");
//    ctest->cd();

//    LevelScheme *LS_To_Plot = new LevelScheme;

//    GLSPlayer *LSPlayer = (GLSPlayer*)LS_To_Plot->GetPlayer();

//    CXNucleus Nuc(NucName);

//    for(Int_t i=0 ; i<lev->GetLinks().GetSize() ; i++) {
//        Gw::GammaLink *Link= (Gw::GammaLink*)lev->GetLinks().At(i);

//        NuclearLevel *NucLevI = (NuclearLevel*)Link->GetIL();
//        NuclearLevel *NucLevF = (NuclearLevel*)Link->GetFL();

//        Float_t Strengh = Link->GetStrength().GetValue();
//        Float_t ELevI   = NucLevI->GetEnergy().GetValue();
//        TString spinI_s = GetSpinFromLev(NucLevI);
//        Float_t spinI   = NucLevI->GetSpin().Get();
//        Float_t ELevF   = NucLevF->GetEnergy().GetValue();
//        TString spinF_s = GetSpinFromLev(NucLevF);
//        Float_t LifeTimeI = NucLevI->GetT().GetValue();
//        Float_t LifeTimeF = NucLevF->GetT().GetValue();
//        TString LifeTimestrI = GetLTString(LifeTimeI);
//        TString LifeTimestrF = GetLTString(LifeTimeF);

//        if( fGuiLSPlayer->UseBranchingRatio() && (Strengh<fMinStrenght || Strengh>fMaxStrenght))
//            continue;

//        if(fGuiLSPlayer->UseELevels() && (ELevI<fMinELevel || ELevI>fMaxELevel))
//            continue;

//        if(fGuiLSPlayer->UseSpins() && (spinI<fMinSpin || spinI>fMaxSpin))
//            continue;

//        if(fGuiLSPlayer->GetYrastMode() && ELevI>fYrastEnergies[2*spinI])
//            continue;

//        //        if(fGuiLSPlayer->GetYrastMode() && ELevF>fYrastEnergies[2*spinF])
//        //            continue;

//        if(fGuiLSPlayer->UseLifeTime() && (LifeTimeI<fMinLifeTime || LifeTimeI>fMaxLifeTime))
//            continue;

//        NuclearLevel *NucLevel_F = GetLevel(NucLevF,LS_To_Plot);
//        if(NucLevel_F == nullptr) {
//            cout<<"tot "<< ELevF<<" "<<(TMath::Abs(ELevF)<0.001)<<endl;
//            if(TMath::Abs(ELevF)<0.001){
//                NucLevel_F = LSPlayer->AddGroundLevel(Nuc.GetZ(),Nuc.GetA(),spinF_s.Data());
//                NucLevel_F->SetLabels(spinF_s.Data(),"",LifeTimestrF.Data(),Form("%g",ELevF));
//            }
//            else {
//                NucLevel_F = LSPlayer->AddLevel(ELevF,0,spinF_s.Data());
//                NucLevel_F->SetLabels(spinF_s.Data(),"",LifeTimestrF.Data(),Form("%g",ELevF));
//            }
//        }

//        LS_To_Plot->SetCLevel(NucLevel_F);

//        NuclearLevel *NucLevel_I = GetLevel(NucLevI,LS_To_Plot);
//        if(NucLevel_I == nullptr) {
//            NucLevel_I = LSPlayer->AddLevel(ELevI,0,spinI_s.Data());
//            NucLevel_I->SetLabels(spinI_s.Data(),"",LifeTimestrI.Data(),Form("%g",ELevI));
//        }

//        LSPlayer->HandleMovement(kKeyPress,-666,kKey_s,NucLevel_I);
//        LSPlayer->HandleMovement(kKeyPress,-666,kKey_s,NucLevel_F);

//        DrawLink(LS_To_Plot,Link);
//    }

//    Float_t YMax=0;
//    for(int i=0 ; i<LS_To_Plot->GetLevels().GetEntries() ; i++) {
//        NuclearLevel *lev = (NuclearLevel*)LS_To_Plot->GetLevels().At(i);
//        lev->SetVisLabel("1111");

//        if(lev->GetEnergy().Get()>YMax)
//            YMax = lev->GetEnergy().Get();

//        lev->SetX1(fRangeXMin);
//        lev->SetX2(fRangeXMax);

//        lev->SetLineColor(kBlack);
//        lev->SetLineWidth(1);
//    }

//    for(int i=0 ; i<LS_To_Plot->GetLinks().GetEntries() ; i++) {
//        Link* link = (Link*)LS_To_Plot->GetLinks().At(i);

//        link->RefreshPoints();
//        link->Paint();
//    }

//    //    LS_To_Plot->SetName("");
//    LS_To_Plot->Draw();
//    LSPlayer->GetLSAxis()->SetRange(0,YMax,fRangeXMin,fRangeXMax);

//    ctest->SetName(Form("ENSDFLS_%s",NucName.Data()));
//    ctest->SetTitle(Form("ENSDF level Scheme for nuc %s",NucName.Data()));

//    gSystem->ProcessEvents();
//    LSPlayer->RefreshLinks();
}

//void CXENSDFLevelSchemePlayer::DrawLink(LevelScheme *ls, GammaLink *gammalink)
//{
//    Float_t energy = gammalink->GetEnergy().Get();
//    Float_t strenght = gammalink->GetStrength().Get();

//    //    cout<<"DrawLink "<<energy<<" "<<strenght<<" "<<GetSpinFromLev((NuclearLevel*)gammalink->GetIL())<<" -> "<<GetSpinFromLev((NuclearLevel*)gammalink->GetFL())<< endl;
//    GLSPlayer *LSPlayer = (GLSPlayer*)ls->GetPlayer();
//    LSPlayer->AddLink(strenght,0,energy);

//    GammaLink *NewLink = (GammaLink*) ls->GetLinks().Last();
//    if(fGuiLSPlayer->GetColorMode()) NewLink->SetArrowStyle(2);
//    else NewLink->SetArrowStyle(1);

//    if(ls->GetLinks().GetEntries()==1) {
//        NewLink->RefreshPoints();
//        NewLink->Paint();
//    }

//    Bool_t ok = false;

//    while(ok==false) {
//        Float_t xmin_new,xmax_new,ymin_new,ymax_new;
//        GetLimits(NewLink,xmin_new,xmax_new,ymin_new,ymax_new);

//        if(xmin_new<fRangeXMin) fRangeXMin = xmin_new;
//        if(xmax_new>fRangeXMax) fRangeXMax = xmax_new;

//        ok = true;

//        for(Int_t i=0 ; i<ls->GetLinks().GetSize()-1 ; i++) {
//            GammaLink *Link= (Gw::GammaLink*)ls->GetLinks().At(i);
//            Float_t xmin,xmax,ymin,ymax;
//            GetLimits(Link,xmin,xmax,ymin,ymax);

//            if((ymin+0.5>ymax_new) || (ymax-0.5<ymin_new))
//                continue;
//            else if((xmax<xmin_new) || (xmin>xmax_new))
//                continue;
//            else {
//                ShiftLink(NewLink,60);
//                ok = false;
//                break;
//            }
//        }
//    }
//}

//void CXENSDFLevelSchemePlayer::GetLimits(GammaLink *link, Float_t &xmin,
//                                    Float_t &xmax, Float_t &ymin,
//                                    Float_t &ymax) {
//    xmin = link->GetX()[0];
//    xmax = link->GetX()[0];
//    ymin = link->GetY()[0];
//    ymax = link->GetY()[0];

//    for (int i = 0; i < 8; i++) {
//        if (link->GetX()[i] < xmin)
//            xmin = link->GetX()[i];
//        if (link->GetX()[i] > xmax)
//            xmax = link->GetX()[i];
//        if (link->GetY()[i] < ymin)
//            ymin = link->GetY()[i];
//        if (link->GetY()[i] > ymax)
//            ymax = link->GetY()[i];
//    }
//}

//void CXENSDFLevelSchemePlayer::ShiftLink(GammaLink *link, Float_t XShift) {
//    for (int i = 0; i < 8; i++) {
//        link->SetPoint(i, link->GetX()[i] + XShift, link->GetY()[i]);
//    }
//}

//void CXENSDFLevelSchemePlayer::ResizeNucLev(NuclearLevel *nuclev, Float_t width) {
//    Float_t Center = nuclev->GetX1() + (nuclev->GetX2() - nuclev->GetX1()) / 2.;

//    nuclev->SetX1(Center - width / 2);
//    nuclev->SetX2(Center + width / 2);
//}

//NuclearLevel *CXENSDFLevelSchemePlayer::GetLevel(NuclearLevel *nuclev,
//                                            LevelScheme *ls) {
//    Float_t ELevel = nuclev->GetEnergy().GetValue();

//    ELevel = TMath::Nint(ELevel * 10.) / 10.;

//    NuclearLevel *level = nullptr;

//    for (Int_t i = 0; i < ls->GetLevels().GetSize(); i++) {
//        level = (NuclearLevel *)ls->GetLevels().At(i);

//        Float_t ELevel2 = level->GetEnergy().GetValue();

//        if (level && TMath::Abs(ELevel2 - ELevel) < 0.1 &&
//            (GetSpinFromLev(nuclev) == GetSpinFromLev(level)))
//            return level;
//    }

//    return nullptr;
//}

void CXENSDFLevelSchemePlayer::ProcessedEventLevelScheme(Int_t eventType, Int_t /*eventX*/, Int_t eventY, TObject* obj)
{
    if( eventType == kKeyPress )
    {
        EKeySym keysym = (EKeySym)eventY;

        if ( keysym == kKey_r)
        {
            Int_t index = -1;
            if(fListOfBoxes->FindObject(obj))
                index = fListOfBoxes->IndexOf(obj);

            if(index != -1)
                RemoveArrow(index);
        }
    }
}

void CXENSDFLevelSchemePlayer::RemoveArrow(Int_t ArrowIndex)
{
    TList *l = fMainWindow->GetCanvas()->GetListOfPrimitives();

    l->Remove(fListOfBoxes->RemoveAt(ArrowIndex));
    l->Remove(fListOfArrows->RemoveAt(ArrowIndex));
    l->Remove(fListOfLatex->RemoveAt(ArrowIndex));

    fMainWindow->GetCanvas()->Modified();
    fMainWindow->GetCanvas()->Update();
}

ClassImp(CXENSDFLevelSchemePlayer);




