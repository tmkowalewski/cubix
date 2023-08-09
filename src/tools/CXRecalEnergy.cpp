//
// cpp version of recal_cob
// improved search of the peaks
// using user-defined sources
//
// Dino Bazzacco, August 2014
//
// adapted by J.Dudouet
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "TSystem.h"
#include "TH1.h"
#include "TAxis.h"
#include "TVirtualFitter.h"
#include "TFitResult.h"
#include "Fit/Fitter.h"
#include "TMath.h"

#include "CXRecalEnergy.h"

///******************************************************************************************///
///************************************ CXRecalEnergy Class ***************************************///
///******************************************************************************************///

CXRecalEnergy::CXRecalEnergy() :
    fSpectraFolderName(""),
    fSourceName(""),
    Peaks(),
    specName(""),
    specLength(0),
    specData(NULL),
    specOffset(0),
    specFromDef(0),
    specToDef(0),
    specFWHMdef(15),
    specAMPLdef(5),
    specPeaks(),
    Energies(),
    Delendae(),
    eBhead(0.0),
    eBstep(0.0),
    eBnumb(0),
    refEner(0),
    useTL(true),
    useTR(true),
    fFlatBg(1),
    fAffineBg(0),
    bTwoLines(false),
    bOneLine(false),
    numZero(0),
    useErrors(false),
    errE(0.01),
    Dmode(1),
    xP_0(0), xP_1(0), xP_2(0), xP_3(0), xP_4(0), xP_5(0), xP_6(0),
    xP_minAmpl(0.0),
    xP_fThreshCFA(0.0),
    xP_nMinWidthP(0),
    xP_nMinWidthN(0),
    xP_nMinWidthP1(0),
    xP_nMinWidthP2(0),
    m_pCFit(NULL),
    tshift(0),
    hGain(1),
    Verbosity(-1)
{
}

void CXRecalEnergy::Reset()
{
    fSourceName = "";
    Peaks.clear();
    specName="";
    specLength=0;
    if(specData) {
        delete [] specData;
        specData =nullptr;
    }
    specOffset = 0;
    specFromDef = 0;
    specToDef = 0;
    specFWHMdef = 15;
    specAMPLdef = 5;
    specPeaks.clear();
    Energies.clear();
    Delendae.clear();
    eBhead=0.0;
    eBstep=0.0;
    eBnumb=0;
    refEner=0;
    useTL=true;
    useTR=true;
    fFlatBg=1;
    fAffineBg=0;
    bTwoLines=false;
    bOneLine=false;
    numZero=0;
    useErrors=false;
    errE=0.01;
    Dmode=1;
    xP_0=0; xP_1=0; xP_2=0; xP_3=0; xP_4=0; xP_5=0; xP_6=0;
    xP_minAmpl=0.0;
    xP_fThreshCFA=0.0;
    xP_nMinWidthP=0;
    xP_nMinWidthN=0;
    xP_nMinWidthP1=0;
    xP_nMinWidthP2=0;
    if(m_pCFit) {
        delete m_pCFit;
        m_pCFit = nullptr;
    }
    tshift=0;
    hGain=1;
    Verbosity=-1;
}

///******************************************************************************************///

void CXRecalEnergy::StartCalib()
{
    initialize();                 // initialize parameters related to digitisers and detectors

    startTime  = clock();

    if(specData == nullptr) {
        cout<<"Data empty"<<endl;
        return;
    }

    int np    = 0;
    int ngood = 0;

    if(bOneLine)
        np = LargePeaks1(specData, specFromDef, specToDef, specFWHMdef, specAMPLdef, specPeaks, 1);
    else if(bTwoLines)
        np = LargePeaks1(specData, specFromDef, specToDef, specFWHMdef, specAMPLdef, specPeaks, 6);
    else if(Dmode == 1)
        np = PeakSearch1(specData, specFromDef, specToDef, specFWHMdef, specAMPLdef, specPeaks);
    else
        np = PeakSearch2(specData, specFromDef, specToDef, specFWHMdef, specAMPLdef, specPeaks);

    cout<<"yo :" <<np<<endl;

    if(np) {
        int fp = FitPeaks( Verbosity );
        if(fp) {
            if(specOffset) {               // correct peak positions for specOffs
                for(size_t np_ = 0; np_ < Peaks.size(); np_++) {
                    Peaks[np_].posi -= specOffset;
                }
            }
            if(fCalibOrder) {
                ngood = EROOTCalibration();
            }
            else {
                tshift = refEner - Peaks[0].posi;
                ngood  = 1;
            }
        }
    }
    else {
        Peaks.clear();
        if(Verbosity > 0)
            cout<<Form("\n");
        return;
    }

    if(fCalibOrder) {
        double refPeakPosi=0, refPeakEner=0;
        int iref = -1;;
        if(refEner > 0 && fCalibOrder) {
            refPeakPosi = fCalibFunction->GetX(refEner);                            // position of the reference peak derived from calibration
            for(size_t irp = 0; irp < Peaks.size(); irp++) {
                if( (abs(Peaks[irp].posi-refPeakPosi) < specFWHMdef)/* && Peaks[irp].good*/ ) { // not restricted to the good peaks
                    iref = irp;
                    refPeakPosi = Peaks[irp].posi;                      // actual peak position
                    refPeakEner = fCalibFunction->Eval(refPeakPosi);
                }
            }
        }
        if(Verbosity >= 0)
            cout << "#" << setw(3) << fId << setw(5) << np << setw(4) << ngood;

        double chi2 = 0;
        int nused = 0;
        for(size_t np_ = 0; np_ < Peaks.size(); np_++) {
            if(Peaks[np_].good) {
                nused++;
                double ee = slope*(Peaks[np_].posi);
                chi2 += pow(Energies[Peaks[np_].erefindex]-ee, 2);
            }
        }
        chi2 = (nused-1) ? chi2/(nused-1) : 0;
        if(Verbosity>=0) {
            cout<<right<<fixed;
            if(iref>=0) {
                double fw05_e = fCalibFunction->Eval(Peaks[iref].fw05)-fCalibFunction->GetParameter(0);
                double fw01_e = fCalibFunction->Eval(Peaks[iref].fw01)-fCalibFunction->GetParameter(0);

                cout << setw(10) << setprecision(2) << refPeakEner  << setw(8) << fw05_e << setw(8) << fw01_e << setw(12) << setprecision(1) << Peaks[iref].area << setw(12) << setprecision(2) << Peaks[iref].posi << setw(8) << Peaks[iref].fwhm << setw(10) << Peaks[iref].ampli << setw(8) << Peaks[iref].tailL << setw(8) << Peaks[iref].tailR;
                cout << scientific<<setprecision(6) << setw(14) << fCalibFunction->GetParameter(0);
                for(int i=1 ; i<=fCalibOrder ; i++) cout << setw(14) << fCalibFunction->GetParameter(i)*TMath::Power(hGain,i);
                cout << setw(8) << fixed<< setprecision(2) << min(999.99, fCalibFunction->GetChisquare()/fCalibFunction->GetNDF()*100.);
            }
            else {
                cout << setw(10) << 0.  << setw(8) << 0. << setw(8) << 0. << setw(10) << 0. << setw(12) << 0. << setw(6) << 0. << setw(10) << 0. << setw(8) << 0. << setw(8) << 0.;
                cout << setw(10) << 0.;
                for(int i=1 ; i<=fCalibOrder ; i++) cout << setw(10) << 0.;
                cout << setw(8) << 0.;
            }
        }
    }
    else if(Verbosity>=0) {
        cout << "#" << setw(3) << np << setw(4) << ngood;
        if(Peaks.size())
            cout << setw(10) << Peaks[0].posi << setw(8) << Peaks[0].fw05 << setw(8) << Peaks[0].fw01 << setw(12) << Peaks[0].area << setw(12) << Peaks[0].posi << setw(8) << Peaks[0].fwhm << setw(10) << Peaks[0].tailL << setw(8) << Peaks[0].tailR << setw(8) << tshift*hGain;
        else
            cout << setw(10) << 0. << setw(8) << 0. << setw(8) << 0. << setw(10) << 0. << setw(12) << 0. << setw(6) << 0. << setw(10) << 0. << setw(8) << 0. << setw(8) << 0.;
    }
    if(Verbosity>=0) cout<<endl;

    stopTime = clock();
}

