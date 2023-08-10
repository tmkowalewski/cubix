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

#ifndef CXCFitSpek_h
#define CXCFitSpek_h

#include <cmath>
#include <algorithm>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

// Numero di parametri del fit gaussiano
#define NUMFITPAR  8

///////////////////////////////////////////////////////////////////////////////////
// Macro per l'indirizzamento C-style [i1][i2] di matrice dimensionata [..][l2]
// e allocata come array in un blocco contiguo di memoria. 
// In C++ si puo' fare in modo piu' elegante, ma cosi' e' facile
#define M12(i1, i2, l2) (i1*l2+i2)

const int p_Free      = 0;
const int p_Linked    = 1;
const int p_Fixed     = 2;

const int p_BNone = 0;
const int p_BLow  = 1;
const int p_BHig  = 2;
const int p_BBoth = 3;

///////////////////////////////////////////////////////////////////////////////////
// In questa classe si usano ( in Curfit() ) puntatori a funzioni non statiche.
// La sintassi e' complicata 
// double  (CXCFitSpek::*pfun)(double); // nella dichiarazione si deve specificare classe e segnatura
// pfun = ExpFunc;                    // niente da segnalare nella assegnazione
// double aa = ExpFunc(10.);          // la funzione usata di per se
// double bb = (*this.*pfun)(10.);    // usata come puntatore, ci si deve accedere attraverso un oggetto !!
// double bb = (this->*pfun)(10.);    //  ""

const double  m_pi    = 4.*atan(1.);
const double  m_sqrt2 = sqrt(2.);
const double  sqrt2pi = sqrt(8.*atan(1.));
const double  s2fwhm  = sqrt(8.*log(2.));
const double  s2fwtm  = sqrt(8.*log(10.));

class CXCFitSpek
{
public:
  CXCFitSpek();
  virtual ~CXCFitSpek();

public:
	void    Reset();
  bool    Valid()      {return  m_bValid;}
  bool    Function()   {return  m_bValid && m_bFunct;}
  bool    Background() {return  m_bValid && m_bBckgr;}

  double  Chi2()      {return (m_bValid) ? m_dChi2   : -1.;}

  int     From()      {return (m_bValid) ? m_nChanA    : 0;}
  int     To()        {return (m_bValid) ? m_nChanB    : 0;}

  int     BgFrom()    {return (m_bValid) ? m_nBgChanA  : 0;}
  int     BgTo()      {return (m_bValid) ? m_nBgChanB  : 0;}

  int     Npeaks()    {return (m_bValid) ? m_nNpeaks   : 0;}

  bool    BackFlat()  {return m_bFlat;}   
  bool    BackSlope() {return m_bSlope;}   

  double  Back0   ()  {return (m_bBckgr) ? m_dBack0    : 0.;}
  double  Back0Err()  {return (m_bBckgr) ? m_dBack0Err : 0.;}

  double  Back1   ()  {return (m_bBckgr) ? m_dBack1    : 0.;}
  double  Back1Err()  {return (m_bBckgr) ? m_dBack1Err : 0.;}

  double  BackL1  ()  {return (m_bBckgr) ? m_dBackL1   : 0.;}
  double  BackL2  ()  {return (m_bBckgr) ? m_dBackL2   : 0.;}

