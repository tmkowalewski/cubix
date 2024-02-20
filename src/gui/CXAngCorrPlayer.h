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

#ifndef CXAngCorrPlayer_H
#define CXAngCorrPlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TObject.h"

using namespace std;

class TGTextEntry;
class CXMainWindow;
class TGNumberEntry;
class TF1;
class TGComboBox;
class TGCheckButton;
class CXCanvas;
class TCanvas;
class TGHSlider;
class TVirtualPad;
class TGLabel;
class TGraph;
class TGHProgressBar;
class TGraphAsymmErrors;
class TF2;
class TGRadioButton;
class TH1;

class CXAngCorrPlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXAngCorrPlayer");

public:

private:

    CXMainWindow *fMainWindow = nullptr;

    TGNumberEntry *fNESpins[3];
    TGNumberEntry *fNEMixing[2];
    TGHSlider *fSlider[2];

    TGRadioButton *fAnglesButtons[2];

    TGLabel* fAksLabel = nullptr;

    TGCheckButton *fFixMixing[2];
    TGNumberEntry *fNEMixingPoints = nullptr;
    TGHProgressBar *fProgressBar = nullptr;
    bool fMixingEvalInProcess = false;
    bool fStopMixingEval = false;


    TGNumberEntry *fExpAks[2][3];
    TGNumberEntry *fExpQks[2][3];

    TGCheckButton *fFixQi[2];

    TGLabel* fMixingLabel = nullptr;


    TVirtualPad *fAngCorrPads[4];
    TF1 *fTheoreticalDistribution = nullptr;
    TF1 *fTheoreticalDistributionOnExp = nullptr;

    TGraph *fA2A4MixingGraph = nullptr;
    TGraph *fA2A4MixingTheoMarker = nullptr;

    TGraph *fAngularDistributionGraph = nullptr;
    TF1 *fAngularDistributionFunction = nullptr;
    TH1 *fAngularDistributionFunction_err = nullptr;

    TGraphAsymmErrors *fA2A4ExpMarker = nullptr;

    TF1 *fChi2Func = nullptr;

    TF2 *fChi2Func2D = nullptr;
    TF2 *fChi2Func2D_errband = nullptr;

public:

    CXAngCorrPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXAngCorrPlayer();

    TVirtualPad *GetPad(int i){return fAngCorrPads[i];}

    CXMainWindow *GetMainWindow(){return fMainWindow;}
    void SetMainWindow(CXMainWindow *w);

    void NewInstance();
    void GetCurrentInstance();
    void HandleButtons(int id=-1);

    template <typename T>
    T *GetElement(TVirtualPad *_pad, TString _namepattern="");

    void DoSlider(Int_t _pos);
    void UpdateTheory();

    void FitDistribution();

    void UpdateData();
    void FitMixing();
    void FitMixing1D();
    void FitMixing2D();

    void PlotMixingEvaluation();
    void StopMixingEvaluation(){fStopMixingEval=true;}

    void FitCorrectionFactors();

    void SaveCorrectionFactors();
    void SetQis(double _Q2, double _Q2low, double _Q2high, double _Q4, double _Q4low, double _Q4high);

    void PlotTheoryOnDist();

private:

    double Fk(int twoL1, int twoL1p, int twoIi, int twoI, int k);
    vector<double> Eval_Ak(int TwoJ1, int TwoJ2, int TwoJ3, double _mix12, double _mix23);
    double TheoreticalAngCorrFunction(double *x, double *p);
    double ExpAngCorrFunction(double *x, double *p);

    double EvalChi2(double *x, double *p);
    double EvalChi2Deriv(double *x,double */*parameters*/);
    double EvalChi2Deriv2(double *x,double */*parameters*/);

    ClassDef(CXAngCorrPlayer,0);
};

#endif
