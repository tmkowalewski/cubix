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

#ifndef CXGuiToolbar_H
#define CXGuiToolbar_H

#include "TGToolBar.h"
#include "TGButton.h"

class TCanvas;
class TGraphErrors;

enum ERootCanvasCommands {
    kToolModify,
    kToolArc,
    kToolLine,
    kToolArrow,
    kToolDiamond,
    kToolEllipse,
    kToolPad,
    kToolPave,
    kToolPLabel,
    kToolPText,
    kToolPsText,
    kToolGraph,
    kToolCurlyLine,
    kToolCurlyArc,
    kToolLatex,
    kToolMarker,
    kToolCutG
};

class CXGuiToolbar : public  TGToolBar
{

public:

protected:

private:

    TCanvas *fCanvas = nullptr;

public:
    CXGuiToolbar(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, EFrameType type);
    ~CXGuiToolbar();

    void Init();

    Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);

    void OpenLS(){Emit("OpenLS()");} //*SIGNAL*

    void SetCanvas(TCanvas *c);

    void ToggleFitTools();

protected:

private:

    TGPictureButton *AddMyButton(TString PicturePath, ERootCanvasCommands command);

    ClassDef(CXGuiToolbar,0)

};

#endif
