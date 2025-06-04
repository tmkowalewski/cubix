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

#ifndef CXFit_H
#define CXFit_H

#include <sstream>
#include <vector>

#include "TObject.h"

class CXHist1DPlayer;
class TVirtualPad;
class TH1;
class TF1;
class CXArrow;
class CXWorkspace;

class CXFit : public TObject
{
private:
    CXHist1DPlayer *fPlayer         = nullptr;
    TList       *fListOfArrows      = nullptr;
    TVirtualPad *fPad               = nullptr;

    TH1         *fHistogram         = nullptr;

    TF1         *fFitFunction       = nullptr;
    TF1         *fBackFunction      = nullptr;
    TF1         *fResidue           = nullptr;
    TList       *fListOfPeaks       = nullptr;

    std::vector<Double_t> fEnergies;
    std::vector<Bool_t> fFixedMean;
    std::vector<Double_t> fBackgd;

    CXWorkspace *fWorkspace = nullptr;

    std::ostringstream fsavedStream;

    Bool_t fBindFWHM = false;

public:
    CXFit(TH1 *hist, TVirtualPad *pad, CXHist1DPlayer *player, CXWorkspace *_workspace=nullptr);
    CXFit(const CXFit &other);  // Copy constructor
    CXFit& operator=(const CXFit &other);  // Copy assignment operator
    virtual ~CXFit();

    virtual TObject* Clone(const char* newname = "") const override;

    void UpdateFit(TH1 *hist, TVirtualPad *pad, CXHist1DPlayer *player, CXWorkspace *_workspace=nullptr);

    void AddArrow(Double_t Energy);
    void RemoveArrow(CXArrow *arrow = nullptr);
    void DrawArrows();
    void Update();
    void Fit();

    Double_t DoubleTailedStepedGaussian(Double_t*xx,Double_t*pp);
    Double_t StepedBackground(Double_t*xx,Double_t*pp);
    Double_t PeakFunction(Double_t*xx,Double_t*pp);
    Double_t Residue(Double_t*xx,Double_t*pp);

    TVirtualPad *GetPad() {return fPad;}

    void Clear(TVirtualPad *pad = nullptr);

    TString Save();

    void BindFWHM(Bool_t on);

    ClassDefOverride(CXFit,0);
};

#endif
