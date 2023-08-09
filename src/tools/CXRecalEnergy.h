#ifndef CXRecalEnergy_h
#define CXRecalEnergy_h

#include <ctime>
#include <csignal>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>
#include <cmath>
#include <memory.h>

#include <vector>
#include <algorithm>
#include <functional>

#include "TString.h"
#include "TF1.h"
#include "TGraphErrors.h"

#include "CXFitSpek.h"

using namespace std;

struct Fitted
{
    Fitted() : NSubPeaks(0), BgdOff(0), BgdSlope(0), BgFrom(0), BgTo(0), area(0), ampli(0), posi(0), fw05(0), fw01(0), fwhm(0), tailL(0), tailR(0), Lambda(0), Rho(0), S(0), erefindex(-1), good(false) {;}
    int NSubPeaks;
    double BgdOff;
    double BgdSlope;
    double BgFrom;
    double BgTo;
    double area;
    double ampli;
    double posi;
    double errposi;
    double fw05;
    double fw01;
    double fwhm;
    double tailL;
    double tailR;
    double Lambda;
    double Rho;
    double S;
    int    erefindex; // the corresponding calibration line, if good==true
    double eref; // the corresponding calibration line, if good==true
    bool   good;
};

struct NWA_t
{
    NWA_t() : index(-1), width(0), ampli(0) {;}
    int   index;
    float width;
    float ampli;
};

struct NLimits_t
{
    NLimits_t() : index(-1), from(0), to(0) {;}
    int   index;
    int from;
    int to;
};

struct smallerPosi
{
    bool operator()( Fitted elem1, Fitted elem2 ) {
        return elem1.posi < elem2.posi;
    }
};
struct largerArea
{
    bool operator()( Fitted elem1, Fitted elem2 ) {
        return elem1.area > elem2.area;
    }
};
struct largerAmplitude
{
    bool operator()( Fitted elem1, Fitted elem2 ) {
        return elem1.ampli > elem2.ampli;
    }
};

class TH1;

class CXRecalEnergy
{

public:

    CXRecalEnergy();
    virtual ~CXRecalEnergy(){}

    void StartCalib();

    TString fSpectraFolderName;
    TString fSourceName;

    Int_t fId;

    // Calibration
    Int_t         fCalibOrder=1;
    Bool_t        fNoOffset = false;
    TF1          *fCalibFunction = nullptr;
    TGraphErrors *fCalibGraph = nullptr;
    TGraphErrors *fResidueGraph = nullptr;

public:

    void SetDataFromHistTH1(TH1 *hist, Int_t Id=0);
    void SetFileName(TString FileName){specName = FileName;}

    void SetChannelOffset(int Off){specOffset = Off;} //channel offset to subtract to the position of the peaks [0]
    void SetGlobalChannelLimits(int ChFrom, int ChTo){specFromDef = ChFrom; specToDef = ChTo; nlim.clear();} //limit the search to this range in channels

    void SetSource(TString SourceName){fSourceName = SourceName; AnalyseSources();}
    void AnalyseSources();
    void AddPeak(double EPeak){Energies.push_back(EPeak);} //add this energy to the list of lines (can be given more than once)
    Int_t GetNEnergies(){return Energies.size();}
    vector< double > GetEnergies(){return Energies;}
    void RemoveClosestPeakTo(double EPeak){Delendae.push_back(EPeak);} //remove the line closest to this energy from the list of lines
    void SetRefPeak(double EPeak){refEner = EPeak;} //energy (keV) of the reference peak for extended printouts
    void SetGlobalPeaksLimits(float DefFWHM, float DefAmpli){specFWHMdef = DefFWHM; specAMPLdef = DefAmpli;} //default fwhm and minmum amplitude for the peaksearch [10 5]

    void UseFlatBackGround(){fFlatBg = true; fAffineBg = false;} // Use a flat function background to fit the peaks
    void UseAffineBackGround(){fAffineBg = true; fFlatBg = false;} // Use a affine function background to fit the peaks
    void NoBackGround(){fAffineBg = false; fFlatBg = false;}    // No background estimation to fit the peaks