///******************************************************************************************///

void CXRecalEnergy::initialize()
{
    m_pCFit   = new CXCFitSpek();
    int  npars = m_pCFit->GFitParNumber();
    int* vpars = (int *) new int [npars];
    m_pCFit->GFitParGet(vpars);

    // exclude the very first and last part of the spectrum
    if(specFromDef == specToDef) {
        specFromDef = int(5*specFWHMdef);
        specToDef   = specLength-1 - specFromDef;
    }

    if(Verbosity>=0) cout << "\n\n# Channel range : [ "<<specFromDef<<" ; "<<specToDef<<" ]"<<endl;
    if(Verbosity>=0) cout << "# Search peak limits : FWHM < "<< specFWHMdef<<" ; Amplitude > "<<specAMPLdef<<endl;

    if(fFlatBg) {
        vpars[0] = 1;
        vpars[1] = 0;
        fAffineBg = false;
        if(Verbosity>=0) cout << "# Flat Background estimation " << endl;
    }
    else if(fAffineBg) {
        vpars[0] = 1;
        vpars[1] = 1;
        fFlatBg = false;
        if(Verbosity>=0) cout << "# Affine Background estimation " << endl;
    }
    else if(!fAffineBg && ! fFlatBg) {
        vpars[0] = 0;
        vpars[1] = 0;
        fFlatBg = false;
        fAffineBg = false;
        if(Verbosity>=0) cout << "# No Background estimation " << endl;
    }
    if(Verbosity>=0) {
        cout << "#  "<< endl;
        if(Dmode==1) cout << "# Using 1st-derivative search"<<endl;
        else if(Dmode==2) cout << "# Using 2nd-derivative search"<<endl;
        cout << "# " << endl;

        if(!useTL) cout << "# Left Tail in peak fit disabled"<<endl<< "# " << endl;
        if(!useTR) cout << "# Right Tail in peak fit disabled"<<endl<< "# " << endl;

        if(fNoOffset) cout << "# No offset for calibration" << endl;
        cout               << "# Calibration: Polynomial order: " << fCalibOrder << endl;
        cout<< "# Scaling factor for the slope = "<<hGain<<endl;
        cout<< "# " << endl;

        cout << "#  Selected source : " << fSourceName<<endl;
        cout << "# " << endl;
    }
    // enabling the tails should be decided from the command line
    useTL ? vpars[6] |= 1 : vpars[6] &= ~1;   // control of the Tail to the Left
    useTR ? vpars[7] |= 1 : vpars[7] &= ~1;   // control of the Tail to the Right
    if(fCalibOrder==0) vpars[5] = 0;  // no step for time shifts
    m_pCFit->GFitParSet(vpars);

    for(int nn = 0; nn < eBnumb; nn++)
        Energies.push_back(eBhead + eBstep*nn);

    //    if(Energies.size() < 1) {
    //        Energies.push_back(1173.238);
    //        Energies.push_back(1332.513);
    //    }
    if(Delendae.size()) {
        for(size_t dd = 0; dd < Delendae.size(); dd++) {
            size_t closest = 1;
            double delta   = fabs(Delendae[dd]-Energies[0]);
            for (size_t nn = 1; nn < Energies.size(); nn++) {
                if(fabs(Delendae[dd]-Energies[nn]) < delta) {
                    closest = nn;
                    delta   = fabs(Delendae[dd]-Energies[nn]);
                }
            }
            Energies.erase(Energies.begin()+closest);
        }
    }
    sort( Energies.begin( ), Energies.end( ) );

    if(Energies.size()==1) {
        bOneLine  = true;
        bTwoLines = false;
        refEner = Energies[0];
    }
    else if(Energies.size()==2) {
        bOneLine  = false;
        bTwoLines = true;
        if(!refEner) {
            refEner = Energies[1];
        }
        else {
            if(abs(refEner-Energies[0])>1. && abs(refEner-Energies[1])>1.)
                refEner = Energies[1];
        }
    }
    else {
        bOneLine  = false;
        bTwoLines = false;
        if(!refEner)
            refEner = Energies[Energies.size()-1];
    }
    if(Verbosity>=0) {
        if(fCalibOrder)
            cout << "#  " << "Energies used for the calibration " << endl;
        else
            cout << "#  " << "Position used for the calibration " << endl;
        for (size_t nn = 0; nn < Energies.size(); nn++) {
            cout << "#  " << setw(3) << nn << fixed << setprecision(3) << setw(10) << Energies[nn];
            if(refEner==Energies[nn]) cout<<" ==> Reference peak for extended printouts";
            cout<<endl;
        }

        cout << "#  " << endl;
        cout<<right;
        if(fCalibOrder) {
            cout << "# Id" << setw(5) << "#pks" << setw(4) << "#ok" << setw(10) << "rEnergy"  << setw(8) << "FW05" << setw(8) << "FW01" << setw(12) << "Area" << setw(12) << "Position" << setw(8) << "Width" << setw(10) << "Ampli" << setw(8) << "WTML" << setw(8) << "WTMR";
            for(int i=0 ; i<=fCalibOrder ; i++) cout << setw(14) << Form("a%d",i);
            cout << setw(8) << "rChi2\%";
        }
        else {
            cout << "#pks" << setw(4) << "#ok" << setw(10) << "rEnergy"  << setw(8) << "FW05" << setw(8) << "FW01" << setw(12) << "Area" << setw(12) << "Position" << setw(8) << "Width" << setw(10) << "Ampli" << setw(8) << "WTML" << setw(8) << "WTMR";
        }
        cout << "\n# " << endl;
    }
}
///******************************************************************************************///

