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

#include "CXGammaSearch.h"
#include "cubix_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "TString.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TGListBox.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include "TGButton.h"

// #if (OS_TYPE == OS_LINUX)
#include "TGResourcePool.h"
// #endif

#include "CXMainWindow.h"
#include "CXSpreadIntensityMatrix.h"
#include "CXBashColor.h"

#include "tkmanager.h"

using namespace std;

CXGammaSearch::CXGammaSearch(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, CXMainWindow *mwin) :
    TGTransientFrame(p, main, w, h, kHorizontalFrame),
    fMainWindow(mwin)
{
    fSp = new CXSpreadIntensityMatrix;

    TGCompositeFrame *Main = new TGCompositeFrame(this,600,100,kVerticalFrame);

    TGLayoutHints *GroupHints = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5, 0, 0);

    AddFrame(Main,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY,10,10,10,10));

    //Gammas Energy

    TGGroupFrame *gFrame = new TGGroupFrame(Main, "Input Gammas rays", kVerticalFrame);
    gFrame->SetTextColor(CXblue);
    Main->AddFrame(gFrame, GroupHints);

    TGCompositeFrame *hFrame = new TGCompositeFrame(gFrame, 60, 20, kHorizontalFrame);
    hFrame->AddFrame(new TGLabel(hFrame, "Energy (keV)"),new TGLayoutHints(kLHintsTop | kLHintsLeft, 21, 0, 3, 0));
    hFrame->AddFrame(new TGLabel(hFrame, "Width (keV)"),new TGLayoutHints(kLHintsTop | kLHintsLeft, 35, 0, 3, 0));
    gFrame->AddFrame(hFrame,new TGLayoutHints(  kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

    ///gammas
    for(int i=0 ; i<3 ; i++) {
        gFrame->AddFrame(hFrame = new TGCompositeFrame(gFrame, 60, 20, kHorizontalFrame),new TGLayoutHints(  kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));
        hFrame->AddFrame(fCheckGammas[i] = new TGCheckButton(hFrame, "", 0), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,0,5,0,0));
        hFrame->AddFrame(fEnergies[i] = new TGNumberEntry(hFrame, 500.0, 8, 21,TGNumberFormat::kNESRealOne, TGNumberFormat::kNEANonNegative),new TGLayoutHints(  kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));
        hFrame->AddFrame(fWidths[i] = new TGNumberEntry(hFrame, 1.0, 5, 21,TGNumberFormat::kNESRealOne, TGNumberFormat::kNEANonNegative), new TGLayoutHints(  kLHintsExpandX | kLHintsExpandY, 20, 0, 0, 0));
        fCheckGammas[i]->Connect("Clicked()", "CXGammaSearch", this, "HandleButtons()");
    }
    fCheckGammas[0]->SetState(kButtonDown);

    // Selected range
    gFrame = new TGGroupFrame(Main, "Selected range", kVerticalFrame);
    gFrame->SetTextColor(CXblue);
    Main->AddFrame(gFrame, GroupHints);

    hFrame = new TGCompositeFrame(gFrame, 60, 20, kHorizontalFrame);
    hFrame->AddFrame(fZRange[0] = new TGNumberEntry(hFrame, 28, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative),new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));
    hFrame->AddFrame(new TGLabel(hFrame, " < Z < "),new TGLayoutHints(kLHintsCenterY, 0, 0, 0, 0));
    hFrame->AddFrame(fZRange[1] = new TGNumberEntry(hFrame, 50, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative),new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));
    gFrame->AddFrame(hFrame,new TGLayoutHints(  kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

    hFrame = new TGCompositeFrame(gFrame, 60, 20, kHorizontalFrame);
    hFrame->AddFrame(fNRange[0] = new TGNumberEntry(hFrame, 20, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative),new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));
    hFrame->AddFrame(new TGLabel(hFrame, " < N < "),new TGLayoutHints(kLHintsCenterY, 0, 0, 0, 0));
    hFrame->AddFrame(fNRange[1] = new TGNumberEntry(hFrame, 82, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative),new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));
    gFrame->AddFrame(hFrame,new TGLayoutHints(  kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

    hFrame = new TGCompositeFrame(gFrame, 60, 20, kHorizontalFrame);
    hFrame->AddFrame(fARange[0] = new TGNumberEntry(hFrame, fZRange[0]->GetNumber()+fNRange[0]->GetNumber(), 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative),new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));
    hFrame->AddFrame(new TGLabel(hFrame, " < A < "),new TGLayoutHints(kLHintsCenterY, 0, 0, 0, 0));
    hFrame->AddFrame(fARange[1] = new TGNumberEntry(hFrame, fZRange[1]->GetNumber()+fNRange[1]->GetNumber(), 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative),new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,1,1,0,0));
    gFrame->AddFrame(hFrame,new TGLayoutHints(  kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

    fZRange[0]->Connect("ValueSet(Long_t)", "CXGammaSearch", this, "HandleButtons()");
    fZRange[1]->Connect("ValueSet(Long_t)", "CXGammaSearch", this, "HandleButtons()");
    fNRange[0]->Connect("ValueSet(Long_t)", "CXGammaSearch", this, "HandleButtons()");
    fNRange[1]->Connect("ValueSet(Long_t)", "CXGammaSearch", this, "HandleButtons()");
    fARange[0]->Connect("ValueSet(Long_t)", "CXGammaSearch", this, "HandleButtons()");
    fARange[1]->Connect("ValueSet(Long_t)", "CXGammaSearch", this, "HandleButtons()");

    //Finder

    gFrame = new TGGroupFrame(Main, "Gamma search", kVerticalFrame);
    gFrame->SetTextColor(CXblue);
    Main->AddFrame(gFrame, GroupHints);

    fStartButton = new TGTextButton(gFrame, "Start");
    fStartButton->Connect("Clicked()", "CXGammaSearch", this, "SetCalMode()");
    gFrame->AddFrame(fStartButton,new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX,15,15,10,10));

    fNoCoincMode = new TGRadioButton(gFrame,"Only Gammas",M_WithOut_Cascade);
    fCoincMode = new TGRadioButton(gFrame,"In Coincidence",M_With_Cascade);
    fNoCoincMode->SetState(EButtonState::kButtonDown);
    fCurrentMode=M_WithOut_Cascade;

    fCoincMode->Connect("Clicked()","CXGammaSearch", this, "HandleButtons()");
    fNoCoincMode->Connect("Clicked()","CXGammaSearch", this, "HandleButtons()");

    gFrame->AddFrame(fNoCoincMode,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,10,10,10,10));
    gFrame->AddFrame(fCoincMode,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,10,10,10,10));


    //Results panel

    Main = new TGCompositeFrame(this,400,100,kVerticalFrame);
    TGLabel *lab = new TGLabel(Main,"Results");
    lab->SetTextColor(CXred);
    Main->AddFrame(lab,new TGLayoutHints(kLHintsTop | kLHintsLeft,0,0,0,0));

    fNNucAnalysed = new TGLabel(Main,Form("Analysed nuclei: %5.0d",0));
    fNNucAnalysed->SetTextColor(CXblue);
    Main->AddFrame(fNNucAnalysed,new TGLayoutHints(kLHintsTop | kLHintsLeft,0,0,0,0));
    fNGrayAnalysed = new TGLabel(Main,Form("Analysed gamma rays: %5.0d",0));
    fNGrayAnalysed->SetTextColor(CXblue);
    Main->AddFrame(fNGrayAnalysed,new TGLayoutHints(kLHintsTop | kLHintsLeft,0,0,0,0));

    fResultsBox = new TGListBox(Main);
    fResultsBox->GetContainer()->RemoveInput(kButtonReleaseMask | kButtonMotionMask);

    Main->AddFrame(fResultsBox, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,0,0,0,0));

    AddFrame(Main,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,10,10,10,10));

    HandleButtons();

    SetCleanup(kDeepCleanup);
    SetWindowName("GammaSearch");
    CenterOnParent();
    MapSubwindows();
    Layout();
    MapWindow();
}

