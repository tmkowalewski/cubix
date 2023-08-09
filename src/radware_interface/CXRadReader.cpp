#include "CXRadReader.h"

#include <sys/stat.h>

#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TSystem.h"
#include "TObjArray.h"

#include "CXBashColor.h"

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

    fName = FileName.Copy().ReplaceAll(".cub","");

    strcpy(xxgd.cubnam,FileName.Data());
    if (open3dfile(xxgd.cubnam, &xxgd.numchs)) {
        gbash_color->ErrorMessage(Form("%s not readable", xxgd.cubnam));
        return 1;
    }
    if (xxgd.numchs > MAXCHS){
        gbash_color->ErrorMessage(Form("Cube file has too many channels (%d), MAXCHS is %d  ==> EXIT", xxgd.numchs ,MAXCHS));
        return 1;
    }

    gbash_color->InfoMessage(Form("%s loaded (%d channels)",xxgd.cubnam, xxgd.numchs));

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

        char  filnam[800];
        strcpy(filnam,FileName.Data());

        if (read_tab_file(filnam, &xxgd.nclook, &xxgd.lookmin, &xxgd.lookmax, xxgd.looktab, 16384)) {
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
        char  filnam[800], namesp[16];
        xxgd.le2pro2d = 0;

        strcpy(filnam,FileName.Data());
        if (read_spe_file(filnam, &xxgd.bspec[0][0], namesp, &nch, MAXCHS)) {
            gbash_color->ErrorMessage(Form("%s not readable ==> EXIT",filnam));
            return 1;
        }
        if (nch != xxgd.numchs){
            gbash_color->ErrorMessage(Form("%s spectrum lenght (%d) has wrong length ( must be %d) ==> EXIT",filnam, nch, xxgd.numchs));
            return 1;
        }

        gbash_color->InfoMessage(Form("Total projection file loaded: %s",filnam));
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

    gbash_color->InfoMessage("Exporting radware spectrum: " + filename + " in ROOT histogram");

    TObjArray *arr = filename.Tokenize("/");
    TString name = arr->Last()->GetName();
    delete arr;

    double spec[MAXCHS];
    char  filnam[800],namesp[800];
    int   nch;
    strcpy(filnam,filename.Data());
    if (read_spe_file(filnam, &spec[0], namesp, &nch, MAXCHS)) {
        gbash_color->ErrorMessage(Form("%s not readable ==> EXIT",filnam));
        return nullptr;
    }

    TH1F *h = new TH1F(name,name,nch,0,nch);
    for(int i=0 ; i<nch ; i++) {
        h->SetBinContent(i+1,spec[i]);
    }

    return h;

    return nullptr;
}

void CXRadReader::SaveBackground(const TString &filename)
{
    char  filnam[800];
    strcpy(filnam,filename.Data());

    wspec(filnam, xxgd.background, xxgd.numchs);
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

        strcpy(filnam,FileName.Data());
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

        double_t eg = (xxgd.elo_sp[i] + xxgd.ehi_sp[i]) / 2.0F;
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

void CXRadReader::ReEvalBackground(double_t Xmin, double_t XMax, double_t LenghtFactor, double_t Width, double_t FWHM_0, double_t FWHM_1, double_t FWHM_2)
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

        double fsum1;
        double bspec2[4][MAXCHS];
        int   nch;
        char  filnam[800], namesp[16];
        xxgd.le2pro2d = 0;

        strcpy(filnam,FileName.Data());
        if (read_spe_file(filnam, &xxgd.bspec[2][0], namesp, &nch, MAXCHS)) {
            gbash_color->ErrorMessage(Form("%s not readable ==> EXIT",filnam));
            return 1;
        }
        if (nch != xxgd.numchs){
            gbash_color->ErrorMessage(Form("%s spectrum lenght (%d) has wrong length ( must be %d) ==> EXIT",filnam,nch,xxgd.numchs));
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

        gbash_color->InfoMessage(Form("Background file loaded: %s",filnam));

        fBackgroundFileName =FileName;
    }

    return 0;
}

