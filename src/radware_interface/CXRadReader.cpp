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

#include "CXRadReader.h"

#include <sys/stat.h>

#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TSystem.h"
#include "TObjArray.h"

#include "CXBashColor.h"
#include "CXProgressBar.h"

CXRadReader::CXRadReader()
{
    xxgd.numchs = 1;
    xxgd.many_cubes = 0;
    xxgd.cubefact[0] = xxgd.cubefact[1] = xxgd.cubefact[2] = xxgd.cubefact[3] = xxgd.cubefact[4] = 0.0;
    setcubenum(0);
}

CXRadReader::~CXRadReader()
{
    delete hLUTHist;
    delete hTotalProj;
    delete hBackground;
    delete h2DProj;
    delete fECalibFunc;
    delete fEffFunc;
}

Int_t CXRadReader::ReadCube(const TString &FileName)
{
    gbash_color->InfoMessage("Reading Cube file: " + FileName);

    xxgd.cubnam = FileName;
    if (open3dfile(xxgd.cubnam, &xxgd.numchs)) {
        gbash_color->ErrorMessage(Form("%s not readable", xxgd.cubnam.Data()));
        return 1;
    }
    if (xxgd.numchs > MAXCHS){
        gbash_color->ErrorMessage(Form("Cube file has too many channels (%d), MAXCHS is %d  ==> EXIT", xxgd.numchs ,MAXCHS));
        return 1;
    }

    gbash_color->InfoMessage(Form("%s loaded (%d channels)",xxgd.cubnam.Data(), xxgd.numchs));

    fCubeFileName = FileName;
    return 0;
}

Int_t CXRadReader::ReadTabFile(const TString &FileName)
{
    if(!gSystem->IsFileInIncludePath(FileName)){
        gbash_color->WarningMessage("No LUT file detected, default values assumed (1keV/bin)");
        xxgd.nclook  = xxgd.numchs;
        xxgd.lookmin = 1;
        xxgd.lookmax = xxgd.numchs;
        for(short i=0 ; i<16384 ; i++) xxgd.looktab[i] = i+1;
    }
    else {
        gbash_color->InfoMessage("Reading ADC-to-Cube lookup table file: " + FileName);

        if (read_tab_file(FileName, &xxgd.nclook, &xxgd.lookmin, &xxgd.lookmax, xxgd.looktab, 16384)) {
            gbash_color->ErrorMessage(FileName + " not readable");
            return 1;
        }
        if (xxgd.lookmax != xxgd.numchs) {
            gbash_color->ErrorMessage(Form("No. of values in lookup table (%d) is not coherent with cube dimension (%d)",xxgd.lookmax, xxgd.numchs));
            return 1;
        }
    }

    gbash_color->InfoMessage(Form("NCLook, LookMin, LookMax = %d, %d, %d",xxgd.nclook, xxgd.lookmin, xxgd.lookmax));

    fLUTFileName = FileName;
    return 0;
}

Int_t CXRadReader::ReadTotProj(const TString &FileName)
{
    // xxgd.bspec[0] ==> Total proj

    if(!gSystem->IsFileInIncludePath(FileName)) {
        gbash_color->WarningMessage("No Total projection file detected, a default one will be built from the Cube");

        if(AutoBuildProj(fCubeFileName,1) == 0) {
            ReadTotProj(fCubeFileName.Copy().ReplaceAll(".cub",".spe"));
        }
        else {
            gbash_color->ErrorMessage("Error occured while building the total projection");
            return 1;
        }
    }
    else {

        gbash_color->InfoMessage("Reading Total projection file: " + FileName);

        int   nch;
        TString  filnam, namesp;
        xxgd.le2pro2d = 0;

        if (read_spe_file(FileName, &xxgd.bspec[0][0], namesp, &nch, MAXCHS)) {
            gbash_color->ErrorMessage(Form("%s not readable ==> EXIT",filnam.Data()));
            return 1;
        }
        if (nch != xxgd.numchs){
            gbash_color->ErrorMessage(Form("%s spectrum lenght (%d) has wrong length ( must be %d) ==> EXIT",filnam.Data(), nch, xxgd.numchs));
            return 1;
        }

        gbash_color->InfoMessage(Form("Total projection file loaded: %s",filnam.Data()));
    }

    fTotalProjFileName = FileName;
    return 0;
}

TH1 *CXRadReader::GetHistFromSpeFile(const TString &filename)
{
    if(!gSystem->IsFileInIncludePath(filename)) {
        gbash_color->WarningMessage("File " + filename + " not found ");
        return nullptr;
    }

    gbash_color->InfoMessage("Exporting radware 1D spectrum: " + filename + " in ROOT histogram");

    TObjArray *arr = filename.Tokenize("/");
    TString name = arr->Last()->GetName();
    delete arr;

    float spec[MAXCHS];
    TString name_spectrum;
    int nch;

    if (read_spe_file(filename, &spec[0], name_spectrum, &nch, MAXCHS)) {
        gbash_color->ErrorMessage(Form("%s not readable ==> EXIT",filename.Data()));
        return nullptr;
    }

    TH1F *h = new TH1F(name,name,nch,0,nch);
    for(int i=0 ; i<nch ; i++) {
        h->SetBinContent(i+1,spec[i]);
    }

    return h;
}

TH2 *CXRadReader::GetHistFrom2dpFile(const TString &filename)
{
    if(!gSystem->IsFileInIncludePath(filename)) {
        gbash_color->WarningMessage("File " + filename + " not found ");
        return nullptr;
    }

    gbash_color->InfoMessage("Exporting radware 2D spectrum: " + filename + " in ROOT histogram");

    TObjArray *arr = filename.Tokenize("/");
    TString name = arr->Last()->GetName();
    delete arr;

    int   reclen, ich;
    char  filnam[800];

    strlcpy(filnam,filename.Data(),800);
    if (!inq_file(filnam, &reclen)) {
        gbash_color->ErrorMessage(Form("File %s does not exist",filnam));
        return nullptr;
    }
    FILE *file = fopen(filnam, "r");
    int histsize = reclen;

    int   luch[MAXCHS];

    luch[0] = 0;
    for (int iy = 1; iy < histsize; ++iy) {
        luch[iy] = luch[iy-1] + histsize - iy;
    }
    reclen *= 4;
    for (int iy = 0; iy < histsize; ++iy) {
        fseek(file, iy*reclen, SEEK_SET);
        fread(xxgd.tmpspec, reclen, 1, file);
        for (int ix = 0; ix <= iy; ++ix) {
            ich = luch[ix] + iy;
            xxgd.pro2d_temp[ich] = xxgd.tmpspec[ix];
        }
    }
    fclose(file);

    TH2F *h = new TH2F(name,name,histsize,0,histsize,histsize,0,histsize);
    for (int iy = 0; iy < histsize; ++iy) {
        for (int ix = 0; ix <= iy; ++ix) {
            int ich = luch[ix] + iy;
            h->SetBinContent(ix+1,iy+1,xxgd.pro2d_temp[ich]);
            h->SetBinContent(iy+1,ix+1,xxgd.pro2d_temp[ich]);
        }
    }

    return h;
}

void CXRadReader::SaveBackground(const TString &filename)
{
    TString name = filename;
    wspec(name, xxgd.background, xxgd.numchs);
}

Int_t CXRadReader::Read2DProj(const TString &FileName)
{
    if(!gSystem->IsFileInIncludePath(FileName)) {
        gbash_color->WarningMessage("No 2D projection file detected, a default one will be built from the Cube");

        if(AutoBuildProj(fCubeFileName,2) == 0) {
            Read2DProj(fCubeFileName.Copy().ReplaceAll(".cub",".2dp"));
        }
        else {
            gbash_color->ErrorMessage("Error occured while building the 2D projection");
            return 1;
        }
    }
    else {

        int   reclen, ich;
        char  filnam[800];

        strlcpy(filnam,FileName.Data(),800);
        if (!inq_file(filnam, &reclen)) {
            gbash_color->ErrorMessage(Form("File %s does not exist",filnam));
            return 1;
        }
        FILE *file = fopen(filnam, "r");

        xxgd.luch[0] = 0;
        for (int iy = 1; iy < xxgd.numchs; ++iy) {
            xxgd.luch[iy] = xxgd.luch[iy-1] + xxgd.numchs - iy;
        }
        xxgd.matchs = xxgd.luch[xxgd.numchs - 1] + xxgd.numchs;

        reclen *= 4;
        for (int iy = 0; iy < xxgd.numchs; ++iy) {
            fseek(file, iy*reclen, SEEK_SET);
            fread(xxgd.spec[0], reclen, 1, file);
            for (int ix = 0; ix <= iy; ++ix) {
                ich = xxgd.luch[ix] + iy;
                xxgd.pro2d[ich] = xxgd.spec[0][ix];
            }
        }
        fclose(file);

        gbash_color->InfoMessage("2D projection file loaded: " + FileName);

        f2DProjFileName = FileName;

    }

    return 0;
}

Int_t CXRadReader::ReadGG(TH2 *h)
{
    if (h == nullptr) {
        gbash_color->ErrorMessage("input is null");
        return 1;
    }

    xxgd.numchs = h->GetNbinsX();

    if(h->GetNbinsX() != h->GetNbinsY()) {
        gbash_color->ErrorMessage(Form("NBinsX(%d) and NBinsY(%d) are different ! IGNORED",h->GetNbinsX(), h->GetNbinsY()));
    }

    //Init
    xxgd.luch[0] = 0;
    for (int iy = 1; iy < xxgd.numchs; ++iy) {
        xxgd.luch[iy] = xxgd.luch[iy-1] + xxgd.numchs - iy;
    }
    xxgd.matchs = xxgd.luch[xxgd.numchs - 1] + xxgd.numchs;

    for (int iy = 0; iy < xxgd.numchs; ++iy) {
        for (auto & ii : xxgd.bspec) {
            ii[iy] = 0;
        }
    }

    // LUT
    for (int i= 0; i < xxgd.numchs; ++i) {
        xxgd.elo_sp[i] = h->GetXaxis()->GetBinLowEdge(i+1);
        xxgd.ehi_sp[i] = h->GetXaxis()->GetBinUpEdge(i+1);

        float eg = (xxgd.elo_sp[i] + xxgd.ehi_sp[i]) / 2.0F;
        xxgd.eff_sp[i] = 1;
        xxgd.energy_sp[i] = eg;
        xxgd.ewid_sp[i] = xxgd.ehi_sp[i] - xxgd.elo_sp[i];
    }

    // 2D Proj
    memset(xxgd.pro2d,0,sizeof(xxgd.pro2d));

    // Fill 2D histos
    for (int iy = 0; iy < xxgd.numchs; ++iy) {
        for (int ix = 0; ix <= iy; ++ix) {
            int ich = xxgd.luch[ix] + iy;
            xxgd.pro2d[ich] = h->GetBinContent(ix+1,iy+1) * ((xxgd.ewid_sp[iy]*xxgd.ewid_sp[ix]) / (xxgd.ewid_sp[0]*xxgd.ewid_sp[0]));
        }
    }

    // Total Proj

    delete hTotalProj;
    hTotalProj = h->ProjectionX();
    hTotalProj->Reset();
    hTotalProj->SetNameTitle("TotalProj","TotalProj");

    for (int y=0; y<xxgd.numchs; y++) {
        for (int x=0; x<=y; x++) {
            int ich = xxgd.luch[x] + y;
            xxgd.bspec[0][y] += xxgd.pro2d[ich];
            xxgd.bspec[0][x] += xxgd.pro2d[ich];
        }
    }

    for (int ich = 0; ich < xxgd.numchs; ++ich) {
        hTotalProj->SetBinContent(ich+1,xxgd.bspec[0][ich] / (xxgd.ewid_sp[ich]/xxgd.ewid_sp[0]));
    }

    // Background

    delete hBackground;
    hBackground = dynamic_cast<TH1*>(hTotalProj->Clone());
    hBackground->Reset();
    hBackground->SetNameTitle("Background","Background");

    autobkgnd();

    for (int i = 0; i < xxgd.numchs; ++i){
        hBackground->SetBinContent(i+1,xxgd.background[i] / (xxgd.ewid_sp[i]/xxgd.ewid_sp[0]));
    }

    // Init gates spectra

    fGatedSpectrum = dynamic_cast<TH1*>(hTotalProj->Clone());
    fGatedSpectrum->Reset();
    fGatedSpectrum->SetNameTitle("GatedSpectrum","GatedSpectrum");
    fGatedSpectrum->SetXTitle("Energy (keV)");
    fGatedSpectrum->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrum->GetXaxis()->SetTitleOffset(1.2);
    fGatedSpectrum->GetYaxis()->SetTitleOffset(0.8);
    fGatedSpectrum->SetStats();

    fGatedSpectrumNoBG = dynamic_cast<TH1*>(hTotalProj->Clone());
    fGatedSpectrumNoBG->Reset();
    fGatedSpectrumNoBG->SetNameTitle("GatedSpectrumBG","GatedSpectrumBG");
    fGatedSpectrumNoBG->SetXTitle("Energy (keV)");
    fGatedSpectrumNoBG->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrum->GetXaxis()->SetTitleOffset(1.2);
    fGatedSpectrum->GetYaxis()->SetTitleOffset(0.8);
    fGatedSpectrum->SetStats();

    gbash_color->InfoMessage(Form("2D matrix file loaded: %s %s",h->GetName(), h->GetTitle()));

    return 0;
}

