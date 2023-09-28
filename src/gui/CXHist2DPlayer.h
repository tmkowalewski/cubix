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

#ifndef CXHist2DPlayer_H
#define CXHist2DPlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"

using namespace std;

class CXMainWindow;
class TH2;
class CXTH1Proj;
class TGListBox;
class TGLBEntry;
class TGCheckButton;
class TGTextEntry;
class TGComboBox;
class TGNumberEntry;

class CXHist2DPlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXHist2DPlayer");

private:

    CXMainWindow *fMainWindow = nullptr;

    Int_t fAxisProj = 0; // X => 0 ; Y => 1

    CXTH1Proj *fCurrentProj = nullptr;

    TGListBox *fStoredSpectraBox = nullptr;
    TList *fListOfStoredSpectra = nullptr;
    TGTextEntry *fDrawOpt = nullptr;
    TGCheckButton *fFixRange = nullptr;

    TGNumberEntry *fRebinValue = nullptr;

    TGComboBox *fProjectionAxis = nullptr;

public:
    CXHist2DPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, CXMainWindow *window);
    ~CXHist2DPlayer();

    void SetMainWindow(CXMainWindow *w);

    void InitGG(TH2 *hist_in =nullptr);
    void UpdateProjection();

    void Project();
    void AddBackgd();
    void AddGate();
    void ClearGates();
    void ApplyLastGate();

    void UpdateDrawOpt();

private:
    CXTH1Proj *GetProj();

    ClassDef(CXHist2DPlayer,0);
};

#endif