Int_t CXRadReader::ReadCalibs(const TString &EnergyFileName, const TString &EfficiencyFileName, double_t CompFactor)
{
    double f1, f2, x1, x2, x3;
    double  x, eg, eff;
    int    i, j, jj, iorder, nterms;
    char   title[800], filnam[800];

    /* get energy calibration */

    bool readcal = false;
    if(gSystem->IsFileInIncludePath(EnergyFileName)){
        gbash_color->InfoMessage("Calibration file: " + EnergyFileName  + " found");
        readcal = true;
    }
    else
        gbash_color->InfoMessage("No Calibration file set or found, no energy calibration applied");

    if(readcal){
        strcpy(filnam,EnergyFileName.Data());
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
        strcpy(filnam,EfficiencyFileName.Data());
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
//            x = ((double) i - 0.5f);
            x = ((double) i);
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
        x = ((double) xxgd.nclook - 1.0);
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
    fEffFunc = new TF1("EfficiencyFunc",this,&CXRadReader::EffFuncFormula,0,xxgd.nclook,10,"CXRadReader","EffFuncFormula");
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
    double EG = x[0];
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

    double A=p[0];
    double B=p[1];
    double C=p[2];

    //high energy
    double D=p[3];
    double E=p[4];
    double F=p[5];

    // interaction parameter between the two regions
    // the larger G is, the sharper will be the turnover at the top, between the two curves.
    // If the efficiency turns over gently, G will be small.
    double G=p[6];


    double E1=p[7]; //100 keV
    double E2=p[8]; //1000 keV

    double x1 = log(EG / E1);
    double x2 = log(EG / E2);

    double f1 = A + B*x1 + C*x1*x1;
    double f2 = D + E*x2 + F*x2*x2;

    if (f1 <= 0. || f2 <= 0.)
        eff = 1.0;
    else {
        double x3 = exp(-G * log(f1)) + exp(-G * log(f2));

        if (x3 <= 0.) eff = 1.0;
        else eff = exp(exp(-log(x3) / G));
    }

    return eff;
}

TH1 *CXRadReader::Project(const vector< pair<double, double> > &gatesX, const vector< pair<double, double> > &gatesY, Bool_t BGSubtract)
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

int CXRadReader::Project2D(vector< pair<double, double> > gates)
{
    /* read gate of energy elo to ehi and calculate expected spectrum */
    /* spec[0][] = background-subtracted gate */
    /* spec[1][] = calculated spectrum from gammas outside gate */
    /* spec[2][] = calculated spectrum from all gammas */
    /* spec[3][] = non-background-subtracted gate */
    /* spec[4][] = square of statistical uncertainty */
    /* spec[5][] = square of statistical plus systematic uncertainties */

    strcpy(xxgd.old_name_gat, xxgd.name_gat);

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

    TString Name = "Rad2D_";
    if(fName != "") Name = "RadCube_";

    for(auto i=0U ; i<gates.size() ; i++){

        double elo = gates[i].first;
        double ehi = gates[i].second;

        Name += Form("%s%.1f(%.1f)", (i==0) ? "" : "+", (ehi+elo)*0.5,(ehi-elo)*0.5);

        add_gate(elo, ehi);
    }

    snprintf(xxgd.name_gat, sizeof(xxgd.name_gat), "%s", Name.Data());

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
    fGatedSpectrum->SetNameTitle(Name,Name);

    delete fGatedSpectrumNoBG;
    fGatedSpectrumNoBG = dynamic_cast<TH1*>(hTotalProj->Clone());
    fGatedSpectrumNoBG->Reset();
    fGatedSpectrumNoBG->SetXTitle("Energy (keV)");
    fGatedSpectrumNoBG->SetYTitle(Form("Counts (%g keV/bin)",fGatedSpectrum->GetBinWidth(1)));
    fGatedSpectrum->GetXaxis()->SetTitleOffset(1.2);
    fGatedSpectrum->GetYaxis()->SetTitleOffset(0.8);
    fGatedSpectrum->SetStats();
    fGatedSpectrumNoBG->SetNameTitle(Form("%s_NoBG",Name.Data()),Form("%s_NoBG",Name.Data()));

    for (int i = 0; i < xxgd.numchs; ++i) {
        fGatedSpectrum->SetBinContent(i+1,xxgd.spec[0][i] * xxgd.ewid_sp[0]);
        fGatedSpectrumNoBG->SetBinContent(i+1,xxgd.spec[3][i] * xxgd.ewid_sp[0]);
    }

    return 0;
}


int CXRadReader::Project3D(vector< pair<double, double> > gatesX, vector< pair<double, double> > gatesY)
{
    strcpy(xxgd.old_name_gat, xxgd.name_gat);

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

        double eloX = gatesX[i].first;
        double ehiX = gatesX[i].second;

        NameX += Form("%s%.1f(%.1f)", (i==0) ? "" : "+",(eloX+ehiX)*0.5,TMath::Abs(eloX-ehiX)*0.5);

        for(auto j=0U ; j<gatesY.size() ; j++) {

            double eloY = gatesY[j].first;
            double ehiY = gatesY[j].second;

            NameY += Form("%s%.1f(%.1f)", (j==0) ? "" : "+",(eloY+ehiY)*0.5,TMath::Abs(eloY-ehiY)*0.5);

            ++nsum;
            add_trip_gate(eloX, ehiX, eloY, ehiY);
        }
    }

    snprintf(xxgd.name_gat, sizeof(xxgd.name_gat), "%s/%s", NameX.Data(), NameY.Data());

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

    TString Name = Form("RadCube_%s/%s", NameX.Data(), NameY.Data());

    fGatedSpectrum->Reset();
    fGatedSpectrum->SetNameTitle(Name,Name);

    fGatedSpectrumNoBG->Reset();
    fGatedSpectrumNoBG->SetNameTitle(Form("%s_NoBG",Name.Data()),Form("%s_NoBG",Name.Data()));

    for (int i = 0; i < xxgd.numchs; ++i) {
        fGatedSpectrum->SetBinContent(i+1,xxgd.spec[0][i] * xxgd.ewid_sp[0]);
        fGatedSpectrumNoBG->SetBinContent(i+1,xxgd.spec[3][i] * xxgd.ewid_sp[0]);
    }

    return 0;
}