  double  Area   (int np)      {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].Area  : 0.;}
  double  AreaErr(int np)      {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pErr[np].Area  : 0.;}

  double  Amplitude   (int np) {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].Ampli  : 0.;}
  double  AmplitudeErr(int np) {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pErr[np].Ampli  : 0.;}

  double  Position   (int np)  {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].Posi  : 0.;}
  double  PositionErr(int np)  {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pErr[np].Posi  : 0.;}

  double  Sigma   (int np) {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].Sigma  : 0.;}
  double  SigmaErr(int np) {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pErr[np].Sigma  : 0.;}

  double  Fwhm   (int np)      {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].Fwhm  : 0.;}
  double  FwhmErr(int np)      {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pErr[np].Fwhm  : 0.;}

  double  Fw05   (int np)      {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].Fw05  : 0.;}
  double  Fw01   (int np)      {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].Fw01  : 0.;}

  double  W01L   (int np)      {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].W01L  : 0.;}
  double  W01R   (int np)      {return (m_bGauss && np >=0 && np < m_nNpeaks) ? m_pRes[np].W01R  : 0.;}

  double  Step   (int np)      {return (m_bStep && np >=0 && np < m_nNpeaks) ? m_pRes[np].Step  : 0.;}
  double  StepErr(int np)      {return (m_bStep && np >=0 && np < m_nNpeaks) ? m_pErr[np].Step  : 0.;}

  double  TailLeft    (int np) {return (m_bTailL && np >=0 && np < m_nNpeaks) ? m_pRes[np].TailL : 0.;}
  double  TailLeftErr (int np) {return (m_bTailL && np >=0 && np < m_nNpeaks) ? m_pErr[np].TailL : 0.;}

  double  TailRight   (int np) {return (m_bTailR && np >=0 && np < m_nNpeaks) ? m_pRes[np].TailR : 0.;}
  double  TailRightErr(int np) {return (m_bTailR && np >=0 && np < m_nNpeaks) ? m_pErr[np].TailR : 0.;}

  double  ExpAmpli()           {return (m_bExp) ? m_dExpAmpli : 0.;}
  double  ExpDecay()           {return (m_bExp) ? m_dExpDecay : 0.;}

  double  SinOmega   ()        {return (m_bSin) ? m_pRes[0].Omega : 0.;}
  double  SinOmegaErr()        {return (m_bSin) ? m_pErr[0].Omega : 0.;}

  double  SinPhase   ()        {return (m_bSin) ? m_pRes[0].Phase : 0.;}
  double  SinPhaseErr()        {return (m_bSin) ? m_pErr[0].Phase : 0.;}

  double  SinAmpli   ()        {return (m_bSin) ? m_pRes[0].Ampli : 0.;}
  double  SinAmpliErr()        {return (m_bSin) ? m_pErr[0].Ampli : 0.;}

  double  FitFunc (double x)         {return (m_bValid) ? (this->*m_pfFitFunc)(x)    : 0;}
  double  FitFuncS(double x) {
    if(m_bGauss) {
      bool tTailL = m_bTailL; bool tTailR = m_bTailR;
      m_bTailL = m_bTailR = false;
      double res = (m_bValid) ? (this->*m_pfFitPeak)(x,0) + FitBack(x) : 0;
      m_bTailL = tTailL; m_bTailR = tTailR;
      return res;
    }
    else {
      return (m_bValid) ? (this->*m_pfFitFunc)(x)    : 0;
    }
  }
  double  FitBack(double x)          {return (m_bBckgr) ? (this->*m_pfFitBack)(x)    : 0;}
  double  FitPeak(double x, int n=0) {return (m_bFunct) ? (this->*m_pfFitPeak)(x, n) : 0;}

  bool    CalcStrightLine(float *pSpek, int chanA, int chanB, int doSlope);
  int     CalcGaussianFit(float *pSpek, int chanA, int chanB, std::vector<double>&vPeaks);
  int     CalcGaussianFit(float *pSpek, int chanA, int chanB);  // one peak autolocated,
  bool    CalcExponential(float *pSpek, int chanA, int chanB, int bgA = -1, int bgB = -1);

public:
  int     GFitParNumber() {return NUMFITPAR;}
  int     GFitParB0 ()     {return 0;}
  int     GFitParB1 ()     {return 1;}
  int     GFitParAMP()     {return 2;}
  int     GFitParPOS()     {return 3;}
  int     GFitParSIG()     {return 4;}
  int     GFitParSTP()     {return 5;}
  int     GFitParTL ()     {return 6;}
  int     GFitParTR ()     {return 7;}
  void    GFitParGet(int *pP) {for(int ii = 0; ii < NUMFITPAR; ii++) pP[ii] = m_pGFitPar[ii];}
  void    GFitParSet(int *pP) {for(int ii = 0; ii < NUMFITPAR; ii++) m_pGFitPar[ii] = pP[ii] & 3;}
  void    GFitParDlg();