void CXRecalEnergy::AnalyseSources()
{
    Energies.clear();

    if(fSourceName.Contains("22Na",TString::kIgnoreCase))
    {
        fSourceName = "22Na";
        Energies.push_back( 511.006);
        Energies.push_back(1274.545);
    }
    if(fSourceName.Contains("56Co",TString::kIgnoreCase))
    {
        fSourceName = "56Co";
        Energies.push_back( 846.772);
        Energies.push_back(1037.840);
        Energies.push_back(1175.102);
        Energies.push_back(1238.282);
        Energies.push_back(1360.250);
        Energies.push_back(1771.351);
        Energies.push_back(2015.181);
        Energies.push_back(2034.755);
        Energies.push_back(2598.458);
        Energies.push_back(3201.962);
        Energies.push_back(3253.416);
        Energies.push_back(3272.990);
        Energies.push_back(3451.152);
        Energies.push_back(3547.925);
    }
    if(fSourceName.Contains("40K",TString::kIgnoreCase))
    {
        fSourceName = "40K";
        Energies.push_back( 122.0614);
        Energies.push_back( 136.4743);
    }
    if(fSourceName.Contains("57Co",TString::kIgnoreCase))
    {
        fSourceName = "57Co";
        //Energies.push_back(  14.4130);
        Energies.push_back( 122.0614);
        Energies.push_back( 136.4743);
    }
    if(fSourceName.Contains("60Co",TString::kIgnoreCase))
    {
        fSourceName = "60Co";
        Energies.push_back(1173.238);
        Energies.push_back(1332.513);
    }
    if(fSourceName.Contains("88Y",TString::kIgnoreCase))
    {
        fSourceName = "88Y";
        Energies.push_back( 898.045);
        Energies.push_back(1836.062);
        //Energies.push_back(2734.087);
    }
    if(fSourceName.Contains("133Ba",TString::kIgnoreCase))
    {
        fSourceName = "133Ba";
        Energies.push_back(  53.156);
        Energies.push_back(  79.623);
        Energies.push_back(  80.999);
        Energies.push_back( 160.609);
        Energies.push_back( 223.116);
        Energies.push_back( 276.404);
        Energies.push_back( 302.858);
        Energies.push_back( 356.014);
        Energies.push_back( 383.859);
    }
    if(fSourceName.Contains("134Cs",TString::kIgnoreCase))
    {
        fSourceName = "134Cs";
        Energies.push_back( 475.36);
        Energies.push_back( 563.27);
        Energies.push_back( 569.30);
        Energies.push_back( 604.68);
        Energies.push_back( 795.78);
        Energies.push_back( 801.86);
        Energies.push_back(1038.53);
        Energies.push_back(1167.89);
        Energies.push_back(1365.17);
    }
    if(fSourceName.Contains("137Cs",TString::kIgnoreCase))
    {
        fSourceName = "137Cs";
        Energies.push_back( 661.661);
    }
    if(fSourceName.Contains("152Eu",TString::kIgnoreCase))
    {
        fSourceName = "152Eu";
        Energies.push_back( 121.7793);
        Energies.push_back( 244.6927);
        Energies.push_back( 344.2724);
        Energies.push_back( 411.111);
        Energies.push_back( 443.979);
        Energies.push_back( 778.890);
        Energies.push_back( 964.014);
        Energies.push_back(1085.793);
        //Energies.push_back(1089.700);
        Energies.push_back(1112.070);
        Energies.push_back(1407.993);
    }
    if(fSourceName.Contains("208Pb",TString::kIgnoreCase))
    {
        fSourceName = "208Pb";
        Energies.push_back(2614.5);   // increase precision
    }
    if(fSourceName.Contains("226Ra",TString::kIgnoreCase))
    {
        fSourceName = "226Ra";
        Energies.push_back( 186.211);
        Energies.push_back( 241.981);
        Energies.push_back( 295.213);
        Energies.push_back( 351.921);
        Energies.push_back( 609.312);
        Energies.push_back( 768.356);
        Energies.push_back( 934.061);
        Energies.push_back(1120.287);
        Energies.push_back(1238.110);
        Energies.push_back(1377.669);
        Energies.push_back(1509.228);
        Energies.push_back(1729.595);
        Energies.push_back(1764.494);
        Energies.push_back(1847.420);
        Energies.push_back(2118.551);
        Energies.push_back(2204.215);
        Energies.push_back(2447.810);
    }
    if(fSourceName.Contains("241Am",TString::kIgnoreCase))
    {
        fSourceName = "241Am";
        Energies.push_back(  26.345);
        Energies.push_back(  59.537);
    }
    if(fSourceName.Contains("28Al",TString::kIgnoreCase))
    {
        fSourceName = "28Al";

        Energies.push_back(30.6382 );
        Energies.push_back(83.899  );
        Energies.push_back(212.017 );
        Energies.push_back(271.175 );
        //            Energies.push_back(510.999 ); //511 added in the calibration
        Energies.push_back(831.45  );
        Energies.push_back(846.764 );
        Energies.push_back(983.02  );
        Energies.push_back(1622.87 );
        Energies.push_back(1778.969);
        Energies.push_back(1810.726);
        Energies.push_back(2282.773);
        Energies.push_back(2590.244);
        Energies.push_back(3033.89 );
        Energies.push_back(3465.07 );
        Energies.push_back(4133.408);
        Energies.push_back(4259.539);
        Energies.push_back(4733.847);
        Energies.push_back(6702.034);
        Energies.push_back(7213.034);
        Energies.push_back(7724.034);
    }
}

///******************************************************************************************///

