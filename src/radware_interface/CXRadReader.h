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
#include <signal.h>

#include "TString.h"

#define RW_MAXCH 16384   /* max number of channels per dimension */
#define MAXCHS 16384    /*    (must be multiple of 8)    */
#define MAXGAM  1500    /* max number of gammas in scheme */

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
    double_t fCompFactor           = 1;

    // Calibs
    double fGains[6];
    double feff_pars[10];

    // basic histograms
    TH1 *hLUTHist      = nullptr;
    TH1D *hTotalProj   = nullptr;
    TH1*hBackground    = nullptr;
    TH2 *h2DProj       = nullptr;

    // calib functions
    TF1 *fECalibFunc    = nullptr;
    TF1 *fEffFunc       = nullptr;

    // Gated spectra
    TH1 *fGatedSpectrum   = nullptr;
    TH1 *fGatedSpectrumNoBG   = nullptr;

    // Binning of histograms
    double_t edges[RW_MAXCH+1];

    double_t fBGLenghtFactor = 1.;
    double_t fDefaultWidth = 6.;
    double_t fFWHMPars[3] = {9.0,0.004,0.0};

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
    double_t   GetCompFactor(){return fCompFactor;}


    // Read Cube conf
    Int_t ReadCube(const TString &FileName);
    Int_t ReadTabFile(const TString &FileName);
    Int_t ReadTotProj(const TString &FileName);
    Int_t ReadBackground(const TString &FileName);
    Int_t Read2DProj(const TString &FileName);
    Int_t ReadCalibs(const TString &EnergyFileName, const TString &EfficiencyFileName, double_t CompFactor);

    Int_t ReadGG(TH2 *h);

    Int_t AutoBuildProj(TString FileName, Int_t Mode);

    // Handle histos
    void BuildHistos();
    TH1 *GetLUTSpectrum(){return hLUTHist;}
    TH1D *GetTotalProjection(){return hTotalProj;}
    TH1 *GetBackground(){return hBackground;}
    TH2  *Get2DProj(){return h2DProj;}

    TF1 *GetECalibFunc(){return fECalibFunc;}
    TF1 *GetEEffFunc(){return fEffFunc;}

    TH1 *GetHistFromSpeFile(const TString &filename);
    void SaveBackground(const TString &filename);

    double EffFuncFormula(double *x, double *p);

    void ReEvalBackground(double_t Xmin, double_t XMax,double_t LenghtFactor, double_t Width, double_t FWHM_0, double_t FWHM_1, double_t FWHM_2);

    TH1 *Project(const vector<pair<double, double> > &gatesX, const vector<pair<double, double> > &gatesY, Bool_t BGSubtract = true);
    int Project3D(vector< pair<double, double> > gatesX, vector< pair<double, double> > gatesY);
    int Project2D(vector< pair<double, double> > gates);



    //******************************* RW util.c Stuffs ***********************************//

private:

    /************************************************
     *  COMPRESSED 3D CUBE FILE FORMAT
     *  1024byte file header
     *  4096byte data records
     *
     *      each record contains:
     *      compressed 8x8x4 mini-cubes, originally 4bytes/channel
     */

    /* 3d cube file header */
    typedef struct {
        char id[16];            /* "Incub8r3/Pro4d  " */
        int  numch;             /* number of channels on axis */
        int  bpc;               /* bytes per channel, = 4 */
        int  cps;               /* 1/cps symmetry compression, = 6 */
        int  numrecs;           /* number of 4kB data records in the file */
        char resv[992];         /* FUTURE flags */
    } FHead3D;

    /* 3Drecord header */
    typedef struct {
        int minmc;               /* start minicube number, starts at 0 */
        unsigned short nummc;    /* number of minicubes stored in here */
        unsigned short offset;   /* offset in bytes to first full minicube */
    } RHead3D;

    typedef struct {
        RHead3D h;
        unsigned char d[4088];  /* the bit compressed data */
    } Record3D;               /* see the compression alg for details */

    /* Some global data */

    int lumx3d[RW_MAXCH+16]; /* look-up table, maps 3d ch to linear minicube */
    int lumy3d[RW_MAXCH+16];
    int lumz3d[RW_MAXCH+16];

    /* look up minicube addr */
#define LUMC3D(x,y,z) lumx3d[x] + lumy3d[y] + lumz3d[z]