public:
  
  class CFitRes
  {
  public:
    double  Ampli;
    double  Area;
    double  Posi;
    double  Sigma;
    double  Fwhm;   // simply Sigma*s2fwhm
    double  Fw05;   // FW at 1/2  considering tails
    double  Fw01;   // FW at 1/10 considering tails
    double  W01L;   // left  width at 1/10 normalized to Fwhm/2
    double  W01R;   // right width at 1/10 normalized to Fwhm/2
    double  Step;
    double  TailL;
    double  TailR;
    double  Omega;
    double  Phase;
    CFitRes() : Ampli(0), Area(0),  Posi(0),  Sigma(0), Fwhm(0), Fw05(0), Fw01(0), W01L(0), W01R(0),
                Step(0),  TailL(0), TailR(0), Omega(0), Phase(0)
    {
    }
  };
  
  class CFitPar
  {
  public:
    double  Value;
    double  Error;
    double  Valmin;
    double  Valmax;
    int     Status;
    int     Bounds;
    int     LinkTo;
    int     PackId;
    CFitPar() : Value(0),  Error(0),  Valmin(0), Valmax(0),
                Status(0), Bounds(0), LinkTo(0), PackId(0)
    {
    }
  };
  
  // i membri puntatori alle funzioni del risultato del fit
  double (CXCFitSpek::*m_pfFitFunc)(double);
  double (CXCFitSpek::*m_pfFitBack)(double);
  double (CXCFitSpek::*m_pfFitPeak)(double, int);
  
  // i membri puntatori alle funzioni del fit per Curfit()
  double (CXCFitSpek::*m_pfFunct)(double, double *);
  void   (CXCFitSpek::*m_pfDeriv)(int, double *, double *, double &, double &);

private:
  double  LineFunc(double xx);

  double  ExpFunc(double xx);
  double  ExpPeak(double xx, int);

  double  GFitFunc(double xx);
  double  GFitBack(double xx);
  double  GFitPeak(double xx, int nn);
  double  GFfunF(double xx, double *par);
  void    GFderF(int id, double *fpar, double *deriv, double &deltay, double &weight);
  void    GF_res(void);
  double  GF_erf(double);
  double  GF_y2w(int np, double yval, int side = 0);  // -1=Left, 0=full, 1=Right

  double  SFitFunc(double xx);
  double  SFitWave(double xx, int nn);
  double  SFfunF(double xx, double *par);
  void    SFderF(int id, double *fpar, double *deriv, double &deltay, double &weight);

  double  Curfit(int ndat, int npar, int mpar);
  double  Chisqr(double *par);
  void    Manage(double *par, int type);
  double  Matinv(double *array, int nord, int mpar);

  bool    m_bValid;
  bool    m_bFunct;
  bool    m_bBckgr;

  bool    m_bLine;
  bool    m_bFlat;
  bool    m_bSlope;

  bool    m_bGauss;
  bool    m_bStep;
  bool    m_bTailL;
  bool    m_bTailR;

  int     m_nChanA;
  int     m_nChanB;
  int     m_nBgChanA;
  int     m_nBgChanB;
  int     m_nNpeaks;

  // control of Gauss Fit parameters
  // [Bit1 enabled] [Bit2 all equal]
  int     m_pGFitPar[NUMFITPAR];

  int     m_nNdat;
  int     m_nNpar;
  int     m_nFpar;
  int     m_nOffB;
  int     m_nOffN;

  double  m_dChi2;

  double  m_dBack0;
  double  m_dBack0Err;
  double  m_dBack1;
  double  m_dBack1Err;
  double  m_dBackL1;
  double  m_dBackL2;

  CFitRes *m_pRes;
  CFitRes *m_pErr;

  bool    m_bExp;
  double  m_dExpAmpli;
  double  m_dExpDecay;

  bool    m_bSin;

  CFitPar *m_pPar;        // the parameters of the fit
  double  *m_pDatVal;     // the data to fit
  double  *m_pDatErr;     // their error (1/sig2)

};

#endif // !defined(AFX_FITSPEK_H__F2A2E380_9934_11D5_B3CF_000000000000__INCLUDED_)
