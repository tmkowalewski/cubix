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

#ifndef CXNucChart_H
#define CXNucChart_H

#include "TGFrame.h"
#include <RQ_OBJECT.h>

#include "tklevel_scheme.h"
#include "tknucleus.h"

class TGListBox;
class CXMainWindow;
class CXCanvas;
class TGStatusBar;
class TGComboBox;
class TH2D;
class NucData;
class TGTextEntry;
class TRootEmbeddedCanvas;

class CXNucChart: public TGTransientFrame
{
    RQ_OBJECT("CXNucChart");
public:
    enum NucChartType {
        M_LifeTime,
        M_1stIsomer,
        M_2ndIsomer,
        M_1rstExcitedState,
        M_DecayMode,
        M_BE2E2B2,
        M_BE2WU,
        M_NucInfo,
        M_LevelsInfo,
        M_GammaInfos,
    };

private:

    CXMainWindow *fMainWindow;
    TGListBox *fInfoBox;
    TRootEmbeddedCanvas *fRootCanvas;
    CXCanvas *fCanvas;
    TGStatusBar *fStatusBar;
    TGComboBox *fViewMode;
    TGComboBox *fPrintMode;
    TGComboBox *fDataSetMode;

    Int_t fLastEventType;
    Int_t fLastEventX;
    Int_t fLastEventY;
    TObject *fLastSelected;

    Double_t fLastXPosition;
    Double_t fLastYPosition;

    Bool_t fCTRL = false;

    TH2D *fNucChartHist;

    TList *fMagicList;
    TList *fListOfBoxes;

    bool fPlotMagics = true;

    Double_t fNMin;
    Double_t fNMax;
    Double_t fZMin;
    Double_t fZMax;

    TString fLastSelectedBox;
    shared_ptr<tkn::tknucleus> fSelectedNucleus;
    shared_ptr<tkn::tklevel_scheme> fSelectedLevelScheme;
    TString fCurrentDataSet = "ADOPTED LEVELS, GAMMAS";

    TGTextEntry *fNucleusTextEntry;

    bool fDoUpdateRange = false;

public:

    CXNucChart(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, CXMainWindow *mainwindow);
    virtual ~CXNucChart();

    void UpdateNucChart();
    void PrintInListBox(TString mess, Int_t Type=-1);

    void ProcessedKeyEvent(Event_t *event);
    void HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected);
    void UpdateRange();
    void PrintInfos(bool inprompt = false);
    void NucNotValidated();
    void UpdateNucFromSymb();
    void UpdatePrintMode(){PrintInfos(true);}
    void UpdateDataSet();
    static void SetPalette(Int_t Mode);


protected:

    void SelectNucleus(Int_t Z, Int_t N);
    void ShowMagicNumbers(bool On=true);
    void PlotBoxes();

    TString PrintNucleusGammas(shared_ptr<tkn::tklevel_scheme> lev, TString NucName, bool print);
    TString PrintNucleusLevels(shared_ptr<tkn::tklevel_scheme> lev, TString NucName, bool print);

    ClassDef(CXNucChart,0)

};


#endif //CXNucChart_H