// - smooth the spectrum with a gaussian kernel
// - differentiate ONCE
// Loop (maxNum times)
//   find the minimum of the derivative and the associated +++--- pattern
//   accept it with conditions with the cross-over as peak position
//   erase the region
// 
int CXRecalEnergy::LargePeaks1(float *m_pSpek, int chA, int chB, float fwhm, float minAmpl, std::vector<double>&vPeaks, int maxNum)
{
    //CheckLimits(chA, chB);
    int nChan = chB-chA+1;

    double  sigma = fwhm/s2fwhm;
    float  *tSpek = NULL;     // the smoothed and differentiated spectrum
    int     xMin  = 0;        // position of chA in tSpek
    SmoothSpek(m_pSpek+chA, nChan, sigma, 1, tSpek, xMin);
    int xMax = xMin + nChan;  // position of chB+1 in tSpek

    // ready to search peaks from xMin to xMax
    vPeaks.clear();

    // put here sensible values
    xP_minAmpl     = float(0.5*abs(minAmpl*exp(-.5)/sigma)); // max(g') = |g'(sigma)| = g(0)*exp(-0.5)/sigma
    xP_fThreshCFA  = 0;
    xP_nMinWidthP  = max(2, int(fwhm)-2);
    xP_nMinWidthN  = max(2, int(fwhm)-2);

    while(vPeaks.size() < size_t(maxNum)) {
        // variables named as in xP_Next1
        xP_0 = xP_1 = xP_2 = -1; // positive lobe
        xP_3 = xP_4 = xP_5 = -1; // negative lobe
        // find the largest negative peak
        float yMin = 0;
        for(int nn = xMin+1; nn < xMax; nn++) {
            if(tSpek[nn] < yMin) {
                yMin = tSpek[nn];
                xP_4 = nn;
            }
        }
        if(xP_4 < 0)
            break;      // empty?
        // find the zero crossing at the left of the minimum
        for(int nn = xP_4; nn > xMin; nn--) {
            if(tSpek[nn] > 0) {
                xP_2 = nn;
                break;
            }
        }
        if(xP_2 < 0)
            break;
        xP_3 = xP_2 + 1;
        // find the end of the negative lobe
        for(int nn = xP_3; nn < xMax; nn++) {
            if(tSpek[nn] >= 0) {
                xP_5 = nn;
                break;
            }
        }
        if(xP_5 < 0)
            break;
        // find the beginning of the positive lobe, and the maximum
        float yMax = 0;
        for(int nn = xP_2; nn > xMin; nn--) {
            if(tSpek[nn] <= 0) {
                xP_0 = nn;
                break;
            }
            if(tSpek[nn] > yMax) {
                yMax = tSpek[nn];
                xP_1 = nn;
            }
        }
        if(xP_0 < 0)
            break;
        int widthP = xP_2-xP_0;
        int widthN = xP_5-xP_3;
        bool good = true;
        if(widthP < xP_nMinWidthP || widthN < xP_nMinWidthN)
            good = false;
        if(yMax < xP_minAmpl || yMin > -xP_minAmpl)
            good = false;
        float cmp = abs(yMax/yMin);
        if(cmp < 0.2f || cmp > 2.f)
            good = false;
        if(good) {
            // peak position is transition from positive to negative
            double xPos = 0.5f*(xP_2+xP_3);     // position of peak in tSpek coordinates
            vPeaks.push_back(xPos-xMin+chA+1);  //     ""            m_pSpek   ""
        }
        // zero the region
        for(int nn = xP_0; nn <xP_5; nn++)
            tSpek[nn] = 0;
    }

    delete [] tSpek;

    return vPeaks.size();
}

///******************************************************************************************///

// imported from TkT
// - smooth the spectrum with a gaussian kernel
// - differentiate ONCE
// Loop
//   look for the +++--- pattern with conditions on its shape and amplitude
//   test significance of peak before accepting it (nothing yet)
// 
int  CXRecalEnergy::PeakSearch1(float *m_pSpek, int chA, int chB, float fwhm, float minAmpl, std::vector<double>&vPeaks)
{
    //CheckLimits(chA, chB);
    int nChan = chB-chA+1;

    double  sigma = fwhm/s2fwhm;
    float  *tSpek = NULL;     // the smoothed and differentiated spectrum
    int     xMin  = 0;        // position of chA in tSpek
    SmoothSpek(m_pSpek+chA, nChan, sigma, 1, tSpek, xMin);
    int xMax = xMin + nChan;  // position of chB+1 in tSpek

    // ready to search peaks from xMin to xMax
    vPeaks.clear();

    // put here sensible values
    xP_minAmpl     = float(0.5*abs(minAmpl*exp(-.5)/sigma)); // max(g') = |g'(sigma)| = g(0)*exp(-0.5)/sigma
    xP_fThreshCFA  = 0;
    xP_nMinWidthP  = max(2, int(fwhm)-2);
    xP_nMinWidthN  = max(2, int(fwhm)-2);

    int xx = xMin;
    while(xx < xMax) {
        if( xP_Next1(tSpek+xx, nChan-xx) <= 0 )
            break;
        // peak position is transition from positive to negative
        double xPos = xx + 0.5f*(xP_2+xP_3);      // position of peak in tSpek coordinates
        vPeaks.push_back(xPos + chA + 1 - xMin);  //     ""            m_pSpek   ""
        xx += xP_5;
    }

#if 0
    //// Copy back the differentiated spectrum so that the caller can reconstruct
    //// the smoothed spectrum by one running integrations (SI).
    //// The reconstruction is exact only if the initial part of the original spectrum
    //// is zero for more than nKern channels, otherwise the passed-back part does
    //// not contain the whole information and this can cause offset-like effects.
    memcpy(m_pSpek+chA, tSpek+xMin, sizeof(float)*nChan);
    float scale = float(sigma/exp(-.5));
    for(int nn = chA; nn < chA+nChan; nn++)
        m_pSpek[nn] *= scale;
#endif

    delete [] tSpek;

    return vPeaks.size();
}

///******************************************************************************************///