Int_t CXRadReader::AutoBuildProj(TString FileName, Int_t Mode)
{
    int length = gd3d->length;
    char outnam[256];
    unsigned int mc[512], *data2d;
    unsigned short mc2[512];
    int i, j, k, x, y, z, ix, iy, iz;
    int ax[512], ay[512], az[512];
    double data1d[RW_MAXCH];

    unsigned char lobyte[RW_MAXCH+6];
    unsigned short overflows[RW_MAXCH][2], numoverflows;
    int overflowaddr;

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
    else
        return 1;

    if (gd3d->cubetype==0) {  /* compressed 1/6 cube */
        for (iz=0; iz<length; iz+=8) {
            for (iy=0; iy<=iz; iy+=8) {
                for (ix=0; ix<=iy; ix+=8) {
                    j = LUMC3D(ix,iy,iz);
                    getmc3d(j,mc);
                    getmc3d(j+1,&mc[256]);
                    if ((j&511)==0) {
                        cout << Form("\rx y z: %4d %4d %4d",ix,iy,iz);
                    }
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
                    if ((j&511)==0) {
                        cout << Form("\rx y z: %4d %4d %4d",ix,iy,iz);
                    }
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
                    if (((j++)&255)==0) {
                        cout << Form("\rx y z: %4d %4d %4d",ix,iy,iz);
                    }
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
            cout << Form("\r z: %4d ", iz);
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

    if(Mode == 2) {
        TString nameout = FileName.ReplaceAll(".cub",".2dp");
        strcpy(outnam,nameout.Data());

        file=fopen(outnam,"w+");

        for (y=0; y<length; y++) {
            for (x=0; x<=y; x++) {
                data1d[x] = (double)(*(data2d + y*length + x));
            }
            data1d[y] = 2.0*data1d[y];
            for (x=y+1; x<length; x++) {
                data1d[x] = (double)(*(data2d + x*length + y));
            }
            if (!(fwrite(data1d, 4*length, 1, file))) {
                gbash_color->ErrorMessage("Cannot write 2d file, aborting...");
                return 1;
            }
        }
        fclose(file);
    }
    if(Mode == 1) {
        TString nameout = FileName.ReplaceAll(".cub",".spe");
        strcpy(outnam,nameout.Data());

        for (y=0; y<length; y++) {
            data1d[y] = 0.0;
            for (x=0; x<=y; x++) {
                data1d[x] += (double)(*(data2d + y*length + x));
                data1d[y] += (double)(*(data2d + y*length + x));
            }
        }

        wspec(outnam, data1d, length);
    }
    cout<<endl;
    return 0;
}

//******************************* RW util.c Stuffs ***********************************//

/* ======================================================================= */
int CXRadReader::read_tab_file(char *filnam, int *nclook, int *lookmin, int *lookmax, short *looktab, int dimlook)
{
    /* read lookup table from .tab file = filnam
     into array looktab of dimension dimlook
     Nclook = number of channels read
     lookmin, lookmax = min/max values in lookup table
     returns 1 for open/read error
     default file extension = .tab */

    char cbuf[800];
    int  j, rl, swap = -1;
    FILE *file;

    setext(filnam, ".tab", 800);

    /* OPEN (ILU,FILE=FILNAM,FORM='UNFORMATTED',STATUS='OLD')
     READ (ILU) NCLOOK,LOOKMIN,LOOKMAX
     READ (ILU) (LOOKTAB(I),I=1,NCLOOK) */

    if (!(file = open_readonly(filnam))) return 1;
    rl = get_file_rec(file, cbuf, 800, 0);
    if (rl != 12  && rl != -12) {
        file_error("read lookup-table from", filnam);
        fclose(file);
        return 1;
    }
    if (rl < 0) {
        /* file has wrong byte order - swap bytes */
        cout<<"*** Swapping bytes read from file " << filnam <<endl;
        swap = 1;
        for (j = 0; j < 3; ++j) {
            swapb4(cbuf + 4*j);
        }
    }
    memcpy(nclook,  cbuf,     4);
    memcpy(lookmin, cbuf + 4, 4);
    memcpy(lookmax, cbuf + 8, 4);

    if (*nclook < 2 || *nclook > dimlook) {
        file_error("read lookup-table from", filnam);
        fclose(file);
        return 1;
    }

    rl = get_file_rec(file, looktab, 2*dimlook, swap);
    fclose(file);
    if (rl != 2*(*nclook) && rl != -2*(*nclook)) {
        file_error("read lookup-table from", filnam);
        return 1;
    }
    if (rl < 0) {
        /* file has wrong byte order - swap bytes */
        for (j = 0; j < *nclook; ++j) {
            swapb2((char *) (looktab + j));
        }
    }

    return 0;
} /* read_tab_file */


/* ====================================================================== */
int CXRadReader::setext(char *filnam, const char *cext, int filnam_len)
{
    /* set default extension of filename filnam to cext
     leading spaces are first removed from filnam
     if extension is present, it is left unchanged
     if no extension is present, cext is used
     returned value pointer to the dot of the .ext
     cext should include the dot plus a three-letter extension */

    int nc, iext;

    /* remove leading spaces from filnam */
    nc = strlen(filnam);
    if (nc > filnam_len) nc = filnam_len;
    while (nc > 0 && filnam[0] == ' ') {
        memmove(filnam, filnam+1, nc--);
        filnam[nc] = '\0';
    }
    /* remove trailing spaces from filnam */
    while (nc > 0 && filnam[nc-1] == ' ') {
        filnam[--nc] = '\0';
    }
    /* look for file extension in filnam
     if there is none, put it to cext */
    iext = 0;
    if (nc > 0) {
        for (iext = nc-1;
             (iext > 0 &&
              filnam[iext] != ']' &&
              filnam[iext] != '/' &&
              filnam[iext] != ':');
             iext--) {
            if (filnam[iext] == '.') return iext;
        }
        iext = nc;
    }
    strncpy(&filnam[iext], cext, filnam_len - iext - 1);
    return iext;
} /* setext */

/* ======================================================================= */
FILE *CXRadReader::open_readonly(char *filename)
{
    /* open old file for READONLY (if possible)
     filename  = file name
     provided for VMS compatibility */

    FILE *file;
    if (!(file = fopen(filename, "r"))) file_error("open", filename);
    return file;
} /* open_readonly */


/* ======================================================================= */
int CXRadReader::get_file_rec(FILE *fd, void *data, int maxbytes, int swap_bytes)
{
    /* read one fortran-unformatted style binary record into data */
    /* for unix systems, swap_bytes controls how get_file_rec deals with
     swapping the bytes of the record length tags at the start and end
     of the records.  Set swap_bytes to
       0 to try to automatically sense if the bytes need swapping
       1 to force the byte swap, or
      -1 to force no byte swap */
    /* returns number of bytes read in record,
     or number of bytes * -1 if bytes need swapping,
     or 0 for error */
    int  reclen, j1, j2;
    if (fread(&reclen, 4, 1, fd) != 1) return 0;
    if (reclen == 0) return 0;
    j1 = reclen;
    if ((swap_bytes == 1) ||
            (swap_bytes == 0 && reclen >= 65536)) swapb4((char *) &reclen);
    if (reclen > maxbytes) goto ERR1;
    if (fread(data, reclen, 1, fd) != 1 ||
            fread(&j2, 4, 1, fd) != 1) goto ERR2;
    /* if (j1 != j2) goto ERR2; */
    if (reclen == j1) return reclen;
    return (-reclen);

ERR1:
    gbash_color->ErrorMessage(Form("ERROR: record is too big for get_file_rec\n"
                                   "       max size = %d, record size = %d.\n",maxbytes, reclen));
    return 0;
ERR2:
    gbash_color->ErrorMessage(Form("ERROR during read in get_file_rec.\n"));
    return 0;
} /* get_file_rec */


/* ======================================================================= */
int CXRadReader::file_error(const char *error_type, char *filename)
{
    /* write error message */
    /* cannot perform operation error_type on file filename */

    if (strlen(error_type) + strlen(filename) > 58) {
        gbash_color->ErrorMessage(Form("ERROR - cannot %s file\n%s\n", error_type, filename));
    } else {
        gbash_color->ErrorMessage(Form("ERROR - cannot %s file %s\n", error_type, filename));
    }
    return 0;
} /* file_error */

/* ======================================================================= */
void CXRadReader::swapb8(char *buf)
{
    char c;
    c = buf[7]; buf[7] = buf[0]; buf[0] = c;
    c = buf[6]; buf[6] = buf[1]; buf[1] = c;
    c = buf[5]; buf[5] = buf[2]; buf[2] = c;
    c = buf[4]; buf[4] = buf[3]; buf[3] = c;
} /* swapb8 */
void CXRadReader::swapb4(char *buf)
{
    char c;
    c = buf[3]; buf[3] = buf[0]; buf[0] = c;
    c = buf[2]; buf[2] = buf[1]; buf[1] = c;
} /* swapb4 */
void CXRadReader::swapb2(char *buf)
{
    char c;
    c = buf[1]; buf[1] = buf[0]; buf[0] = c;
} /* swapb2 */

int CXRadReader::open3dfile(char *name, int *numchs_ret)
{
    int length, i, nummc, sr;
    FHead3D head;
    Record3D rec;
    char cname[256], *tmp;

    strcpy(cname, name);
    if ((tmp = strchr(cname,' '))) *tmp = 0;

    if (!gd3d) gd3d = &gd3dn[0];

    if (!cname[0] || !(gd3d->CubeFile3d=fopen(cname,"r"))) {
        printf("\007  ERROR -- Cannot open file %s for reading.\n", cname);
        return 1;
    }

#define QUIT close3dfile(); return 1;
    if (!fread(&head,1024,1,gd3d->CubeFile3d)) {
        printf("\007  ERROR -- Cannot read file %s.\n",name);
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
        printf("\007  ERROR -- Invalid header in file %s.\n",name);
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
            printf("\007  ERROR -- Cannot open file %s for writing.\n", name);
            QUIT;

        }
        if (!fwrite(gd3d->mcnum,4*(1+gd3d->numsr),1,gd3d->CubeFile3d)) {
            printf("\007  ERROR -- Could not write SuperRecord list... aborting.\n");
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

void CXRadReader::swap4(int *in)
{
    char *c, tmp;

    c = (char *) in;
    tmp = *c;
    *c = *(c+3);
    *(c+3) = tmp;
    tmp = *(c+1);
    *(c+1) = *(c+2);
    *(c+2) = tmp;
}

void CXRadReader::swap2(unsigned short *in)
{
    char *c, tmp;

    c = (char *) in;
    tmp = *c;
    *c = *(c+1);
    *(c+1) = tmp;
}

int CXRadReader::read_spe_file(char *filnam, double *sp, char *namesp, int *numch, int idimsp)
{
    float *data = new float[idimsp];

    /* read spectrum from .spe file = filnam
     into array sp of dimension idimsp
     numch = number of channels read
     namesp = name of spectrum (character*8)
     returns 1 for open/read error
     default file extension = .spe */

    char cbuf[800];
    int  idim1, idim2, j, rl, swap = -1;
    FILE *file;

    setext(filnam, ".spe", 800);

    /* read spectrum in standard gf3 format
     OPEN(ILU,FILE=FILNAM,FORM='UNFORMATTED',STATUS='OLD')
     READ(ILU) NAMESP, IDIM1, IDIM2, IRED1, IRED2
     READ(ILU,ERR=90) (SP(I),I=1,NUMCH) */
    if (!(file = open_readonly(filnam))) return 1;
    rl = get_file_rec(file, cbuf, 800, 0);
    if (rl != 24  && rl != -24) {
        file_error("read", filnam);
        fclose(file);
        return 1;
    }
    if (rl < 0) {
        /* file has wrong byte order - swap bytes */
        cout<<Form("*** Swapping bytes read from file %s", filnam)<<endl;
        swap = 1;
        for (j = 2; j < 4; ++j) {
            swapb4(cbuf + 4*j);
        }
    }
    strncpy(namesp, cbuf, 8);
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
int CXRadReader::inq_file(char *filename, int *reclen)
{
    /* inquire for file existence and record length in longwords
     returns 0 for file not exists, 1 for file exists */

    int  ext;
    char jfile[800];
    struct stat statbuf;

    *reclen = 0;
    if (stat(filename, &statbuf)) return 0;

    ext = 0;
    strncpy(jfile, filename, 800);
    ext = setext(jfile, "    ", 800);
    if (!strcmp(&jfile[ext], ".mat") ||
            !strcmp(&jfile[ext], ".MAT") ||
            !strcmp(&jfile[ext], ".esc") ||
            !strcmp(&jfile[ext], ".ESC")) {
        *reclen = 2048;
    } else if (!strcmp(&jfile[ext], ".spn") ||
               !strcmp(&jfile[ext], ".SPN") ||
               !strcmp(&jfile[ext], ".m4b") ||
               !strcmp(&jfile[ext], ".M4B") ||
               !strcmp(&jfile[ext], ".e4k") ||
               !strcmp(&jfile[ext], ".E4K")) {
        *reclen = 4096;
    } else if (!strcmp(&jfile[ext], ".cub") ||
               !strcmp(&jfile[ext], ".CUB")) {
        *reclen = 256;
    } else if (!strcmp(&jfile[ext], ".2dp") ||
               !strcmp(&jfile[ext], ".2DP")) {
        if (statbuf.st_size <= 0) {
            *reclen = 0;
        } else {
            *reclen = (int) (0.5 + sqrt((double) (statbuf.st_size/4)));
        }
    }
    return 1;
} /* inq_file */


/* ======================================================================= */
int CXRadReader::read_cal_file(char *filnam, char *title, int *iorder, double *gain)
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
int CXRadReader::read_eff_file(char *filnam, char *title, double *pars)
{
    /* read efficiency parameters from .aef/.eff file = filnam
     into title, pars
     returns 1 for open/read erro
     default file extension = .aef, .eff */

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
                (sscanf(line, "%lf %lf %lf %lf %lf",
                        pars, pars+1, pars+2, pars+3, pars+4) != 5) ||
                !(fgets(line, 120, file)) ||
                (sscanf(line, "%lf %lf %lf %lf %lf",
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
    return 0;
} /* read_eff_file */

void CXRadReader::setcubenum(int cubenum)
{
    gd3d = &gd3dn[cubenum];
    recloaded = 0;
}

/* ======================================================================= */
int CXRadReader::add_gate(double elo, double ehi)
{
    double r1;
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
            if ((double) abs(ix - iy) < xxgd.v_width) {
                r1 = (double) (ix - iy) / xxgd.v_width;
                xxgd.spec[0][ix] += xxgd.v_depth[iy] * (1.0f - r1 * r1);
            }
        }
    }

    return 0;
} /* add_gate */

/* ======================================================================= */
int CXRadReader::add_trip_gate(double elo1, double ehi1, double elo2, double ehi2)
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
            xxgd.spec[0][i] += (double) ispec[i];
            xxgd.spec[3][i] += (double) ispec[i];
        }

    } else {
        /* stuff added for linear combinations of cubes: */
        setcubenum(0);
        read3dcube(ilo1, ihi1, ilo2, ihi2, ispec);
        for (i = 0; i < xxgd.numchs; ++i) {
            xxgd.spec[0][i] += (double) ispec[i];
            xxgd.spec[3][i] += (double) ispec[i];
            xxgd.spec[4][i] += (double) ispec[i] /
                    (xxgd.ewid_sp[i] * xxgd.ewid_sp[i]);
        }

        for (icubenum = 0; icubenum < 5 && xxgd.cubefact[icubenum]; icubenum++) {
            setcubenum(icubenum + 1);
            read3dcube(ilo1, ihi1, ilo2, ihi2, ispec);
            for (i = 0; i < xxgd.numchs; ++i) {
                xxgd.spec[0][i] += xxgd.cubefact[icubenum] * (double) ispec[i];
                xxgd.spec[3][i] += xxgd.cubefact[icubenum] * (double) ispec[i];
                xxgd.spec[4][i] += xxgd.cubefact[icubenum] *
                        xxgd.cubefact[icubenum] * (double) ispec[i] /
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
int CXRadReader::autobkgnd(double_t Xmin, double_t XMax)
{
    /* maxspec needs to be modified since gfexec is called recursively
     and forgets the number of displayed spectra
     find background over +/- w2*FWHM */

    double fwhm, x, x1, y1, x2, y2, bg, r1;
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
            x = (double) ich;
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
                x = (double) jch;
                bg = y1 - (x1 - x) * (y1 - y2) / (x1 - x2);
                xxgd.tmpspec[jch] = bg;
            }
        }
    }

    for (int ich = ilo2; ich < ihi2; ++ich) {
        xxgd.tmpspec[ich] += sqrt(xxgd.tmpspec[ich]) * 1.6f;
    }

    for (int ich = 0; ich < xxgd.numchs; ++ich) xxgd.bspec[2][ich] = xxgd.tmpspec[ich] * xxgd.ewid_sp[ich];

    double fsum1;
    double bspec2[4][MAXCHS];

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
int CXRadReader::find_bg2(int loch, int hich, double *x1, double *y1, double *x2, double *y2)
{
    double a, y;
    int   i, mid;

    mid = (loch + hich) / 2;

    /* find channel with least counts on each side of middle */
    *y1 = (xxgd.tmpspec[loch] + xxgd.tmpspec[loch + 1] + xxgd.tmpspec[loch + 2]) / 3.f;
    *x1 = (double) (loch + 1);
    for (i = loch + 2; i <= mid - 2; ++i) {
        a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
        if (*y1 > a) {
            *y1 = a;
            *x1 = (double) i;
        }
    }
    *y2 = (xxgd.tmpspec[mid + 2] + xxgd.tmpspec[mid + 3] + xxgd.tmpspec[mid + 4]) / 3.f;
    *x2 = (double) (mid + 3);
    for (i = mid + 4; i <= hich - 1; ++i) {
        a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
        if (*y2 > a) {
            *y2 = a;
            *x2 = (double) i;
        }
    }
    /* check that there are no channels between loch and hich
     that are below the background. */
    if (*y2 > *y1) {
        for (i = rint(*x1) + 1; i <= mid - 2; ++i) {
            y = *y1 - (*x1 - (double) i) * (*y1 - *y2) / (*x1 - *x2);
            a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
            if (y > a) {
                *y1 = a;
                *x1 = (double) i;
            }
        }
        for (i = rint(*x2) + 1; i <= hich - 1; ++i) {
            y = *y1 - (*x1 - (double) i) * (*y1 - *y2) / (*x1 - *x2);
            a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
            if (y > a) {
                *y2 = a;
                *x2 = (double) i;
            }
        }
    } else {
        for (i = rint(*x1) - 1; i >= loch + 1; --i) {
            y = *y1 - (*x1 - (double) i) * (*y1 - *y2) / (*x1 - *x2);
            a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
            if (y > a) {
                *y1 = a;
                *x1 = (double) i;
            }
        }
        for (i = rint(*x2) - 1; i >= mid + 3; --i) {
            y = *y1 - (*x1 - (double) i) * (*y1 - *y2) / (*x1 - *x2);
            a = (xxgd.tmpspec[i-1] + xxgd.tmpspec[i] + xxgd.tmpspec[i+1]) / 3.f;
            if (y > a) {
                *y2 = a;
                *x2 = (double) i;
            }
        }
    }
    if (*y1 < 1.f) *y1 = 1.f;
    if (*y2 < 1.f) *y2 = 1.f;
    return 0;
} /* find_bg2 */


/* ======================================================================= */
int CXRadReader::wspec(char *filnam, double *spec, int idim)
{
    /* write spectra in gf3 format
     filnam = name of file to be created and written
     spec = spectrum of length idim */

    char buf[32];
    int  j, c1 = 1, rl = 0;
    char namesp[8];
    FILE *file;

    j = setext(filnam, ".spe", 800);
    if (!(file = open_new_file(filnam, 0))) return 1;
    strncpy(namesp, filnam, 8);
    if (j < 8) memset(&namesp[j], ' ', 8-j);

    /* WRITE(1) NAMESP,IDIM,1,1,1 */
    /* WRITE(1) SPEC */
#define W(a,b) { memcpy(buf + rl, a, b); rl += b; }
    W(namesp,8); W(&idim,4); W(&c1,4); W(&c1,4); W(&c1,4);
#undef W
    if (put_file_rec(file, buf, rl) ||
            put_file_rec(file, spec, 4*idim)) {
        file_error("write to", filnam);
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
} /* wspec */


/* ======================================================================= */
FILE *CXRadReader::open_new_file(char *filename, int /*force_open*/)
{
    /* safely open a new file
     filename: name of file to be opened
     force_open = 0 : allow return value NULL for no file opened
     force_open = 1 : require that a file be opened */

    char tfn[800];
    FILE *file = NULL;

    strncpy(tfn, filename, 800);

    if ((file = fopen(filename, "w+")))
        return file;
    file_error("open or overwrite", filename);
    return nullptr;

} /* open_new_file */

/* ======================================================================= */
int CXRadReader::put_file_rec(FILE *fd, void *data, int numbytes)
{
    /* write one fortran-unformatted style binary record into data */
    /* returns 1 for error */

#ifdef VMS  /* vms */
    int   j1;
    short rh[2];
    char  *buf;

    buf = data;
    j1 = numbytes;
    if (numbytes <= 2042) {
        rh[0] = numbytes + 2; rh[1] = 3;
    } else {
        rh[0] = 2044; rh[1] = 1;
        while (j1 > 2042) {
            if (fwrite(rh, 2, 2, fd) != 2 ||
                    fwrite(buf, 2042, 1, fd) != 1) return 1;
            rh[1] = 0; j1 -= 2042; buf += 2042;
        }
        rh[0] = j1 + 2; rh[1] = 2;
    }
    if (fwrite(rh, 2, 2, fd) != 2 ||
            fwrite(buf, j1, 1, fd) != 1) return 1;
    /* if numbytes is odd, write an extra (padding) byte */
    if (2*(numbytes>>1) != numbytes) {
        j1 = 0;
        fwrite(&j1, 1, 1, fd);
    }

#else /* unix */

    if (fwrite(&numbytes, 4, 1, fd) != 1 ||
            fwrite(data, numbytes, 1, fd) != 1 ||
            fwrite(&numbytes, 4, 1, fd) != 1) return 1;
#endif
    return 0;
} /*put_file_rec */