void CXRadReader::ReEvalBackground(float Xmin, float XMax, float LenghtFactor, float Width, float FWHM_0, float FWHM_1, float FWHM_2)
{
    if(hBackground == nullptr) {
        gbash_color->ErrorMessage("No existing background");
        return;
    }

    fBGLenghtFactor = LenghtFactor;
    fDefaultWidth = Width;
    fFWHMPars[0] = FWHM_0;
    fFWHMPars[1] = FWHM_1;
    fFWHMPars[2] = FWHM_2;

    hBackground->Reset();

    autobkgnd(Xmin, XMax);

    for (int i = 0; i < xxgd.numchs; ++i){
        hBackground->SetBinContent(i+1,xxgd.background[i] / (xxgd.ewid_sp[i]/xxgd.ewid_sp[0]));
    }
}


Int_t CXRadReader::ReadBackground(const TString &FileName)
{
    // xxgd.bspec[1] ==> Background

    if(!gSystem->IsFileInIncludePath(FileName)) {
        gbash_color->WarningMessage("No Background file detected, a default one will be built from the total projection");
        autobkgnd();
    }
    else {

        gbash_color->InfoMessage("Reading Total projection file: " + FileName);

        float fsum1;
        float bspec2[4][MAXCHS];
        int   nch;
        TString  filnam, namesp;
        xxgd.le2pro2d = 0;

        if (read_spe_file(FileName, &xxgd.bspec[2][0], namesp, &nch, MAXCHS)) {
            gbash_color->ErrorMessage(Form("%s not readable ==> EXIT",FileName.Data()));
            return 1;
        }
        if (nch != xxgd.numchs){
            gbash_color->ErrorMessage(Form("%s spectrum lenght (%d) has wrong length ( must be %d) ==> EXIT",FileName.Data(),nch,xxgd.numchs));
            return 1;
        }

        fsum1 = 0.0;
        for (int i = 0; i < xxgd.numchs; ++i) {
            fsum1 += xxgd.bspec[0][i];
            bspec2[0][i] = xxgd.bspec[0][i] - xxgd.bspec[2][i];
        }
        for (int j = 0; j < xxgd.numchs; ++j) {
            xxgd.background[j] = xxgd.bspec[2][j];
            bspec2[1][j] = bspec2[2][j] = bspec2[3][j] = 0.0;
            xxgd.bspec[2][j] /= fsum1;
        }

        for (int i = 0; i < xxgd.numchs; ++i) {
            xxgd.bspec[1][i] = xxgd.bspec[2][i];
            xxgd.bspec[2][i] = bspec2[0][i];
            xxgd.bspec[3][i] = bspec2[1][i];
            xxgd.bspec[4][i] = bspec2[2][i];
            xxgd.bspec[5][i] = bspec2[3][i];
        }

        gbash_color->InfoMessage(Form("Background file loaded: %s",FileName.Data()));

        fBackgroundFileName =FileName;
    }

    return 0;
}

Int_t CXRadReader::ReadCalibs(const TString &EnergyFileName, const TString &EfficiencyFileName, float CompFactor)
{
    float f1, f2, x1, x2, x3;
    float  x, eg, eff;
    int    i, j, jj, iorder, nterms;
    char   title[800];
    TString filnam;

    /* get energy calibration */

    bool readcal = false;
    if(gSystem->IsFileInIncludePath(EnergyFileName)){
        gbash_color->InfoMessage("Calibration file: " + EnergyFileName  + " found");
        readcal = true;
    }
    else
        gbash_color->InfoMessage("No Calibration file set or found, no energy calibration applied");

    if(readcal){
        filnam = EnergyFileName;
        if (read_cal_file(filnam, title, &iorder, fGains)) {
            gbash_color->ErrorMessage(EnergyFileName + " not readable");
            return 1;
        }
    }
    else {
        iorder = 1;
        nterms = iorder + 1;
        memset(fGains,0,sizeof(fGains));
        fGains[1] = 1.;
    }

    fCompFactor = CompFactor;
    gbash_color->InfoMessage(Form("Contraction factor: %f",fCompFactor));

    /* get efficiency calibration */

    readcal = false;
    if(gSystem->IsFileInIncludePath(EfficiencyFileName)){
        gbash_color->InfoMessage("Efficiency file: " + EfficiencyFileName  + " found");
        readcal = true;
    }
    else
        gbash_color->InfoMessage("No Efficiency file set or found, no efficiency value applied");

    if(readcal){
        filnam = EfficiencyFileName;
        if (read_eff_file(filnam, title, feff_pars)) {
            gbash_color->ErrorMessage(EfficiencyFileName + " not readable ==> EXIT");
            return 1;
        }
    }
    else {
        memset(feff_pars,0,sizeof(feff_pars));
        feff_pars[7] = 100;
        feff_pars[8] = 1000;
    }

    /* calculate energy and efficiency spectra */
    jj = 0;

    for (i = 0; i < xxgd.nclook; ++i) {
        if (xxgd.looktab[i] != jj) {
//            x = ((float) i - 0.5f);
            x = ((float) i);
            eg = fGains[nterms - 1];
            for (j = nterms - 1; j >= 1; --j) {
                eg = fGains[j - 1] + eg * x;
            }
            if (jj != 0) xxgd.ehi_sp[jj - 1] = eg / fCompFactor;
            jj = xxgd.looktab[i];
            if (jj != 0) xxgd.elo_sp[jj - 1] = eg / fCompFactor;
        }
    }

    if (xxgd.looktab[xxgd.nclook - 1] != 0) {
//        x = ((float) xxgd.nclook - 1.0);
        x = ((float) xxgd.nclook);
        eg = fGains[nterms - 1];
        for (j = nterms - 1; j >= 1; --j) {
            eg = fGains[j - 1] + eg * x;
        }
        xxgd.ehi_sp[xxgd.looktab[xxgd.nclook - 1] - 1] = eg / fCompFactor;
    }

    for (i = 0; i < xxgd.numchs; ++i) {
        eg = (xxgd.elo_sp[i] + xxgd.ehi_sp[i]) / 2.0;
        x1 = log(eg / feff_pars[7]);
        x2 = log(eg / feff_pars[8]);
        f1 = feff_pars[0] + feff_pars[1] * x1 + feff_pars[2] * x1 * x1;
        f2 = feff_pars[3] + feff_pars[4] * x2 + feff_pars[5] * x2 * x2;
        if (f1 <= 0. || f2 <= 0.) {
            eff = 1.0;
        } else {
            x3 = exp(-feff_pars[6] * log(f1)) + exp(-feff_pars[6] * log(f2));
            if (x3 <= 0.) {
                eff = 1.0;
            } else {
                eff = exp(exp(-log(x3) / feff_pars[6]));
            }
        }
        xxgd.eff_sp[i] = eff;
        xxgd.energy_sp[i] = eg;

        xxgd.ewid_sp[i] = xxgd.ehi_sp[i] - xxgd.elo_sp[i];
    }

    // define edges for non constant binning
    for(int i=0 ; i<xxgd.numchs ; i++) {
        edges[i] = xxgd.elo_sp[i];
    }
    edges[xxgd.numchs] = xxgd.ehi_sp[xxgd.numchs-1];

    fECalFileName = EnergyFileName;
    fEffFileName = EfficiencyFileName;
    return 0;
}

void CXRadReader::BuildHistos()
{
    gErrorIgnoreLevel = kError;

    // LUT
    delete hLUTHist;
    hLUTHist = new TH1D("LUT","LUT",xxgd.numchs,edges);
    hLUTHist->SetXTitle("Energy (keV)");
    hLUTHist->SetYTitle("BinWidth (keV)");

    // Total Proj
    delete hTotalProj;
    hTotalProj = new TH1D("TotalProj","TotalProj",xxgd.numchs,edges);
    hTotalProj->SetXTitle("Energy (keV)");
    hTotalProj->SetYTitle(Form("Counts (%g keV/bin)",hTotalProj->GetBinWidth(1)));

    // Background
    delete hBackground;
    hBackground = new TH1D("Background","Background",xxgd.numchs,edges);
    hBackground->SetXTitle("Energy (keV)");
    hBackground->SetYTitle(Form("Counts (%g keV/bin)",hBackground->GetBinWidth(1)));

    // 2DProj

    delete h2DProj;
    h2DProj = new TH2D("2DProj","2DProj",xxgd.numchs,edges,xxgd.numchs,edges);
    h2DProj->SetXTitle("E#gamma_{1}");
    h2DProj->SetYTitle("E#gamma_{2}");

    // Calib functions

    delete fECalibFunc;
    fECalibFunc = new TF1("ECalib","pol5",0,xxgd.nclook);
    fECalibFunc->SetParameters(fGains);
    fECalibFunc->GetXaxis()->SetTitle("Energy (keV)");
    fECalibFunc->SetNpx(1000);

    delete fEffFunc;
    fEffFunc = new TF1("EfficiencyFunc",
        [this](double* x, double* p) -> double {
            return this->EffFuncFormula(x, p);
        },
        0, xxgd.nclook, 10);
    fEffFunc->SetParameters(feff_pars);
    //    fEffFunc->SetParameters(7.04,0.7,0.,5.273,-0.863,0.01,11,100.,1000.); //default from radware's web site

    fEffFunc->GetXaxis()->SetTitle("Energy (keV)");
    fEffFunc->SetNpx(1000);

    // Fill 1D histos
    for (int i = 0; i < xxgd.numchs; ++i){
        hLUTHist->SetBinContent(i+1,xxgd.ewid_sp[i]);
        hTotalProj->SetBinContent(i+1,xxgd.bspec[0][i] / (xxgd.ewid_sp[i]/xxgd.ewid_sp[0]));
        hBackground->SetBinContent(i+1,xxgd.background[i] / (xxgd.ewid_sp[i]/xxgd.ewid_sp[0]));
    }

    // Fill 2D histos
    for (int iy = 0; iy < xxgd.numchs; ++iy) {
        for (int ix = 0; ix <= iy; ++ix) {
            int ich = xxgd.luch[ix] + iy;
            h2DProj->SetBinContent(ix+1,iy+1,xxgd.pro2d[ich] / ((xxgd.ewid_sp[iy]*xxgd.ewid_sp[ix])/(xxgd.ewid_sp[0]*xxgd.ewid_sp[0])));
            h2DProj->SetBinContent(iy+1,ix+1,xxgd.pro2d[ich] / ((xxgd.ewid_sp[iy]*xxgd.ewid_sp[ix])/(xxgd.ewid_sp[0]*xxgd.ewid_sp[0])));
        }
    }

    fGatedSpectrum = new TH1D("GatedSpectrum","GatedSpectrum",xxgd.numchs,edges);
    fGatedSpectrum->SetXTitle("Energy (keV)");
    fGatedSpectrum->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrum->SetStats();

    fGatedSpectrumNoBG = new TH1D("GatedSpectrumBG","GatedSpectrumBG",xxgd.numchs,edges);
    fGatedSpectrumNoBG->SetXTitle("Energy (keV)");
    fGatedSpectrumNoBG->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrumNoBG->SetStats();

    gErrorIgnoreLevel = kPrint;
}