#define MCLEN(mcptr) (mcptr[0] + (mcptr[0] >= 7*32 ? mcptr[1]*32+2 : 1))

    typedef struct {
        FILE *CubeFile3d;
        int length;     /* length of axis on cube */
        int nummc;      /* number of minicubes */
        int *mcnum;     /* SuperRecord minicube number table */
        int numrecs;    /* number of 4kB data records in the file */
        int cubetype;   /* 0 for compressed 1/6 cube,
                 1 for compressed 1/2 cube,
                 2 for uncompressed 1/6 cube, or
                 3 for uncompressed 1/2 cube  */
        int numsr;
        int swapbytes;
    } CubeData3D;

    /* stuff added for linear combinations of cubes: */
    CubeData3D gd3dn[6], *gd3d = &gd3dn[0];
    int recloaded;

    /* Common Block Declarations */
    struct {
        int   numchs;
        double spec[6][MAXCHS], old_spec[6][MAXCHS], bspec[6][MAXCHS], background[MAXCHS], tmpspec[MAXCHS];
        double v_depth[MAXCHS], v_width;
        char  name_gat[800], old_name_gat[800];  /* changed from 20 or 30 */

        int   lo_ch[MAXGAM], hi_ch[MAXGAM];
        int   npks_ch[MAXCHS];                     /* no. of gammas for each ch. */
        short pks_ch[MAXCHS][60];                  /* gammas w/ counts in ch. */
        int   fitchx[MAXCHS], fitchy[MAXCHS];

        char  cubnam[800], levfile[800], fdname[800], fdgfile[800];
        char  progname[8];

        double eff_sp[MAXCHS], energy_sp[MAXCHS];
        double elo_sp[MAXCHS], ehi_sp[MAXCHS], ewid_sp[MAXCHS];

        int   luch[MAXCHS+1], matchs ,le2pro2d;
        double pro2d[MAXCHS*(MAXCHS+1)/2];
        double e2pro2d[MAXCHS*(MAXCHS+1)/2], e2e2spec[MAXCHS], e2e2e2sum;

        double bf1, bf2, bf4, bf5;

        double pk_shape[MAXGAM][15];
        double pk_deriv[MAXGAM][15];
        double w_deriv [MAXGAM][15];

        short looktab[16384];
        int   nclook, lookmin, lookmax;

        /* stuff added for linear combinations of cubes: */
        double dpro2d[MAXCHS*(MAXCHS+1)/2];
        char  cubenam1[5][800];
        double cubefact[5];
        int   many_cubes;
    } xxgd;

public:

    int autobkgnd(double_t Xmin=-1, double_t XMax=-1);

private:

    int read_tab_file(char *infilnam, int *outnclook, int *outlookmin, int *outlookmax, short *outlooktab, int dimlook);
    int setext(char *filnam, const char *cext, int filnam_len);
    FILE *open_readonly(char *filename);
    int get_file_rec(FILE *fd, void *data, int maxbytes, int swap_bytes);
    int file_error(const char *error_type, char *filename);
    void swapb8(char *buf);
    void swapb4(char *buf);
    void swapb2(char *buf);
    int open3dfile(char *name, int *numchs_ret); /* returns 1 on error */
    int close3dfile(void);
    void swap4(int *in);
    void swap2(unsigned short *in);
    int read_spe_file(char *filnam, double *sp, char *namesp, int *numch, int idimsp);
    int inq_file(char *filename, int *reclen);

    int read_cal_file(char *filnam, char *title, int *iorder, double *gain);
    int read_eff_file(char *filnam, char *title, double *pars);
    void setcubenum(int cubenum);

    int add_gate(double elo, double ehi);
    int add_trip_gate(double elo1, double ehi1, double elo2, double ehi2);
    void getmc3d(int num, unsigned int mc[256]);
    int read3dcube(int lo1, int hi1, int lo2, int hi2, int *spec_ret);
    int read_cube(int ilo1, int ihi1, int ilo2, int ihi2);
    void decompress3d (unsigned char in[1024], unsigned int out[256]);
    int find_bg2(int loch, int hich, double *x1, double *y1, double *x2, double *y2);
    int wspec(char *filnam, double *spec, int idim);
    FILE *open_new_file(char *filename, int force_open);
    int put_file_rec(FILE *fd, void *data, int numbytes);
};

#endif