    void UseFirstDerivativeSearch(){Dmode = 1;} // use the 1st-derivative search (default, always for 2-line sources)
    void UseSecondDerivativeSearch(){Dmode = 2;} // use the 2nd-derivative search

    void UseLeftTail(bool on){useTL = on;} // disable using the Left  Tail in peak fit
    void UseRightTail(bool on){useTR = on;} // disable using the Right Tail in peak fit

    void SetFitPlynomialOrder(Int_t order){fCalibOrder = order;}
    void SetNoOffset(Bool_t on) { fNoOffset = on;}

    void SetGain(float Gain){hGain = Gain;} //scaling factor for the slope [1]

    void SetVerbosityLevel(int Verb){Verbosity = Verb;} // verbosity bit0=fit_details, bit1=calib_details, bit2=more_calib_details [0]
    void SetFullVerbosity(){Verbosity = 0xFFFF;} //Set full verbosity
    void Reset();

    Float_t GetOffset(){return offset1;}
    Float_t GetSlope(){return slope1*hGain;}

    Double_t PolynomialFunc(Double_t*xx,Double_t*pp);

public:

    vector<Fitted> Peaks;
    vector<Fitted> GetFitResults(){return Peaks;}

    string  specName;
    int     specLength;
    float  *specData = nullptr;
    int     specOffset;           // subtracted immediately after peak fit

    int     specFromDef, specToDef;

    float   specFWHMdef;
    float   specAMPLdef;

    vector<double> specPeaks;
    vector<double> Energies;
    vector<double> Delendae;

    double eBhead;      // band head
    double eBstep;      // delta
    int    eBnumb;  // numbero of peaks

    double refEner;
    bool  useTL;
    bool  useTR;

    bool fFlatBg;
    bool fAffineBg;

    vector<NWA_t> nwa;
    vector<NLimits_t> nlim;

    bool   bTwoLines;
    bool   bOneLine;
    bool   doSlope;
    bool   doPoly1;   // linear
    bool   doPoly2;   // cubic
    int    numZero;       // number of fake peaks at (0,0)

    bool   useErrors ;   // fit using position and energy errors
    double errE;

    int    Dmode;   // peak search using first derivative
    int    xP_0, xP_1, xP_2, xP_3, xP_4, xP_5, xP_6;
    float  xP_minAmpl;
    float  xP_fThreshCFA;
    int    xP_nMinWidthP;
    int    xP_nMinWidthN;
    int    xP_nMinWidthP1;
    int    xP_nMinWidthP2;

    CXCFitSpek   *m_pCFit;

    double slope, tshift;
    double offset1, slope1;               // should be generalized to polynomials
    double offset2, slope2, curv2;

    float hGain;

    int  Verbosity;  // &1 PeakFit  &2 CalibPeaks   &4 CalibSort

    // functions of ReadATCA
    void    initialize();
    int     PeakSearch1(float * data, int chA, int chB, float fwhm, float minAmpl, std::vector<double>&vPeaks);
    int     PeakSearch2(float * data, int chA, int chB, float fwhm, float minAmpl, std::vector<double>&vPeaks);
    int     LargePeaks1(float * data, int chA, int chB, float fwhm, float minAmpl, std::vector<double>&vPeaks, int maxNum);
    void    SmoothSpek(float *spek, int nchan, double sigma, int ndiff, float *& tSpek, int &start);
    int     xP_Next1(float *yVal, int yLen);
    int     xP_Next2(float *yVal, int yLen);
    int     FitPeaks(int verbose);
    Int_t   EROOTCalibration();
    double  TCalibration();   // shift of the largest peak from its reference position (to be done)
    bool    InvertMatrix3(const double m[9], double invOut[9]);
    double  Calibrated(double x);

    clock_t startTime, stopTime;

    ClassDef(CXRecalEnergy,0);
};

#endif
