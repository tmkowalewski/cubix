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

#include <memory.h>
#include <stdlib.h>

#include "CXFitSpek.h"

using namespace std;

CXCFitSpek::CXCFitSpek()
{
  m_bValid = m_bFunct = m_bBckgr = false;
  m_bLine  = m_bFlat  = m_bSlope = false;
  m_bGauss = m_bExp   = m_bSin   = false;
  m_bStep  = m_bTailL = m_bTailR = false;
  m_pRes    = NULL;
  m_pErr    = NULL;
  m_pPar    = NULL;
  m_pDatVal = NULL;
  m_pDatErr = NULL;

  // the default settings
  // [Bit1 enabled] [Bit2 all equal]
  m_pGFitPar[0] = 1;   // 0   Bg0
  m_pGFitPar[1] = 0;   // 1   Bg1 not enabled
  m_pGFitPar[2] = 1;   // 2+0 Amplitude
  m_pGFitPar[3] = 1;   // 2+1 Position
  m_pGFitPar[4] = 3;   // 2+2 FWMH  all equal
  m_pGFitPar[5] = 3;   // 2+3 Step  all equal
  m_pGFitPar[6] = 2;   // 2+4 TailL all equal but not enabled
  m_pGFitPar[7] = 2;   // 2+5 TailR all equal but not enabled
}

CXCFitSpek::~CXCFitSpek()
{
  Reset();
}

void CXCFitSpek::Reset()
{
  m_bValid = m_bFunct = m_bBckgr = false;
  m_bLine  = m_bFlat  = m_bSlope = false;
  m_bGauss = m_bExp   = m_bSin   = false;
  m_bStep  = m_bTailL = m_bTailR = false;
  if(m_pRes)    {delete [] m_pRes;    m_pRes    = NULL;}
  if(m_pErr)    {delete [] m_pErr;    m_pErr    = NULL;}
  if(m_pPar)    {delete [] m_pPar;    m_pPar    = NULL;}
  if(m_pDatVal) {delete [] m_pDatVal; m_pDatVal = NULL;}
  if(m_pDatErr) {delete [] m_pDatErr; m_pDatErr = NULL;}
}

void CXCFitSpek::GFitParDlg()
{
#if 0
  CXCFitSpekDlg setUp;

  setUp.m_bB0      = m_pGFitPar[0]&1;
  setUp.m_bB1      = m_pGFitPar[1]&1;
  setUp.m_bAmpl    = m_pGFitPar[2]&1;
  setUp.m_bPosi    = m_pGFitPar[3]&1;
  setUp.m_bFWHM    = m_pGFitPar[4]&1;
  setUp.m_bStep    = m_pGFitPar[5]&1;
  setUp.m_bTailL   = m_pGFitPar[6]&1;
  setUp.m_bTailR   = m_pGFitPar[7]&1;
  setUp.m_bFWHMeq  = m_pGFitPar[4]&2;
  setUp.m_bTailLeq = m_pGFitPar[6]&2;
  setUp.m_bTailReq = m_pGFitPar[7]&2;

  if(setUp.DoModal() != IDOK)
    return;

  m_pGFitPar[0]  = setUp.m_bB0      ? 1 : 0;
  m_pGFitPar[1]  = setUp.m_bB1      ? 1 : 0;
  m_pGFitPar[2]  = setUp.m_bAmpl    ? 1 : 0;
  m_pGFitPar[3]  = setUp.m_bPosi    ? 1 : 0;
  m_pGFitPar[4]  = setUp.m_bFWHM    ? 1 : 0;
  m_pGFitPar[5]  = setUp.m_bStep    ? 1 : 0;
  m_pGFitPar[6]  = setUp.m_bTailL   ? 1 : 0;
  m_pGFitPar[7]  = setUp.m_bTailR   ? 1 : 0;
  m_pGFitPar[4] |= setUp.m_bFWHMeq  ? 2 : 0;
  m_pGFitPar[5] |=                    2    ;
  m_pGFitPar[6] |= setUp.m_bTailLeq ? 2 : 0;
  m_pGFitPar[7] |= setUp.m_bTailReq ? 2 : 0;
#endif

}

///////////////////////////
// Fit of a stright line //
///////////////////////////

bool CXCFitSpek::CalcStrightLine(float *pSpek, int chanA, int chanB, int doSlope)
{
  // nothing valid yet
  m_bValid  = false;

  float  *pd;

  if(chanA < chanB) {
    m_nChanA = chanA;
    m_nChanB = chanB;
  }
  else {
    m_nChanA = chanB;
    m_nChanB = chanA;
  }
  m_nBgChanA = m_nChanA;
  m_nBgChanB = m_nChanB;
  m_nNdat    = m_nChanB - m_nChanA + 1;

  // tilted line (needs at least 2 channels)
  if(doSlope&1) {
    if(m_nNdat < 2) return false;

    double s1 = 0., sx = 0., sxx = 0., sy = 0., syx = 0.;
    double x, y, e;
    pd = pSpek + m_nChanA;
    if(doSlope&2) {
      // use only the two extreme channels
      for(int ii = 0; ii < m_nNdat; ii+=m_nNdat-1) {
        y    = pd[ii];
        x    = ii+0.5;    // considera il centro del canale
        e    = 1.;        // andrebbe sostituito con l'errore statistico
        s1  += e;
        sx  += e*x;
        sxx += e*x*x;
        sy  += e*y;
        syx += e*y*x;
      }
    }
    else {
      // use all channels in region
      x  = 0.5;           // considera il centro del canale
      for(int ii = 0; ii < m_nNdat; ii++, x += 1.) {
        y    = *pd++;
        e    = 1.;        // andrebbe sostituito con l'errore statistico
        s1  += e;
        sx  += e*x;
        sxx += e*x*x;
        sy  += e*y;
        syx += e*y*x;
      }
    }
    double deter = s1*sxx-sx*sx;
    if(deter >= 0.) {
      m_dBack0 = (sxx*sy - sx*syx)/deter;
      m_dBack1 = (-sx*sy + s1*syx)/deter;
      m_dBack0Err = sqrt(sxx/deter);
      m_dBack1Err = sqrt(s1 /deter);
//      double eback01 = -sx /deter;
      m_bFlat  = true;
      m_bSlope = true;
      double sy1 = 0.;
      double sy2 = 0.;
      x  = 0.5;           // considera il centro del canale
      pd = pSpek + m_nChanA;
      for(int ii = 0; ii < m_nNdat; ii++, x += 1.) {
        double dy = *pd++ - (m_dBack0 + m_dBack1*x);
        sy1 += fabs(dy);
        sy2 += dy*dy;
      }
      m_dBackL1 = sy1 / m_nNdat;
      m_dBackL2 = sqrt(sy2 / m_nNdat);
    }
  }

  // Flat line
  else {
    double sy0 = 0.;
    pd = pSpek + m_nChanA;
    if(doSlope&2) {
      // use only the two extreme channels
      for(int ii = 0; ii < m_nNdat; ii+=m_nNdat-1)
        sy0 += pd[ii];
      m_dBack0 = sy0 / 2;
    }
    else {
      // use all channels in region
      for(int ii = 0; ii < m_nNdat; ii++)
        sy0 += *pd++;
      m_dBack0 = sy0 / m_nNdat;
    }
    double sy1 = 0.;
    double sy2 = 0.;
    pd = pSpek + m_nChanA;
    for(int ii = 0; ii < m_nNdat; ii++) {
      double dy = *pd++ - m_dBack0;
      sy1 += fabs(dy);
      sy2 += dy*dy;
    }
    m_dBackL1 = sy1 / m_nNdat;
    m_dBackL2 = sqrt(sy2 / m_nNdat);
    m_dBack1  = m_dBack1Err = 0.;
    m_bFlat   = true;
    m_bSlope  = false;
  }

  m_bValid  = true;
  m_bLine   = true;
  m_bBckgr  = true;
  m_bFunct  = false;

  // puntatori alle funzioni risultato del calcolo della retta
  m_pfFitFunc = &CXCFitSpek::LineFunc;
  m_pfFitBack = &CXCFitSpek::LineFunc;

  return m_bValid;
}

