#include "CXHist1DCalib.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "TGNumberEntry.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TROOT.h"
#include "TObjArray.h"
#include "TF1.h"
#include "TMath.h"
#include "TGComboBox.h"
#include "TRatioPlot.h"

#include "CXBashColor.h"
#include "CXMainWindow.h"

using namespace std;

CXHist1DCalib::CXHist1DCalib(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Energy calibration", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fGroupFrame->AddFrame(new TGLabel(fGroupFrame,"Sources"), new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 5, 0));
    fSources = new TGTextEntry(fGroupFrame, "");
    fSources->SetToolTipText("List of sources (files in DataBase/Sources)");
    fSources->Connect("TextChanged(const char *)", "CXHist1DCalib", this, "UpdateText()");
    fSources->Connect("ReturnPressed()", "CXHist1DCalib", this, "UpdateSources()");
    fGroupFrame->AddFrame(fSources,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,0,0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Verbose level"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,20,0,0));
    fVerboseLevel = new TGNumberEntry(fHorizontalFrame, 1, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELLimitMinMax,0,3);
    fHorizontalFrame->AddFrame(fVerboseLevel,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,5,5));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Calibration order"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,10,0,0));
    fCalibOrder = new TGNumberEntry(fHorizontalFrame, 1, 5, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fHorizontalFrame->AddFrame(fCalibOrder,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "No offset"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,10,0,0));
    fNoOffset = new TGCheckButton(fHorizontalFrame);
    fNoOffset->SetState(kButtonUp);
    fHorizontalFrame->AddFrame(fNoOffset,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "FWHM"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,40,0,0));
    fFWHMSPEntry = new TGNumberEntry(fHorizontalFrame, 15, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fFWHMSPEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,3,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Amplitude"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,15,10,0,0));
    fThresholdSPEntry = new TGNumberEntry(fHorizontalFrame, 5, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fThresholdSPEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Range:   From "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,5,0,0));
    fRangeMin = new TGNumberEntry(fHorizontalFrame, 0, 3, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fRangeMin,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "To "), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,15,5,0,0));
    fRangeMax = new TGNumberEntry(fHorizontalFrame, 32000, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fRangeMax,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    TGTextButton *DoCurrentRange = new TGTextButton(fHorizontalFrame, "Current");
    DoCurrentRange->Connect("Clicked()", "CXHist1DCalib", this, "GetCurrentRange()");
    fHorizontalFrame->AddFrame(DoCurrentRange,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Left tail"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,20,0,0));
    fLeftTail = new TGCheckButton(fHorizontalFrame);
    fLeftTail->SetState(kButtonDown);
    fHorizontalFrame->AddFrame(fLeftTail,new TGLayoutHints(kLHintsCenterY | kLHintsLeft  | kLHintsExpandX ,1,-1,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Right tail"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,15,20,0,0));
    fRightTail = new TGCheckButton(fHorizontalFrame);
    fRightTail->SetState(kButtonDown);
    fHorizontalFrame->AddFrame(fRightTail,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Use 2nd-derivative search"), new TGLayoutHints(kLHintsCenterY | kLHintsLeft,5,20,0,0));
    f2DSearch = new TGCheckButton(fHorizontalFrame);
    fHorizontalFrame->AddFrame(f2DSearch,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX ,1,3,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *DoCalib = new TGTextButton(fHorizontalFrame, "Calibrate");
    DoCalib->SetTextColor(CXred);
    DoCalib->Connect("Clicked()", "CXHist1DCalib", this, "Calibrate()");
    fHorizontalFrame->AddFrame(DoCalib,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,50,50,10,10));

    fListOfObjects = new TList;
    fListOfObjects->SetOwner();

    fSourcesFolder = Form("%s/DataBase/Sources",getenv("GWSYS"));

    fRecalEnergy = new CXRecalEnergy;
}

CXHist1DCalib::~CXHist1DCalib()
{
}

void CXHist1DCalib::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXHist1DCalib::HandleMouse(Int_t /*EventType*/,Int_t /*EventX*/,Int_t /*EventY*/, TObject* /*selected*/)
{
    TPad *pad = dynamic_cast<TPad*>(gROOT->GetSelectedPad());

    if(pad==nullptr)
        return;
}

void CXHist1DCalib::HandleMyButton()
{

}

void CXHist1DCalib::UpdateText()
{
    fSources->SetTextColor(CXred);
}

void CXHist1DCalib::UpdateSources()
{
    fEnergies.clear();
    fERef = 0.;

    TString tmp = fSources->GetText();
    tmp.ReplaceAll(",", " ");

    TObjArray *arr = tmp.Tokenize(" ");
    if(arr->GetEntries()==0) {
        delete arr;
        return;
    }

    fSources->SetTextColor(CXblack);

    gbash_color->InfoMessage("Energies used for calibration: ");

    for(int i=0 ; i<arr->GetEntries() ; i++) {
        TString FileName = Form("%s/%s.source",fSourcesFolder.Data(),arr->At(i)->GetName());
        if(gSystem->IsFileInIncludePath(FileName)) {
            ifstream file(FileName);
            cout << "Source: " << arr->At(i)->GetName() << endl;
            string line;
            TString Buffer;

            while(file) {
                getline(file,line);
                Buffer=line;
                if(Buffer.BeginsWith("#"))
                    continue;
                Buffer.ReplaceAll("\t"," ");
                TObjArray *arr2 = Buffer.Tokenize(" ");
                if(arr2->GetEntries()>0) {
                    bool ref = false;
                    TString textline = (TString)arr2->First()->GetName();
                    if(textline.Contains("*")) {
                        textline.ReplaceAll("*","");
                        ref = true;
                    }
                    Double_t E = ((TString)arr2->First()->GetName()).Atof();
                    fEnergies.push_back(E);
                    if(ref) fERef = E;
                    cout << " --> E: " << E << " keV" << endl;
                }
                delete arr2;
            }

            file.close();
        }
        else if(((TString)arr->At(i)->GetName()) == "-ener" && i<arr->GetEntries()-1) {
            TString manualvalue = ((TString)arr->At(i+1)->GetName());
            if(manualvalue.IsFloat()) {
                fEnergies.push_back(manualvalue.Atof());
                cout << "Value: " << arr->At(i+1)->GetName() << " (keV) manually added." << endl;
                i++;
                continue;
            }
            else {
                gbash_color->WarningMessage(" Error in manually added value");
                fSources->SetTextColor(CXred);
                continue;
            }
        }
        else {
            gbash_color->WarningMessage(FileName + " not found ");
            fSources->SetTextColor(CXred);
        }
    }
    delete arr;

    if(fEnergies.size() && fERef>0.) {
        gbash_color->InfoMessage(Form("Reference energy for printouts: %f keV",fERef));
    }

    cout<<endl;
}

void CXHist1DCalib::CleanCalib()
{
    fListOfObjects->Clear();
}

void CXHist1DCalib::GetCurrentRange()
{
    TH1 *hist = fMainWindow->GetCanvas()->FindHisto();

    if(hist == nullptr || hist->GetDimension()>1) {
        gbash_color->WarningMessage("No 1D histogram in the current pad, ignored ");
        return;
    }

    fRangeMin->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetFirst()));
    fRangeMax->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetLast()));
}

