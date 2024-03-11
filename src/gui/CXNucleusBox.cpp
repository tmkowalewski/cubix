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

#include "CXNucleusBox.h"

#include "TLatex.h"

#include "CXNucChart.h"
#include "tkmanager.h"

using namespace std;

CXNucleusBox::CXNucleusBox(shared_ptr<tkn::tknucleus> nuc, Double_t size, Int_t /*colStable*/, Int_t colRadio, Int_t textsize, bool withlt, Int_t mode) : TBox(nuc->get_n() - size, nuc->get_z() - size, nuc->get_n() + size, nuc->get_z() + size)
{
    fZ = nuc->get_z();
    fN = nuc->get_n();

    SetLineColor(kBlack);
    SetFillColor(colRadio);
    SetFillStyle(0);
    SetBit(kCannotPick);
    SetBit(kCannotMove);

    fName = Form("(Z=%d,N=%d)",fZ,fN);
    fText = new TLatex(fN,fZ,Form("^{%d}%s",nuc->get_a(),nuc->get_element_symbol().data()));

    fText->SetTextSizePixels(textsize);
    fText->SetTextFont(132);
    fText->SetTextAlign(22);
    fText->SetBit(kCannotMove);
    fText->SetBit(kCannotPick);

//    Float_t width = fText->GetXsize();
    Float_t height = fText->GetYsize();

    //    fText->SetX(fText->GetX()-width*0.75);
    //    fText->SetY(fText->GetY()-height*0.5);

    vector<shared_ptr<tkn::tklevel>> isomers;
    if(mode == CXNucChart::M_1stIsomer || mode == CXNucChart::M_2ndIsomer) {
        isomers = nuc->get_level_scheme()->get_levels( [](auto lvl) {
            return lvl->is_isomer();
        });
    }

    double LifeTime=0.;
    TString LifeTimeStr;

    Float_t BE2;
    Float_t E1rst;

    if(mode == CXNucChart::M_LifeTime ) {
        if(!nuc->get_lifetime_measure())
            withlt = false;
        else {
            LifeTime = nuc->get_lifetime();
            LifeTimeStr = nuc->get_lifetime_str();
        }
    }
    else if(mode == CXNucChart::M_1stIsomer ) {
        if(isomers.size()==0)
            withlt = false;
        else {
            LifeTime = isomers.at(0)->get_lifetime();
            LifeTimeStr = isomers.at(0)->get_lifetime_str();
        }
    }
    else if(mode == CXNucChart::M_2ndIsomer ) {
        if(isomers.size()>1) {
            LifeTime = isomers.at(1)->get_lifetime();
            LifeTimeStr = isomers.at(1)->get_lifetime_str();
        }
        else {
            withlt = false;
        }
    }

    if(mode == CXNucChart::M_1rstExcitedState) {
        auto level_scheme = nuc->get_level_scheme();
        if(level_scheme->get_levels().size()>1) E1rst = level_scheme->get_levels().at(1)->get_energy();
        else withlt = false;
    }
    if(mode == CXNucChart::M_BE2E2B2) {
        auto level_scheme = nuc->get_level_scheme();
        auto gamma = level_scheme->get_decay<tkn::tkgammadecay>("2+1->0+1",false);
        if(gamma) {
            BE2 = gamma->get_trans_prob(true,2,false);
            if(BE2<=0||std::isnan(BE2)) withlt = false;
        }
        else withlt = false;
    }
    if(mode == CXNucChart::M_BE2WU) {
        auto level_scheme = nuc->get_level_scheme();
        auto gamma = level_scheme->get_decay<tkn::tkgammadecay>("2+1->0+1",false);
        if(gamma) {
            BE2 = gamma->get_trans_prob(true,2,true);
            if(BE2<=0||std::isnan(BE2)) withlt = false;
            else BE2 = BE2/((double)nuc->get_a());
        }
        else withlt = false;
    }

    if(withlt) {
        TString Value="";

        if(mode == CXNucChart::M_LifeTime || mode == CXNucChart::M_1stIsomer || mode == CXNucChart::M_2ndIsomer ) {
            Value = LifeTimeStr;
        }
        else if(mode == CXNucChart::M_BE2E2B2 || mode == CXNucChart::M_BE2WU ) {
            Value = Form("%g",BE2);
        }
        else if(mode == CXNucChart::M_1rstExcitedState ) {
            Value = Form("%g",E1rst);
        }

        fTextLT = new TLatex(fN,fZ,Form("%s",Value.Data()));

        fTextLT->SetTextSizePixels(textsize);
        fTextLT->SetTextFont(132);
        fText->SetTextAlign(21);
        fTextLT->SetTextAlign(23);
        fTextLT->SetBit(kCannotMove);
        fTextLT->SetBit(kCannotPick);

        fText->SetY(fText->GetY()+height*0.4);
        fTextLT->SetY(fTextLT->GetY()-height*0.4);

        if(LifeTime>1e15) fTextLT->SetTextColor(0);
    }

    if(LifeTime>1e15) fText->SetTextColor(0);
}

CXNucleusBox::CXNucleusBox(const CXNucleusBox& obj)  : TBox()
{
    obj.Copy(*this);
}

CXNucleusBox::~CXNucleusBox()
{
    delete fText;
    delete fTextLT;
}

void CXNucleusBox::Draw(Option_t *option)
{
    //    cout<<fText->GetTitle()<<" "<<fText->GetX()<<" "<<fText->GetY()<<endl;
    fText->Draw();
    if(fTextLT) fTextLT->Draw();
    TBox::Draw(option);
}

void CXNucleusBox::Copy(TObject& obj) const
{
    TBox::Copy(obj);
}

ClassImp(CXNucleusBox)
