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

#include "CXGuiToolbar.h"

#include "TMap.h"
#include "TROOT.h"

using namespace std;

static ToolBarData_t gToolBarData1[] = {
    { "pointer.xpm",    "Modify",           kFALSE,    kToolModify,     0 },
    { "arc.xpm",        "Arc",              kFALSE,    kToolArc,        0 },
    { "line.xpm",       "Line",             kFALSE,    kToolLine,       0 },
    { "arrow.xpm",      "Arrow",            kFALSE,    kToolArrow,      0 },
    { "diamond.xpm",    "Diamond",          kFALSE,    kToolDiamond,    0 },
    { "ellipse.xpm",    "Ellipse",          kFALSE,    kToolEllipse,    0 },
    { "pad.xpm",        "Pad",              kFALSE,    kToolPad,        0 },
    { "pave.xpm",       "Pave",             kFALSE,    kToolPave,       0 },
    { "pavelabel.xpm",  "Pave Label",       kFALSE,    kToolPLabel,     0 },
    { "pavetext.xpm",   "Pave Text",        kFALSE,    kToolPText,      0 },
    { "pavestext.xpm",  "Paves Text",       kFALSE,    kToolPsText,     0 },
    { "graph.xpm",      "Graph",            kFALSE,    kToolGraph,      0 },
    { "curlyline.xpm",  "Curly Line",       kFALSE,    kToolCurlyLine,  0 },
    { "curlyarc.xpm",   "Curly Arc",        kFALSE,    kToolCurlyArc,   0 },
    { "latex.xpm",      "Text/Latex",       kFALSE,    kToolLatex,      0 },
    { "marker.xpm",     "Marker",           kFALSE,    kToolMarker,     0 },
    { "cut.xpm",        "Graphical Cut",    kFALSE,    kToolCutG,       0 },
    { 0,                0,                  kFALSE,    0,               0 },
};

CXGuiToolbar::CXGuiToolbar(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, EFrameType type) : TGToolBar(MotherFrame, w, h, type)
{
    Init();
}

void CXGuiToolbar::Init()
{
    int spacing = 6;
    for (int i = 0; gToolBarData1[i].fPixmap; i++) {
        if (strlen(gToolBarData1[i].fPixmap) == 0) {
            spacing = 6;
            continue;
        }
        AddButton(this, &gToolBarData1[i], spacing);
        spacing = 0;
    }
}

TGPictureButton *CXGuiToolbar::AddMyButton(TString PicturePath, ERootCanvasCommands command)
{
    const TGPicture *pic = fClient->GetPicture(PicturePath);

    TGLayoutHints   *layout = new TGLayoutHints(kLHintsTop | kLHintsRight, 10, 0, 2, 2);

    fPictures->Add((TObject*)pic);

    TGPictureButton *pbut = new TGPictureButton(this, pic, command);
    fTrash->Add(pbut);

    pbut->SetStyle(gClient->GetStyle());

    AddFrame(pbut, layout);

    pbut->AllowStayDown(kFALSE);
    pbut->Associate(this);

    fMapOfButtons->Add(pbut, (TObject*)((Long_t)command));

    Connect(pbut, "Released()", "CXGuiToolbar", this, "ButtonReleased()");

    return pbut;
}

CXGuiToolbar::~CXGuiToolbar()
{

}

Bool_t CXGuiToolbar::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
    // Handle menu and other command generated by the user.

    switch (GET_MSG(msg)) {

    case kC_COMMAND:

        switch (GET_SUBMSG(msg)) {

        case kCM_BUTTON:
        case kCM_MENU:

            switch (parm1) {
            // Handle toolbar items...
            case kToolModify:
                gROOT->SetEditorMode();
                break;
            case kToolArc:
                gROOT->SetEditorMode("Arc");
                break;
            case kToolLine:
                gROOT->SetEditorMode("Line");
                break;
            case kToolArrow:
                gROOT->SetEditorMode("Arrow");
                break;
            case kToolDiamond:
                gROOT->SetEditorMode("Diamond");
                break;
            case kToolEllipse:
                gROOT->SetEditorMode("Ellipse");
                break;
            case kToolPad:
                gROOT->SetEditorMode("Pad");
                break;
            case kToolPave:
                gROOT->SetEditorMode("Pave");
                break;
            case kToolPLabel:
                gROOT->SetEditorMode("PaveLabel");
                break;
            case kToolPText:
                gROOT->SetEditorMode("PaveText");
                break;
            case kToolPsText:
                gROOT->SetEditorMode("PavesText");
                break;
            case kToolGraph:
                gROOT->SetEditorMode("PolyLine");
                break;
            case kToolCurlyLine:
                gROOT->SetEditorMode("CurlyLine");
                break;
            case kToolCurlyArc:
                gROOT->SetEditorMode("CurlyArc");
                break;
            case kToolLatex:
                gROOT->SetEditorMode("Text");
                break;
            case kToolMarker:
                gROOT->SetEditorMode("Marker");
                break;
            case kToolCutG:
                gROOT->SetEditorMode("CutG");
                break;
            }
        default:
            break;
        }
    default:
        break;
    }
    return kTRUE;
}

void CXGuiToolbar::SetCanvas(TCanvas *c)
{
    fCanvas = c;
}

ClassImp(CXGuiToolbar);