void CXHist1DCalib::Calibrate()
{
    CleanCalib();

    TH1 *hist = fMainWindow->GetCanvas()->FindHisto();

    if(hist == nullptr || hist->GetDimension()>1) {
        gbash_color->WarningMessage("No 1D histogram in the current pad, ignored ");
        return;
    }
    if(fEnergies.size()==0) {
        gbash_color->WarningMessage("No source defined, ignored ");
        return;
    }

    if(fRangeMin->GetNumber()<hist->GetXaxis()->GetBinLowEdge(1))
        fRangeMin->SetNumber(hist->GetXaxis()->GetBinLowEdge(1));
    if(fRangeMax->GetNumber()>hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()))
        fRangeMax->SetNumber(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()));

    fRecalEnergy->Reset();
    fRecalEnergy->SetDataFromHistTH1(hist,0);

    for (auto ie : fEnergies)
        fRecalEnergy->AddPeak(ie);
    if(fERef>0.) fRecalEnergy->SetRefPeak(fERef);

    fRecalEnergy->SetGain(1.);                          // scaling factor for the slope [1]
    //    fRecalEnergy->SetChannelOffset(0);                  // channel offset to subtract to the position of the peaks [0]
    fRecalEnergy->SetVerbosityLevel(fVerboseLevel->GetNumber()-1);                 // verbosity -1=noprint 0=fit_details, 1=calib_details, 2=more_calib_details [-1]

    fRecalEnergy->SetFitPlynomialOrder(fCalibOrder->GetNumber());
    fRecalEnergy->SetNoOffset(fNoOffset->GetState());

    fRecalEnergy->UseLeftTail(fLeftTail->GetState());
    fRecalEnergy->UseRightTail(fRightTail->GetState());

    if(f2DSearch->GetState() == kButtonUp)
        fRecalEnergy->UseFirstDerivativeSearch();
    else
        fRecalEnergy->UseSecondDerivativeSearch();

    //fRecalEnergy->UseSecondDerivativeSearch();        // use the 2nd-derivative search
    fRecalEnergy->SetGlobalChannelLimits(fRangeMin->GetNumber(),fRangeMax->GetNumber());      // limit the search to this range in channels
    fRecalEnergy->SetGlobalPeaksLimits(fFWHMSPEntry->GetNumber(),fThresholdSPEntry->GetNumber());   // default fwhm and minmum amplitude for the peaksearch [15 5]

    fRecalEnergy->StartCalib();

    vector < Fitted > FitResults = fRecalEnergy->GetFitResults();

    if(fVerboseLevel->GetNumber()==0 && fRecalEnergy->fCalibFunction) {
        cout<< left << scientific << setprecision(6);
        cout<< hist->GetName()<<": ";
        cout << setw(14) << fRecalEnergy->fCalibFunction->GetParameter(0);
        for(int i=1 ; i<=fRecalEnergy->fCalibOrder ; i++) cout << setw(14) << fRecalEnergy->fCalibFunction->GetParameter(i)*TMath::Power(fRecalEnergy->hGain,i);
        cout<<endl;
    }

    double xmin=-1;
    double xmax=-1;

    int NGoodPeak=0;

    for(size_t i=0 ; i<FitResults.size() ; i++) {
        Fitted FitRes = FitResults[i];

        if(!FitRes.good) continue;
        NGoodPeak++;

        // Calc n good sub peaks
        int NSubPeaks = FitRes.NSubPeaks;
        int NGoodSubPeaks = 0;

        for(int j=0 ; j<NSubPeaks ; j++) {
            FitRes = FitResults[i+j];
            if(FitRes.good) NGoodSubPeaks++;
        }

        FitRes = FitResults[i];

        TF1 *f = GetDinoFct(Form("Peak%d_%.1f",NGoodPeak,FitRes.eref),FitRes.BgFrom,FitRes.BgTo,5+6*NGoodSubPeaks);
        f->SetNpx(10000);

        f->SetParameter(0,NGoodSubPeaks);
        f->SetParameter(1,FitRes.BgFrom);
        f->SetParameter(2,FitRes.BgTo);
        f->SetParameter(3,FitRes.BgdOff);
        f->SetParameter(4,FitRes.BgdSlope);

        if(xmin == -1) xmin = FitRes.BgFrom;
        xmax = FitRes.BgTo;

        int peakid=0;
        for(int j=0 ; j<NSubPeaks ; j++) {

            FitRes = FitResults[i+j];

            if(!FitRes.good) continue;

            f->SetParameter(5+peakid*6+0,FitRes.ampli);
            f->SetParameter(5+peakid*6+1,FitRes.posi);
            f->SetParameter(5+peakid*6+2,FitRes.fwhm);
            f->SetParameter(5+peakid*6+3,FitRes.Lambda);
            f->SetParameter(5+peakid*6+4,FitRes.Rho);
            f->SetParameter(5+peakid*6+5,FitRes.S);

            if(fLeftTail->GetState()==kButtonUp)
                f->SetParameter(5+peakid*6+3,-50);
            if(fRightTail->GetState()==kButtonUp)
                f->SetParameter(5+peakid*6+4,50);

            peakid++;
        }
        f->Draw("same");
        fListOfObjects->Add(f);
        i += NSubPeaks-1;
    }

    if(xmin!=-1) hist->GetXaxis()->SetRangeUser(xmin-(xmax-xmin)*0.1,xmax+(xmax-xmin)*0.1);

    fMainWindow->RefreshPads();

    if(FitResults.size()>1 && fRecalEnergy->fCalibFunction) {
        if(fCalibCanvas != nullptr) delete fCalibCanvas;
        fCalibCanvas = new TCanvas;
        fCalibCanvas->SetName("CalibrationResults");
        fCalibCanvas->SetTitle("Calibration Results");
        fCalibCanvas->Divide(1,2,0.0001,0.0001);
        fCalibCanvas->cd(1);
        fRecalEnergy->fCalibGraph->Draw("ap");
        fRecalEnergy->fCalibFunction->Draw("same");
        fCalibCanvas->cd(2);
        fRecalEnergy->fResidueGraph->Draw("ape");
        fCalibCanvas->Update();
        fCalibCanvas->Modified();
    }

    fMainWindow->GetCanvas()->cd();
}