double CXRadReader::EffFuncFormula(double *x, double *p)
{
    float EG = x[0];
    double eff=0;

    // Paramters from Radware's web site: https://radware.phy.ornl.gov/gf3/#5.3.
    // A=7.04 B=0.7 C=0.
    // D=5.273 E=-0.863 F=0.01
    // G=11

    //low energy part
    // Experience shows that, unless there is a good reason to do otherwise, the parameter C should usually be left fixed to zero.
    // In addition, many gamma-ray sources do not provide enough data points at low energy to unambiguously determine both B and G,
    // so that at least one of these parameters may also have to be fixed before beginning the fit.
    // If more data that better defines the turnover region is added later, B and/or G may then be freed.
    // Typical values for B and G, for coaxial Ge detectors, are of the order 1 and 20, respectively.

    float A=p[0];
    float B=p[1];
    float C=p[2];

    //high energy
    float D=p[3];
    float E=p[4];
    float F=p[5];

    // interaction parameter between the two regions
    // the larger G is, the sharper will be the turnover at the top, between the two curves.
    // If the efficiency turns over gently, G will be small.
    float G=p[6];


    float E1=p[7]; //100 keV
    float E2=p[8]; //1000 keV

    float x1 = log(EG / E1);
    float x2 = log(EG / E2);

    float f1 = A + B*x1 + C*x1*x1;
    float f2 = D + E*x2 + F*x2*x2;

    if (f1 <= 0. || f2 <= 0.)
        eff = 1.0;
    else {
        float x3 = exp(-G * log(f1)) + exp(-G * log(f2));

        if (x3 <= 0.) eff = 1.0;
        else eff = exp(exp(-log(x3) / G));
    }

    return eff;
}

TH1 *CXRadReader::Project(const vector< pair<float, float> > &gatesX, const vector< pair<float, float> > &gatesY, Bool_t BGSubtract)
{
    Int_t Status;

    if(gatesX.empty() && !gatesY.empty())
        Status = Project2D(gatesY);
    else if(gatesY.empty() && !gatesX.empty())
        Status = Project2D(gatesX);
    else if(!gatesX.empty() && !gatesY.empty())
        Status = Project3D(gatesX,gatesY);
    else {
        gbash_color->WarningMessage("Empty gates, projection ignored");
        return nullptr;
    }
    if(Status>0){
        gbash_color->ErrorMessage("Error in the Projection, ignored");
        return nullptr;
    }

    if(BGSubtract) return fGatedSpectrum;

    return fGatedSpectrumNoBG;
}

int CXRadReader::Project2D(vector< pair<float, float> > gates)
{
    /* read gate of energy elo to ehi and calculate expected spectrum */
    /* spec[0][] = background-subtracted gate */
    /* spec[1][] = calculated spectrum from gammas outside gate */
    /* spec[2][] = calculated spectrum from all gammas */
    /* spec[3][] = non-background-subtracted gate */
    /* spec[4][] = square of statistical uncertainty */
    /* spec[5][] = square of statistical plus systematic uncertainties */

    gbash_color->InfoMessage("2D Projection:");
    for(auto &gate: gates) cout << " -- E [" << gate.first << " -  " << gate.second << "]" << endl;

    xxgd.old_name_gat = xxgd.name_gat;

    /* copy data from 2D projection */
    xxgd.bf1 = 0.0;
    xxgd.bf2 = 0.0;
    xxgd.bf4 = 0.0;
    xxgd.bf5 = 0.0;

    for (int j = 0; j < 6; ++j) {
        for (int ix = 0; ix < xxgd.numchs; ++ix) {
            xxgd.old_spec[j][ix] = xxgd.spec[j][ix];
            xxgd.spec[j][ix] = 0.0;
        }
    }

    xxgd.name_gat = "Rad2D_";
    if(fName != "") xxgd.name_gat = "RadCube_";

    for(auto i=0U ; i<gates.size() ; i++){

        float elo = gates[i].first;
        float ehi = gates[i].second;

        xxgd.name_gat += Form("%s%.1f(%.1f)", (i==0) ? "" : "+", (ehi+elo)*0.5,(ehi-elo)*0.5);

        add_gate(elo, ehi);
    }

    /* subtract background */
    for (int ix = 0; ix < xxgd.numchs; ++ix) {
        xxgd.spec[0][ix] =  xxgd.spec[0][ix]- xxgd.bf1 * xxgd.bspec[0][ix]
                - xxgd.bf2 * xxgd.bspec[1][ix]
                - xxgd.bf4 * xxgd.bspec[3][ix]
                - xxgd.bf5 * xxgd.bspec[4][ix];
    }

    /* correct for width of channel in keV */
    for (int i = 0; i < xxgd.numchs; ++i) {
        for (int j = 0; j < 4; ++j) {
            xxgd.spec[j][i] /= xxgd.ewid_sp[i];
        }
    }

    delete fGatedSpectrum;
    fGatedSpectrum = dynamic_cast<TH1*>(hTotalProj->Clone());
    fGatedSpectrum->Reset();
    fGatedSpectrum->SetXTitle("Energy (keV)");
    fGatedSpectrum->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrum->GetXaxis()->SetTitleOffset(1.2);
    fGatedSpectrum->GetYaxis()->SetTitleOffset(0.8);
    fGatedSpectrum->SetStats();
    fGatedSpectrum->SetNameTitle(xxgd.name_gat,xxgd.name_gat);

    delete fGatedSpectrumNoBG;
    fGatedSpectrumNoBG = dynamic_cast<TH1*>(hTotalProj->Clone());
    fGatedSpectrumNoBG->Reset();
    fGatedSpectrumNoBG->SetXTitle("Energy (keV)");
    fGatedSpectrumNoBG->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrum->GetXaxis()->SetTitleOffset(1.2);
    fGatedSpectrum->GetYaxis()->SetTitleOffset(0.8);
    fGatedSpectrum->SetStats();
    fGatedSpectrumNoBG->SetNameTitle(Form("%s_NoBG",xxgd.name_gat.Data()),Form("%s_NoBG",xxgd.name_gat.Data()));

    for (int i = 0; i < xxgd.numchs; ++i) {
        fGatedSpectrum->SetBinContent(i+1,xxgd.spec[0][i] * xxgd.ewid_sp[0]);
        fGatedSpectrumNoBG->SetBinContent(i+1,xxgd.spec[3][i] * xxgd.ewid_sp[0]);
    }

    return 0;
}

int CXRadReader::Project3D(vector< pair<float, float> > gatesX, vector< pair<float, float> > gatesY)
{
    gbash_color->InfoMessage("3D Projection:");
    for(auto &gate: gatesX) cout << " -- E [" << gate.first << " -  " << gate.second << "]" << endl;
    cout << " AND " << endl;
    for(auto &gate: gatesY) cout << " -- E [" << gate.first << " -  " << gate.second << "]" << endl;

    xxgd.old_name_gat = xxgd.name_gat;

    /* copy data from 2D projection */
    xxgd.bf1 = 0.0;  xxgd.bf2 = 0.0;  xxgd.bf4 = 0.0;  xxgd.bf5 = 0.0;

    for (int j = 0; j < 6; ++j) {
        for (int ix = 0; ix < xxgd.numchs; ++ix) {
            xxgd.old_spec[j][ix] = xxgd.spec[j][ix];
            xxgd.spec[j][ix] = 0.0;
        }
    }

    int nsum = 0;

    TString NameX="";
    TString NameY="";

    for(auto i=0U ; i<gatesX.size() ; i++){

        float eloX = gatesX[i].first;
        float ehiX = gatesX[i].second;

        NameX += Form("%s%.1f(%.1f)", (i==0) ? "" : "+",(eloX+ehiX)*0.5,TMath::Abs(eloX-ehiX)*0.5);

        for(auto j=0U ; j<gatesY.size() ; j++) {

            float eloY = gatesY[j].first;
            float ehiY = gatesY[j].second;

            NameY += Form("%s%.1f(%.1f)", (j==0) ? "" : "+",(eloY+ehiY)*0.5,TMath::Abs(eloY-ehiY)*0.5);

            ++nsum;
            add_trip_gate(eloX, ehiX, eloY, ehiY);
        }
    }

    xxgd.name_gat = Form("%s/%s", NameX.Data(), NameY.Data());

    if (nsum > 1)
        cout<<Form("Sum of %d double-gates...", nsum)<<endl;

    /* subtract background */
    for (int ix = 0; ix < xxgd.numchs; ++ix) {
        xxgd.spec[0][ix] =  xxgd.spec[0][ix]- xxgd.bf1 * xxgd.bspec[0][ix]
                - xxgd.bf2 * xxgd.bspec[1][ix]
                - xxgd.bf4 * xxgd.bspec[3][ix]
                - xxgd.bf5 * xxgd.bspec[4][ix];
    }

    /* correct for width of channel in keV */
    for (int i = 0; i < xxgd.numchs; ++i) {
        for (int j = 0; j < 4; ++j) xxgd.spec[j][i] /= xxgd.ewid_sp[i];
        if (!xxgd.many_cubes) xxgd.spec[4][i] = xxgd.spec[3][i] / xxgd.ewid_sp[i];
        xxgd.spec[5][i] = xxgd.spec[4][i];
        if (xxgd.spec[5][i] < 1.0f) xxgd.spec[5][i] = 1.0f;
    }

    delete fGatedSpectrum;
    fGatedSpectrum = dynamic_cast<TH1*>(hTotalProj->Clone());
    fGatedSpectrum->Reset();
    fGatedSpectrum->SetXTitle("Energy (keV)");
    fGatedSpectrum->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrum->GetXaxis()->SetTitleOffset(1.2);
    fGatedSpectrum->GetYaxis()->SetTitleOffset(0.8);
    fGatedSpectrum->SetStats();
    fGatedSpectrum->SetNameTitle(xxgd.name_gat,xxgd.name_gat);

    delete fGatedSpectrumNoBG;
    fGatedSpectrumNoBG = dynamic_cast<TH1*>(hTotalProj->Clone());
    fGatedSpectrumNoBG->Reset();
    fGatedSpectrumNoBG->SetXTitle("Energy (keV)");
    fGatedSpectrumNoBG->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrum->GetXaxis()->SetTitleOffset(1.2);
    fGatedSpectrum->GetYaxis()->SetTitleOffset(0.8);
    fGatedSpectrum->SetStats();
    fGatedSpectrumNoBG->SetNameTitle(Form("%s_NoBG",xxgd.name_gat.Data()),Form("%s_NoBG",xxgd.name_gat.Data()));

    for (int i = 0; i < xxgd.numchs; ++i) {
        fGatedSpectrum->SetBinContent(i+1,xxgd.spec[0][i] * xxgd.ewid_sp[0]);
        fGatedSpectrumNoBG->SetBinContent(i+1,xxgd.spec[3][i] * xxgd.ewid_sp[0]);
    }

    return 0;
}

