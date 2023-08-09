#include "CXBgdUtility.h"

#include <iostream>
#include <iomanip>

#include "TFrame.h"
#include "TGCanvas.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"

using namespace std;

CXBgdUtility::CXBgdUtility(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{

    gClient->GetColorByName("red", red);
    gClient->GetColorByName("blue", blue);
    gClient->GetColorByName("black", black);

    fListOfButtons1D = new TList();

    auto *GroupHints = new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, -10, -10, 0, 0);
    auto *HFHints = new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,8,0);

    auto *f1DGroupFrame = new TGGroupFrame(MotherFrame, "1D background", kVerticalFrame);
    f1DGroupFrame->SetTextColor(red);
    f1DGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(f1DGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0));

    //Activation du mode background supress

    fGroupFrame = new TGGroupFrame(f1DGroupFrame, "Activate utility", kVerticalFrame);
    fGroupFrame->SetTextColor(blue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    f1DGroupFrame->AddFrame(fGroupFrame, GroupHints);

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "On"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 0, 0));
    fActivate = new TGCheckButton(fHorizontalFrame,"");
    fActivate->Connect("Clicked()","CXBgdUtility", this, "ToggleBckSupp()");
    fHorizontalFrame->AddFrame(fActivate, HFHints);
    fGroupFrame->AddFrame(fHorizontalFrame, GroupHints);


    ///Paramers

    fGroupFrame = new TGGroupFrame(f1DGroupFrame, "Parameters", kVerticalFrame);
    fGroupFrame->SetTextColor(blue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    f1DGroupFrame->AddFrame(fGroupFrame, GroupHints);


    /// Iterations

    fNumberIterations1D = 20;
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "Iterations"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 0, 0));
    fNItersButton1D = new TGNumberEntry(fHorizontalFrame, fNumberIterations1D, 6,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fNItersButton1D->Connect("ValueSet(Long_t)","CXBgdUtility", this, "GetParams()");
    fHorizontalFrame->AddFrame(fNItersButton1D,new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);

    fListOfButtons1D->Add(fNItersButton1D);

    /// Window

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "Window direction :"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 0, 0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fDirectionButton1D[0] = new TGRadioButton(fHorizontalFrame, "Increasing", 0);
    fDirectionButton1D[0]->Connect("Clicked()", "CXBgdUtility", this, "HandleWindowButtons()");
    fDirectionButton1D[0]->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
    fHorizontalFrame->AddFrame(fDirectionButton1D[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,20,0,0,0));
    fDirectionButton1D[1] = new TGRadioButton(fHorizontalFrame, "Decreasing", 1);
    fDirectionButton1D[1]->SetState(kButtonDown);
    fDirection1D = TSpectrum::kBackDecreasingWindow;
    fDirectionButton1D[1]->Connect("Clicked()", "CXBgdUtility", this, "HandleWindowButtons()");
    fDirectionButton1D[1]->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
    fHorizontalFrame->AddFrame(fDirectionButton1D[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);

    fListOfButtons1D->Add(fDirectionButton1D[0]);
    fListOfButtons1D->Add(fDirectionButton1D[1]);

    /// Filter order

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "Filter order"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 0, 0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fFilterOrderButton1D[0] = new TGRadioButton(fHorizontalFrame, "2", 0);
    fFilterOrderButton1D[0]->Connect("Clicked()", "CXBgdUtility", this, "HandleFilterButtons()");
    fFilterOrderButton1D[0]->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
    fHorizontalFrame->AddFrame(fFilterOrderButton1D[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,20,0,0,0));
    fFilterOrderButton1D[0]->SetState(kButtonDown);
    fFilterOrder1D = TSpectrum::kBackOrder2;

    fFilterOrderButton1D[1] = new TGRadioButton(fHorizontalFrame, "4", 1);
    fFilterOrderButton1D[1]->Connect("Clicked()", "CXBgdUtility", this, "HandleFilterButtons()");
    fFilterOrderButton1D[1]->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
    fHorizontalFrame->AddFrame(fFilterOrderButton1D[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));

    fFilterOrderButton1D[2] = new TGRadioButton(fHorizontalFrame, "6", 2);
    fFilterOrderButton1D[2]->Connect("Clicked()", "CXBgdUtility", this, "HandleFilterButtons()");
    fFilterOrderButton1D[2]->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
    fHorizontalFrame->AddFrame(fFilterOrderButton1D[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));

    fFilterOrderButton1D[3] = new TGRadioButton(fHorizontalFrame, "8", 3);
    fFilterOrderButton1D[3]->Connect("Clicked()", "CXBgdUtility", this, "HandleFilterButtons()");
    fFilterOrderButton1D[3]->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
    fHorizontalFrame->AddFrame(fFilterOrderButton1D[3],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);


    fListOfButtons1D->Add(fFilterOrderButton1D[0]);
    fListOfButtons1D->Add(fFilterOrderButton1D[1]);
    fListOfButtons1D->Add(fFilterOrderButton1D[2]);
    fListOfButtons1D->Add(fFilterOrderButton1D[3]);

    /// Smoothing

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "Smoothing : "),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 0, 0));
    fDoSmoothing = new TGCheckButton(fHorizontalFrame,"",-2);
    fDoSmoothing->Connect("Clicked()", "CXBgdUtility", this, "HandleSmoothButtons()");
    fDoSmoothing->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
    fDoSmoothing->SetState(kButtonDown);
    fHorizontalFrame->AddFrame(fDoSmoothing, new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame, HFHints);
    fSmoothing = true;
    fSmoothingWindow = 3;

    fListOfButtons1D->Add(fDoSmoothing);

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "Smoothing order : "),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 0, 0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);

    for(int i=0 ; i<7 ; i++) {
        fSmoothingButton1D.at(i) = new TGRadioButton(fHorizontalFrame, Form("%d",3+2*i), i);
        if(i==0) fSmoothingButton1D.at(i)->SetState(kButtonDown);
        fSmoothingButton1D.at(i)->Connect("Clicked()", "CXBgdUtility", this, "HandleSmoothButtons()");
        fSmoothingButton1D.at(i)->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
        if(i==0) fHorizontalFrame->AddFrame(fSmoothingButton1D.at(i),new TGLayoutHints(kLHintsCenterY | kLHintsLeft,20,0,0,0));
        else fHorizontalFrame->AddFrame(fSmoothingButton1D.at(i),new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));

        fListOfButtons1D->Add(fSmoothingButton1D.at(i));
    }

    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);


    /// Compton

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "Compton : "),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 0, 0));
    fDoCompton = new TGCheckButton(fHorizontalFrame,"");
    fDoCompton->Connect("Clicked()","CXBgdUtility", this, "GetParams()");
    fHorizontalFrame->AddFrame(fDoCompton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame, HFHints);
    fCompton = false;

    fListOfButtons1D->Add(fDoCompton);


    /// Do subtract

    fGroupFrame = new TGGroupFrame(f1DGroupFrame, "Do subtract", kVerticalFrame);
    fGroupFrame->SetTextColor(blue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    f1DGroupFrame->AddFrame(fGroupFrame, GroupHints);

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fSubtractButton = new TGTextButton(fHorizontalFrame, "Subtract");
    fSubtractButton->Connect("Clicked()", "CXBgdUtility", this, "DoSubtract()");
    fHorizontalFrame->AddFrame(fSubtractButton,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,10,10,10,10));
    fGroupFrame->AddFrame(fHorizontalFrame, GroupHints);

    fListOfButtons1D->Add(fSubtractButton);

    SetButtonsStatus(false);


    /// Background 2D
    auto *f2DGroupFrame = new TGGroupFrame(MotherFrame, "2D background", kVerticalFrame);
    f2DGroupFrame->SetTextColor(red);
    f2DGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(f2DGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0));

    fGroupFrame = new TGGroupFrame(f2DGroupFrame, "Parameters", kVerticalFrame);
    fGroupFrame->SetTextColor(blue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    f2DGroupFrame->AddFrame(fGroupFrame, GroupHints);

    /// Iterations

    fNumberIterations2DX = 20;
    fNumberIterations2DY = 20;
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "IterationsX"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 0, 0));
    fNItersButton2D[0] = new TGNumberEntry(fHorizontalFrame, fNumberIterations2DX, 4,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fNItersButton2D[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,10,5,0));
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "IterationsY"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 0, 0));
    fNItersButton2D[1] = new TGNumberEntry(fHorizontalFrame, fNumberIterations2DY, 4,0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fNItersButton2D[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,0,5,0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);

    /// Window

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "Window direction :"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 0, 0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fDirectionButton2D[0] = new TGRadioButton(fHorizontalFrame, "Increasing", 10);
    fDirectionButton2D[0]->Connect("Clicked()", "CXBgdUtility", this, "HandleWindowButtons()");
    fHorizontalFrame->AddFrame(fDirectionButton2D[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,20,0,0,0));
    fDirectionButton2D[1] = new TGRadioButton(fHorizontalFrame, "Decreasing", 11);
    fDirectionButton2D[1]->SetState(kButtonDown);
    fDirection2D = TSpectrum::kBackDecreasingWindow;
    fDirectionButton2D[1]->Connect("Clicked()", "CXBgdUtility", this, "HandleWindowButtons()");
    fHorizontalFrame->AddFrame(fDirectionButton2D[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,20,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);

    /// Filter

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(fLabel = new TGLabel(fHorizontalFrame, "Filter algorithm :"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 0, 0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);
    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fFilterOrderButton2D[0] = new TGRadioButton(fHorizontalFrame, "Successive", 10);
    fFilterOrderButton2D[0]->Connect("Clicked()", "CXBgdUtility", this, "HandleFilterButtons()");
    fFilterOrderButton2D[0]->SetState(kButtonDown);
    fFilterOrder2D = TSpectrum2::kBackSuccessiveFiltering;
    fHorizontalFrame->AddFrame(fFilterOrderButton2D[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,20,0,0,0));
    fFilterOrderButton2D[1] = new TGRadioButton(fHorizontalFrame, "OneStep", 11);
    fFilterOrderButton2D[1]->Connect("Clicked()", "CXBgdUtility", this, "HandleFilterButtons()");
    fHorizontalFrame->AddFrame(fFilterOrderButton2D[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft,20,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,HFHints);

    /// Eval

    fGroupFrame = new TGGroupFrame(f2DGroupFrame, "Evaluate background", kVerticalFrame);
    fGroupFrame->SetTextColor(blue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    f2DGroupFrame->AddFrame(fGroupFrame, GroupHints);

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    f2DEvalButton = new TGTextButton(fHorizontalFrame, "Evaluate");
    f2DEvalButton->Connect("Clicked()", "CXBgdUtility", this, "Do2DEvaluation()");
    fHorizontalFrame->AddFrame(f2DEvalButton,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,10,10,10,10));
    fGroupFrame->AddFrame(fHorizontalFrame, GroupHints);
}

CXBgdUtility::~CXBgdUtility() = default;

void CXBgdUtility::DoSubtract()
{
    gPad = fMainWindow->GetSelectedPad();

    TH1 *hist = fMainWindow->GetHisto();

    if(hist == nullptr || hist->InheritsFrom("TH2")) {
        cout<<"No 1D istogram found, ignored"<<endl;
        return;
    }

    TH1 *bkgrd = dynamic_cast<TH1*>(gPad->GetListOfPrimitives()->FindObject(Form("%s_BG",hist->GetName())));

    if(bkgrd == nullptr) {
        cout<<"No background histogram built in the pad for hist: "<<hist->GetName()<<endl;
        return;
    }

    hist->Add(bkgrd,-1.);
    delete bkgrd;
    hist->Draw(hist->GetDrawOption());

    gPad->Update();
    gPad->Modified();

    gPad->SetBit(TPad::kCannotMove);
    gPad->GetFrame()->SetBit(TObject::kCannotPick);

    fActivate->SetState(kButtonUp);
}

void CXBgdUtility::ToggleBckSupp()
{
    gPad = fMainWindow->GetSelectedPad();

    if(gPad == nullptr)
        return;

    if(fActivate->GetState() == kButtonDown) {
        SetButtonsStatus(true);
        GetParams();
    }
    else {
        SetButtonsStatus(false);

        for(int i=gPad->GetListOfPrimitives()->GetEntries()-1 ; i>=0 ; i--) {
            TObject *o = gPad->GetListOfPrimitives()->At(i);
            if(o->InheritsFrom("TH1") && ((TString)o->GetName()).EndsWith("_BG")) {
                delete o;
                break;
            }
        }
    }

    gPad->Modified();
    gPad->Update();
}

void CXBgdUtility::SetButtonsStatus(bool on)
{
    fNItersButton1D->SetState(on);

    for(int i=1 ; i<fListOfButtons1D->GetEntries() ; i++) {
        auto *btn = dynamic_cast<TGButton*>(fListOfButtons1D->At(i));

        if(!fSmoothing && i>=8 && i<=14)
            continue;
        if(!on)
            btn->SetState(kButtonDisabled);
        else if(btn->IsDown())
            btn->SetState(kButtonDown);
        else
            btn->SetState(kButtonUp);
    }
}

void CXBgdUtility::GetParams()
{
    gPad = fMainWindow->GetSelectedPad();

    fNumberIterations1D = fNItersButton1D->GetIntNumber();
    fCompton = fDoCompton->GetState();

    TString Option = "same";

    if(fDirection1D == TSpectrum::kBackDecreasingWindow) Option += "BackDecreasingWindow";
    else if(fDirection1D == TSpectrum::kBackIncreasingWindow) Option += "BackIncreasingWindow";

    if(fFilterOrder1D == TSpectrum::kBackOrder2) Option += "BackOrder2";
    else if(fFilterOrder1D == TSpectrum::kBackOrder4) Option += "BackOrder4";
    else if(fFilterOrder1D == TSpectrum::kBackOrder6) Option += "BackOrder6";
    else if(fFilterOrder1D == TSpectrum::kBackOrder8) Option += "BackOrder8";

    if(!fSmoothing) Option += "nosmoothing";
    else {
        if(fSmoothingWindow == TSpectrum::kBackSmoothing3) Option += "BackSmoothing3";
        else if(fSmoothingWindow == TSpectrum::kBackSmoothing5) Option += "BackSmoothing5";
        else if(fSmoothingWindow == TSpectrum::kBackSmoothing7) Option += "BackSmoothing7";
        else if(fSmoothingWindow == TSpectrum::kBackSmoothing9) Option += "BackSmoothing9";
        else if(fSmoothingWindow == TSpectrum::kBackSmoothing11) Option += "BackSmoothing11";
        else if(fSmoothingWindow == TSpectrum::kBackSmoothing13) Option += "BackSmoothing13";
        else if(fSmoothingWindow == TSpectrum::kBackSmoothing15) Option += "BackSmoothing15";
        else Option += "nosmoothing";
    }
    if(fCompton) Option += "Compton";

    TH1 *hist = dynamic_cast<TH1*>(fMainWindow->GetHisto());

    if(hist == nullptr || hist->InheritsFrom("TH2")) {
        cout<<"No 1D istogram found, ignored"<<endl;
        return;
    }

    TH1D *clone = dynamic_cast<TH1D*>(hist->Clone(Form("%s_clone",hist->GetName())));
    for(int i=0 ; i<clone->GetNbinsX() ; i++)
        clone->SetBinContent(i+1,hist->GetBinContent(i+1));

    TH1 *OldBG = dynamic_cast<TH1*>(gPad->GetListOfPrimitives()->FindObject((TString)hist->GetName() + "_BG"));
    delete OldBG;

    auto *tspec = new TSpectrum;

    TH1 *Backgd = tspec->Background(clone,fNumberIterations1D,Option.Data());
    TString bgname = hist->GetName();
    bgname += "_BG";
    Backgd->SetName(bgname.Data());

    delete clone;
    delete tspec;

    gPad->Modified();
    gPad->Update();
}

void CXBgdUtility::HandleWindowButtons(Int_t id)
{
    // Handle different buttons
    auto *btn = static_cast<TGButton*>(gTQSender);

    if (id == -1)
        id = btn->WidgetId();

    if(id == 0) {
        fDirectionButton1D[1]->SetState(kButtonUp);
        fDirection1D = TSpectrum::kBackIncreasingWindow;
    }
    if(id == 1) {
        fDirectionButton1D[0]->SetState(kButtonUp);
        fDirection1D = TSpectrum::kBackDecreasingWindow;
    }
    if(id == 10) {
        fDirectionButton2D[1]->SetState(kButtonUp);
        fDirection2D = TSpectrum::kBackIncreasingWindow;
    }
    if(id == 11) {
        fDirectionButton2D[0]->SetState(kButtonUp);
        fDirection2D = TSpectrum::kBackDecreasingWindow;
    }
}

void CXBgdUtility::HandleFilterButtons(Int_t id)
{
    // Handle different buttons
    auto *btn = static_cast<TGButton*>(gTQSender);
    if (id == -1)
        id = btn->WidgetId();

    if(id<4) {
        for(int i=0 ; i<4 ; i++) {
            if(id != i)
                fFilterOrderButton1D.at(i)->SetState(kButtonUp);
            else
                fFilterOrder1D = i;
        }
    }

    if(id == 10) {
        fFilterOrderButton2D[1]->SetState(kButtonUp);
        fFilterOrder2D = TSpectrum2::kBackSuccessiveFiltering;
    }
    if(id == 11) {
        fFilterOrderButton2D[0]->SetState(kButtonUp);
        fFilterOrder2D = TSpectrum2::kBackOneStepFiltering;
    }
}

void CXBgdUtility::HandleSmoothButtons(Int_t id)
{
    // Handle different buttons
    auto *btn = static_cast<TGButton*>(gTQSender);
    if (id == -1)
        id = btn->WidgetId();

    if(id == -2) {
        if(btn->GetState() == kButtonDown) {
            fSmoothing = true;
            for(int i=0 ; i<7 ; i++) {
                if(fSmoothingButton1D.at(i)->IsDown()) {
                    fSmoothingButton1D.at(i)->SetState(kButtonDown);
                    fSmoothingWindow = 3+2*i;
                }
                else
                    fSmoothingButton1D.at(i)->SetState(kButtonUp);
            }
        }
        else {
            fSmoothing = false;
            for(int i=0 ; i<7 ; i++)
                fSmoothingButton1D.at(i)->SetState(kButtonDisabled);
        }
        return;
    }

    for(int i=0 ; i<7 ; i++) {
        if(id != i)
            fSmoothingButton1D.at(i)->SetState(kButtonUp);
        else
            fSmoothingWindow = 3+2*i;
    }
}

void CXBgdUtility::Do2DEvaluation(TH1 *hist_in)
{
    TH1 *hist;

    if(hist_in == nullptr)
        hist = fMainWindow->GetHisto();
    else
        hist = hist_in;

    if(hist && hist->InheritsFrom("TH2"))
        fCurrent2DHist = dynamic_cast<TH2*>(hist);
    else {
        cout<<"No 2D histogram found in the current pad, ignored"<<endl;
        fCurrent2DHist = nullptr;
        return;
    }

    TString draw_opt = hist->GetDrawOption();
    if(draw_opt=="")
        draw_opt = "col";

    fCurrent2DBackd = dynamic_cast<TH2*>(fCurrent2DHist->Clone());
    fCurrent2DBackd->SetName(Form("%s_bck",fCurrent2DBackd->GetName()));
    fCurrent2DBackd->Reset();

    fCurrent2DSubtract = dynamic_cast<TH2*>(fCurrent2DHist->Clone());
    fCurrent2DSubtract->SetName(Form("%s_sub",fCurrent2DBackd->GetName()));
    fCurrent2DSubtract->Reset();

    auto *spectrum = new TSpectrum2;

    Int_t nbinsx, nbinsy;
    nbinsx = fCurrent2DHist->GetNbinsX();
    nbinsy = fCurrent2DHist->GetNbinsY();

    auto ** source = new Double_t*[nbinsx];
    for (int i=0;i<nbinsx;i++)
       source[i]=new Double_t[nbinsy];

    for (int i = 0; i < nbinsx; i++){
       for (int j = 0; j < nbinsy; j++){
          source[i][j] = fCurrent2DHist->GetBinContent(i + 1,j + 1);
       }
    }

    spectrum->Background(source,nbinsx,nbinsy,fNumberIterations2DX,fNumberIterations2DY,fDirection2D,fFilterOrder2D);

    for (int i = 0; i < nbinsx; i++){
       for (int j = 0; j < nbinsy; j++)
          fCurrent2DBackd->SetBinContent(i + 1,j + 1, source[i][j]);
    }

    fCurrent2DSubtract->Add(fCurrent2DHist,fCurrent2DBackd,1,-1);

    for (int i=0;i<nbinsx;i++)
       delete[] source[i];

    fMainWindow->NewTab(1,2,"Bckd");

    TVirtualPad *pad = fMainWindow->GetCanvas()->cd(1);
    fCurrent2DSubtract->Draw(draw_opt);
    pad->Update();
    pad->SetBit(TPad::kCannotMove);
    pad->GetFrame()->SetBit(TObject::kCannotPick);

    pad = fMainWindow->GetCanvas()->cd(2);
    fCurrent2DBackd->Draw(draw_opt);
    pad->Update();
    pad->SetBit(TPad::kCannotMove);
    pad->GetFrame()->SetBit(TObject::kCannotPick);
}

ClassImp(CXBgdUtility)
