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

#ifndef CXHist1DPlayer_H
#define CXHist1DPlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TObject.h"
#include "TGComboBox.h"
#include "TGNumberEntry.h"

using namespace std;

class CXMainWindow;
class TF1;
class TGraphErrors;
class TH1;
class TGCheckButton;
class TGListBox;
class TVirtualPad;
class CXArrow;
class CXHist1DPlayer;
class CXFit;
class CXBgdFit;
class CXWorkspace;

class CXHist1DPlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXHist1DPlayer");

public:
    TGCheckButton *fUseLT, *fUseRT;
    TGCheckButton *fFixFWHM, *fFWHMFromWS, *fFixLT, *fFixRT, *fFixMean, *fFixAmpli;
    TGCheckButton *fBckStep, *fBckPol1, *fBckExp;
    TGCheckButton *fBckPol1_Bgd, *fBckExp_Bgd;
    TGNumberEntry *fFWHMSigma;
    TGNumberEntry *fNE_FWHM[3];
    TGNumberEntry *fNE_LT[3];
    TGNumberEntry *fNE_RT[3];
    TGTextEntry   *fFitOptions;

    bool fUseStep = true;
    bool fUsePol1 = false;
    bool fUseExp = false;
    bool fUseFWHM = false;
    double fFWHMSigmaValue = 2.;

    bool fUseBgdPol1 = false;
    bool fUseBgdExp = false;

    bool DoNewFit = false;
    bool DoNewBgdFit = false;

private:

    CXMainWindow *fMainWindow = nullptr;

    TGNumberEntry *fSigmaSPEntry = nullptr;
    TGNumberEntry *fThresholdSPEntry = nullptr;
    TGNumberEntry *fTextSize = nullptr;

    TGComboBox *fMinimizer = nullptr;
    TGComboBox *fAlgorithm = nullptr;
    TGNumberEntry *fTolenrance = nullptr;
    TGNumberEntry *fPrintLevel = nullptr;

    TList *fListOfFitObjects = nullptr;
    TList *fListOfBgdFitObjects = nullptr;

    vector<Float_t> fEnergies;
    vector<Float_t> fFitEnergies;
    vector<Float_t> fFitBackgd;

    TH1 *fCurrentHist = nullptr;

    TF1 *fFitFunction = nullptr;
    TF1 *fBackFunction = nullptr;
    TF1 *fResidue = nullptr;
    TList *fListOfPeakFunctions = nullptr;

    TGListBox *fFitResultsBox = nullptr;

    CXFit     *fCurrentFit = nullptr;
    CXBgdFit  *fCurrentBgdFit = nullptr;

public:
    CXHist1DPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXHist1DPlayer();

    Bool_t IsCurrentFit(){return DoNewFit;}

    void PeakSearch();
    void PeakSearchClear();
    void NewFit();
    void NewBgdFit();

    void EndFit(){DoNewFit = false;}
    void DoFit();
    void DoBgdFit();
    void ClearFits();
    void RemoveFit(CXFit *fit);
    void RemoveBgdFit(CXBgdFit *fit);

    void HandleMouse(Int_t EventType,Int_t EventX,Int_t EventY, TObject* selected);
    void HandleMyButton();

    TGListBox *GetFitResultsBox(){return fFitResultsBox;}
    void PrintInListBox(TString mess, Int_t Type);

    CXMainWindow *GetMainWindow(){return fMainWindow;}
    void SetMainWindow(CXMainWindow *w);

    void UpdateMinimizer();
    const char *GetMinimizer() {return fMinimizer->GetSelectedEntry()->GetTitle();}
    const char *GetAlgorithm() {return fAlgorithm->GetSelectedEntry()->GetTitle();}
    Double_t GetTolerance() {return fTolenrance->GetNumber();}
    Int_t GetPrintLevel() {return fPrintLevel->GetNumber();}

    void SaveFit();

    ClassDef(CXHist1DPlayer,0);
};

#endif
