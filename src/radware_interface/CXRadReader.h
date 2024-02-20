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

#ifndef CXRadReader_H
#define CXRadReader_H

#include <vector>

#include "TString.h"

#include "CXRadInterface.h"

using namespace std;

class TH2;
class TH1;
class TH1D;
class TF1;

class CXRadReader
{
private:

    TString fName = "";

    TString fCubeFileName       = "None";
    TString f2DProjFileName     = "None";
    TString fLUTFileName        = "None";
    TString fECalFileName       = "None";
    TString fEffFileName        = "None";
    TString fTotalProjFileName  = "None";
    TString fBackgroundFileName = "None";

    //compression factor
    float fCompFactor           = 1;

    // Calibs
    double fGains[6];
    double feff_pars[10];

    // basic histograms
    TH1 *hLUTHist      = nullptr;
    TH1D *hTotalProj   = nullptr;
    TH1 *hBackground    = nullptr;
    TH1 *h2DProj       = nullptr;

    // calib functions
    TF1 *fECalibFunc    = nullptr;
    TF1 *fEffFunc       = nullptr;

    // Gated spectra
    TH1 *fGatedSpectrum   = nullptr;
    TH1 *fGatedSpectrumNoBG   = nullptr;

    // Binning of histograms
    float edges[RW_MAXCH+1];

    float fBGLenghtFactor = 1.;
    float fDefaultWidth = 6.;
    float fFWHMPars[3] = {9.0,0.004,0.0};

    CommonBlock xxgd;

public:
    CXRadReader();
    ~CXRadReader();

    TString GetName(){return fName;}

    TString GetCubeFileName(){return fCubeFileName;}
    TString Get2DProjFileName(){return f2DProjFileName;}
    TString GetLUTFileName(){return fLUTFileName;}
    TString GetECalFileName(){return fECalFileName;}
    TString GetEffFileName(){return fEffFileName;}
    TString GetTotalProjFileName(){return fTotalProjFileName;}
    TString GetBackgroundFileName(){return fBackgroundFileName;}
    float   GetCompFactor(){return fCompFactor;}


    // Read Cube conf
    Int_t ReadCube(const TString &FileName);
    Int_t ReadTabFile(const TString &FileName);
    Int_t ReadTotProj(const TString &FileName);
    Int_t ReadBackground(const TString &FileName);
    Int_t Read2DProj(const TString &FileName);
    Int_t ReadCalibs(const TString &EnergyFileName, const TString &EfficiencyFileName, float CompFactor);

    Int_t ReadGG(TH2 *h);

    Int_t AutoBuildProj(TString FileName, Int_t Mode);

    // Handle histos
    void BuildHistos();
    TH1 *GetLUTSpectrum(){return hLUTHist;}
    TH1D *GetTotalProjection(){return hTotalProj;}
    TH1 *GetBackground(){return hBackground;}
    TH1 *Get2DProj(){return h2DProj;}

    TF1 *GetECalibFunc(){return fECalibFunc;}
    TF1 *GetEEffFunc(){return fEffFunc;}

    TH1 *GetHistFromSpeFile(const TString &filename);
    TH2 *GetHistFrom2dpFile(const TString &filename);

    void SaveBackground(const TString &filename);

    double EffFuncFormula(double *x, double *p);

    void ReEvalBackground(float Xmin, float XMax, float LenghtFactor, float Width, float FWHM_0, float FWHM_1, float FWHM_2);

    TH1 *Project(const vector<pair<float, float> > &gatesX, const vector<pair<float, float> > &gatesY, Bool_t BGSubtract = true);
    int Project3D(vector< pair<float, float> > gatesX, vector< pair<float, float> > gatesY);
    int Project2D(vector< pair<float, float> > gates);



    //******************************* RW util.c Stuffs ***********************************//

private:

    /* Some global data */

    int lumx3d[RW_MAXCH+16]; /* look-up table, maps 3d ch to linear minicube */
    int lumy3d[RW_MAXCH+16];
    int lumz3d[RW_MAXCH+16];

    /* look up minicube addr */
#define LUMC3D(x,y,z) lumx3d[x] + lumy3d[y] + lumz3d[z]

    /* stuff added for linear combinations of cubes: */
    CubeData3D gd3dn[6], *gd3d = &gd3dn[0];
    int recloaded;

public:

    int autobkgnd(float Xmin=-1, float XMax=-1);

private:

    int open3dfile(TString &name, int *numchs_ret); /* returns 1 on error */
    int close3dfile(void);

    //! read a radware 1D spectrum
    int read_spe_file(const TString &filnam, float *sp, TString &namesp, int *numch, int idimsp);

    int read_cal_file(TString &filnam, char *title, int *iorder, double *gain);
    int read_eff_file(TString &filnam, char *title, double *pars_d);
    void setcubenum(int cubenum);

    int add_gate(float elo, float ehi);
    int add_trip_gate(float elo1, float ehi1, float elo2, float ehi2);
    void getmc3d(int num, unsigned int mc[256]);
    int read3dcube(int lo1, int hi1, int lo2, int hi2, int *spec_ret);
    int read_cube(int ilo1, int ihi1, int ilo2, int ihi2);
    void decompress3d (unsigned char in[1024], unsigned int out[256]);
    int find_bg2(int loch, int hich, float *x1, float *y1, float *x2, float *y2);
};

#endif