CXGammaSearch::~CXGammaSearch()
{
    fMainWindow->fGammaSearchWindow = nullptr;

    UnmapWindow();
    CloseWindow();
}

void CXGammaSearch::SetCalMode()
{
    if(!fMainWindow->is_db_loaded()) {
        fMainWindow->pause_db_loading(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    switch (fCurrentMode) {
    case M_With_Cascade: {
        if(fNGammas<=1)
            gbash_color->ErrorMessage("Soory, I cannot determine coincidence between one or less gamma ray...");
        else if(fNGammas==2) {
            FindGammaRays(true);
            FindInDoubleCoincidence();
        }
        else {
            FindGammaRays(true);
            FindInTripleCoincidence();
        }
        break;
    }
    case M_WithOut_Cascade: {
        if(fNGammas>0)
            FindGammaRays();
        else
            gbash_color->ErrorMessage("Soory, no requested energies...");
        break;
    }
    }

    if(!fMainWindow->is_db_loaded()) fMainWindow->pause_db_loading(false);
}

void CXGammaSearch::FindGammaRays(Bool_t Bash)
{
    Int_t AnalysedNuclei=0;
    Int_t AnalysedGammaRays=0;

    fResultsBox->RemoveAll();

    Int_t ZMin = fZRange[0]->GetNumber();
    Int_t ZMax = fZRange[1]->GetNumber();
    Int_t AMin = fARange[0]->GetNumber();
    Int_t AMax = fARange[1]->GetNumber();

    double Gates[fNGammas];
    double Width[fNGammas];

    for(int ig=0 ; ig<fNGammas ; ig++) {
        Gates[ig] = fEnergies[ig]->GetNumber();
        Width[ig] = fWidths[ig]->GetNumber();
    }

    fListOfGoodGammas.clear();

    int inuc=0;
    for(Int_t iz=ZMin ; iz<=ZMax ; iz++) {
        for(Int_t ia=AMin ; ia<=AMax ; ia++) {
            inuc++;
            if(!gmanager->known_nucleus(iz,ia)) continue;
            glog.progress_bar((ZMax-ZMin+1)*(AMax-AMin+1),inuc,"Search for energies in nuclei level schemes...");
            AnalysedNuclei++;

            tkn::tknucleus Nuc(iz,ia);
            fLevelScheme = Nuc.get_level_scheme();
            if(fLevelScheme->get_decays().size()==0) continue;
            AnalysedGammaRays+=fLevelScheme->get_decays().size();

            vector<shared_ptr<tkn::tkdecay>> GoodGammas;

            Bool_t gatetrig[fNGammas];
            memset(gatetrig,0,sizeof(fNGammas));

            Int_t NGatesTrig=0;
            for(int ig=0 ; ig<fNGammas ; ig++) {

                auto decays = fLevelScheme->get_decays<tkn::tkgammadecay>( [&ig, &Gates, &Width](auto dec) {
                    return TMath::Abs(dec->get_energy()-Gates[ig])<=Width[ig];
                });

                if(decays.size()==0) continue;

                gatetrig[ig] = true;
                NGatesTrig++;

                for(auto &dec: decays) {
                    GoodGammas.push_back(dec);
                }
            }

            if(NGatesTrig == fNGammas) {
                if(!Bash) PrintInListBox(Form("%s (Z=%d, A=%d, N=%d)",Nuc.get_symbol().data(),Nuc.get_z(),Nuc.get_a(),Nuc.get_n()),kInfo);

                std::vector< GammaTransition > avect;

                for(size_t ig=0 ; ig<GoodGammas.size() ; ig++) {
                    GammaTransition agammatrans;

                    auto Link = GoodGammas[ig];
                    auto NucLevI = Link->get_level_from();
                    auto NucLevF = Link->get_level_to();

                    double Energy = Link->get_energy();

                    double ELevI = NucLevI->get_energy(tkn::tkunit_manager::keV,true);
                    TString spinI = NucLevI->get_spin_parity_str();
                    TString offsetI="";
                    if(NucLevI->is_energy_offset()) offsetI += NucLevI->get_offset_bandhead() + "+";

                    double ELevF = NucLevF->get_energy(tkn::tkunit_manager::keV,true);
                    TString spinF = NucLevF->get_spin_parity_str();

                    agammatrans.NucName = Nuc.get_symbol();
                    agammatrans.EGamma = Energy;
                    agammatrans.EI = ELevI;
                    agammatrans.EF = ELevF;
                    agammatrans.SpinI = spinI;
                    agammatrans.SpinF = spinF;
                    agammatrans.LifeTime = NucLevI->get_lifetime_str().data();
                    agammatrans.Offset =offsetI ;

                    avect.push_back(agammatrans);

                    TString GammaTitle = Form(" => %6.1f keV : %5s (%s%6.1f keV) --> %5s (%s%6.1f keV)%s",Energy,spinI.Data(),offsetI.Data(),ELevI,spinF.Data(),offsetI.Data(),ELevF, agammatrans.LifeTime.Data());
                    if(!Bash) PrintInListBox(GammaTitle.Data(),kPrint);
                }

                fListOfGoodGammas.push_back(avect);

                fNNucAnalysed->SetText(Form("Analysed nuclei: %5.0d",AnalysedNuclei));
                fNGrayAnalysed->SetText(Form("Analysed gamma rays: %5.0d",AnalysedGammaRays));

                gSystem->ProcessEvents();
            }
        }
    }
    cout << endl;

    MapSubwindows();
    MapWindow();
    Layout();
}

void CXGammaSearch::FindInDoubleCoincidence(Bool_t Bash)
{
    //Pre-find nuclei with the right gammas
    std::vector<double> Tab_P;

    std::vector<int> Index_Nuclei;
    std::vector<string> Nuclei_ID;
    std::vector<double> DeltaE;

    Int_t Count=0;

    Int_t NumberOfNuclei = fListOfGoodGammas.size();

    for(int inuc=0 ; inuc<NumberOfNuclei ; inuc++) {

        glog.progress_bar(NumberOfNuclei,inuc,"Search for double coincidences...");

        vector < GammaTransition > avec = fListOfGoodGammas[inuc];

        if(avec.size()<2) continue;

        tkn::tknucleus Nuc(((GammaTransition)avec[0]).NucName.Data());
        fLevelScheme = Nuc.get_level_scheme();

        /// taille du schema de niveau
        const Int_t Dim_LS = fLevelScheme->get_decays().size();

        double Flag[Dim_LS];
        for(int ii=0 ; ii<Dim_LS ; ii++) Flag[ii] = -1;

        TMatrixD Mat_LS(Dim_LS,Dim_LS); // Raw matrix in TS Space
        TMatrixD Mat_P(Dim_LS,Dim_LS);  // Total probability matrix
        TMatrixD Mat_E(Dim_LS,1);       // Total probability matrix

        Mat_LS = Fill_TS_Matrix(fLevelScheme);
        Mat_P  = fSp->Get_Total_P(Mat_LS,Dim_LS);
        Mat_E  = Get_E_Matrix(fLevelScheme);

        Bool_t Det = fSp->IsDet(Mat_LS,Dim_LS);

        TString TransitionName[Dim_LS];
        double ETrans[Dim_LS];
        double EI[Dim_LS];
        double EF[Dim_LS];

        //get the coordinates of the gammas
        for(Int_t ig=0 ; ig<Dim_LS ; ig++) {
            for(size_t j=0 ; j<avec.size() ; j++) {
                GammaTransition GT = avec[j];
                if(GT.EGamma == Mat_E[ig][0]) {
                    Flag[ig] = Mat_E[ig][0];
                    TransitionName[ig] = Form(" %6.1f keV : %5s (%s%6.1f keV) --> %5s (%s%6.1f keV)%s ",GT.EGamma,GT.SpinI.Data(),GT.Offset.Data(),GT.EI,GT.SpinF.Data(),GT.Offset.Data(),GT.EF,GT.LifeTime.Data());
                    ETrans[ig] = GT.EGamma;
                    EI[ig] = GT.EI;
                    EF[ig] = GT.EF;
                }
            }
        }

        Bool_t gatetrig[fNGammas];
        double Gates[fNGammas];
        double Width[fNGammas];

        for(int ig=0 ; ig<fNGammas ; ig++) {
            Gates[ig] = fEnergies[ig]->GetNumber();
            Width[ig] = fWidths[ig]->GetNumber();
        }

        for(Int_t j=0 ; j<Dim_LS ; j++) {
            for(Int_t m=0 ; m<j ; m++) {
                memset(gatetrig,0,sizeof(fNGammas));
                Int_t NGatesTrig=0;

                for(int ig=0 ; ig<fNGammas ; ig++) {
                    if(TMath::Abs(ETrans[j]-Gates[ig])<=Width[ig])
                        gatetrig[ig] = true;
                    if(TMath::Abs(ETrans[m]-Gates[ig])<=Width[ig])
                        gatetrig[ig] = true;
                }

                for(int ii=0 ; ii<fNGammas ; ii++)
                    NGatesTrig += gatetrig[ii];

                if(NGatesTrig != 2)
                    continue;

                double Prob = TMath::Abs(Mat_P[j][m]);
                if(Flag[j]>0 && Flag[m]>0 && Prob>0.01) {
                    //Save the probabilities
                    Tab_P.push_back(Prob);
                    Index_Nuclei.push_back(Count);

                    if(EI[j]>EI[m])
                        DeltaE.push_back(EF[j]-EI[m]);
                    else
                        DeltaE.push_back(EF[m]-EI[j]);

                    TString Name_Output = Form("%s# --> %s# --> %s, Prob = %1.3f", Nuc.get_symbol().data(),TransitionName[j].Data(),TransitionName[m].Data(),Prob);

                    if (Det==false)
                        Name_Output+="Warning Matrix Singular";

                    Nuclei_ID.push_back(Name_Output.Data());
                    Count++;
                }
            }
        }

        gSystem->ProcessEvents();
    }

    cout << endl;

    std::vector<int> Index_Sorted;
    Index_Sorted = Sort_Index(Index_Nuclei,Tab_P, DeltaE);

    if(!Bash) {
        for(Int_t j=(Int_t)Tab_P.size()-1 ; j>=0 ; j--) {
            TString Name = Nuclei_ID.at(Index_Sorted[j]);
            TObjArray *arr = Name.Tokenize("#");
            TString NucName = Form("%s : Energy Diff = %1.3f",arr->First()->GetName(),DeltaE[Index_Sorted[j]]);
            TString Transition1 = arr->At(1)->GetName();
            TString Transition2 = arr->At(2)->GetName();

            PrintInListBox(NucName,kInfo);
            PrintInListBox(Transition1,kPrint);
            PrintInListBox(Transition2,kPrint);
        }
        PrintInListBox(Form("Done... %d corresponding nuclei found",((Int_t)Tab_P.size())),kInfo);
    }
}

void CXGammaSearch::FindInTripleCoincidence()
{
    std::vector<double> Tab_P;

    std::vector<int> Index_Nuclei;
    std::vector<string> Nuclei_ID;
    std::vector<double> DeltaE;

    Int_t Count=0;

    Int_t NumberOfNuclei = fListOfGoodGammas.size();

    for(int inuc=0 ; inuc<NumberOfNuclei ; inuc++) {

        glog.progress_bar(NumberOfNuclei,inuc,"Search for triple coincidences...");

        vector < GammaTransition > avec = fListOfGoodGammas[inuc];

        if(avec.size()<3) continue;

        tkn::tknucleus Nuc(((GammaTransition)avec[0]).NucName.Data());
        fLevelScheme = Nuc.get_level_scheme();

        /// taille du schema de niveau
        const Int_t Dim_LS = fLevelScheme->get_decays().size();

        double Flag[Dim_LS];
        for(int ii=0 ; ii<Dim_LS ; ii++)
            Flag[ii] = -1;

        TMatrixD Mat_LS(Dim_LS,Dim_LS); // Raw matrix in TS Space
        TMatrixD Mat_P(Dim_LS,Dim_LS);  // Total probability matrix
        TMatrixD Mat_E(Dim_LS,1);       // Total probability matrix

        Mat_LS = Fill_TS_Matrix(fLevelScheme);
        Mat_P  = fSp->Get_Total_P(Mat_LS,Dim_LS);
        Mat_E  = Get_E_Matrix(fLevelScheme);

        Bool_t Det = fSp->IsDet(Mat_LS,Dim_LS);

        TString TransitionName[Dim_LS];
        double ETrans[Dim_LS];
        double EI[Dim_LS];
        double EF[Dim_LS];

        //get the coordinates of the gammas
        for(Int_t ig=0 ; ig<Dim_LS ; ig++) {
            for(size_t j=0 ; j<avec.size() ; j++) {
                GammaTransition GT = avec[j];

                if(GT.EGamma == Mat_E[ig][0]) {
                    Flag[ig] = Mat_E[ig][0];
                    TransitionName[ig] = Form(" %6.1f keV : %5s (%6.1f keV) --> %5s (%6.1f keV)%s ",GT.EGamma,GT.SpinI.Data(),GT.EI,GT.SpinF.Data(),GT.EF,GT.LifeTime.Data());
                    ETrans[ig] = GT.EGamma;
                    EI[ig] = GT.EI;
                    EF[ig] = GT.EF;
                }
            }
        }

        Bool_t gatetrig[fNGammas];
        double Gates[fNGammas];
        double Width[fNGammas];

        for(int ig=0 ; ig<fNGammas ; ig++) {
            Gates[ig] = fEnergies[ig]->GetNumber();
            Width[ig] = fWidths[ig]->GetNumber();
        }

        for(Int_t j=0 ; j<Dim_LS ; j++) {
            for(Int_t m=0 ; m<j ; m++) {
                for(Int_t k=0 ; k<m ; k++) {
                    memset(gatetrig,0,sizeof(fNGammas));
                    Int_t NGatesTrig=0;

                    for(int ig=0 ; ig<fNGammas ; ig++) {
                        if(TMath::Abs(ETrans[j]-Gates[ig])<=Width[ig])
                            gatetrig[ig] = true;
                        if(TMath::Abs(ETrans[m]-Gates[ig])<=Width[ig])
                            gatetrig[ig] = true;
                        if(TMath::Abs(ETrans[k]-Gates[ig])<=Width[ig])
                            gatetrig[ig] = true;
                    }

                    for(int ii=0 ; ii<fNGammas ; ii++) NGatesTrig += gatetrig[ii];

                    if(NGatesTrig != 3) continue;

                    double Prob = TMath::Abs(Mat_P[j][m]*Mat_P[j][k]*Mat_P[m][k]);

                    if(Flag[j]>0 && Flag[m]>0 && Flag[k]>0 && Prob>0.01 ) {
                        //Save the probabilities
                        Tab_P.push_back(Prob);
                        Index_Nuclei.push_back(Count);

                        if(EI[j]>EI[m] && EI[j]>EI[k]) {
                            if(EI[m]>EI[k])
                                DeltaE.push_back(EF[j]-EI[m] + EF[m]-EI[k]);
                            else
                                DeltaE.push_back(EF[j]-EI[k] + EF[k]-EI[m]);
                        }
                        else if(EI[m]>EI[j] && EI[m]>EI[k]) {
                            if(EI[j]>EI[k])
                                DeltaE.push_back(EF[m]-EI[j] + EF[j]-EI[k]);
                            else
                                DeltaE.push_back(EF[m]-EI[k] + EF[k]-EI[j]);
                        }
                        else if(EI[k]>EI[j] && EI[k]>EI[m]) {
                            if(EI[j]>EI[m])
                                DeltaE.push_back(EF[k]-EI[j] + EF[j]-EI[m]);
                            else
                                DeltaE.push_back(EF[k]-EI[m] + EF[m]-EI[j]);
                        }

                        TString Name_Output = Form("%s# --> %s# --> %s# --> %s, Prob = %1.3f", Nuc.get_symbol().data(),TransitionName[j].Data(),TransitionName[m].Data(),TransitionName[k].Data(),Prob);

                        if (Det==false)
                            Name_Output+="Warning Matrix Singular";

                        Nuclei_ID.push_back(Name_Output.Data());
                        Count++;
                    }
                }
            }
            gSystem->ProcessEvents();
        }
    }

    cout << endl;

    std::vector<int> Index_Sorted;
    Index_Sorted = Sort_Index(Index_Nuclei,Tab_P, DeltaE);


    for(Int_t j=(Int_t)Tab_P.size()-1 ; j>=0 ; j--) {
        TString Name = Nuclei_ID.at(Index_Sorted[j]);
        TObjArray *arr = Name.Tokenize("#");
        TString NucName = Form("%s : Energy Diff = %1.3f",arr->First()->GetName(),DeltaE[Index_Sorted[j]]);
        TString Transition1 = arr->At(1)->GetName();
        TString Transition2 = arr->At(2)->GetName();
        TString Transition3 = arr->At(3)->GetName();

        PrintInListBox(NucName,kInfo);
        PrintInListBox(Transition1,kPrint);
        PrintInListBox(Transition2,kPrint);
        PrintInListBox(Transition3,kPrint);
    }

    PrintInListBox(Form("Done... %d corresponding nuclei found",((Int_t)Tab_P.size())),kInfo);
}

std::vector<int> CXGammaSearch::Sort_Index(std::vector<int> Index_Nuclei, std::vector<double> Tab_P, std::vector<double> Tab_DeltaE)
{
    std::vector<int> vecX;

    for(unsigned int i=1 ; i<Index_Nuclei.size() ; i++)
        for(unsigned int j=0 ; j<Index_Nuclei.size()-1 ; j++)
            if(Tab_P[j]>Tab_P[j+1]) {
                swap(Index_Nuclei[j],Index_Nuclei[j+1]);
                swap(Tab_P[j],Tab_P[j+1]);
                swap(Tab_DeltaE[j],Tab_DeltaE[j+1]);
            }

    for(unsigned int i=1 ; i<Index_Nuclei.size() ; i++)
        for(unsigned int j=0 ; j<Index_Nuclei.size()-1 ; j++) {
            if(TMath::Abs(Tab_P[j]-Tab_P[j+1])<1e-4) {
                if(Tab_DeltaE[j]<Tab_DeltaE[j+1]) {
                    swap(Index_Nuclei[j],Index_Nuclei[j+1]);
                    swap(Tab_P[j],Tab_P[j+1]);
                    swap(Tab_DeltaE[j],Tab_DeltaE[j+1]);
                }
            }
        }


    for(uint i=0 ; i<Tab_P.size() ; i++)
        vecX.push_back(Index_Nuclei[i]);

    return vecX ;
}

void CXGammaSearch::PrintInListBox(TString mess, Int_t Type)
{

// #if (OS_TYPE == OS_LINUX)
    const TGFont *ufont;         // will reflect user font changes
    ufont = gClient->GetFont("-adobe-courier-medium-r-*-*-14-*-*-*-*-*-iso8859-1");
    // ufont = gClient->GetFont("-adobe-times-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
    if (!ufont)
        ufont = fClient->GetResourcePool()->GetDefaultFont();

    TGGC   *uGC;           // will reflect user GC changes
    // graphics context changes
    GCValues_t val;
    val.fMask = kGCFont;
    val.fFont = ufont->GetFontHandle();
    uGC = gClient->GetGC(&val, kTRUE);

    TGTextLBEntry *entry = new TGTextLBEntry(fResultsBox->GetContainer(), new TGString(mess), fResultsBox->GetNumberOfEntries()+1, uGC->GetGC(), ufont->GetFontStruct());
// #else
//     TGTextLBEntry *entry = new TGTextLBEntry(fResultsBox->GetContainer(), new TGString(mess), fResultsBox->GetNumberOfEntries()+1);
// #endif

    if(Type == kError)
        entry->SetBackgroundColor((Pixel_t)0xff0000);
    else if(Type == kInfo)
        entry->SetBackgroundColor((Pixel_t)0x87a7d2);
    else if(Type == kWarning)
        entry->SetBackgroundColor((Pixel_t)0xdfdf44);
    else if(Type == kPrint)
        entry->SetBackgroundColor((Pixel_t)0x90f269);

    fResultsBox->AddEntry((TGLBEntry *)entry, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX));
    fResultsBox->Layout();
}

void CXGammaSearch::HandleButtons()
{
    if(((TObject*)gTQSender)->InheritsFrom("TGNumberEntry")) {
        if(gTQSender == fNRange[0])
            fARange[0]->SetNumber(fZRange[0]->GetNumber()+fNRange[0]->GetNumber());
        if(gTQSender == fNRange[1])
            fARange[1]->SetNumber(fZRange[1]->GetNumber()+fNRange[1]->GetNumber());
        if(gTQSender == fARange[0])
            fNRange[0]->SetNumber(fARange[0]->GetNumber()-fZRange[0]->GetNumber());
        if(gTQSender == fARange[1])
            fNRange[1]->SetNumber(fARange[1]->GetNumber()-fZRange[1]->GetNumber());
        if(gTQSender == fZRange[0])
            fARange[0]->SetNumber(fZRange[0]->GetNumber()+fNRange[0]->GetNumber());
        if(gTQSender == fZRange[1])
            fARange[1]->SetNumber(fZRange[1]->GetNumber()+fNRange[1]->GetNumber());
    }

    if(((TObject*)gTQSender)->InheritsFrom("TGRadioButton")) {
        TGButton *btn = (TGButton *) gTQSender;
        Int_t id = btn->WidgetId();

        if(id==M_With_Cascade) {
            fNoCoincMode->SetState(kButtonUp);
            fCoincMode->SetState(kButtonDown);
            fCurrentMode=M_With_Cascade;
        }
        if(id ==M_WithOut_Cascade) {
            fCoincMode->SetState(kButtonUp);
            fNoCoincMode->SetState(kButtonDown);
            fCurrentMode=M_WithOut_Cascade;
        }
    }


    fNGammas = 0;
    for(int i=0 ; i<3 ; i++) {
        fEnergies[i]->SetState(fCheckGammas[i]->GetState());
        fWidths[i]->SetState(fCheckGammas[i]->GetState());

        if(fCheckGammas[i]->GetState() == kButtonDown)
            fNGammas++;
    }
}

TMatrixD CXGammaSearch::Fill_TS_Matrix(shared_ptr<tkn::tklevel_scheme> lev)
{
    auto decays = lev->get_decays<tkn::tkgammadecay>();
    size_t Size = decays.size();

    TMatrixD Fill_M(Size,Size);

    for(size_t i=0 ; i<Size ; i++) {
        auto Link = decays.at(i);
        auto FL = Link->get_level_to();

        double Br=0;

        //Get The branching ratio
        for(size_t j=0 ; j<Size ; j++) {

            auto Link2 = decays.at(j);
            auto IL = Link2->get_level_from();
            auto S = Link2->get_relative_intensity();
            if(isnan(S) || S<1e-6) S = 1e-6;
            if(FL==IL) Br += S;
        }

        // Fill the matrix
        for(size_t j=0 ; j<Size ; j++) {

            auto Link2 = decays.at(j);
            auto IL = Link2->get_level_from();
            auto S = Link2->get_relative_intensity();
            if(isnan(S) || S<1e-6) S = 1e-6;
            if(FL==IL && !isnan(S/Br)) Fill_M[i][j]=S/Br;
        }
    }

    return Fill_M;
}

TMatrixD CXGammaSearch::Get_E_Matrix(shared_ptr<tkn::tklevel_scheme> lev)
{
    auto decays = lev->get_decays<tkn::tkgammadecay>();
    size_t Size = decays.size();

    TMatrixD Fill_E(Size,1);

    // Fill the matrix with the Energy
    for(size_t j=0 ; j<Size ; j++) {
        Fill_E[j][0] = decays.at(j)->get_energy();
    }

    return Fill_E;
}

ClassImp(CXGammaSearch);