Int_t CXRadReader::AutoBuildProj(TString FileName, Int_t Mode)
{
    int length = gd3d->length;
    unsigned int mc[512], *data2d;
    unsigned short mc2[512];
    int i, j, k, x, y, z, ix, iy, iz;
    int ax[512], ay[512], az[512];
    float data1d[RW_MAXCH];

    unsigned char lobyte[RW_MAXCH+6];
    unsigned short overflows[RW_MAXCH][2], numoverflows;
    int overflowaddr;

    CXProgressBar progress(gd3d->nummc);

    /* malloc the 2D projection space */
    if (!(data2d=(uint *)malloc(4*(length+8)*length))) {
        gbash_color->ErrorMessage("Ooops, data2d malloc failed... please free up some memory");
        return 1;
    }

    /* calculate the mappings from 3d channel space to 2d channel space */
    i = 0;
    for (z=0; z<8; z++) {
        for (y=0; y<8; y++) {
            for (x=0; x<8; x++) {
                ax[i] = x;
                ay[i] = y;
                az[i] = z;
                i++;
            }
        }
    }

    memset(data2d, 0, 4*(length+8)*length);

    if(Mode==1)
        printf("Projecting cube to 1D...\n");
    else if(Mode==2)
        printf("Projecting cube to 2D...\n");
    else if(Mode==3)
        printf("Projecting cube to 1D and 2D...\n");
    else
        return 1;

    if (gd3d->cubetype==0) {  /* compressed 1/6 cube */
        for (iz=0; iz<length; iz+=8) {
            for (iy=0; iy<=iz; iy+=8) {
                for (ix=0; ix<=iy; ix+=8) {
                    progress+=2;
                    j = LUMC3D(ix,iy,iz);
                    getmc3d(j,mc);
                    getmc3d(j+1,&mc[256]);
//                    if ((j&511)==0) {
//                        cout << Form("\rx y z: %4d %4d %4d",ix,iy,iz);
//                    }
                    for (i=0; i<512; i++) {
                        *(data2d + (iy+ay[i])*length + ix+ax[i]) += mc[i];
                        *(data2d + (iz+az[i])*length + ix+ax[i]) += mc[i];
                        *(data2d + (iz+az[i])*length + iy+ay[i]) += mc[i];
                    }
                }
            }
        }
    }
    else if (gd3d->cubetype==1) {  /* compressed 1/2 cube */
        j = 0;
        for (iz=0; iz<length; iz+=8) {
            for (iy=0; iy<=iz; iy+=8) {
                for (ix=0; ix<length; ix+=8) {
                    getmc3d(j++,mc);
                    if ((ix+4) < length) {
                        getmc3d(j++,&mc[256]);
                    } else {
                        memset(&mc[256], 0, 1024);
                    }
                    progress+=2;
//                    if ((j&511)==0) {
//                        cout << Form("\rx y z: %4d %4d %4d",ix,iy,iz);
//                    }
                    for (i=0; i<512; i++)
                        *(data2d + (iz+ay[i])*length + iy+ax[i]) += mc[i];
                }
            }
        }
        for (iz=0; iz<length; iz++)
            *(data2d + iz*length + iz) = *(data2d + iz*length + iz)/2;
    }

    else if (gd3d->cubetype==2) {  /* uncompressed 1/6 cube */
        j = 0;
        for (iz=0; iz<length; iz+=8) {
            for (iy=0; iy<=iz; iy+=8) {
                for (ix=0; ix<=iy; ix+=8) {
                    if (!fread(mc2,1024,1,gd3d->CubeFile3d)) {
                        gbash_color->ErrorMessage(Form("Could not read record %d... aborting.", j));
                        return 1;
                    }
                    if (gd3d->swapbytes) {
                        for (i=0; i<512; i++)
                            swap2(&mc2[i]);
                    }
                    progress+=1;
//                    if (((j++)&255)==0) {
//                        cout << Form("\rx y z: %4d %4d %4d",ix,iy,iz);
//                    }
                    for (i=0; i<512; i++) {
                        *(data2d + (iy+ay[i])*length + ix+ax[i]) += mc2[i];
                        *(data2d + (iz+az[i])*length + ix+ax[i]) += mc2[i];
                        *(data2d + (iz+az[i])*length + iy+ay[i]) += mc2[i];
                    }
                }
            }
        }
    }
    else if (gd3d->cubetype==3) {  /* uncompressed 1/2 cube */
        j = 1024;
        k = 1 + ((length*(length+1)/2 * (length+6) + 1023) / 1024);
        k = k * 1024;
        for (iz=0; iz<length; iz++) {
            progress++;
//            cout << Form("\r z: %4d ", iz);
            for (iy=0; iy<=iz; iy++) {
                fseek(gd3d->CubeFile3d, ((long)j), SEEK_SET);
                if (!fread(lobyte,length+6,1,gd3d->CubeFile3d)) {
                    gbash_color->ErrorMessage("Could not read cube file... aborting");
                    return 1;
                }
                j += length+6;
                for (ix=0; ix<length; ix++)
                    *(data2d + iz*length + iy) += lobyte[ix];

                memcpy(&numoverflows, &lobyte[length], 2);
                if (numoverflows == 0)
                    continue;
                memcpy(&overflowaddr, &lobyte[length+2], 4);
                if (gd3d->swapbytes) {
                    swap2(&numoverflows);
                    swap4(&overflowaddr);
                }
                fseek(gd3d->CubeFile3d, (long)k+(long)(overflowaddr*4), SEEK_SET);
                if (!fread(overflows,4*numoverflows,1,gd3d->CubeFile3d)) {
                    gbash_color->ErrorMessage("Could not read cube file... aborting");
                    return 1;
                }
                for (ix=0; ix<numoverflows; ix++) {
                    if (gd3d->swapbytes) swap2(&overflows[ix][1]);
                    *(data2d + iz*length + iy) += (int)overflows[ix][1];
                }
            }
            *(data2d + iz*length + iz) = *(data2d + iz*length + iz)/2;
        }
    }

    FILE *file;

    if(Mode == 2 || Mode == 3) {
        TString nameout = FileName.Copy().ReplaceAll(".cub",".2dp");

        file=fopen(nameout,"w+");

        for (y=0; y<length; y++) {
            for (x=0; x<=y; x++) {
                data1d[x] = (float)(*(data2d + y*length + x));
            }
            data1d[y] = 2.0*data1d[y];
            for (x=y+1; x<length; x++) {
                data1d[x] = (float)(*(data2d + x*length + y));
            }
            if (!(fwrite(data1d, 4*length, 1, file))) {
                gbash_color->ErrorMessage("Cannot write 2d file, aborting...");
                return 1;
            }
        }
        fclose(file);
    }
    if(Mode == 1 || Mode == 3) {
        TString nameout = FileName.Copy().ReplaceAll(".cub",".spe");

        for (y=0; y<length; y++) {
            data1d[y] = 0.0;
            for (x=0; x<=y; x++) {
                data1d[x] += (float)(*(data2d + y*length + x));
                data1d[y] += (float)(*(data2d + y*length + x));
            }
        }

        wspec(nameout, data1d, length);
    }
    cout<<endl;
    return 0;
}

int CXRadReader::open3dfile(TString &name, int *numchs_ret)
{
    int length, i, nummc, sr;
    FHead3D head;
    Record3D rec;
    TString cname;
    char *tmp;

    cname = name;

    if (!gd3d) gd3d = &gd3dn[0];

    if (!cname[0] || !(gd3d->CubeFile3d=fopen(cname,"r"))) {
        gbash_color->ErrorMessage(Form("Cannot open file %s for reading", cname.Data()));
        return 1;
    }

#define QUIT close3dfile(); return 1;
    if (!fread(&head,1024,1,gd3d->CubeFile3d)) {
        gbash_color->ErrorMessage(Form("Cannot read file %s",name.Data()));
        QUIT;
    }
    if (head.cps > 6) {
        gd3d->swapbytes = 1;
        swap4(&head.numch);
        swap4(&head.bpc);
        swap4(&head.cps);
        swap4(&head.numrecs);
    }
    else
        gd3d->swapbytes = 0;

    if (!(strncmp(head.id,"Incub8r3/Pro4d  ",16)) && head.cps == 6)
        gd3d->cubetype = 0;
    else if (!(strncmp(head.id,"Foldout         ",16)) && head.cps == 2)
        gd3d->cubetype = 1;
    else if (!(strncmp(head.id,"INCUB8R2        ",16)) && head.cps == 6)
        gd3d->cubetype = 2;
    else if (!(strncmp(head.id,"FOLDOUT         ",16)) && head.cps == 2)
        gd3d->cubetype = 3;
    else {
        gbash_color->ErrorMessage(Form("Invalid header in file %s",name.Data()));
        QUIT;
    }

    length = head.numch;
    *numchs_ret = length;
    gd3d->length = length;
    if (length > RW_MAXCH) {
        printf("ERROR: Number of channels in cube (%d)\n"
               "         is greater than RW_MAXCH (%d) in pro3d.h\n",
               length, RW_MAXCH);
        exit(-1);
    }
    gd3d->numrecs = head.numrecs;
    nummc = (length+7)/8;  /* minicubes per side */
    if (gd3d->cubetype==0 || gd3d->cubetype==2)
        nummc = nummc*(nummc+1)*(nummc+2)/3; /* minicubes in cube */
    else if (gd3d->cubetype==1 || gd3d->cubetype==3)
        nummc = (nummc*(nummc+1)/2) * ((length+3)/4); /* minicubes in cube */
    gd3d->nummc = nummc;

    /* calculate look-up tables to convert (x,y,z) 3d cube address
        to linearized 3d minicube address.  x<y<z
     lum{x,y,z}3d[ch] returns the number of subcubes to skip over
        to get to the subcube with ch in it.
     note that a 3d subcube on disk is 4x8x8 channels, but we always read them
        in pairs, i.e. we always read 8x8x8 channels.
  */
    for (i=0;i<8;i++) {
        lumx3d[i] = 0;
        lumy3d[i] = 0;
        lumz3d[i] = 0;
    }
    for (i=8;i<gd3d->length+16;i++) {
        lumx3d[i] = lumx3d[i-8] + 2;
        lumy3d[i] = lumy3d[i-8] + lumx3d[i];
        lumz3d[i] = lumz3d[i-8] + lumy3d[i];
    }

    printf("Axis length of cube is %d.\n", length);
    if (gd3d->cubetype>1) return 0;

    gd3d->numrecs = head.numrecs;
    printf("Cube has %d minicubes and %d records.\n", nummc, gd3d->numrecs);
    gd3d->numsr = (head.numrecs + 15)/16;

    /* read SR list from end of file */
    while (!(gd3d->mcnum = (int *)malloc(4*(1+gd3d->numsr))))
        printf("trying to malloc gd3d->mcnum...\n");
    if ((fseek(gd3d->CubeFile3d, (((long) head.numrecs)*4+1)*1024, SEEK_SET)) ||
            (!fread(gd3d->mcnum,4*(1+gd3d->numsr),1,gd3d->CubeFile3d))) {

        /* build SuperRecord physical # and minicube # lookup table */
        printf("  Building 3d SuperRecord list...\n");
        for (sr=0; sr<gd3d->numsr; sr++) {
            if ((fseek(gd3d->CubeFile3d, (((long) sr)*64+1)*1024, SEEK_SET)) ||
                    (!fread(&rec,4096,1,gd3d->CubeFile3d))) {
                printf("\007  ERROR -- Could not read Record %d... aborting.\n", sr*16);
                QUIT;
            }
            gd3d->mcnum[sr] = rec.h.minmc;
        }
        gd3d->mcnum[gd3d->numsr] = nummc;
        if (gd3d->swapbytes)
            swap4(&gd3d->mcnum[gd3d->numsr]);
        printf("  ...Done building SuperRecord list,\n"
               "       will now save it at end of cube file.\n");

        /* save SR list to end of file */
        fclose(gd3d->CubeFile3d);
        if (!(gd3d->CubeFile3d=fopen(cname,"a+"))) {
            gbash_color->ErrorMessage(Form("Cannot open file %s for writing",name.Data()));
            QUIT;

        }
        if (!fwrite(gd3d->mcnum,4*(1+gd3d->numsr),1,gd3d->CubeFile3d)) {
            gbash_color->ErrorMessage("Could not write SuperRecord list... aborting");
            QUIT;
        }
        fflush(gd3d->CubeFile3d);
    }
    if (gd3d->swapbytes) {
        for (sr=0; sr<=gd3d->numsr; sr++)
            swap4(&gd3d->mcnum[sr]);
    }
#undef QUIT
    return 0;
}