// imported from TkT
// - smooth the spectrum with a gaussian kernel
// - differentiate TWICE
// Loop
//   look for the next mexican-hat pattern with conditions on its shape and amplitude
//   test significance of peak before accepting it (nothing yet)
// 
int CXRecalEnergy::PeakSearch2(float *m_pSpek, int chA, int chB, float fwhm, float minAmpl, std::vector<double>&vPeaks)
{
    //CheckLimits(chA, chB);
    int nChan = chB-chA+1;

    double  sigma = fwhm/s2fwhm;
    float  *tSpek = NULL;     // the smoothed and differentiated spectrum
    int     xMin  = 0;        // position of chA in tSpek
    SmoothSpek(m_pSpek+chA, nChan, sigma, 2, tSpek, xMin);
    int xMax = xMin + nChan;  // position of chB+1 in tSpek

    // ready to search peaks from xMin to xMax
    vPeaks.clear();

    // put here sensible values
    xP_minAmpl     = float(abs(minAmpl/(sigma*sigma))); // max(g'') =  |g''(0)| = |g(0)|/sigma**2
    xP_fThreshCFA  = 0;
    xP_nMinWidthN  = max(2, int(fwhm)-2);
    xP_nMinWidthP1 = max(2, xP_nMinWidthN-1);
    xP_nMinWidthP2 = max(2, (xP_nMinWidthN+1)/2);

    int xx = xMin;
    while(xx < xMax) {
        if( xP_Next2(tSpek+xx, xMax-xx) <= 0 )
            break;
        // first moments of the negative lobe to get the peak position
        float xP0 = 0, xP1 = 0;
        int nn0 = xx + xP_2 + 1;
        int nn1 = xx + xP_4 + 1;
        for(int nn = nn0; nn < nn1; nn++) {
            float x = float(nn-nn0)+0.5f;
            float y = tSpek[nn];
            xP0 += y;
            xP1 += y*x;
        }
        float xPos = nn0 + xP1/xP0;           // position of peak in tSpek coordinates
        vPeaks.push_back(xPos + chA - xMin);  //     ""            m_pSpek   ""
        xx += xP_4;
    }

#if 0
    //// Copy back the double-diff spectrum so that the caller can reconstruct
    //// the smoothed spectrum by two successive running integrations (SI).
    //// The reconstruction is exact only if the initial part of the original spectrum
    //// is zero for more than nKern channels, otherwise the passed-back part does
    //// not contain the whole information and this can cause offset-like effects.
    memcpy(m_pSpek+chA, tSpek+xMin, sizeof(float)*nChan);
    float scale = float(sigma*sigma);
    for(int nn = chA; nn < chA+nChan; nn++)
        m_pSpek[nn] *= scale;
#endif

    delete [] tSpek;

    return vPeaks.size();
}

///******************************************************************************************///

// generates a smoothed and differentiated version of the input data
// returns the  pointer and the position of the first valid channel
// the caller has to free tSpek
void CXRecalEnergy::SmoothSpek(float *spek, int nchan, double sigma, int ndiff, float *&tSpek, int &start)
{
    // generate the gaussian kernel for the smoothing operation
    int     nKern  = 2*int(5.5*sigma)+1;    // < 1ppm
    int     zKern  =  nKern/2;
    double  gfact1 = -1./(2.*sigma*sigma);
    double  gfact2 = 1./(sigma*sqrt(2.*m_pi));

    float  *gKern  = new float [nKern];
    for(int nn = 0; nn <= zKern; nn++) {
        double gvalue = gfact2*exp( ((nn-zKern)*(nn-zKern))*gfact1 );
        gKern[nn] = gKern[nKern-1-nn] = float(gvalue);
    }

    // the temporary space for the data
    int tLen = nKern + nchan + nKern;

    start = nKern;
    tSpek = new float [tLen];

    // copy the original spectrum to the temporary storage,
    // leaving nKern empty channels on both sides
    memset(tSpek, 0, sizeof(float)*nKern);
    memcpy(tSpek+nKern, spek, nchan*sizeof(float));
    memset(tSpek+nKern+nchan, 0, sizeof(float)*nKern);

    // convolution --> the resulting spectrum is right shifted by zKern channels
    for(int ii = tLen-1; ii >= nKern; ii--) {
        float csum = 0;
        for(int jj = 0; jj < nKern; jj++)
            csum += gKern[jj]*tSpek[ii-jj];
        tSpek[ii] = csum;
    }
    start += zKern;

    // differentiate as requested
    for(int nd = 0; nd < ndiff; nd++) {
        for(int ii = tLen-1; ii >0; ii--)
            tSpek[ii] -= tSpek[ii-1];
    }
    //start += ndiff/2;

    delete [] gKern;
}

///******************************************************************************************///

// trigger on a +++--- sequence, first derivative of a gaussian
// with conditions on the width and amplitude of the lobes
int CXRecalEnergy::xP_Next1(float *yDat, int yLen)
{
    enum {
        stateUnknown,   // initial or negative before starting a new sequence
        stateP,         // positive: xP_0(first positive chan),xP_1(max), xP_2(last positive)
        stateN         // negative: xP_3(first negative), xP_4(min), xP_5(last negative)
    };

    int    state = stateUnknown;            // start in an indefinite state
    int    widthP = 0, widthN = 0;
    float  yMax=0;
    float  yMin=0;

    for(int nn = 0; nn < yLen; nn++) {
        float yInp = yDat[nn];
        switch (state) {
        case stateUnknown:                  // was below CF threshold in an indefinite state
            if(yInp > xP_fThreshCFA) {        // and goes above
                state = stateP;                 // initiate a possible trigger sequence
                xP_0  = xP_1 = xP_2 = nn;
                yMax = yInp;
                widthP = 1;                     // start checking width
            }                                 // if it stays below there is nothing to do
            break;
        case stateP:                        // was above threshold
            if(yInp >= xP_fThreshCFA) {       // and stays above
                if(yInp > yMax) {
                    xP_1 = nn;                    // record maximum of first positive lobe
                    yMax = yInp;
                }
                xP_2 = nn;
                widthP++;                       // incr positive width and stay in this state
            }
            else {                            // transition to negative
                if(widthP >=  xP_nMinWidthP &&  // positive lobe wide enough
                    //widthP < 4*xP_nMinWidthP &&  // but not too wide
                    xP_1 > xP_0 &&               // and has a maximum
                    yMax > xP_minAmpl) {         // which is large enough
                    state = stateN;             // initiate a negative lobe sequence
                    xP_3  = xP_4 = xP_5 = nn;
                    yMin  = yInp;
                    widthN = 1;                // start counting width of negative lobe
                }
                else
                    state = stateUnknown;         // go back to indefinite below threshold
            }
            break;
        case stateN:                        // below threshold in a valid-sequence state
            if(yInp < xP_fThreshCFA) {        // and stays below
                if(yInp < yMin) {
                    xP_4 = nn;                    // record minimum
                    yMin = yInp;
                }
                xP_5 = nn;
                widthN++;                       // incr negative width and stay in this state
                if(widthN > xP_nMinWidthN &&    // second positive lobe wide enough for a valid sequence
                    yInp > yMin &&                // and signal is beyond minimum
                    yMin < -xP_minAmpl) {         // and minimum large enough
                    float cmp = abs(yMax/yMin);
                    if(cmp < 0.2f || cmp > 2.f)
                        state = stateUnknown;
                    else
                        return nn;                  // this is a good trigger
                }
                break;
            }
            else {                            // goes positive without having triggered
                state = stateP;                 // initiate a new possible trigger sequence
                xP_0 = xP_1 = xP_2 = nn;
                yMax = yInp;
                widthP = 1;                     // start checking width
                break;
            }
        }
    }
    return -1;
}

