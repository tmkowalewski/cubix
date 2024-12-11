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

#ifndef CXHist1DCalib_H
#define CXHist1DCalib_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TObject.h"

#include "CXRecalEnergy.h"

using namespace std;

class TGTextEntry;
class CXMainWindow;
class TGNumberEntry;
class TF1;
class TH2;
class TGComboBox;
class TGCheckButton;
class CXCanvas;
class TCanvas;

class CXHist1DCalib : public  TGVerticalFrame
{
    RQ_OBJECT("CXHist1DCalib");

public:

private:

    TGTextEntry *fSources = nullptr;
    CXMainWindow *fMainWindow = nullptr;

    TGNumberEntry *fSourceEnergyRangeMin = nullptr;
    TGNumberEntry *fSourceEnergyRangeMax = nullptr;
    TGNumberEntry *fSourceIntensityRangeMin = nullptr;
    TGNumberEntry *fSourceIntensityRangeMax = nullptr;

    TGNumberEntry *fRangeMin = nullptr;
    TGNumberEntry *fRangeMax = nullptr;
    TGNumberEntry *fFWHMSPEntry = nullptr;
    TGNumberEntry *fThresholdSPEntry = nullptr;
    TGNumberEntry *fVerboseLevel = nullptr;

    TGNumberEntry *fCalibOrder = nullptr;
    TGCheckButton *fNoOffset = nullptr;

    TGCheckButton *fLeftTail = nullptr;
    TGCheckButton *fRightTail = nullptr;
    TGCheckButton *f2DSearch = nullptr;

    TList *fListOfObjects = nullptr;

    vector< pair<double,double> > fEnergies;
    vector<array<double, 4>> fIntensities;
    Double_t fERef=0.;

    CXRecalEnergy *fRecalEnergy = nullptr;
    TCanvas *fCalibCanvas = nullptr;
    TCanvas *fCalib2DCanvas = nullptr;
    TF1 *fCalibFunction = nullptr;
    TGraph *fCalib2DGraph = nullptr;

    TCanvas *fFWHMCanvas = nullptr;
    TGraph *fFWHMGraph = nullptr;
    TF1 *fFWHMFunction = nullptr;
    TH1 *fFWHMConfidenceIntervall = nullptr;

public:

    CXHist1DCalib(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXHist1DCalib();

    CXMainWindow *GetMainWindow(){return fMainWindow;}
    void SetMainWindow(CXMainWindow *w);

    void ShowSources();
    void UpdateSources();
    void UpdateText();
    void CleanCalib();
    void GetCurrentRange();

    void CloseCanvas();

    bool CheckFitProperties(TH1 *_hist);
    void Calibrate();
    void Calibrate2D(TH2 *hist);
    void ApplyCalibration(TH1 *_hist=nullptr, TF1 *_func=nullptr);

    void FWHMCalib();
    void BuildFWHMGraph();

    double DinoFct(double*xx,double*pp);
    TF1 *GetDinoFct(TString Name, double min, double max, int Npar);

    void SaveECal();
    void SaveFWHM();

    ClassDef(CXHist1DCalib,0);
};

#endif