int CXRadReader::close3dfile(void)
{
    if (gd3d->CubeFile3d) {
        fclose(gd3d->CubeFile3d);
        free(gd3d->mcnum);
        gd3d->CubeFile3d = 0;
    }
    return 0;
}

int CXRadReader::read_spe_file(const TString &filnam, float *sp, TString &namesp, int *numch, int idimsp)
{
    float *data = new float[idimsp];
    for(int i=0 ; i<idimsp ; i++) data[i]=0.;

    /* read spectrum from .spe file = filnam into array sp of dimension idimsp
     numch = number of channels read
     namesp = name of spectrum (character*8)
     returns 1 for open/read error
     default file extension = .spe */

    char cbuf[800];
    int  idim1, idim2, j, rl, swap = -1;
    FILE *file;

    /* read spectrum in standard gf3 format
     OPEN(ILU,FILE=FILNAM,FORM='UNFORMATTED',STATUS='OLD')
     READ(ILU) NAMESP, IDIM1, IDIM2, IRED1, IRED2
     READ(ILU,ERR=90) (SP(I),I=1,NUMCH) */
    if (!(file = open_readonly(filnam))) {
        delete [] data;
        return 1;
    }
    rl = get_file_rec(file, cbuf, 800, 0);
    if (rl != 24  && rl != -24) {
        file_error("read", filnam);
        fclose(file);
        delete [] data;
        return 1;
    }
    if (rl < 0) {
        /* file has wrong byte order - swap bytes */
        cout<<Form("*** Swapping bytes read from file %s", filnam.Data())<<endl;
        swap = 1;
        for (j = 2; j < 4; ++j) {
            swapb4(cbuf + 4*j);
        }
    }
    namesp = TString(cbuf,8);
    memcpy(&idim1, cbuf + 8, 4);
    memcpy(&idim2, cbuf + 12, 4);
    *numch = idim1 * idim2;

    if (*numch > idimsp) {
        *numch = idimsp;
        cout<<Form("First %d chs only taken.", idimsp)<<endl;
    }
    rl = get_file_rec(file, data, 4*idimsp, swap);

    fclose(file);
    if (rl != 4*(*numch) && rl != -4*(*numch)) {
        file_error("read spectrum from", filnam);
        delete [] data;
        return 1;
    }
    if (rl < 0) {
        /* file has wrong byte order - swap bytes */
        for (j = 0; j < *numch; ++j) {
            swapb4((char *) (data + j));
        }
    }

    for(int i=0 ; i<idimsp ; i++) sp[i] = data[i];
    delete [] data;

    return 0;
} /* read_spe_file */

/* ======================================================================= */
int CXRadReader::read_cal_file(TString &filnam, char *title, int *iorder, double *gain)
{
    /* read energy calib parameters from .aca/.cal file = filnam
     into title, iorder, gain
     returns 1 for open/read error
     default file extension = .aca, .cal */

    char cbuf[800];
    int  j, rl;
    char fn1[800], fn2[800], line[120], *c;
    FILE *file;
    struct stat buf;

    strncpy(fn1, filnam, 800);
    strncpy(fn2, filnam, 800);
    j = setext(fn1, ".aca", 800);
    j = setext(fn2, ".cal", 800);
    if (strcmp(fn1+j, ".aca") && strcmp(fn2+j, ".cal")) {
        gbash_color->ErrorMessage("ERROR - filename .ext must be .aca or .cal!");
        return 1;
    } else if (strcmp(fn1+j, ".aca") && stat(fn2, &buf)) {
        gbash_color->ErrorMessage(Form("ERROR - file %s does not exist!", fn2));
        return 1;
    } else if (strcmp(fn2+j, ".cal") && stat(fn1, &buf)) {
        gbash_color->ErrorMessage(Form("ERROR - file %s does not exist!", fn1));
        return 1;
    } else if (stat(fn1, &buf) && stat(fn2, &buf)) {
        gbash_color->ErrorMessage(Form("ERROR - neither file %s nor file %s exists!", fn1, fn2));
        return 1;
    }

#define ERR { file_error("read", fn1); fclose(file); return 1; }
    /* first try ascii cal file (.aca) */
    if (!strcmp(fn1+j, ".aca") &&
            (file = fopen(fn1, "r"))) {
        if (!(fgets(line, 120, file)) ||
                !strstr(line, "ENCAL OUTPUT FILE") ||
                !(fgets(line, 120, file)) ||
                !strncpy(title, line, 800) ||
                !(fgets(line, 120, file))) ERR;
        while ((c = strchr(line, 'D'))) *c = 'E'; /* convert fortran-stye format */
        if (sscanf(line, "%d %lE %lE %lE", iorder, gain, gain+1, gain+2) != 4 ||
                !(fgets(line, 120, file))) ERR;
        while ((c = strchr(line, 'D'))) *c = 'E'; /* convert fortran-stye format */
        if (sscanf(line, "%lE %lE %lE", gain+3, gain+4, gain+5) != 3) ERR;
        fclose(file);
#undef ERR

        /* next try binary cal file (.cal) */
    } else {
        /* read parameters from disk file
       OPEN(ILU,FILE=FILNAM,FORM='UNFORMATTED',STATUS='OLD',ERR=200)
       READ(ILU) TEST, TITLE
       READ(ILU) IORDER, GAIN */
        if (!(file = open_readonly(fn2))) return 1;
        rl = get_file_rec(file, cbuf, 800, 0);
        if ((rl != 98 && rl != -98) ||
                strncmp(cbuf, " ENCAL OUTPUT FILE", 18)) {
            file_error("read", fn2);
            gbash_color->WarningMessage("...File is not an ENCAL output file.");
            fclose(file);
            return 1;
        }
        strncpy(title, &cbuf[18], 800);
        rl = get_file_rec(file, cbuf, 800, 0);
        fclose(file);
        if (rl != 52 && rl != -52) {
            file_error("read", fn2);
            return 1;
        }
        if (rl < 0) {
            /* file has wrong byte order - swap bytes */
            gbash_color->WarningMessage(Form("*** Swapping bytes read from file %s", fn2));
            swapb4(cbuf);
            for (j = 0; j < 6; ++j) {
                swapb8(cbuf + 4 + 8*j);
            }
        }
        memcpy(iorder, cbuf, 4);
        for (j = 0; j < 6; ++j) {
            memcpy(&gain[j], cbuf + 4 + 8*j, 8);
        }
    }
    if (strlen(title) > 79) title[79] = '\0';
    if ((c = strchr(title,'\n'))) *c = '\0';
    gbash_color->InfoMessage(Form("%s", title));
    return 0;
} /* read_cal_file */

/* ======================================================================= */
int CXRadReader::read_eff_file(TString &filnam, char *title, double *pars_d)
{
    /* read efficiency parameters from .aef/.eff file = filnam
     into title, pars
     returns 1 for open/read erro
     default file extension = .aef, .eff */

    float pars[10];

    char cbuf[800];
    int  j, rl;
    char fn1[800], fn2[800], line[120], *c;
    FILE *file;
    struct stat buf;

    strncpy(fn1, filnam, 800);
    strncpy(fn2, filnam, 800);
    j = setext(fn1, ".aef", 800);
    j = setext(fn2, ".eff", 800);
    if (strcmp(fn1+j, ".aef") && strcmp(fn2+j, ".eff")) {
        gbash_color->ErrorMessage("ERROR - filename .ext must be .aef or .eff!");
        return 1;
    } else if (strcmp(fn1+j, ".aef") && stat(fn2, &buf)) {
        gbash_color->ErrorMessage(Form("ERROR - file %s does not exist!", fn2));
        return 1;
    } else if (strcmp(fn2+j, ".eff") && stat(fn1, &buf)) {
        gbash_color->ErrorMessage(Form("ERROR - file %s does not exist!", fn1));
        return 1;
    } else if (stat(fn1, &buf) && stat(fn2, &buf)) {
        gbash_color->ErrorMessage(Form("ERROR - neither file %s nor file %s exists!", fn1, fn2));
        return 1;
    }

    /* first try ascii eff file (.aef) */
    if (!strcmp(fn1+j, ".aef") &&
            (file = fopen(fn1, "r"))) {
        if (!(fgets(line, 120, file)) ||
                !strstr(line, "EFFIT PARAMETER FILE") ||
                !strncpy(title, line+1, 40) ||
                !(fgets(line, 120, file)) ||
                (sscanf(line, "%f %f %f %f %f",
                        pars, pars+1, pars+2, pars+3, pars+4) != 5) ||
                !(fgets(line, 120, file)) ||
                (sscanf(line, "%f %f %f %f %f",
                        pars+5, pars+6, pars+7, pars+8, pars+9) != 5)) {
            file_error("read", fn1);
            fclose(file);
            return 1;
        }
        fclose(file);

        /* next try binary eff file (.eff) */
    } else {
        /* read parameters from disk file
       OPEN(ILU,FILE=FILNAM,FORM='UNFORMATTED',STATUS='OLD')
       READ(ILU) TITLE
       READ(ILU) PARS */
        /* first record has 40 or 42 bytes. */

        if (!(file = open_readonly(fn2))) return 1;
        rl = get_file_rec(file, cbuf, 800, 0);
        if (rl != 40  && rl != 42 &&
                rl != -40 && rl != -42) {
            file_error("read", fn2);
            fclose(file);
            return 1;
        }
        strncpy(title, cbuf, 40);

        rl = get_file_rec(file, cbuf, 800, 0);
        fclose(file);
        if (rl != 40 && rl != -40) {
            file_error("read", fn2);
            return 1;
        }
        if (rl < 0) {
            /* file has wrong byte order - swap bytes */
            gbash_color->WarningMessage(Form("*** Swapping bytes read from file %s", fn2));
            for (j = 0; j < 10; ++j) {
                swapb4(cbuf + 4*j);
            }
        }
        for (j = 0; j < 10; ++j) {
            memcpy(&pars[j], cbuf + 4*j, 4);
        }
    }
    if (strlen(title) > 39) title[39] = '\0';
    if ((c = strchr(title,'\n'))) *c = '\0';
    gbash_color->InfoMessage(Form("%s", title));

    for(int i=0 ; i<10 ; i++) pars_d[i] = pars[i];

    return 0;
} /* read_eff_file */