///******************************************************************************************///

// trigger on a +++---++++ sequence
// with conditions on the width and amplitude of the lobes
int CXRecalEnergy::xP_Next2(float *yDat, int yLen)
{
    enum {
        stateUnknown,   // initial or negative before starting a new sequence
        stateP1,        // positive: xP_0(first positive chan),xP_1(max), xP_2(last positive)
        stateN,         // negative: xP_3(min), xP_4(last negative)
        stateP2         // positive: xP_5(max), xP_6(last)
    };

    int    state = stateUnknown;            // start in an indefinite state
    int    widthP1 = 0, widthN = 0, widthP2 = 0;
    float  yMax1=0;
    float  yMin=0;
    float  yMax2=0;

    for(int nn = 0; nn < yLen; nn++) {
        float yInp = yDat[nn];
        switch (state) {
        case stateUnknown:                  // was below CF threshold in an indefinite state
            if(yInp > xP_fThreshCFA) {        // and goes above
                state = stateP1;                // initiate a possible trigger sequence
                xP_0  = xP_1 = xP_2 = nn;
                yMax1 = yMin = yInp;
                widthP1 = 1;                    // start checking width
            }                                 // if it stays below there is nothing to do
            break;
        case stateP1:                       // was above threshold
            if(yInp >= xP_fThreshCFA) {       // and stays above
                if(yInp > yMax1) {
                    xP_1 = nn;                    // record maximum of first positive lobe
                    yMax1 = yInp;
                }
                xP_2 = nn;
                widthP1++;                      // incr positive width and stay in this state
            }
            else {                            // transition to negative
                if(widthP1 >= xP_nMinWidthP1 && // positive lobe wide enough
                    xP_1 > xP_0) {              // and has a maximum
                    state = stateN;             // initiate a negative lobe sequence
                    xP_3  = xP_4 = nn;
                    yMin    = yInp;
                    widthN  = 1;                // start counting width of negative lobe
                }
                else
                    state = stateUnknown;         // go back to indefinite below threshold
            }
            break;
        case stateN:                        // below threshold in a valid-sequence state
            if(yInp < xP_fThreshCFA) {        // and stays below
                if(yInp < yMin) {
                    xP_3 = nn;                    // record minimum
                    yMin = yInp;
                }
                xP_4 = nn;
                widthN++;                       // incr positive width and stay in this state
            }
            else {                            // transition to positive
                if(widthN >= xP_nMinWidthN &&   // negative lobe wide enough
                    yMin < -xP_minAmpl &&         // its amplitude large enough
                    yMin < -1.1*yMax1 &&          // and minimum-to-maximum is acceptable
                    yMin > -4.5*yMax1) {          // (exact value is -0.5*exp(1.5)) = -2.2408)
                    state = stateP2;            // initiate the second positive lobe sequence
                    xP_5  = xP_6 = nn;
                    yMax2 = yInp;
                    widthP2 = 1;                // start counting width of second positive lobe
                }
                else {
                    state = stateP1;              // restart a new sequence
                    xP_0  = xP_1 = xP_2 = nn;
                    yMax1 = yMin =  yInp;
                    widthP1 = 1;                  // start checking width
                }
            }
            break;
        case stateP2:                       // above threshold in a valid-sequence state
            if(yInp > xP_fThreshCFA) {        // and is still above
                if(yInp > yMax2) {
                    xP_5  = nn;                   // record max of second positive lobe
                    yMax2 = yInp;
                }
                xP_6 = nn;
                widthP2++;                      // incr positive width and stay in this state
                if(widthP2 > xP_nMinWidthP2 &&  // second positive lobe wide enough for a valid sequence
                    yInp < yMax2) {              // and signal is beyond maximum
                    return nn;                 // this is a good trigger
                }
            }
            else {                            // goes positive (with or without trigger)
                state = stateUnknown;           // ready  to start a possible new trigger sequence
            }
            break;
        }
    }
    return -1;
}

///******************************************************************************************///

int CXRecalEnergy::FitPeaks(int verbose)
{
    if(specPeaks.size() < 1)
        return 0;

    // order peaks (needed to merge correctly close neighbours)
    sort( specPeaks.begin( ), specPeaks.end( ) );

    const float nwLe = 4.f; //  define left border
    const float nwOv = 2.f; //  check region overlap
    const float nwRi = 3.f; //  define right border

    Peaks.clear();

    // instead of specFWHMdef, we should use realistic width to define regions and overlaps

    int nmult = 0;
    int numpk = int(specPeaks.size());
    for(int nn = 0; nn < numpk; nn += 1 + nmult) {
        int chanA = int(specPeaks[nn]-nwLe*specFWHMdef);
        int chanB = int(specPeaks[nn]+nwRi*specFWHMdef);
        chanA = max(1,chanA);
        chanB = min(chanB, specLength-2);
        nmult = 0;
        if(nn < numpk-2) {
            // check if the next peaks are in the same region
            for(size_t jj = nn+1; jj < specPeaks.size(); jj++) {
                if(specPeaks[jj]-nwOv*specFWHMdef >= chanB)
                    break;
                nmult++;
                chanB = int(specPeaks[jj]+nwRi*specFWHMdef);
                chanB = min(chanB, specLength-2);
            }
        }

        int np = m_pCFit->CalcGaussianFit(specData, chanA, chanB, specPeaks);

        for(int jj = 0; jj < np; jj++) {
            Fitted res;

            res.NSubPeaks = np;
            res.BgFrom= m_pCFit->BgFrom();
            res.BgTo  = m_pCFit->BgTo();
            res.BgdOff= m_pCFit->Back0();
            res.BgdSlope= m_pCFit->Back1();

            //      cout<<"[ "<<res.BgFrom<<" ; "<<res.BgTo<<" ] ==> Bg = "<<res.BgdSlope<<"*x + "<<res.BgdOff<<endl;

            res.area   = m_pCFit->Area(jj);
            res.ampli  = m_pCFit->Amplitude(jj);
            res.posi   = m_pCFit->Position(jj);
            res.fw05   = m_pCFit->Fw05(jj);
            res.fw01   = m_pCFit->Fw01(jj);
            res.fwhm   = m_pCFit->Fwhm(jj);
            res.tailL  = m_pCFit->W01L(jj);
            res.tailR  = m_pCFit->W01R(jj);
            res.Lambda = m_pCFit->TailLeft(jj);
            res.Rho    = m_pCFit->TailRight(jj);
            res.S      = m_pCFit->Step(jj);
            res.errposi = m_pCFit->PositionErr(jj);

            bool bad = (res.area < 10) || (res.fwhm >= 5*specFWHMdef) || (res.tailR > 5.f*res.tailL);
            if(verbose > 0) {
                cout<<Form("#1   %4d %3d %9s %8.3f %8.3f %9.0f %9.2f %6.1f %7.0f %7.3f %7.3f", nn+jj, !bad, "", res.fw05, res.fw01, res.area, res.posi, res.fwhm, res.ampli, res.tailL, res.tailR)<<endl;
            }
            if(!bad) {
                Peaks.push_back(res);
            }
        }
    }
    return Peaks.size();
}