double CXHist1DCalib::DinoFct(double*xx,double*pp)
{
    double x   = xx[0];

    int    NSubPeaks = (int)pp[0]; //Number of subpeaks in the peak range
    double BgFrom    = pp[1]; //First Channel for the Bg estimation
    double BgTo      = pp[2]; //Last Channel for the Bg estimation
    double BgdOff    = pp[3]; //Bg offset
    double BgdSlope  = pp[4]; //Bg slope

    double f_tot = 0.;

    if(x<BgFrom || x>BgTo) return 0.;
    else
    {
        double BGd = BgdSlope*(x-BgFrom) + BgdOff;
        f_tot += BGd;
    }

    //    cout<<NSubPeaks<<" "<<BgFrom<<" "<<BgTo<<" "<<BgdOff<<" "<<BgdSlope<<endl;
    for(int i=0 ; i<NSubPeaks ; i++)
    {
        double Ampli     = pp[5+i*6+0];
        double Mean      = pp[5+i*6+1];
        double Sigma     = pp[5+i*6+2]*1./sqrt(8.*log(2.));;
        double Lambda    = pp[5+i*6+3];
        double Rho       = pp[5+i*6+4];
        double S         = pp[5+i*6+5];

        //        cout<<Ampli<<" "<<Mean<<" "<<Sigma<<" "<<Lambda<<" "<<Rho<<" "<<S<<endl;

        double U         = (x-Mean)/Sigma;
        double f_g       = Ampli*TMath::Exp(-U*U*0.5);
        double f_lambda  = Ampli*TMath::Exp(-0.5*Lambda*(2.*U-Lambda));
        double f_rho     = Ampli*TMath::Exp(-0.5*Rho*(2.*U-Rho));
        double f_S       = Ampli*S*1./((1+TMath::Exp(U))*(1+TMath::Exp(U)));

        if(U<Lambda) f_tot += f_lambda;
        else if(U>Rho) f_tot += f_rho;
        else f_tot += f_g;

        f_tot += f_S;
    }

    return f_tot;
}

///******************************************************************************************///

TF1 *CXHist1DCalib::GetDinoFct(TString Name,double min, double max, int Npar)
{
    TF1 *f = new TF1(Name,this,&CXHist1DCalib::DinoFct,min,max,Npar,"CXHist1DCalib","DinoFct");

    return f;
}

ClassImp(CXHist1DCalib)