void CXRadReader::setcubenum(int cubenum)
{
    gd3d = &gd3dn[cubenum];
    recloaded = 0;
}

/* ======================================================================= */
int CXRadReader::add_gate(float elo, float ehi)
{
    float r1;
    int   ix, iy, ich, ihi, ilo;

    /* read gate of energy elo to ehi and calculate expected spectrum */
    /* spec[0][] = background-subtracted gate */
    /* spec[3][] = non-background-subtracted gate */

    ilo = hTotalProj->GetXaxis()->FindBin(elo)-1;
    ihi = hTotalProj->GetXaxis()->FindBin(ehi)-1;

    //    cout<<"add_gate("<<elo<<","<<ehi<<"): ilo="<<ilo<<" ; ihi="<<ihi<<endl;

    /* copy data from 2D projection */
    for (iy = ilo; iy < ihi; ++iy) {
        for (ix = 0; ix < iy; ++ix) {
            ich = xxgd.luch[ix] + iy;
            xxgd.spec[0][ix] += xxgd.pro2d[ich];
            xxgd.spec[3][ix] += xxgd.pro2d[ich];
        }
        for (ix = iy; ix < xxgd.numchs; ++ix) {
            ich = xxgd.luch[iy] + ix;
            xxgd.spec[0][ix] += xxgd.pro2d[ich];
            xxgd.spec[3][ix] += xxgd.pro2d[ich];
        }
    }

    for (iy = ilo; iy <= ihi; ++iy) {
        /* subtract background */
        xxgd.bf1 += xxgd.bspec[1][iy];
        xxgd.bf2 += xxgd.bspec[2][iy];
        xxgd.bf4 += xxgd.bspec[4][iy];
        xxgd.bf5 += xxgd.bspec[5][iy];

        for (ix = 0; ix < xxgd.numchs; ++ix) {
            if ((float) abs(ix - iy) < xxgd.v_width) {
                r1 = (float) (ix - iy) / xxgd.v_width;
                xxgd.spec[0][ix] += xxgd.v_depth[iy] * (1.0f - r1 * r1);
            }
        }
    }

    return 0;
} /* add_gate */

/* ======================================================================= */
int CXRadReader::add_trip_gate(float elo1, float ehi1, float elo2, float ehi2)
{
    int   ichxy, ichxz, ichyz, ix, iy, iz;
    int   ihi1, ihi2, ilo1, ilo2;

    /* read gate of energy elo to ehi and calculate expected spectrum */
    /* spec[0][] = background-subtracted gate */
    /* spec[1][] = calculated spectrum from gammas outside gate */
    /* spec[2][] = calculated spectrum from all gammas */
    /* spec[3][] = non-background-subtracted gate */
    /* spec[4][] = square of statistical uncertainty */
    /* spec[5][] = square of statistical plus systematic uncertainties */

    ilo1 = hTotalProj->GetXaxis()->FindBin(elo1)-1;
    ihi1 = hTotalProj->GetXaxis()->FindBin(ehi1)-2;

    ilo2 = hTotalProj->GetXaxis()->FindBin(elo2)-1;
    ihi2 = hTotalProj->GetXaxis()->FindBin(ehi2)-2;

    read_cube(ilo1, ihi1, ilo2, ihi2);
    for (iy = ilo2; iy <= ihi2; ++iy) {
        for (ix = ilo1; ix <= ihi1; ++ix) {
            if (ix <= iy) {
                ichxy = xxgd.luch[ix] + iy;
            } else {
                ichxy = xxgd.luch[iy] + ix;
            }
            xxgd.bf1 -= xxgd.bspec[1][ix] * xxgd.bspec[1][iy];
            xxgd.bf2 = xxgd.bf2 + xxgd.pro2d[ichxy] - xxgd.bspec[1][ix] *
                    xxgd.bspec[0][iy] - xxgd.bspec[2][ix] * xxgd.bspec[1][iy];
            xxgd.bf2 = xxgd.bf2 - xxgd.bspec[4][ix] * xxgd.bspec[3][iy] -
                    xxgd.bspec[5][ix] * xxgd.bspec[4][iy];
            xxgd.bf4 = xxgd.bf4 - xxgd.bspec[1][ix] * xxgd.bspec[4][iy] -
                    xxgd.bspec[4][ix] * xxgd.bspec[1][iy];
            xxgd.bf5 = xxgd.bf5 + xxgd.e2pro2d[ichxy] - xxgd.bspec[1][ix] *
                    xxgd.bspec[5][iy] - xxgd.bspec[5][ix] * xxgd.bspec[1][iy] -
                    xxgd.e2e2spec[ix] * xxgd.bspec[4][iy] - xxgd.bspec[4][ix] *
                    xxgd.e2e2spec[iy] + xxgd.bspec[4][ix] * xxgd.bspec[4][iy] *
                    xxgd.e2e2e2sum;
            for (iz = 0; iz < xxgd.numchs; ++iz) {
                /* subtract part of the background */
                if (ix < iz) {
                    ichxz = xxgd.luch[ix] + iz;
                } else {
                    ichxz = xxgd.luch[iz] + ix;
                }
                if (iy < iz) {
                    ichyz = xxgd.luch[iy] + iz;
                } else {
                    ichyz = xxgd.luch[iz] + iy;
                }
                xxgd.spec[0][iz] = xxgd.spec[0][iz] - xxgd.pro2d[ichxz] *
                        xxgd.bspec[1][iy] - xxgd.pro2d[ichyz] * xxgd.bspec[1][ix] -
                        xxgd.e2pro2d[ichxz] * xxgd.bspec[4][iy] - xxgd.e2pro2d[ichyz] *
                        xxgd.bspec[4][ix] + xxgd.bspec[4][ix] * xxgd.bspec[4][iy] *
                        xxgd.e2e2spec[iz];
            }
        }
    }

    return 0;
} /* add_trip_gate */

/* ======================================================================= */
int CXRadReader::read_cube(int ilo1, int ihi1, int ilo2, int ihi2)
{
    int   i, icubenum, ispec[MAXCHS];

    if (!xxgd.many_cubes) {
        read3dcube(ilo1, ihi1, ilo2, ihi2, ispec);
        for (i = 0; i < xxgd.numchs; ++i) {
            xxgd.spec[0][i] += (float) ispec[i];
            xxgd.spec[3][i] += (float) ispec[i];
        }

    } else {
        /* stuff added for linear combinations of cubes: */
        setcubenum(0);
        read3dcube(ilo1, ihi1, ilo2, ihi2, ispec);
        for (i = 0; i < xxgd.numchs; ++i) {
            xxgd.spec[0][i] += (float) ispec[i];
            xxgd.spec[3][i] += (float) ispec[i];
            xxgd.spec[4][i] += (float) ispec[i] /
                    (xxgd.ewid_sp[i] * xxgd.ewid_sp[i]);
        }

        for (icubenum = 0; icubenum < 5 && xxgd.cubefact[icubenum]; icubenum++) {
            setcubenum(icubenum + 1);
            read3dcube(ilo1, ihi1, ilo2, ihi2, ispec);
            for (i = 0; i < xxgd.numchs; ++i) {
                xxgd.spec[0][i] += xxgd.cubefact[icubenum] * (float) ispec[i];
                xxgd.spec[3][i] += xxgd.cubefact[icubenum] * (float) ispec[i];
                xxgd.spec[4][i] += xxgd.cubefact[icubenum] *
                        xxgd.cubefact[icubenum] * (float) ispec[i] /
                        (xxgd.ewid_sp[i]*xxgd.ewid_sp[i]);
            }
        }
    }
    return 0;
} /* read_cube */