///******************************************************************************************///

// the energies should be already sorted in ascending order
// the peaks will be sorted in largest area order and the
// first Energies.size() of them will be connected to all energies, checking
// how many other peaks match the specific gain

Int_t CXRecalEnergy::EROOTCalibration()
{
    vector <Fitted>::iterator Iter;

    double bestSlope = 1.;
    Float_t Xmin = 0;
    Float_t Xmax = 0.;

    if(Energies.size() == 1) {
        // select the peak with the largest area
        int pn_max = 0;
        for(size_t nn = 1; nn < Peaks.size(); nn++) {
            if(Peaks[nn].area > Peaks[pn_max].area) {
                pn_max = nn;
            }
        }
        bestSlope  = Energies[0] / Peaks[pn_max].posi;
        Peaks[pn_max].good = true;
        Peaks[pn_max].erefindex = 0;
        Xmin = Peaks[pn_max].posi-100;
        Xmax = Peaks[pn_max].posi+100;
    }
    else if( Peaks.size() < 2 || Energies.size() < 2){
        if(Verbosity> 1) cout << "#   Number of peaks (" << Peaks.size() << ") or number of energies (" << Energies.size() << ")  too small" << endl;
        return 0;
    }
    else {

        // only the peak with largest area used as pivot
        sort( Peaks.begin( ), Peaks.end( ), largerAmplitude() );
        int npmax = min( min(Peaks.size(), size_t(4)), Energies.size() );  // limit the number of peaks to min(np,ne,4)
        for(int np = 0; np < npmax; np++)
            Peaks[np].good = true;
        for(size_t np = npmax; np < Peaks.size(); np++)
            Peaks[np].good = false;

        // (re)order the peaks
        sort( Peaks.begin( ), Peaks.end( ), smallerPosi() );
        if(Verbosity> 2) {
            cout<<Form("#  Sorted Peak Positions = (");
            for ( Iter = Peaks.begin( ) ; Iter != Peaks.end( ) ; Iter++ ) cout<<Form( " %d", int((*Iter).posi) );
            cout<<Form(" )\n");
        }

        // Connect every (large) peak with every energy and check how many other (peak,energy) pairs agree with this slope
        // The combination with the largest number of agreeing pairs provides the reference gain for the final average
        // Equal number of matches are ordered by chi2
        int    np_max = 0;      // number of matches
        int    np_ind = 0;      // reference peak
        int    np_ref = 0;      // reference energy
        double np_chi = 1.e40;  // to distinguish among equal matchers
        double dpos2max = specFWHMdef*specFWHMdef;
        for(size_t np = 0; np < Peaks.size(); np++) {
            if(!Peaks[np].good)
                continue;
            int    ec_max = 0;      // number of matches for peak np
            int    ec_ref = 0;      // reference energy
            double ec_chi = 1.e30;  // its chi2
            for(size_t ne = 0; ne < Energies.size(); ne++) {            // connecting peak np with energy ne
                double lgain = (Peaks[np].posi)/Energies[ne];             // gives this gain
                int match = 0;
                double chi2 = 0;
                for(size_t nee = 0; nee < Energies.size(); nee++) {       // count the number of matches for this combination
                    double epos = Energies[nee]*lgain;                      // expected position
                    for(size_t ip = 0; ip < Peaks.size(); ip++) {
                        double dpos  = Peaks[ip].posi-epos;
                        double dpos2 = dpos*dpos;
                        if(dpos2 < dpos2max) {
                            match++;
                            chi2 += dpos2;
                            break;
                        }
                    }
                }
                if( (match > ec_max)  ||                        // more matches
                    ((match == ec_max) && (chi2 < ec_chi)) ) {  // or better chi2
                    ec_max = match;       // record the best combination so far
                    ec_ref = ne;
                    ec_chi = chi2;
                }
            }
            if( (ec_max > np_max) ||                           // more matches
                ((ec_max == np_max) && (ec_chi < np_chi)) ) {  // or better chi2
                np_max = ec_max;       // record the best combination so far
                np_ind = np;
                np_ref = ec_ref;
                np_chi = ec_chi;
            }
            //if(np_max == Energies.size())
            //  break;    // cannot do better than this ?
        }

        bestSlope = Energies[np_ref]/(Peaks[np_ind].posi);
        if(Verbosity > 2) cout<<Form("#  Best-match slope %g [p=%d e=%d] with %d values\n", bestSlope, np_ind, np_ref, np_max);

        // find the good peaks
        for(size_t np = 0; np < Peaks.size(); np++) {
            Peaks[np].erefindex = -1;
            Peaks[np].eref = 0.;
            Peaks[np].good = false;
        }

        int match = 0;
        for(size_t ne = 0; ne < Energies.size(); ne++) {
            double epos = Energies[ne]/bestSlope;
            for(size_t np = 0; np < Peaks.size(); np++) {
                if(abs(Peaks[np].posi-epos) < specFWHMdef && !Peaks[np].good) {
                    match++;
                    Peaks[np].erefindex = ne;
                    Peaks[np].eref = Energies[ne];
                    Peaks[np].good = true;
                    //                Peaks[np].errposi = pow(Peaks[np].fwhm/s2fwhm, 2.)/Peaks[np].area;

                    if(Xmin==0. || Peaks[np].posi < Xmin) Xmin = Peaks[np].posi;
                    if(Peaks[np].posi > Xmax) Xmax = Peaks[np].posi;
                    break;
                }
            }
        }
        Xmin-=100;
        Xmax+=100;
    }

    if(fCalibFunction) delete fCalibFunction;
    if(fCalibGraph) delete fCalibGraph;
    if(fResidueGraph) delete fResidueGraph;

    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Migrad");

    fCalibFunction = new TF1("ECalibration", this, &CXRecalEnergy::PolynomialFunc, Xmin, Xmax, fCalibOrder+1, "CXRecalEnergy", "PolynomialFunc");
    fCalibFunction->SetLineColor(kBlue);
    fCalibFunction->SetNpx(1000);
    for(int i=0 ; i<=fCalibOrder ; i++) fCalibFunction->SetParName(i,Form("a%d",i));
    fCalibFunction->SetParameter(0,0.);
    if(fNoOffset) fCalibFunction->FixParameter(0,0);
    fCalibFunction->SetParameter(1,bestSlope);
    for(int i=2 ; i<=fCalibOrder ; i++) fCalibFunction->SetParameter(i,0);

    fCalibGraph = new TGraphErrors;
    fCalibGraph->SetName("CalibrationGraph");
    fCalibGraph->SetMarkerColor(kRed);
    fCalibGraph->SetMarkerStyle(20);
    fCalibGraph->GetXaxis()->SetTitle("Energy (channels)");
    fCalibGraph->GetYaxis()->SetTitle("Energy (keV)");

    fResidueGraph = new TGraphErrors;
    fResidueGraph->SetName("Residue");
    fResidueGraph->SetMarkerColor(kRed);
    fResidueGraph->SetMarkerStyle(20);
    fResidueGraph->GetXaxis()->SetTitle("Energy (keV)");
    fResidueGraph->GetYaxis()->SetTitle("Residue (keV)");

    // fit slope using the good peaks, starting without errors
    for(size_t np = 0; np < Peaks.size(); np++) {
        if(Peaks[np].good) {
            fCalibGraph->SetPoint(fCalibGraph->GetN(), Peaks[np].posi, Energies[Peaks[np].erefindex]);
            //            fCalibGraph->SetPointError(fCalibGraph->GetN()-1, Peaks[np].errposi, 0.);
        }
    }

    cout<<scientific<<setprecision(5);

    for(int i=1; i<=fCalibOrder ; i++) {
        for(int j=1; j<i ; j++) {
            if(fCalibFunction->GetParameter(j)>0) fCalibFunction->SetParLimits(j,fCalibFunction->GetParameter(j)*0.2,fCalibFunction->GetParameter(j)*5);
            else fCalibFunction->SetParLimits(j,fCalibFunction->GetParameter(j)*5,fCalibFunction->GetParameter(j)*0.2);
        }
        fCalibFunction->SetParLimits(i,-TMath::Abs(fCalibFunction->GetParameter(i-1)),TMath::Abs(fCalibFunction->GetParameter(i-1)));
        for(int j=i+1; j<=fCalibOrder ; j++) {
            fCalibFunction->FixParameter(j,0.);
        }
        fCalibGraph->Fit(fCalibFunction,"Q0R");
    }

    TFitResultPtr r = fCalibGraph->Fit(fCalibFunction,"SRQ");

    if(Verbosity>1) r->Print();

    r->GetConfidenceIntervals();

    cout<<right<<fixed;

    if(Verbosity > 1) cout<<"#2" << setw(12) << "Area" << setw(12) << "E (Ch)" << setw(12) << "FWHM (Ch)" << setw(12) << "FWHM (keV)" << setw(13) << "energy" << setw(15) << "Residue (keV)"<<endl;

    int ngood=0;
    for(size_t np = 0; np < Peaks.size(); np++) {
        if(Peaks[np].good) {
            double ecal = fCalibFunction->Eval(Peaks[np].posi);
            double fwhm_e = fCalibFunction->Eval(Peaks[np].fwhm)-fCalibFunction->GetParameter(0);
            double res = ecal-Energies[Peaks[np].erefindex];
            fResidueGraph->SetPoint(fResidueGraph->GetN(),Energies[Peaks[np].erefindex],res);
            //            fResidueGraph->SetPointError(fResidueGraph->GetN()-1,0.,r->GetConfidenceIntervals()[fResidueGraph->GetN()-1]);
            if(Verbosity > 1) cout<<"#2" << setw(12) << setprecision(1) << Peaks[np].area << setw(12) << setprecision(2) << Peaks[np].posi << setw(12) << setprecision(3) << Peaks[np].fwhm << setw(12) << setprecision(3) << fwhm_e << setw(13) << setprecision(3) << ecal << setw(15) << setprecision(3) << res <<endl;
            ngood++;
        }
    }

    if(r->Status()>0) cout<<"Warning: Fit failed"<<endl;

    return ngood;
}

