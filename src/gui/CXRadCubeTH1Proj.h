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

#ifndef CXRadCubeTH1Proj_H
#define CXRadCubeTH1Proj_H

#include "RQ_OBJECT.h"
#include "KeySymbols.h"
#include "TH1.h"

#include "CXRadReader.h"

using namespace std;

class TH2;
class TPad;
class TVirtualPad;
class CXMainWindow;
class CXGateBox;
class TBox;
class CXRad2DPlayer;
class CXRadCubePlayer;


class CXRadCubeTH1Proj : public  TH1D
{
    RQ_OBJECT("CXRadCubeTH1Proj");

public:
    TH2 *fGGHist = nullptr;
    CXRadReader *fRadReader = nullptr;
    TPad *fCurrentPad = nullptr;

    vector<TPad *> fProjPad;

private:

    CXMainWindow *fMainWindow = nullptr;

    Bool_t fAddNewGate1 = false;
    Bool_t fAddNewGate2 = false;

    Bool_t   moved = false;
    Double_t oldx, oldy;
    Double_t xinit, yinit;
    Double_t xmin, xmax, ymin, ymax;

    TList *fListOfGates = nullptr;

    TBox *fGateVirtualBox = nullptr;

    CXRadCubePlayer *fRadCubePlayer = nullptr;
    CXRad2DPlayer   *fRad2DPlayer = nullptr;

    Float_t fLenghtFactor = 1.;
    Float_t fWidth        = 6.;
    Float_t fFWHM_0       = 9.;
    Float_t fFWHM_1       = 0.004;
    Float_t fFWHM_2       = 0.;

    TString fMode;

    EKeySym fLastSym;

    int fNProjs;
    int fCurrentProjPad = 0;

    vector<TH1 *> fProjection;
    bool fprojok = true;

public:
    CXRadCubeTH1Proj(CXRadReader *radreader, int _nprojs=1);
    CXRadCubeTH1Proj(TH2 *hist, int _nprojs=1);
    CXRadCubeTH1Proj();

    ~CXRadCubeTH1Proj();

    void SetMainWindow(CXMainWindow *w);

    void SetTH2(TH2 *hist);
    TH2 *GetTH2() {return fGGHist;}

    void SetProjPad(int _ipad, TPad *_pad);
    void SetCurrentPad(TPad *pad);
    void SetCubePlayer(CXRadCubePlayer *player);
    void Set2DPlayer(CXRad2DPlayer *player);
    void HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected);
    void Project(Bool_t FixRange=false, Bool_t BGSubtract = true, Int_t Rebin=1);

    void AddNewGate1(){fAddNewGate1 = true;}
    void AddGate1(Float_t TheMean=0., Float_t TheWidth=0.); //*MENU*
    void AddNewGate2(){fAddNewGate2 = true;}
    void AddGate2(Float_t Mean=0., Float_t Width=0.); //*MENU*
    void ClearGates();
    TList *GetGatesList(){return fListOfGates;}
    void UpdateGates();
    void RemoveGate(CXGateBox *box);

    TH1 *GetTotalProj(){return fRadReader->GetTotalProjection();}
    TH1 *GetBackground(){return fRadReader->GetBackground();}

    Float_t GetLenghtFactor() { return fLenghtFactor; }
    Float_t GetWidth() { return fWidth; }
    Float_t GetFWHM_0() { return fFWHM_0; }
    Float_t GetFWHM_1() { return fFWHM_1; }
    Float_t GetFWHM_2() { return fFWHM_2; }
    void SetBackground(Float_t LenghtFactor, Float_t Width, Float_t FWHM_0, Float_t FWHM_1, Float_t FWHM_2); //*MENU* *ARGS={LenghtFactor=>fLenghtFactor,Width=>fWidth,FWHM_0=>fFWHM_0,FWHM_1=>fFWHM_1,FWHM_2=>fFWHM_2}
    void SaveBackground(); //*MENU*

    TObject*         Clone(const char* newname=0) const {return fRadReader->GetTotalProjection()->Clone(newname);}

    ClassDef(CXRadCubeTH1Proj,0);
};

#endif
