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

#ifndef CXTH1Proj_H
#define CXTH1Proj_H

#include "RQ_OBJECT.h"
#include "TH1.h"
#include "KeySymbols.h"

using namespace std;

class TH2;
class TPad;
class TVirtualPad;
class CXMainWindow;
class CXGateBox;
class TBox;
class CXHist2DPlayer;

class CXTH1Proj : public  TH1D
{
    RQ_OBJECT("CXTH1Proj");

public:
    TH2 *fGGHist = nullptr;
    TPad *fProjPad = nullptr;
    TPad *fCurrentPad = nullptr;

private:

    CXMainWindow *fMainWindow = nullptr;

    Bool_t fAddNewGate = false;
    Bool_t fAddNewBacground = false;

    Bool_t   moved = false;
    Double_t oldx{}, oldy{};
    Double_t xinit{}, yinit{};
    Double_t xmin{}, xmax{}, ymin{}, ymax{};

    TList *fListOfGates = nullptr;

    TBox *fGateVirtualBox = nullptr;

    CXHist2DPlayer *f2DPlayer = nullptr;

    Int_t fProjectionAxis = 0;

    EKeySym fLastSym;

public:
    CXTH1Proj(const TH1D &hist);
    CXTH1Proj();
    ~CXTH1Proj();

    void SetMainWindow(CXMainWindow *w);

    void UpdateProjection(Int_t Axis);
    void SetTH2(TH2 *hist);
    void SetProjPad(TPad *pad);
    void SetCurrentPad(TPad *pad);
    void SetPlayer(CXHist2DPlayer *player);
    void HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected);

    void Project(Bool_t FixRange=false, int _rebin_value=1);
    void AddBackgd(){fAddNewBacground = true;}
    void AddBackgd(Float_t Mean, Float_t Width); //*MENU*
    void AddGate(){fAddNewGate = true;}
    void AddGate1(Float_t Mean, Float_t Width); //*MENU*
    void ClearGates();
    TList *GetGatesList(){return fListOfGates;}
    void UpdateGates();
    void RemoveGate(CXGateBox *box);

private:

    ClassDef(CXTH1Proj,0);
};

#endif