///******************************************************************************************///

Double_t CXRecalEnergy::PolynomialFunc(Double_t*xx,Double_t*pp)
{
    Double_t value=0;
    for(int i=0 ; i<=fCalibOrder ; i++) {
        value += pp[i]*TMath::Power(xx[0],i);
    }
    return value;
}

///******************************************************************************************///

double CXRecalEnergy::TCalibration()
{
    return 0;
}

///******************************************************************************************///

bool CXRecalEnergy::InvertMatrix3(const double m[9], double invOut[9])
{
    double d00 = m[0];
    double d01 = m[1];
    double d02 = m[2];
    double d10 = m[3];
    double d11 = m[4];
    double d12 = m[5];
    double d20 = m[6];
    double d21 = m[7];
    double d22 = m[8];

    double Det =  d00*(d11*d22 - d12*d21) + d01*(d12*d20 - d10*d22) + d02*(d10*d21 - d11*d20);
    if (Det == 0)
        return false;

    double D00 =  d11*d22 - d12*d21; double D01 =  d12*d20 - d10*d22; double D02 =  d10*d21 - d11*d20;
    double D10 =  d21*d02 - d22*d01; double D11 =  d22*d00 - d20*d02; double D12 =  d20*d01 - d21*d00;
    double D20 =  d01*d12 - d02*d11; double D21 =  d02*d10 - d00*d12; double D22 =  d00*d11 - d01*d10;

    Det = 1/Det;

    invOut[0] = D00 * Det;
    invOut[1] = D01 * Det;
    invOut[2] = D02 * Det;
    invOut[3] = D10 * Det;
    invOut[4] = D11 * Det;
    invOut[5] = D12 * Det;
    invOut[6] = D20 * Det;
    invOut[7] = D21 * Det;
    invOut[8] = D22 * Det;

    return true;
}

///******************************************************************************************///

double CXRecalEnergy::Calibrated(double x)
{
    if(fCalibOrder==0) return tshift  + x;
    else if(fCalibFunction) return fCalibFunction->Eval(x);
    else return x;
}

///******************************************************************************************///

void CXRecalEnergy::SetDataFromHistTH1(TH1 *hist, Int_t Id)
{
    if(specData) delete [] specData;
    specLength = (Int_t) hist->GetXaxis()->GetXmax();
    specData = new float[specLength];
    for(int i=0 ; i<specLength ; i++) {
        if(i<hist->GetXaxis()->GetXmin()) specData[i] = 0.;
        else specData[i] = hist->GetBinContent(hist->FindBin(i));
    }
    specName = hist->GetName();

    fId =Id;
}

// ROOT dictionnary
ClassImp(CXRecalEnergy);