int CXRadReader::read3dcube(int lo1, int hi1, int lo2, int hi2, int *spec_ret)
{
    int lim[2][2];
    int i, j, k, tmp, x, y, z, iy, iz;
    int ymc, zmc, ylo, zlo, yhi, zhi;
    int x1, y1, y2, z1, z2, yy1, yy2, zz1, zz2;
    unsigned int mc[8][8][8];
    unsigned short mc2[8][8][8];
    unsigned char lobyte[RW_MAXCH+6];
    unsigned short overflows[RW_MAXCH][2], numoverflows;
    int overflowaddr;
    int mcnum;
    int length;  /* number of channels per side of cube */


    length = gd3d->length;
    memset(spec_ret, 0, 4*gd3d->length);

    lim[0][0] = lo1; lim[0][1] = hi1;
    lim[1][0] = lo2; lim[1][1] = hi2;
    for (i=0; i<2; i++) {
        if (lim[i][0]>lim[i][1]) {
            tmp = lim[i][0];
            lim[i][0] = lim[i][1];
            lim[i][1] = tmp;
        }
        if (lim[i][0]<0 || lim[i][1] >= gd3d->length) {
            printf(" OVERFLOW.  The valid range is 0 to %d.\n", gd3d->length-1);
            return 0;
        }
    }

    /* order the lower limits of the gates */
    if (lim[0][0]>lim[1][0]) {
        tmp = lim[1][0];
        lim[1][0] = lim[0][0];
        lim[0][0] = tmp;
        tmp = lim[1][1];
        lim[1][1] = lim[0][1];
        lim[0][1] = tmp;
    }

    /* now go read the cube to get the gate */

    if (gd3d->cubetype==3) {  /* uncompressed 1/2 cube */
        j = 1024;
        k = 1 + ((length*(length+1)/2 * (length+6) + 1023) / 1024);
        k = k * 1024;
        for (iz=lim[1][0]; iz<=lim[1][1]; iz++) {
            for (iy=lim[0][0]; iy<=lim[0][1]; iy++) {
                if (iy<=iz)
                    j = 1024 + (iz*(iz+1)/2 + iy)*(length+6);
                else
                    j = 1024 + (iy*(iy+1)/2 + iz)*(length+6);
                fseek(gd3d->CubeFile3d, ((long)j), SEEK_SET);
                if (!fread(lobyte,length+6,1,gd3d->CubeFile3d)) {
                    printf("\007  ERROR -- Could not read cube file... aborting.\n");
                    exit(-1);
                }
                for (x=0; x<length; x++) {
                    spec_ret[x] += (int)lobyte[x];
                }

                memcpy(&numoverflows, &lobyte[length], 2);
                if (numoverflows == 0)
                    continue;
                memcpy(&overflowaddr, &lobyte[length+2], 4);
                if (gd3d->swapbytes) {
                    swap2(&numoverflows);
                    swap4(&overflowaddr);
                }
                fseek(gd3d->CubeFile3d, (long)k+(long)(overflowaddr*4), SEEK_SET);
                if (!fread(overflows,4*numoverflows,1,gd3d->CubeFile3d)) {
                    printf("\007  ERROR -- Could not read cube file... aborting.\n");
                    exit(-1);
                }
                for (x=0; x<numoverflows; x++) {
                    if (gd3d->swapbytes) {
                        swap2(&overflows[x][0]);
                        swap2(&overflows[x][1]);
                    }
                    /*  ACKK!! A bug!     DCR   3-April-1998
           *	  spec_ret[overflows[x][0]] += (int)overflows[x][1];
           */
                    spec_ret[overflows[x][0]-1] += (int)overflows[x][1];
                }
            }
        }
        return 0;
    }

    /* remember the data are organised as minicubes, 8 x 8 x 4 chs */
    for (zmc = lim[1][0]>>3; zmc<=lim[1][1]>>3; zmc++) {
        zlo = zmc*8;
        zhi = zlo+7;
        if (zlo<lim[1][0])
            zlo = lim[1][0];
        if (zhi>lim[1][1])
            zhi = lim[1][1];

        for (ymc = lim[0][0]>>3; ymc<=lim[0][1]>>3; ymc++) {
            ylo = ymc*8;
            yhi = ylo+7;
            if (ylo<lim[0][0])
                ylo = lim[0][0];
            if (yhi>lim[0][1])
                yhi = lim[0][1];

            if (ylo>zlo) {
                y1 = zlo;
                y2 = zhi;
                z1 = ylo;
                z2 = yhi;
            }
            else{
                y1 = ylo;
                y2 = yhi;
                z1 = zlo;
                z2 = zhi;
            }
            /* (y,z) gate is now ((y1,y2),(z1,z2)) */
            yy1 = y1&7;
            yy2 = y2&7;
            zz1 = z1&7;
            zz2 = z2&7;

            if (gd3d->cubetype==1) {  /* compressed 1/2 cube */
                /* j = first minicube to read */
                j = ((y1/8) + ((z1/8)*((z1/8)+1))/2) * ((length+3)/4);
                for (x1=0; x1<length; x1+=8) {
                    getmc3d(j++,mc[0][0]);
                    getmc3d(j++,mc[4][0]);
                    for (z=zz1; z<=zz2; z++) {
                        for (y=yy1; y<=yy2; y++) {
                            for (x=0; x<8; x++)
                                spec_ret[x1+x] += (int)mc[x][z][y];
                        }
                    }
                    if (y2>=z1) {
                        for (z=yy1; z<=yy2; z++) {
                            for (y=zz1; y<z; y++) {
                                for (x=0; x<8; x++)
                                    spec_ret[x1+x] += (int)mc[x][z][y];
                            }
                        }
                    }
                }
            }

            else if (gd3d->cubetype==0) { /* compressed 1/6 cube */
                /* read gate for x <= y */
                for (x1=0; x1<=y1; x1+=8) {
                    mcnum = LUMC3D(x1,y1,z1);
                    getmc3d(mcnum, mc[0][0]);
                    getmc3d(mcnum+1, mc[4][0]);
                    for (z=zz1; z<=zz2; z++) {
                        for (y=yy1; y<=yy2; y++) {
                            for (x=0; x<8; x++)
                                spec_ret[x1+x] += mc[z][y][x];
                        }
                    }
                    if (y2>=z1) {
                        for (z=yy1; z<=yy2; z++) {
                            for (y=zz1; y<=z; y++) {
                                for (x=0; x<8; x++)
                                    spec_ret[x1+x] += mc[z][y][x];
                            }
                        }
                    }
                }

                /* read gate for y <= x <= z */
                for (x1=(y1>>3)<<3; x1<=z1; x1+=8) {
                    mcnum = LUMC3D(y1,x1,z1);
                    getmc3d(mcnum, mc[0][0]);
                    getmc3d(mcnum+1, mc[4][0]);
                    for (z=zz1; z<=zz2; z++) {
                        for (y=yy1; y<=yy2; y++) {
                            for (x=0; x<8; x++)
                                spec_ret[x1+x] += mc[z][x][y];
                        }
                    }
                    if (y2>=z1) {
                        for (z=yy1; z<=yy2; z++) {
                            for (y=zz1; y<=z; y++) {
                                for (x=0; x<8; x++)
                                    spec_ret[x1+x] += mc[z][x][y];
                            }
                        }
                    }
                }

                /* read gate for z <= x */
                for (x1=(z1>>3)<<3; x1<length; x1+=8) {
                    mcnum = LUMC3D(y1,z1,x1);
                    getmc3d(mcnum, mc[0][0]);
                    getmc3d(mcnum+1, mc[4][0]);
                    for (z=zz1; z<=zz2; z++) {
                        for (y=yy1; y<=yy2; y++) {
                            for (x=0; x<8; x++)
                                spec_ret[x1+x] += mc[x][z][y];
                        }
                    }
                    if (y2>=z1) {
                        for (z=yy1; z<=yy2; z++) {
                            for (y=zz1; y<=z; y++) {
                                for (x=0; x<8; x++)
                                    spec_ret[x1+x] += mc[x][z][y];
                            }
                        }
                    }
                }
            }

#define GETMC2() \
    if ((fseek(gd3d->CubeFile3d, (long)(mcnum+2)*512, SEEK_SET)) || \
    (!fread(mc2[0][0],1024,1,gd3d->CubeFile3d))) { \
    printf("\007 ERROR -- Could not read rec %d... aborting.\n", mcnum/2); \
    exit(-1); \
        } \
    if (gd3d->swapbytes) { \
    for (i=0; i<512; i++) \
    swap2(mc2[0][0]+i); \
        }

            else if (gd3d->cubetype==2) { /* uncompressed 1/6 cube */
                /* read gate for x <= y */
                for (x1=0; x1<=y1; x1+=8) {
                    mcnum = LUMC3D(x1,y1,z1);
                    GETMC2()
                            for (z=zz1; z<=zz2; z++) {
                        for (y=yy1; y<=yy2; y++) {
                            for (x=0; x<8; x++)
                                spec_ret[x1+x] += mc2[z][y][x];
                        }
                    }
                    if (y2>=z1) {
                        for (z=yy1; z<=yy2; z++) {
                            for (y=zz1; y<=z; y++) {
                                for (x=0; x<8; x++)
                                    spec_ret[x1+x] += mc2[z][y][x];
                            }
                        }
                    }
                }

                /* read gate for y <= x <= z */
                for (x1=(y1>>3)<<3; x1<=z1; x1+=8) {
                    mcnum = LUMC3D(y1,x1,z1);
                    GETMC2()
                            for (z=zz1; z<=zz2; z++) {
                        for (y=yy1; y<=yy2; y++) {
                            for (x=0; x<8; x++)
                                spec_ret[x1+x] += mc2[z][x][y];
                        }
                    }
                    if (y2>=z1) {
                        for (z=yy1; z<=yy2; z++) {
                            for (y=zz1; y<=z; y++) {
                                for (x=0; x<8; x++)
                                    spec_ret[x1+x] += mc2[z][x][y];
                            }
                        }
                    }
                }

                /* read gate for z <= x */
                for (x1=(z1>>3)<<3; x1<length; x1+=8) {
                    mcnum = LUMC3D(y1,z1,x1);
                    GETMC2()
                            for (z=zz1; z<=zz2; z++) {
                        for (y=yy1; y<=yy2; y++) {
                            for (x=0; x<8; x++)
                                spec_ret[x1+x] += mc2[x][z][y];
                        }
                    }
                    if (y2>=z1) {
                        for (z=yy1; z<=yy2; z++) {
                            for (y=zz1; y<=z; y++) {
                                for (x=0; x<8; x++)
                                    spec_ret[x1+x] += mc2[x][z][y];
                            }
                        }
                    }
                }
            }

        }
    }
    return 0;
}


