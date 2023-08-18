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

#ifndef CXGammaSearch_H
#define CXGammaSearch_H

#include "TGFrame.h"
#include "TString.h"
#include "TMatrixDfwd.h"
#include <RQ_OBJECT.h>

#include "tklevel_scheme.h"

class CXLevelSchemePlayer;
class CXSpreadIntensityMatrix;
class CXMainWindow;
class TGListBox;
class TGNumberEntry;
class TGLabel;
class TGCheckButton;
class TGRadioButton;

struct GammaTransition{
    double EGamma;
    double EI;
    double EF;
    TString SpinI;
    TString SpinF;
    TString NucName;
    TString LifeTime;
};

class CXGammaSearch: public TGTransientFrame
{
    enum ECheckCommand
    {
        M_With_Cascade,
        M_WithOut_Cascade
    };

    RQ_OBJECT("CXGammaSearch");

private:

    CXMainWindow *fMainWindow;

    CXSpreadIntensityMatrix *fSp;

    TGLabel *fNNucAnalysed;
    TGLabel *fNGrayAnalysed;

    TGNumberEntry *fEnergies[3];
    TGNumberEntry *fWidths[3];
    TGCheckButton *fCheckGammas[3];

    TGNumberEntry *fZRange[2];
    TGNumberEntry *fARange[2];
    TGNumberEntry *fNRange[2];

    Int_t fNGammas=1;

    TGTextButton *fStartButton,*fLogButton;

    shared_ptr<tkn::tklevel_scheme> fLevelScheme;

    ECheckCommand fCurrentMode;

    TGRadioButton *fCoincMode;
    TGRadioButton *fNoCoincMode;

    TGListBox *fResultsBox;

    std::vector < std::vector < GammaTransition > > fListOfGoodGammas;

public:

    CXGammaSearch(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, CXMainWindow *mwin);
    virtual ~CXGammaSearch();

    void HandleButtons();

    void FindGammaRays(Bool_t Bash=false);

    void SetCalMode();

    void PrintInListBox(TString mess, Int_t Type);

protected:

    void FindInDoubleCoincidence(Bool_t Bash = false);
    void FindInTripleCoincidence();

    std::vector<int> Sort_Index(std::vector<int> Index_Nuclei, std::vector<double> Tab_P, std::vector<double> Tab_DeltaE);

    TMatrixD Fill_TS_Matrix(shared_ptr<tkn::tklevel_scheme> fLevelScheme);
    TMatrixD Get_E_Matrix(shared_ptr<tkn::tklevel_scheme> fLevelScheme);

    ClassDef(CXGammaSearch,0)
};


#endif //CXGammaSearch_H

