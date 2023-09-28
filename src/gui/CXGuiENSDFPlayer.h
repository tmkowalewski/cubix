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

#ifndef CXGuiENSDFPlayer_H
#define CXGuiENSDFPlayer_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"

#include "tklevel_scheme.h"

using namespace std;

class TGCheckButton;
class CXENSDFLevelSchemePlayer;
class CXMainWindow;
class TGComboBox;

class CXGuiENSDFPlayer : public  TGVerticalFrame
{
    RQ_OBJECT("CXGuiENSDFPlayer");

public:

private:

    CXMainWindow *fMainWindow = nullptr;

    TGTextEntry *fListOfNucleus = nullptr;

    TGComboBox *fDataSetMode = nullptr;

    Bool_t fYrastMode;

    TGCheckButton *fYrastButton;

    TGNumberEntry *fTextSize;
    TGCheckButton *fFullGammaTitle;
    TGNumberEntry *fBranchingRatio[2];
    TGCheckButton *fUseBranchingRatio;
    TGNumberEntry *fELevel[2];
    TGCheckButton *fUseELevels;
    TGNumberEntry *fSpins[2];
    TGCheckButton *fUseSpins;
    TGNumberEntry *fLifeTime[2];
    TGCheckButton *fUseLifeTime;
    TGComboBox    *fLifeTimeScale[2];

    CXENSDFLevelSchemePlayer *fLSPlayer = nullptr;

    Bool_t fNucleiAreKnown = false;
    Int_t fNumberOfNuclei = 0;

    shared_ptr<tkn::tklevel_scheme> fSelectedLevelScheme;
    TString fCurrentDataSet = "ADOPTED LEVELS, GAMMAS";
    Int_t fSelectedEntry=0;
    Bool_t fNoUpdateDataSet = false;

public:
    CXGuiENSDFPlayer(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXGuiENSDFPlayer();

    void SetMainWindow(CXMainWindow *w);

    void ManuallyAddNucleus();
    void CheckListOfNuclei();
    void UpdateGammaRays();

    void GetBranchingRatio(Int_t &min, Int_t &max);
    void GetELevels(Float_t &min, Float_t &max);
    void GetSpins(Int_t &min, Int_t &max);
    void GetLifeTime(Float_t &min, Float_t &max);

    Bool_t GetYrastMode(){return fYrastMode;}

    Float_t GetTextSize(){return fTextSize->GetNumber();}
    Bool_t IsFullTitleMode(){return fFullGammaTitle->GetState();}
    Bool_t UseBranchingRatio(){return fUseBranchingRatio->GetState();}
    Bool_t UseELevels(){return fUseELevels->GetState();}
    Bool_t UseLifeTime(){return fUseLifeTime->GetState();}
    Bool_t UseSpins(){return fUseSpins->GetState();}

    void UpdateDataSet();

protected:

private:

    ClassDef(CXGuiENSDFPlayer,0);
};

#endif