void CXRadReader::getmc3d(int num, unsigned int mc[256])
{
    int i, skip, mcl, nbytes;
    static Record3D rec;
    static int recnum=0;      /* number of any currently loaded record */
    static int oldnum=0;      /* number of last minicube decompressed, if any */
    static int srnum=0;
    static unsigned char *mcptr;
    unsigned char cbuf[1024];

    if ((!recloaded) ||
            num < rec.h.minmc ||
            num > rec.h.minmc+rec.h.nummc-1) {
        /* no record is loaded, or it does not have the required minicube */
        /* first check to see that the required minicube does not */
        /* lie just ahead of any currently loaded record */
        /* if it does, we can just read ahead in the file, rather than fseeking */
        srnum = recnum/16;
        if ((!recloaded) ||
                num < rec.h.minmc ||
                num > rec.h.minmc+4*rec.h.nummc-1) {

            /* next look to see if it lies outside any current SuperRecord */
            if ((!recloaded) ||
                    num < gd3d->mcnum[srnum] ||
                    num > gd3d->mcnum[srnum+1]) {

                /* search for the record that contains minicube # num */
                /* first search in units of 64MB */
                for (srnum=1024; srnum<=gd3d->numsr; srnum+=1024)
                    if (gd3d->mcnum[srnum] > num) break;

                /* now search in units of 2MB */
                for (srnum-=1024; srnum<=gd3d->numsr; srnum+=32)
                    if (gd3d->mcnum[srnum] > num) break;

                /* now search in units of 1/16 MB */
                for (srnum-=32; srnum<=gd3d->numsr; srnum++)
                    if (gd3d->mcnum[srnum] > num) break;
                srnum--;

                if (num < gd3d->mcnum[srnum] || num > gd3d->mcnum[srnum+1]) {
                    printf("\007  ERROR -- SR search failed!! ...aborting.\n"
                           "  mc minmc maxmc SRnum: %d %d %d %d\n",
                           num, gd3d->mcnum[srnum], gd3d->mcnum[srnum+1], srnum);
                    exit(-1);
                }
            }

            /* we know that the minicube lies in SuperRecord srnum */
            /* take a conservative guess at the correct record number */
            if ((num-gd3d->mcnum[srnum]) < (gd3d->mcnum[srnum+1]-num)) {
                recnum = 16*srnum + 15*(num - gd3d->mcnum[srnum])/
                        (gd3d->mcnum[srnum+1]-gd3d->mcnum[srnum]+1);
            }
            else {
                recnum = 16*(srnum+1) - 17*(gd3d->mcnum[srnum+1] - num)/
                        (gd3d->mcnum[srnum+1]-gd3d->mcnum[srnum]+1);
            }
            if (recnum >= gd3d->numrecs) recnum = gd3d->numrecs - 1;

            while (1) {
                fseek(gd3d->CubeFile3d, ((long)recnum)*4096+1024, SEEK_SET);
                if (!fread(&rec,4096,1,gd3d->CubeFile3d)) {
                    printf("\007  ERROR -- Could not read record %d... aborting.\n"
                           "  mc minmc maxmc SRnum: %d %d %d %d\n", recnum,
                           num, gd3d->mcnum[srnum], gd3d->mcnum[srnum+1], srnum);
                    exit(-1);
                }
                if (gd3d->swapbytes) {
                    swap4(&rec.h.minmc);
                    swap2(&rec.h.nummc);
                    swap2(&rec.h.offset);
                }
                /* printf("record number %d seeked and read...\n",recnum); */
                if (num >= rec.h.minmc) break;
                recnum--;
                /* printf(" Ooops, seeking backwards, record number %d...\n",recnum); */
            }
        }

        /* if we're not quite there yet, read forward in the file */
        while (num > rec.h.minmc+rec.h.nummc-1) {
            recnum++;
            if (!fread(&rec,4096,1,gd3d->CubeFile3d)) {
                printf("\007  ERROR -- Could not read record %d... "
                       "aborting.\n", recnum);
                exit(-1);
            }
            if (gd3d->swapbytes) {
                swap4(&rec.h.minmc);
                swap2(&rec.h.nummc);
                swap2(&rec.h.offset);
            }
            /* printf("record number %d read...\n",recnum); */
        }

        recloaded=1;
        mcptr = (unsigned char *)(&rec) + rec.h.offset;
        oldnum = rec.h.minmc;
    }

    /* ok, by now we've loaded the rec with our mc */
    if (num < oldnum) {
        mcptr = (unsigned char *)(&rec) + rec.h.offset;
        oldnum = rec.h.minmc;
    }
    skip = num - oldnum;
    for (i=0; i<skip; i++)
        mcptr += MCLEN(mcptr);
    if (mcptr > (unsigned char *)(&rec)+4096 ) {
        printf("\007  ERROR -- mcptr outside record!! ...aborting.\n"
               "  mc SRnum minmc nummc recnum mcptr(-rec): %d %d %d %d %d %ld\n",
               num, srnum, rec.h.minmc, rec.h.nummc, recnum,
               mcptr-(unsigned char *)(&rec) );
        exit(-1);
    }

    oldnum = num;
    mcl=MCLEN(mcptr);
    if (mcl<1 || mcl>1024) {
        printf("Ack!! mclength = %d!\n"
               "  mc SRnum minmc nummc recnum mcptr(-rec): %d %d %d %d %d %ld\n",
               mcl, num, srnum, rec.h.minmc, rec.h.nummc, recnum,
               mcptr-(unsigned char *)(&rec) );
        printf("  rec.h.offset = %d\n",rec.h.offset);
        mcptr = (unsigned char *)(&rec) + rec.h.offset;
        for (i=rec.h.minmc; i<=num; i++) {
            printf("mc, mcl, head: %d %d %d %d\n",i,MCLEN(mcptr),*mcptr,*(mcptr+1));
            mcptr += MCLEN(mcptr);
        }
        exit(-1);
    }

    if (mcptr + mcl > (unsigned char *)(&rec)+4096 ) {
        /* minicube spills over to next record */
        /* copy first part of minicube into cbuf */
        nbytes = (unsigned char *)(&rec) + 4096 - mcptr;
        memcpy (cbuf, mcptr, nbytes);

        /* now read in next record */
        recnum++;
        if (!fread(&rec,4096,1,gd3d->CubeFile3d)) {
            printf("\007  ERROR -- Could not read record %d... "
                   "aborting.\n", recnum);
            exit(-1);
        }
        if (gd3d->swapbytes) {
            swap4(&rec.h.minmc);
            swap2(&rec.h.nummc);
            swap2(&rec.h.offset);
        }
        /* printf("record number %d read...\n",recnum); */

        /* now copy second part of minicube to cbuf */
        memcpy ((cbuf+nbytes), (char *)(&rec)+8, mcl-nbytes);
        decompress3d (cbuf, mc);

        /* lastly fix up mcptr in case next call is for a mc in the same record */
        mcptr = (unsigned char *)(&rec) + rec.h.offset;
        oldnum = rec.h.minmc;
    }
    else
        decompress3d (mcptr,mc);
}

void CXRadReader::decompress3d (unsigned char in[1024], unsigned int out[256])
{
    static int bitmask[] = { 0,1,3,7,15,31,63,127,255,511,1023,2047,
                             4095,8191,16383,32767 };
    int nplanes, noverflows, nbits, savenplanes;
    unsigned char *bitptr;
    int i, j, t;

    nplanes = in[0] >> 5;
    noverflows = in[0] & 31;
    if (nplanes == 7) {
        nplanes += in[1];
        bitptr = in+2;
    }
    else {
        bitptr = in+1;
    }
    /* printf("%d %d %d\n",nplanes,noverflows,*bitptr); */

    /* extract bit planes */
    savenplanes = nplanes;
    for (i=0; i<256; i++)
        out[i] = 0;
    j = 0;
    while (nplanes>=8) {
        for (i=0; i<256; i++)
            out[i] += ((unsigned int)*bitptr++)<<j;
        nplanes -= 8;
        j += 8;
    }

    if (nplanes > 0) {
        nbits = 0;
        for (i=0; i<256; i++) {
            if (nbits+nplanes < 9) {
                out[i] += ((*bitptr >> (8-nbits-nplanes)) & bitmask[nplanes])<<j;
                nbits += nplanes;
                if (nbits > 7) {
                    bitptr++;
                    nbits -= 8;
                }
            }
            else {
                t = nplanes-8+nbits;
                out[i] += (((*bitptr & bitmask[8-nbits]) << t)
                        + (*(bitptr+1) >> (8-t)))<<j;
                bitptr++;
                nbits = t;
            }
        }
    }

    /* extract overflows */
    for (i=0; i<noverflows; i++)
        out[*bitptr++] += 1 << savenplanes;
}

/* ======================================================================= */
int CXRadReader::autobkgnd(float Xmin, float XMax)
{
    /* maxspec needs to be modified since gfexec is called recursively
     and forgets the number of displayed spectra
     find background over +/- w2*FWHM */

    float fwhm, x, x1, y1, x2, y2, bg, r1;
    int   i, hi, lo, jch, ihi=0, ilo=0, ihi2, ilo2;


    for (int ich = 0; ich < xxgd.numchs; ++ich) {
        xxgd.tmpspec[ich] = xxgd.bspec[0][ich] / xxgd.ewid_sp[ich];
        //        cout<<xxgd.bspec[0][ich]<<" "<<xxgd.ewid_sp[ich]<<endl;
    }

    if(Xmin<0 && XMax<0) {
        ilo = 0;
        ihi = xxgd.numchs-1;
    }
    else {
        ilo = hTotalProj->GetXaxis()->FindBin(Xmin)-1;
        ihi = hTotalProj->GetXaxis()->FindBin(XMax)-1;
    }

    ilo2 = ihi;
    ihi2 = ilo;

    /* do three background-smoothing-type iterations */
    for (i = 1; i <= 3; ++i) {
        for (int ich = ilo + 2; ich < ihi - 2; ich += 5) {
            x = (float) ich;
            fwhm = sqrt(fFWHMPars[0] + fFWHMPars[1] * x + fFWHMPars[2] * x * x);
            r1 = fBGLenghtFactor * fDefaultWidth * fwhm;
            lo = ich - rint(r1);
            r1 = fBGLenghtFactor * fDefaultWidth * fwhm;
            hi = ich + rint(r1);
            if (lo < ilo) lo = ilo;
            if (hi > ihi) hi = ihi;
            find_bg2(lo, hi, &x1, &y1, &x2, &y2);
            if (ilo2 > rint(x1)) ilo2 = rint(x1);
            if (ihi2 < rint(x2)) ihi2 = rint(x2);
            for (jch = rint(x1); jch <= rint(x2); ++jch) {
                x = (float) jch;
                bg = y1 - (x1 - x) * (y1 - y2) / (x1 - x2);
                xxgd.tmpspec[jch] = bg;
            }
        }
    }

    for (int ich = ilo2; ich < ihi2; ++ich) {
        xxgd.tmpspec[ich] += sqrt(xxgd.tmpspec[ich]) * 1.6f;
    }

    for (int ich = 0; ich < xxgd.numchs; ++ich) xxgd.bspec[2][ich] = xxgd.tmpspec[ich] * xxgd.ewid_sp[ich];

    float fsum1;
    float bspec2[4][MAXCHS];

    xxgd.le2pro2d = 0;

    fsum1 = 0.0f;
    for (int i = 0; i < xxgd.numchs; ++i) {
        fsum1 += xxgd.bspec[0][i];
        bspec2[0][i] = xxgd.bspec[0][i] - xxgd.bspec[2][i];
    }
    for (int j = 0; j < xxgd.numchs; ++j) {
        xxgd.background[j] = xxgd.bspec[2][j];
        bspec2[1][j] = bspec2[2][j] = bspec2[3][j] = 0.0f;
        xxgd.bspec[2][j] /= fsum1;
    }

    for (int i = 0; i < xxgd.numchs; ++i) {
        xxgd.bspec[1][i] = xxgd.bspec[2][i];
        xxgd.bspec[2][i] = bspec2[0][i];
        xxgd.bspec[3][i] = bspec2[1][i];
        xxgd.bspec[4][i] = bspec2[2][i];
        xxgd.bspec[5][i] = bspec2[3][i];
    }

    gbash_color->InfoMessage("Background automatically generated with standard parameters");

    return 0;

} /* autobkgnd */


/* ======================================================================= */
int CXRadReader::find_bg2(int loch, int hich, float *x1, float *y1, float *x2, float *y2)
{
    float a, y;
    int   i, mid;

    mid = (loch + hich) / 2;

    /* find channel with least counts on each side of middle */
    *y1 = (xxgd.tmpspec[loch] + xxgd.tmpspec[loch + 1] + xxgd.tmpspec[loch + 2]) / 3.f;
    *x1 = (float) (loch + 1);
    for (i = loch + 2; i <= mid - 2; ++i) {
        a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
        if (*y1 > a) {
            *y1 = a;
            *x1 = (float) i;
        }
    }
    *y2 = (xxgd.tmpspec[mid + 2] + xxgd.tmpspec[mid + 3] + xxgd.tmpspec[mid + 4]) / 3.f;
    *x2 = (float) (mid + 3);
    for (i = mid + 4; i <= hich - 1; ++i) {
        a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
        if (*y2 > a) {
            *y2 = a;
            *x2 = (float) i;
        }
    }
    /* check that there are no channels between loch and hich
     that are below the background. */
    if (*y2 > *y1) {
        for (i = rint(*x1) + 1; i <= mid - 2; ++i) {
            y = *y1 - (*x1 - (float) i) * (*y1 - *y2) / (*x1 - *x2);
            a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
            if (y > a) {
                *y1 = a;
                *x1 = (float) i;
            }
        }
        for (i = rint(*x2) + 1; i <= hich - 1; ++i) {
            y = *y1 - (*x1 - (float) i) * (*y1 - *y2) / (*x1 - *x2);
            a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
            if (y > a) {
                *y2 = a;
                *x2 = (float) i;
            }
        }
    } else {
        for (i = rint(*x1) - 1; i >= loch + 1; --i) {
            y = *y1 - (*x1 - (float) i) * (*y1 - *y2) / (*x1 - *x2);
            a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
            if (y > a) {
                *y1 = a;
                *x1 = (float) i;
            }
        }
        for (i = rint(*x2) - 1; i >= mid + 3; --i) {
            y = *y1 - (*x1 - (float) i) * (*y1 - *y2) / (*x1 - *x2);
            a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
            if (y > a) {
                *y2 = a;
                *x2 = (float) i;
            }
        }
    }
    if (*y1 < 1.f) *y1 = 1.f;
    if (*y2 < 1.f) *y2 = 1.f;
    return 0;
} /* find_bg2 */