// metodo pubblico (attraverso membro puntatore)
// che lavora sui risultati del fit
double CXCFitSpek::LineFunc(double xx)
{
  if(!m_bLine) return 0.;

  double val = 0;
  if(m_bFlat ) val += m_dBack0;
  if(m_bSlope) val += m_dBack1*(xx-m_nChanA);

  return val;
}

//////////////////////////////////////////////////////////////////////
// Fit of a gaussian peak with tails over a stright+step background //
//////////////////////////////////////////////////////////////////////

/*
           X  dove si vuole calcolare (riferito all'inizio dello spettro)
  chanA    Z  canale di riferimento
  back0    B0 fondo costante
  back1    B1 slope del fondo riferito a chanA
  ampli    A  ampiezza del picco
  posi     P  posizione del picco (riferita all'inizio dello spettro)
  sigma    W  varianza del picco
  step     S  ampiezza step         (frazione dell'unita',positiva)   ToDo
  tailL    L  attacco coda sinistra (in unita' di sigma, negativo)    ToDo
  tailR    R  attacco coda destra   (in unita' di sigma, positivo)    ToDo

  yy = (X-P)/W

  Funct(yy) = B + SUMpeaks(A * ( F1 + F2 ))

  B  =  B0 + B1 * (X-Z)

     =  exp(-0.5*L*(2*yy-L))        per  yy < L
  F1 =  exp(-0.5*yy^2)              per       L <= yy <= R
     =  exp(-0.5*R*(2*yy-R))        per                  R < yy

  F2 = S/(1+exp(yy))^2

*/

