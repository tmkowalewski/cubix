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
