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

#ifndef CXRadCubePlayer_H
#define CXRadCubePlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TGNumberEntry.h"

using namespace std;

class CXMainWindow;
class CXRadReader;
class CXRadCubeTH1Proj;
class TGListBox;
class TGLBEntry;
class TGTextEntry;

class CXRadCubePlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXRadCubePlayer");

private:

    CXMainWindow *fMainWindow = nullptr;
    CXRadCubeTH1Proj *fCurrentProj = nullptr;

    TGCheckButton *fFixRange = nullptr;
    TGCheckButton *fBckSubtract = nullptr;

    TGCheckButton *fUseFWHM = nullptr;
    TGNumberEntry *fFWHMGateFraction = nullptr;

    TGNumberEntry *fRebinValue = nullptr;
    TGCheckButton *fDynamicProj = nullptr;

    TGNumberEntry *fNProjections = nullptr;
    int fNProjs=1;

    TGListBox *fStoredSpectraBox = nullptr;
    TList *fListOfStoredSpectra = nullptr;
    TGTextEntry *fDrawOpt = nullptr;

public:
    CXRadCubePlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, CXMainWindow *window);
    ~CXRadCubePlayer();

    void SetMainWindow(CXMainWindow *w);

    void ChangeNProjections();
    void Init(CXRadReader *radreader);
    void Project();
    void AddGate1();
    void AddGate2();
    void ClearGates();
    void ApplyLastGates();

    void UpdateDrawOpt();
    void ToggleFWHM();

    bool UseFWHM();
    double GetFWHMGateFraction() {return fFWHMGateFraction->GetNumber();}

    bool UseDynamicProjection() {return (fDynamicProj->GetState()==kButtonDown);}

private:
    CXRadCubeTH1Proj *GetProj();

    ClassDef(CXRadCubePlayer,0);
};

#endif