// Questo obbrobrio e' una traduzione di un programma Fortran (recal_cob ...)
// Il metodo e' complicato dalla possibilita' di imporre limiti ai parametri
// I parametri sono controllati da m_pGFitPar che va caricata dall'esterno con GFitParSet()
int CXCFitSpek::CalcGaussianFit(float *pSpek, int chanA, int chanB, std::vector<double>&vPeaks)
{
  // nothing valid yet
  Reset();

  // puntatori alle funzioni per Curfit()
  m_pfFunct = &CXCFitSpek::GFfunF;    // funzione
  m_pfDeriv = &CXCFitSpek::GFderF;    // derivata

  m_nOffB = 2;
  m_nOffN = 6;

  int ii;
  int ierr=0;
  double  chi2, chi2n;

  // Verify fit region
  if(chanA < chanB) {
    m_nChanA = chanA;
    m_nChanB = chanB;
  }
  else {
    m_nChanA = chanB;
    m_nChanB = chanA;
  }
  m_nBgChanA = m_nChanA;
  m_nBgChanB = m_nChanB;

  // nothing to fit if region empty
  bool empty = true;
  for(ii = m_nChanA; ii <=  m_nChanB; ii++) {
    if(pSpek[ii]) {
      empty = false;
      break;
    }
  }
  if(empty) {
    ierr = -1;
    return ierr;
  }

  // keep only peaks internal to fit region
  double *lPeaks = new double[std::max(1,(int)vPeaks.size())];
  int nPeaks = 0;
  for(size_t np = 0; np < vPeaks.size(); np++) {
    if(vPeaks[np] > m_nChanA && vPeaks[np] < m_nChanB) {
      lPeaks[nPeaks++] = vPeaks[np];
    }
  }

  // if no peak present, set one at max amplitude channel
  if(nPeaks == 0) {
    int   pm = (m_nChanA+m_nChanB+1)/2;
    float vm = pSpek[pm];
    for(ii = m_nChanA+1; ii < m_nChanB-1; ii++) {
      if(pSpek[ii] > vm) {
        pm = ii;
        vm = pSpek[pm];
      }
    }
    lPeaks[0] = pm + 0.5;
    nPeaks = 1;
  }

  m_nNdat   = m_nChanB - m_nChanA + 1;    // margins included

  if(m_nNdat<=2) {
    ierr = -1;      // nothing to fit
    delete [] lPeaks;
    return ierr;
  }

  m_nNpeaks = nPeaks;
  m_nNpar   = m_nOffB + m_nOffN * m_nNpeaks;

  // create parameters structure
  m_pPar = new CFitPar [m_nNpar];

  // the data to fit
  m_pDatVal = new double [m_nNdat];
  m_pDatErr = new double [m_nNdat];

  // prepare vector of data and 1/(sigma^2)
  float  *ps = pSpek + m_nChanA;
  for(ii = 0; ii <  m_nNdat; ii++) {
    double yy  = *ps++;
    m_pDatVal[ii] = yy;
    m_pDatErr[ii] = (yy) ? (1./fabs(yy)) : (1.);
  }

  ////////////////////////////////////////
  // estimates for the first fit round  //
  // limiting number of free parameters //
  ////////////////////////////////////////

  double bgL = (m_pDatVal[0        ] + m_pDatVal[1        ])/2.;
  double bgR = (m_pDatVal[m_nNdat-1] + m_pDatVal[m_nNdat-2])/2.;

  // background level [0]
  // background slope [1]
  int bflag = 0;
  if(m_pGFitPar[0]&1) bflag |= 1;   // BG0 enabled ?
  if(m_pGFitPar[1]&1) bflag |= 2;   // BG1 enabled ?
  switch (bflag) {
  case 0:
    m_bFlat  = false;
    m_bSlope = false;
    m_pPar[0].Value  = 0.;
    m_pPar[1].Value  = 0.;
    break;
  case 1:
    m_bFlat  = true;
    m_bSlope = false;
    m_pPar[0].Value  = (bgR+bgL)/2;
    m_pPar[1].Value  = 0.;
    break;
  case 2:
    m_bFlat  = false;
    m_bSlope = true;
    m_pPar[0].Value  = 0.;
    m_pPar[1].Value  = (bgR-bgL)/m_nNdat;
    break;
  case 3:
    m_bFlat  = true;
    m_bSlope = true;
    m_pPar[0].Value  = bgL;
    m_pPar[1].Value  = (bgR-bgL)/m_nNdat;
    break;
  }

  m_pPar[0].Status = (m_bFlat ) ? p_Free : p_Fixed;
  m_pPar[1].Status = (m_bSlope) ? p_Free : p_Fixed;

  m_pPar[0].Bounds = p_BNone;
  m_pPar[1].Bounds = p_BNone;

  bgL = m_pPar[0].Value;
  bgR = m_pPar[1].Value;

  // offs+0 amplitude
  // offs+1 positions
  // offs+2 sigma
  // offs+3 step
  // offs+4 tailL
  // offs+5 tailR

  // set-up the starting values
  for(int np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    double ppos  = lPeaks[np] - m_nChanA;
    int    ipos  = int(ppos);
    double ampl  = m_pDatVal[ipos];
    double step  = 0.;
    double back  = bgL + ppos*bgR;
    double ampl2 = (ampl+back)/2.;
    int    width = 1;
    for(int iL = ipos-1; iL > 0; iL--) {  // estimate of peak width
      if(m_pDatVal[iL] < ampl2) break;
      width++;
    }
    for(int iR = ipos+1; iR < m_nNdat-1; iR++) {
      if(m_pDatVal[iR] < ampl2) break;
      width++;
    }
    m_pPar[offs+0].Value = ampl - back;
    m_pPar[offs+1].Value = ppos;
    m_pPar[offs+2].Value = width/s2fwhm;
    m_pPar[offs+3].Value = step;
    m_pPar[offs+4].Value = -100;      // non existing
    m_pPar[offs+5].Value =  100;      // non existing
  }

  // replace sigmas with weighted average
  double sigma = 0;
  double total = 0;
  for(int np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    sigma += fabs(m_pPar[offs+0].Value)*m_pPar[offs+2].Value;
    total += fabs(m_pPar[offs+0].Value);
  }
  sigma /= total;
  sigma  = max(sigma, 1./sqrt(12.));  // 1 channel uniformly distributed
  for(int np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    m_pPar[offs+2].Value = sigma;
  }

  // now set the controls for the first fit
  for(int np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    // amplitude    --> free but positive
    m_pPar[offs+0].Status = p_Free;
    m_pPar[offs+0].LinkTo = offs+0;
    m_pPar[offs+0].Bounds = p_BLow;
    m_pPar[offs+0].Valmin = 0.;

    // position     --> free, but inside fit region
    m_pPar[offs+1].Status = p_Free;
    m_pPar[offs+1].LinkTo = offs+1;
    m_pPar[offs+1].Bounds = p_BBoth;
    m_pPar[offs+1].Valmin = 0.;
    m_pPar[offs+1].Valmax = m_nNdat-1.;

    // sigma        --> all equal to the first
    if(np == 0) {
      m_pPar[offs+2].Status = p_Free;
      m_pPar[offs+2].LinkTo = offs+2;
      m_pPar[offs+2].Bounds = p_BBoth;
      m_pPar[offs+2].Valmin = 1./sqrt(12.); // 1 channel uniformly distributed
      m_pPar[offs+2].Valmax = m_nNdat/(s2fwhm*2.);
    }
    else {
      m_pPar[offs+2].Status = p_Linked;
      m_pPar[offs+2].LinkTo = m_nOffB+2;
    }

    // step         --> fixed
    m_bStep = false;
    m_pPar[offs+3].Status = p_Fixed;

    // tail left     --> fixed
    m_bTailL = false;
    m_pPar[offs+4].Status = p_Fixed;

    // tail right   --> fixed
    m_bTailR = false;
    m_pPar[offs+5].Status = p_Fixed;
  }

  // count number of free parameters
  m_nFpar = 0;
  for(int np = 0; np < m_nNpar; np++) {
    if(m_pPar[np].Status == p_Free) m_nFpar++;
  }

  if(m_nFpar < 1) {
    ierr = -3;      // nothing to fit
    goto cleanup;
  }

  if(m_nNdat - m_nFpar < 1) {
    ierr = -4;      // insufficient degrees of freedom
    goto cleanup;
  }

///////////////////////
//// first fit round //
///////////////////////

  chi2 = Curfit(m_nNdat, m_nNpar, m_nFpar);
  if(chi2 < 0) {
    ierr = -5;
    goto cleanup;
  }

////////////////////
//// the real fit //
////////////////////

  for(int np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {

    // amplitude    --> free but positive
    m_pPar[offs+0].Status = p_Free;
    m_pPar[offs+0].LinkTo = offs+0;
    m_pPar[offs+0].Bounds = p_BLow;
    m_pPar[offs+0].Valmin = 0.;

    // position     --> free, but inside fit region
    m_pPar[offs+1].Status = p_Free;
    m_pPar[offs+1].LinkTo = offs+1;
    m_pPar[offs+1].Bounds = p_BBoth;
    m_pPar[offs+1].Valmin = 1.;
    m_pPar[offs+1].Valmax = m_nNdat-2.;

    // sigma        --> free; possibly all equal to first
    if(np == 0 || !(m_pGFitPar[4]&2)) {
      m_pPar[offs+2].Value = fabs(m_pPar[offs+2].Value);
      m_pPar[offs+2].Status = p_Free;
      m_pPar[offs+2].LinkTo = offs+2;
      m_pPar[offs+2].Bounds = p_BBoth;
      m_pPar[offs+2].Valmin = 1./sqrt(12.); // 1 channel uniformly distributed
      m_pPar[offs+2].Valmax = m_nNdat/(s2fwhm*2.);
    }
    else {
      m_pPar[offs+2].Value = fabs(m_pPar[offs+2].Value);
      m_pPar[offs+2].Status = p_Linked;
      m_pPar[offs+2].LinkTo = m_nOffB+2;
    }

    // step         --> if free can be all equal to that of first peak
    if(m_pGFitPar[5]&1) {
      m_bStep = true;
      if(np == 0 || !(m_pGFitPar[5]&2)) {
        m_pPar[offs+3].Status = p_Free;
        m_pPar[offs+3].LinkTo = offs+3;
        m_pPar[offs+3].Bounds = p_BBoth;
        m_pPar[offs+3].Valmin = 0;
        m_pPar[offs+3].Valmax = 1;
      }
      else {
        m_pPar[offs+3].Status = p_Linked;
        m_pPar[offs+3].LinkTo = m_nOffB+3;
      }
    }

    // tail left    --> if free can be all equal to that of first peak
    if(m_pGFitPar[6]&1) {
      m_bTailL = true;
      m_pPar[offs+4].Value = -2.;       // attacco iniziale a -2 sigma (~FWHM)
      if(np == 0 || !(m_pGFitPar[6]&2)) {
        m_pPar[offs+4].Status = p_Free;
        m_pPar[offs+4].LinkTo = offs+4;
        m_pPar[offs+4].Bounds = p_BHig|p_BLow;
        m_pPar[offs+4].Valmax = -0.1;   // upper limit of left tail
        m_pPar[offs+4].Valmin = -50.;   // lower limit of left tail
      }
      else {
        m_pPar[offs+4].Status = p_Linked;
        m_pPar[offs+4].LinkTo = m_nOffB+4;
      }
    }

    // tail right   --> if free can be all equal to that of first peak
    if(m_pGFitPar[7]&1) {
      m_bTailR = true;
      m_pPar[offs+5].Value = 2.;        // attacco iniziale a +2 sigma (~FWHM)
      if(np == 0 || !(m_pGFitPar[7]&2)) {
        m_pPar[offs+5].Status = p_Free;
        m_pPar[offs+5].LinkTo = offs+5;
        m_pPar[offs+5].Bounds = p_BLow|p_BHig;
        m_pPar[offs+5].Valmin = 0.1;    // lower limit of right tail
        m_pPar[offs+5].Valmax = 50.;    // upper limit of right tail
      }
      else {
        m_pPar[offs+5].Status = p_Linked;
        m_pPar[offs+5].LinkTo = m_nOffB+5;
      }
    }
  }

  // count number of free parameters
  m_nFpar = 0;
  for(int np = 0; np < m_nNpar; np++) {
    if(m_pPar[np].Status == p_Free) m_nFpar++;
  }

  if(m_nFpar < 1) {
    ierr = -6;      // nothing to fit
    goto cleanup;
  }

  if(m_nNdat - m_nFpar < 1) {
    ierr = -7;      // insufficient degrees of freedom
    goto cleanup;
  }

  chi2n = Curfit(m_nNdat, m_nNpar, m_nFpar);
  if(chi2n < 0) {
    if(chi2n != -2) {   // -2 vuol dire che non e' riuscito a fare meglio
      ierr = -8;
      goto cleanup;
    }
    chi2n = chi2;
  }

  m_dChi2       = chi2n/(m_nNdat-m_nFpar);

  m_bValid = true;
  m_bGauss = true;
  m_bLine  = m_bFlat || m_bSlope;
  m_bFunct = true;
  m_bBckgr = m_bLine;

  // save result in data_members
  GF_res();

  // puntatori alle funzioni risultato del fit
  m_pfFitFunc = &CXCFitSpek::GFitFunc;    // background + all peaks
  m_pfFitBack = &CXCFitSpek::GFitBack;    // background only
  m_pfFitPeak = &CXCFitSpek::GFitPeak;    // one particular peak

cleanup:
  delete [] m_pPar;    m_pPar    = NULL;
  delete [] m_pDatVal; m_pDatVal = NULL;
  delete [] m_pDatErr; m_pDatErr = NULL;
  delete [] lPeaks;

  return (m_bValid) ? (m_nNpeaks) : (ierr);
}

int CXCFitSpek::CalcGaussianFit(float *pSpek, int chanA, int chanB)
{
  std::vector<double> vPeaks;
  if(chanA > chanB)
    std::swap(chanA, chanB);
  vPeaks.push_back(chanA-1);   // outside fit region so that CalcGaussianFit will assumeand auto locate 1 peak
  return CalcGaussianFit(pSpek, chanA, chanB, vPeaks);
}

// the private methods for the gaussian fit
// works in offset-zero
double CXCFitSpek::GFfunF(double xx, double *xpar)
{
/*
           X     dove si vuole calcolare (riferito all'inizio della regione di fit)
  Par(0) = B0    fondo costante
  Par(1) = B1    fondo slope
  Par(2) = A     ampiezza
  Par(3) = P     posizione
  Par(4) = W     sigma
  Par(5) = S     ampiezza step
  Par(6) = L     attacco coda sinistra (distanza da P, misurata in unit� di sigma; L < 0)
  Par(7) = R     attacco coda destra   (distanza da P, misurata in unit� di sigma; R > 0)

  yy = (X-P)/W   distanza da P in unit� di sigma

  Funct(yy) = B0 + B1*X + A * ( F1 + F2 )

     =  exp(-0.5*L*(2*yy-L))        yy <= L
  F1 =  exp(-0.5*yy^2)                    L <= yy <= R
     =  exp(-0.5*R*(2*yy-R))                         R <= yy

  F2 = S/(1+exp(yy))^2
*/

  double amp, pos, sig, yy, yl, yr, ex, f1, f2, ey, eyp1i;

  int    np, offs;
  double fsum = 0.;
  for(np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    amp = xpar[offs+0];
    pos = xpar[offs+1];
    sig = xpar[offs+2];
    yy = (xx-pos)/sig;
    yl = xpar[offs+4];
    yr = xpar[offs+5];

    if(m_bTailL && yy < yl) {
      ex = -0.5*yl*(2.*yy-yl);
    }
    else if(m_bTailR && yy > yr) {
      ex = -0.5*yr*(2.*yy-yr);
    }
    else {
      ex = -0.5*yy*yy;
    }
    f1 = exp(ex);

    f2 = 0;
    if (m_bStep) {
      ey = exp(yy);
      eyp1i = 1./(1.+ey);
      f2 = xpar[offs+3]*eyp1i*eyp1i;
    }

    fsum += amp*(f1+f2);
  }
  return xpar[0] + xpar[1]*xx + fsum;

}

// calcola le derivate parziali della funzione nel punto i
void CXCFitSpek::GFderF(int id, double *xpar, double *deriv, double &deltay, double &weight)
{
  double amp, pos, sig, stp, f0, f1, f2, f3;
  double yy, yl, yr, ex, ey, dd;

  double xx = id + 0.5; // derivatives calculated at mid channel

  deltay = m_pDatVal[id] - GFfunF(xx, xpar);
  weight = m_pDatErr[id];

  memset(deriv, 0, m_nNpar*sizeof(double));

  deriv[0] = 1.;   // background level
  deriv[1] = xx;   // background slope

  int np, offs;
  for(np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    amp = xpar[offs+0];
    pos = xpar[offs+1];
    sig = xpar[offs+2];
    stp = xpar[offs+3];
    yl  = xpar[offs+4];
    yr  = xpar[offs+5];
    yy  = (xx-pos)/sig;

    if(m_bTailL && yy < yl) {
      ex  = -0.5*yl*(2.*yy-yl);
      f1  = exp(ex);
      f0  = yl/sig;
      deriv[offs+4] = amp*f1*(yl-yy);
    }
    else if(m_bTailR && yy > yr) {
      ex  = -0.5*yr*(2.*yy-yr);
      f1  = exp(ex);
      f0  = yr/sig;
      deriv[offs+5] = amp*f1*(yr-yy);
    }
    else {
      ex = -0.5*yy*yy;
      f1  = exp(ex);
      f0  = yy/sig;
    }

    f2 = f3 = 0;
    if(m_bStep) {
      ey  = exp(yy);
      f3  = 1./(1.+ey);
      f2  = f3*f3;
      f3 *= 2*ey/sig;
      deriv[offs+3] = amp*f2;
    }

    dd = amp*(f0*f1 + stp*f2*f3);

    deriv[offs+0] = f1+stp*f2;    // amplitude
    deriv[offs+1] = dd;           // position
    deriv[offs+2] = dd*yy;        // sigma
  }

  return;
}

// Salva i risultati in m_pRes e m_pErr
void CXCFitSpek::GF_res()
{
  m_dBack0    = m_pPar[0].Value; m_dBack0Err = m_pPar[0].Error;
  m_dBack1    = m_pPar[1].Value; m_dBack1Err = m_pPar[1].Error;

  m_pRes = new CFitRes [m_nNpeaks];
  m_pErr = new CFitRes [m_nNpeaks];

  int np, offs;
  for(np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    m_pRes[np].Ampli = m_pPar[offs+0].Value;
    m_pRes[np].Area  = m_pPar[offs+0].Value * m_pPar[offs+2].Value * sqrt2pi;
    m_pRes[np].Posi  = m_pPar[offs+1].Value+m_nChanA;
    m_pRes[np].Sigma = m_pPar[offs+2].Value;
    m_pRes[np].Fwhm  = m_pPar[offs+2].Value * s2fwhm;
    m_pRes[np].Fw05  = m_pPar[offs+2].Value * s2fwhm;
    m_pRes[np].Fw01  = m_pPar[offs+2].Value * s2fwtm;
    m_pRes[np].W01L  = sqrt(log(10.)/log(2.));
    m_pRes[np].W01R  = sqrt(log(10.)/log(2.));
    m_pRes[np].Step  = m_pPar[offs+3].Value;
    m_pRes[np].TailL = m_pPar[offs+4].Value;
    m_pRes[np].TailR = m_pPar[offs+5].Value;

    m_pErr[np].Ampli = m_pPar[offs+0].Error;
    m_pErr[np].Area  = m_pRes[np].Area*sqrt(pow(m_pPar[offs+0].Error/m_pPar[offs+0].Value,2.) +
                                            pow(m_pPar[offs+2].Error/m_pPar[offs+2].Value,2.));
    m_pErr[np].Posi  = m_pPar[offs+1].Error;
    m_pErr[np].Sigma = m_pPar[offs+2].Error;
    m_pErr[np].Fwhm  = m_pPar[offs+2].Error * s2fwhm;
    m_pErr[np].Step  = m_pPar[offs+3].Error;
    m_pErr[np].TailL = m_pPar[offs+4].Error;
    m_pErr[np].TailR = m_pPar[offs+5].Error;
  }

  //// to test the area calculations: keep it commented
  //for(np = 0; np < m_nNpeaks; np++) {
  //  double xxl  = m_pRes[np].Posi - 10.*m_pRes[np].Fwhm;
  //  double xxr  = m_pRes[np].Posi + 10.*m_pRes[np].Fwhm;
  //  double xxx;
  //  double area = 0., yval = 0.;
  //  for(xxx = xxl; xxx <= xxr; xxx += 0.1) {
  //    yval  = GFitPeak(xxx, np);
  //    area += yval;
  //  }
  //  area = area/10;
  //  continue;
  //}

  // aree e larghezze gia' calcolate analiticamente se gaussiane senza code
  if(!m_bTailL && !m_bTailR)
    return;

  // area dei picchi calcolata usando erf() e l'integrale analitico delle code esponenziali
  for(np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    double area = 0.;
    if(m_bTailL) {
      double L = fabs(m_pPar[offs+4].Value);
      double a = exp(-0.5*L*L)/L;
      double b = sqrt2pi/2*GF_erf(L/m_sqrt2);
      area += a + b;
    }
    else {
      area += sqrt2pi/2;
    }
    if(m_bTailR) {
      double R = fabs(m_pPar[offs+5].Value);
      double a = exp(-0.5*R*R)/R;
      double b = sqrt2pi/2*GF_erf(R/m_sqrt2);
      area += a + b;
    }
    else {
      area += sqrt2pi/2;
    }
    area *= m_pPar[offs+0].Value*m_pPar[offs+2].Value;
    m_pErr[np].Area *= area/m_pRes[np].Area; // errore solo ri-scalato ???
    m_pRes[np].Area  = area;
  }

  for(np = 0; np < m_nNpeaks; np++) {
    m_pRes[np].Fw05 = GF_y2w(np, 0.5);
    m_pRes[np].Fw01 = GF_y2w(np, 0.1);
    m_pRes[np].W01L = GF_y2w(np, 0.1, -1)/(m_pRes[np].Fwhm/2);
    m_pRes[np].W01R = GF_y2w(np, 0.1, +1)/(m_pRes[np].Fwhm/2);
  }

  return;

}

// NUMERICAL RECIPES IN C: THE ART OF SCIENTIFIC COMPUTING (ISBN 0-521-43108-5)
// Pag 221.
// erfcc returns the complementary error function erfc(x)
// with fractional error everywhere less than 1.2 10-7
// Here it has been transformed to erf() and double
double CXCFitSpek::GF_erf(double x)
{
  double t,z,ans;
  z=fabs(x);
  t=1.0/(1.0+0.5*z);
  ans=t*exp(-z*z-1.26551223+
    t*( 1.00002368+
    t*( 0.37409196+
    t*( 0.09678418+
    t*(-0.18628806+
    t*( 0.27886807+
    t*(-1.13520398+
    t*( 1.48851587+
    t*(-0.82215223+
    t*  0.17087277  )))))))) );
//return x >= 0.0 ? ans : 2.0-ans;      // erfc
  return x >= 0.0 ? 1-ans : ans-1;      // erf
}

// find actual width of the peak at level yval considering the tails
double CXCFitSpek::GF_y2w(int np, double yval, int side)
{
  const double precision = 1.e-6;

  if(!m_bGauss || np < 0 || np > m_nNpeaks) return 0.;
  if(yval <=0 || yval >= 1.)                return 0.;

  CFitRes *pRes = m_pRes+ np;
  if(pRes->Ampli == 0)                      return 0.;

  double ff = 1./pRes->Ampli;
  double x0 = pRes->Posi;

  double dl = pRes->Sigma*sqrt(2.*log(1./yval));
  double xl = x0 - dl;
  double yl = GFitPeak(xl, np)*ff;

  while(fabs(yl/yval-1) > precision) {
    dl /= 4.;
    if(yl > yval) {
      while(yl > yval) {
        xl -= dl;
        yl  = GFitPeak(xl, np)*ff;
      }
    }
    else {
      while(yl < yval) {
        xl += dl;
        yl  = GFitPeak(xl, np)*ff;
      }
    }
  }

  double dr = pRes->Sigma*sqrt(2.*log(1./yval));
  double xr = x0 + dr;
  double yr = GFitPeak(xr, np)*ff;

  while(fabs(yr/yval-1) > precision) {
    dr /= 4.;
    if(yr > yval) {
      while(yr > yval) {
        xr += dr;
        yr  = GFitPeak(xr, np)*ff;
      }
    }
    else {
      while(yr < yval) {
        xr -= dr;
        yr  = GFitPeak(xr, np)*ff;
      }
    }
  }

  if(side < 0)
    return x0 - xl;
  else if (side > 0)
    return xr - x0;
  else
    return xr - xl;
}

// metodo pubblico (attraverso membro puntatore)
// che lavora sui risultati del fit e calcola la funzione+fondo
double CXCFitSpek::GFitFunc(double xx)
{
  double yy, f1, f2, fsum, back, val;

  if(!m_bGauss) return 0.;

  fsum = 0.;
  for(int np = 0; np < m_nNpeaks; np++) {
    yy   = (xx - m_pRes[np].Posi) / m_pRes[np].Sigma;

    if(m_bTailL && yy < m_pRes[np].TailL) {
      double yl = m_pRes[np].TailL;
      f1 = exp(-0.5*yl*(2.*yy-yl));
    }
    else if(m_bTailR && yy > m_pRes[np].TailR) {
      double yr = m_pRes[np].TailR;
      f1 = exp(-0.5*yr*(2.*yy-yr));
    }
    else {
      f1 = exp(-0.5*yy*yy);
    }

    f2 = 0.;
    if(m_bStep) {
      double ey = exp(yy);
      double ez = 1. / (1. + ey);
      f2 = m_pRes[np].Step * ez * ez;
    }

    fsum += m_pRes[np].Ampli * (f1 + f2);
  }

  back = 0;
  if(m_bFlat ) back += m_dBack0;
  if(m_bSlope) back += m_dBack1*(xx-m_nChanA);

  val = back + fsum;
  return val;

}

// metodo pubblico (attraverso membro puntatore)
// che lavora sui risultati del fit e calcola la funzione per un picco
double CXCFitSpek::GFitPeak(double xx, int np)
{
  double yy, f1;

  if(!m_bGauss || np < 0 || np > m_nNpeaks) return 0.;

    yy   = (xx - m_pRes[np].Posi) / m_pRes[np].Sigma;

    if(m_bTailL && yy < m_pRes[np].TailL) {
      double yl = m_pRes[np].TailL;
      f1 = exp(-0.5*yl*(2.*yy-yl));
    }
    else if(m_bTailR && yy > m_pRes[np].TailR) {
      double yr = m_pRes[np].TailR;
      f1 = exp(-0.5*yr*(2.*yy-yr));
    }
    else {
      f1 = exp(-0.5*yy*yy);
    }

    f1 *= m_pRes[np].Ampli;

  return f1;

}

// metodo pubblico (attraverso membro puntatore)
// che lavora sui risultati del fit e calcola il solo fondo
double CXCFitSpek::GFitBack(double xx)
{
  double yy, fsum, back, val;

  if(!m_bGauss) return 0.;

  fsum = 0.;
  if(m_bStep) {
    for(int np = 0; np < m_nNpeaks; np++) {
      yy = (xx - m_pRes[np].Posi) / m_pRes[np].Sigma;
      double ey = exp(yy);
      double ez = 1. / (1. + ey);
      double f2 = m_pRes[np].Step * ez * ez;
      fsum += m_pRes[np].Ampli * f2;
    }
  }

  back = 0;
  if(m_bFlat ) back += m_dBack0;
  if(m_bSlope) back += m_dBack1*(xx-m_nChanA);

  val =  back + fsum;
  return val;

}

// Minimizzazione del chi2 con il metodo Marquardt, secondo Bevington-1 pag. 237
// Puntatori alle funzioni m_pfFunct, m_pfDeriv sono membri privati della classe CXCFitSpek
// Matrici [][] usate come 1D-array indirizzate per righe (C-style) con la macro M12(i1,i2,l2)

double CXCFitSpek::Curfit(int ndat, int npar, int mpar)
{
  double *vvalue = new double [npar];     // vettore dei  parametri di fit
  double *verror = new double [npar];     // vettore degli errori dei parametri di fit
  double *deriv  = new double [npar];     // vettore delle derivate calcolato in m_pfDeriv()

  double *beta  = new double [mpar];      // Vettore del gradiente
  double *alpha = new double [mpar*mpar]; // Matrice di curvatura
  double *sqrp  = new double [mpar];      // sqrt(alpha[i][i])
  double *array = new double [mpar*mpar]; // matrice (modificata) di curvatura

  int i, j, k;
  double dyi,wwi, wwiderj;
  double dpar;

  // riporta i parametri di fit
  for(i = 0; i < npar; i++) vvalue[i] = m_pPar[i].Value;

  double chisqr = Chisqr(vvalue);   // chi2 iniziale
  double chisqt, chisq1, deter;
  double flamda = 0.001;            // valore iniziale variabile controllo
  int    nsteps = 0;                // conta il numero di passi effettuati

  //////////////////////////////////////////////////////////////
  // max 20 iterazione se chi2 aumenta dopo la minimizzazione

  for(int iter = 0; iter < 20; iter++) {
    // il vecchio valore di chi2
    chisqt = chisqr;

    // accumula alpha e beta
    memset(beta,  0, sizeof(double)*mpar);
    memset(alpha, 0, sizeof(double)*mpar*mpar);
    for(i = 0; i < ndat; i++) {
      // calcola le derivate
      (this->*m_pfDeriv)(i, vvalue, deriv, dyi, wwi);

      // impacchetta le derivate
      if(mpar < npar) Manage(deriv, 1);

      // accumula alpha e beta sugli mpar parametri liberi
      for(j = 0; j < mpar; j++) {
        if(deriv[j]) {
          wwiderj  = wwi*deriv[j];
          beta[j] += dyi*wwiderj;
          for(k = 0; k <= j; k++) {
            alpha[M12(j,k,mpar)] += deriv[k]*wwiderj;
          }
        }
      }
    }
    // riempie il triangolo superiore
    for(j=0; j < mpar; j++) {
      for(k=0; k < j; k++) {
        alpha[M12(k,j,mpar)] = alpha[M12(j,k,mpar)];
      }
    }

    // con questa modifica degli elementi diagonali nulli, e' possibile continuare
    // a usare tutta la matrice e cioe' anche righe e colonne di parametri fissi
    // e' ancora necessaria ???
    for(j = 0; j < mpar; j++) {
      if(alpha[M12(j,j,mpar)] < 1.e-15)
        alpha[M12(j,j,mpar)] = 1.e-15;
    }

    // salva le normalizzazioni per array
    for(j = 0; j < mpar; j++) {
      sqrp[j] = sqrt(alpha[M12(j,j,mpar)]);
    }

    // loop su flambda
    while(true) {

      // calcola la matrice di curvatura modificata
      for(j = 0; j < mpar; j++) {
        for(k = 0; k < mpar; k++) {
          array[M12(j,k,mpar)] = alpha[M12(j,k,mpar)]/(sqrp[j]*sqrp[k]) ;
        }
        array[M12(j,j,mpar)] = 1. + flamda ;
      }

      // inversione con gauss-jordan
      deter = Matinv(array, mpar, mpar);
      if(deter <= 0.) {                           // errore
        chisqr = -1.;
        goto cleanup;
      }

      // impacchetta le variabili per poter calcolare gli incrementi
      if(mpar < npar) Manage(vvalue, 2);

      // calcola gli incrementi
      for(j = 0; j < mpar; j++) {
        dpar = 0.;
        for(k = 0; k < mpar; k++) {
          dpar += array[M12(j,k,mpar)]*beta[k]/(sqrp[j]*sqrp[k]);
        }
        vvalue[j] += dpar;
      }

      // spacchettamento delle variabili
      if(mpar < npar) Manage(vvalue, 3);

      // verifica dei parametri legati
      Manage(vvalue, 4);

      // il nuovo valore di chi2
      chisq1 = Chisqr(vvalue);

      // se diminuito, accetta i valori ed eventualmente re-itera
      if(chisq1 < chisqr) {
        chisqr = chisq1;
        break;
      }

      // scarta gli incrementi appena calcolati
      for(i = 0; i < npar; i++)
        vvalue[i] = m_pPar[i].Value;
      // ri-prova con flambda piu' grande
      flamda *= 10.;
      if(flamda > 1.e20)                // ma solo fino a questo valore
        goto cleanup;

    }   // fine loop su flambda

    // accetta i nuovi parametri e decide se re-iterare
    for(i = 0; i < npar; i++) m_pPar[i].Value = vvalue[i];
    nsteps++;

    // calcola gli errori (impacchettati!!)
    for(j = 0; j < mpar; j++) {
      if(alpha[M12(j,j,mpar)] == 0)
        verror[j] = 0.;
      else
        verror[j] = sqrt(array[M12(j,j,mpar)]/alpha[M12(j,j,mpar)]);
    }
    //spacchetta gli errori
    if(mpar < npar) Manage(verror, 5);

    if((chisqt-chisqr)/chisqr < 0.001)      // chi2 stabilizzato --> fine delle iterazioni
      break;

    // diminuisce flambda (fino a 10^-7) e re-itera
    if(flamda > 1.e-6)
      flamda *= 0.1;

  }  // fine loop sulle iterazioni

cleanup:
  delete [] deriv;
  delete [] beta;
  delete [] alpha;
  delete [] sqrp;
  delete [] array;
  delete [] vvalue;
  delete [] verror;

  return (nsteps) ? (chisqr) : (-2);
}

// calcolo del chiquadro
double CXCFitSpek::Chisqr(double *xpar)
{
  double xx = 0.5;
  double chi=0;
  for(int ii = 0; ii < m_nNdat; ii++, xx += 1.) {
    double xval = (this->*m_pfFunct)(xx, xpar);
    double dval = m_pDatVal[ii] - xval;
    chi += m_pDatErr[ii] * dval*dval;
  }
  return chi;
}

// gestione dell'impacchettamento e dei limiti
void CXCFitSpek::Manage(double *xpar, int flag)
{
  int j, k;

  switch (flag) {
  case 1:
    // somma le derivate dei parametri legati
    for(j = 0; j < m_nNpar; j++) {
      if(m_pPar[j].Status == p_Linked) {
        xpar[m_pPar[j].LinkTo] += xpar[j];
      }
    }
    // impacchettamento delle derivate
    k = 0;
    for(j = 0; j < m_nNpar; j++) {
      if(m_pPar[j].Status == p_Free) {
        xpar[k] = xpar[j];
        k++;
      }
    }
    break;
  case 2:
    // impacchettamento dei parametri liberi
    k = 0;
    for(j = 0; j < m_nNpar; j++) {
      if(m_pPar[j].Status == p_Free) {
        xpar[k] = xpar[j];
        m_pPar[j].PackId = k;
        k++;
      }
    }
    break;
  case 3:
    // spacchettamento dei parametri liberi
    for(j = m_nNpar-1; j >=0; j--) {
      if(m_pPar[j].Status == p_Free)
        xpar[j] = xpar[m_pPar[j].PackId];
    }
    // riprende quelli legati e quelli fissi
    for(j = m_nNpar-1; j >=0; j--) {
      if(m_pPar[j].Status == p_Fixed)
        xpar[j] = m_pPar[j].Value;
      else if(m_pPar[j].Status == p_Linked)
        xpar[j] = xpar[m_pPar[j].LinkTo];
    }
    break;
  case 4:
    // verifica i limiti imposti ai parametri liberi
    for(j = 0; j < m_nNpar; j++) {
      if(m_pPar[j].Status == p_Free && m_pPar[j].Bounds != p_BNone) {
        if(m_pPar[j].Bounds & p_BLow)
          xpar[j] = max(m_pPar[j].Valmin, xpar[j]);
        if(m_pPar[j].Bounds & p_BHig)
          xpar[j] = min(m_pPar[j].Valmax, xpar[j]);
      }
    }
    // aggiorna i valori dei parametri legati ad altri parametri
    // che possono essere stati modificati dai limiti
    for(j = 0; j < m_nNpar; j++) {
      if(m_pPar[j].Status == p_Linked) {
        xpar[j] = xpar[m_pPar[j].LinkTo];
      }
    }
    break;
  case 5:
    // spacchettamento degli errori
    for(j = m_nNpar-1; j >=0; j--) {
      if(m_pPar[j].Status == p_Free)
        m_pPar[j].Error = xpar[m_pPar[j].PackId];
      else
        m_pPar[j].Error = 0;
    }
    for(j = 0; j < m_nNpar; j++) {
      if(m_pPar[j].Status == p_Fixed)
        m_pPar[j].Error = 0.;
      else if(m_pPar[j].Status == p_Linked)
        m_pPar[j].Error = m_pPar[m_pPar[j].LinkTo].Error;
    }
    break;
  }
}

// inversione di matrice simmetrica definita positiva
// con il metodo di eliminazione Gauss-Jordan e pivot
// Bevington-1 pag.302
// Matrice [mpar][mpar] usa un unico blocco di memoria (vettore) con elementi
// della matrice indirizzati per righe (C-style) con la macro M12(i1,i2,mpar)
// Viene invertita solo la sottomatrice [npar][npar]

double CXCFitSpek::Matinv(double  *array, int mpar, int npar)
{
  int ii, jj, kk;
  double det, amax, damax, dd;

  if(npar > mpar) return 0.;
  if(npar < 1)    return 0.;
  if(npar == 1) {
    amax = array[0];
    if(amax == 0) return 0.;
    array[0] = 1./amax;
    return amax;
  }

  int *iik = new int[npar];
  int *jjk = new int[npar];

  det = 1.;
  for(kk = 0; kk < npar; kk++) {
    // find largest element in rest of array
    amax = 0;
    for(ii = kk; ii < npar; ii++) {
      for(jj = kk; jj < npar; jj++) {
        dd = array[M12(ii,jj,mpar)];
        if(fabs(dd) > fabs(amax)) {
          amax = dd;
          iik[kk] = ii;
          jjk[kk] = jj;
        }
      }
    }

    if(amax == 0.) {
      det = 0.;
      goto cleanup;
    }
    // interchange rows and columns to bring amax into array[kk][kk]
    damax = 1./amax;
    ii = iik[kk];
    if(ii > kk) {
      for(jj = 0; jj < npar; jj++) {
        dd = array[M12(kk,jj,mpar)];
        array[M12(kk,jj,mpar)] = array[M12(ii,jj,mpar)];
        array[M12(ii,jj,mpar)] = -dd;
      }
    }
    jj = jjk[kk];
    if(jj > kk) {
      for(ii = 0; ii < npar; ii++) {
        dd = array[M12(ii,kk,mpar)];
        array[M12(ii,kk,mpar)] = array[M12(ii,jj,mpar)];
        array[M12(ii,jj,mpar)] = -dd;
      }
    }

    // accumulate elements of inverse matrix
    for(ii = 0; ii < npar; ii++) {
      if(ii != kk)
        array[M12(ii,kk,mpar)] *= -damax;
    }
    for(ii = 0; ii < npar; ii++) {
      if(ii == kk) continue;
      for(jj = 0; jj < npar; jj++) {
//      if(ii != kk && jj != kk)
        if(jj != kk)
          array[M12(ii,jj,mpar)] += array[M12(ii,kk,mpar)] * array[M12(kk,jj,mpar)];
      }
    }
    for(jj = 0; jj < npar; jj++) {
      if(jj != kk)
        array[M12(kk,jj,mpar)] *= damax;
    }
    array[M12(kk,kk,mpar)] = damax;
    det *= amax;
  }

  // restore ordering of matrix;
  for(kk = npar-1;  kk >= 0; kk--) {
    jj = iik[kk];
    if(jj > kk) {
      for(ii = 0; ii < npar; ii++) {
        dd = array[M12(ii,kk,mpar)];
        array[M12(ii,kk,mpar)] = -array[M12(ii,jj,mpar)];
        array[M12(ii,jj,mpar)] =  dd;
      }
    }
    ii = jjk[kk];
    if(ii > kk) {
      for(jj = 0; jj < npar; jj++) {
        dd = array[M12(kk,jj,mpar)];
        array[M12(kk,jj,mpar)] = -array[M12(ii,jj,mpar)];
        array[M12(ii,jj,mpar)] =  dd;
      }
    }
  }

cleanup:
  delete [] iik;
  delete [] jjk;
  return det;
}

/////////////////////////////////////////////////////
// Calcola la costante di decadimento esponenziale //
// su un fondo piatto calcolato in [bgA--bgB]      //
/////////////////////////////////////////////////////

bool CXCFitSpek::CalcExponential(float *pSpek, int chA, int chB, int bgA, int bgB)
{
  // nothing valid yet
  Reset();

  m_dBack0    = 0.;
  m_dBack1    = 0.;
  m_dBack0Err = 0.;
  m_dBack1Err = 0.;

  //  fondo piatto
  int ndat;
  if(bgA >-1 && bgB > -1) {
    if(bgA > bgB) {int tmp = bgA; bgA = bgB; bgB = tmp;}
    ndat = bgB-bgA+1;
    for(int nn = bgA; nn <= bgB; nn++)
      m_dBack0 += pSpek[nn];
    m_dBack0 /= ndat;
    m_bFlat  = true;
    m_bSlope = false;
  }

  if(chA > chB) {int tmp = chA; chA = chB; chB = tmp;}
  ndat = chB-chA+1;
  if(ndat < 2)
    return false;

  double s1 = 0., sx = 0., sxx = 0., sy = 0., syx = 0.;
  double x, y, e;
  x  = 0.5;           // considera il centro del canale
  float * pd = pSpek + chA;
  for(int ii = chA; ii <= chB; ii++, x += 1.) {
    y    = *pd++ - m_dBack0;
    if(y > 0) {
      y = log(y);
      e    = 1.;    // come trattare l'errore ??
      s1  += e;
      sx  += e*x;
      sxx += e*x*x;
      sy  += e*y;
      syx += e*y*x;
    }
  }
  double deter = s1*sxx-sx*sx;
  if(deter < 0.)
    return false;

  m_bValid  = true;
  m_bExp    = true;
  m_bBckgr  = m_bFlat || m_bSlope;
  m_bFunct  = true;
  m_bLine   = true;
  m_nNpeaks = 0;

  m_nChanA   = chA;
  m_nChanB   = chB;
  m_nBgChanA = bgA;
  m_nBgChanB = bgB;

  m_dExpAmpli = (sxx*sy - sx*syx)/deter;
  m_dExpDecay = (-sx*sy + s1*syx)/deter;
  m_dExpAmpli = exp(m_dExpAmpli);
  m_dExpDecay = -1./m_dExpDecay;

  // puntatori alle funzioni risultato del fit
  m_pfFitFunc = &CXCFitSpek::ExpFunc;     // exp + back
  m_pfFitPeak = &CXCFitSpek::ExpPeak;     // exp only
  m_pfFitBack = &CXCFitSpek::LineFunc;    // background only


  return true;
}

// metodo pubblico (attraverso membro puntatore)
// che lavora sui risultati del fit e ritorna esponenziale + fondo
double CXCFitSpek::ExpFunc(double xx)
{
  if(!m_bExp) return 0.;

  double val;
  xx  -= m_nChanA;
  val  = m_dExpAmpli * exp(-xx/m_dExpDecay);

  if(m_bFlat)
    val += m_dBack0;

  return val;
}

// metodo pubblico (attraverso membro puntatore)
// che lavora sui risultati del fit e ritorna esponenziale + fondo
double CXCFitSpek::ExpPeak(double xx, int /*nn*/)
{
  if(!m_bExp) return 0.;

  double val;
  xx  -= m_nChanA;
  val  = m_dExpAmpli * exp(-xx/m_dExpDecay);

  return val;
}

// the private methods for the sinusoidal fit
// works in offset-zero
double CXCFitSpek::SFfunF(double xx, double *xpar)
{
/*
           X   dove si vuole calcolare (riferito all'inizio della regione di fit)
  Par(0) = B0  fondo costante
  Par(1) = B1  fondo slope
  Par(2) = A   ampiezza
  Par(3) = B   frequenza
  Par(4) = C   fase

  Funct(X) = B0 + B1*X + A * sin(B*X + C)
*/

  double ampli, omega, phase, f1;

  int    np, offs;
  double fsum = 0.;
  for(np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    ampli = xpar[offs+0];
    omega = xpar[offs+1];
    phase = xpar[offs+2];
    f1 = ampli*sin(omega*xx+phase);
    fsum += f1;
  }
  return xpar[0] + xpar[1]*xx + fsum;
}

// calcola le derivate parziali della funzione nel punto i
void CXCFitSpek::SFderF(int id, double *xpar, double *deriv, double &deltay, double &weight)
{
  double ampli, omega, phase, angle, cosvv;

  double xx = id + 0.5; // derivatives calculated at mid channel

  deltay = m_pDatVal[id] - SFfunF(xx, xpar);
  weight = 1.;

  memset(deriv, 0, m_nNpar*sizeof(double));

  deriv[0] = 1.;   // background level
  deriv[1] = xx;   // background slope

  int np, offs;
  for(np = 0, offs = m_nOffB; np < m_nNpeaks; np++, offs += m_nOffN) {
    ampli = xpar[offs+0];
    omega = xpar[offs+1];
    phase = xpar[offs+2];
    angle = omega*xx+phase;
    cosvv = cos(angle) * ampli;

    deriv[offs+0] = sin(angle);
    deriv[offs+1] = cosvv * xx ;
    deriv[offs+2] = cosvv;
  }

  return;
}

// metodo pubblico (attraverso membro puntatore)
// che lavora sui risultati del fit e calcola la funzione+fondo
double CXCFitSpek::SFitFunc(double xx)
{
  double yy, f1, fsum, back, val;

  if(!m_bSin) return 0.;

  fsum = 0.;
  yy   = xx - m_nChanA;
  for(int np = 0; np < m_nNpeaks; np++) {
    f1    = m_pRes[np].Ampli *  sin( m_pRes[np].Omega * yy + m_pRes[np].Phase);
    fsum += f1;
  }

  back = 0;
  if(m_bFlat ) back += m_dBack0;
  if(m_bSlope) back += m_dBack1*(xx-m_nChanA);

  val = back + fsum;
  return val;

}

// metodo pubblico (attraverso membro puntatore)
// che lavora sui risultati del fit e calcola la funzione per un picco
double CXCFitSpek::SFitWave(double xx, int np)
{
  double yy, f1;

  if(!m_bValid || np < 0 || np > m_nNpeaks) return 0.;

  yy = xx - m_nChanA;
  f1 = m_pRes[np].Ampli *  sin( m_pRes[np].Omega * yy + m_pRes[np].Phase);

  return f1;

}
