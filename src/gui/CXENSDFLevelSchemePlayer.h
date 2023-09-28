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

#ifndef CXENSDFLevelSchemePlayer_H
#define CXENSDFLevelSchemePlayer_H

#include <map>
#include "TNamed.h"

#include "tklevel_scheme.h"
#include "tknucleus.h"

class CXMainWindow;
class CXGuiENSDFPlayer;
class TH1;

using namespace tkn;

class CXENSDFLevelSchemePlayer : public TNamed
{

private:

    CXMainWindow *fMainWindow = nullptr;
    CXGuiENSDFPlayer *fGuiLSPlayer = nullptr;

    Int_t fMinStrenght;
    Int_t fMaxStrenght;

    Int_t fMinSpin;
    Int_t fMaxSpin;

    Float_t fRangeXMin;
    Float_t fRangeXMax;

    Float_t fMinELevel;
    Float_t fMaxELevel;

    Float_t fMinLifeTime;
    Float_t fMaxLifeTime;

    TH1 *fCurrentHist = nullptr;

    Color_t fColorWheel[48];

    Color_t fCurrentColor;

    TList *fListOfArrows = nullptr;
    TList *fListOfLatex = nullptr;
    TList *fListOfBoxes = nullptr;
    TList *fListOfCXArrows = nullptr;

    Bool_t fCanReplot;
    Bool_t fLevelDraw;

public:

    CXENSDFLevelSchemePlayer(const char* name="", const char *title="");
    virtual ~CXENSDFLevelSchemePlayer();

    void SetMainWindow(CXMainWindow *w){fMainWindow = w;}
    void SetGuiLSPlayer(CXGuiENSDFPlayer *player){fGuiLSPlayer = player;}

    void CleanArrows();
    shared_ptr<tklevel_scheme> DrawArrows(TString ListOfNuclei, TH1 *h, TString DataSet);
    shared_ptr<tklevel_scheme> DrawArrowsForNuc(TString NucName, TString DataSet, bool do_draw=true);

    Bool_t CanReplot(){return fCanReplot;}

    void ProcessedEventLevelScheme(Int_t eventType, Int_t eventX, Int_t eventY, TObject*obj);
    void RemoveArrow(Int_t ArrowIndex);

    void PlotLevelScheme(TString NucName, TString Type="ENSDF", TString DataSet="ADOPTED LEVELS, GAMMAS");

    TString PrintNucleusLevels(Int_t Z, Int_t N, bool print = false);
    TString PrintNucleusLevels(tklevel_scheme *lev, TString NucName, bool print = false);

//    TString PrintNucleusGammas(Int_t Z, Int_t N, bool print = false);
//    TString PrintNucleusGammas(tklevel_scheme *lev, TString NucName, bool print = false);

    TString GetLTString(double LifeTime);

private:

    void DrawArrow(TH1 *hist, const std::shared_ptr<tkn::tkgammadecay> gamma, tknucleus &Nuc);
    void ConnectCanvas();

    tklevel *GetLevel(tklevel *nuclev, tklevel_scheme *ls);
    void DrawLink(tklevel_scheme *ls, tkgammadecay *gammalink);
    void ShiftLink(tkgammadecay *link,Float_t XShift);
    void GetLimits(tkgammadecay *link, Float_t &xmin,Float_t &xmax,Float_t &ymin,Float_t &ymax);
    void ResizeNucLev(tklevel *nuclev,Float_t width);


    ClassDef(CXENSDFLevelSchemePlayer,0)
};

#endif // CXENSDFLevelSchemePlayer_H
