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

#ifndef __CXHistoManipulator_H
#define __CXHistoManipulator_H

#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "TList.h"
#include "TString.h"
#include "TGraph.h"
#include "TGraphErrors.h"

class TCanvas;
class TMultiGraph;

class CXHistoManipulator {
   Bool_t kVisDebug;// = kTRUE for visual debugging
   TCanvas* fVDCanvas;//! used for visual debugging

public:

   void init(void)
   {

   }

   CXHistoManipulator();
   virtual ~CXHistoManipulator(void);

   void SetVisDebug(Bool_t on = kTRUE)
   {
      // Turn on/off 'VisualDebugging' for RescaleX methods.
      // After calling SetVisDebug(kTRUE), whenever the rescaling procedure
      // is executed, a TCanvas is drawn showing 4 pads, which show:
      //  (1) the two histograms to be rescaled
      //  (2) the cumulative distributions of the two histograms
      //  (3) the fit to the comparison points of the two cumulative distributions
      //  (4) a comparison of the resulting rescaled histograms
      // See MakeHistoRescaleX for an example.
      kVisDebug = on;
   };
   Bool_t IsVisDebug() const
   {
      return kVisDebug;
   };

   Int_t CutStatBin(TH1* hh, Int_t stat_min = -1, Int_t stat_max = -1);

   Int_t Apply_TCutG(TH2* hh, TCutG* cut, TString mode = "in");

   TH1*  ScaleHisto(TH1* hh, TF1* fx, TF1* fy = NULL, Int_t nx = -1, Int_t ny = -1,
                    Double_t xmin = -1., Double_t xmax = -1., Double_t ymin = -1., Double_t ymax = -1., Option_t* norm = "");
   TGraph* ScaleGraph(TGraph* hh, TF1* fx, TF1* fy);

   TH1*  CentreeReduite(TH1* hh, Int_t nx = -1, Int_t ny = -1, Double_t xmin = -1., Double_t xmax = -1., Double_t ymin = -1., Double_t ymax = -1.);
   TH2*  CentreeReduiteX(TH2* hh, Int_t nx = -1, Double_t xmin = -1., Double_t xmax = -1.);
   TH2*  CentreeReduiteY(TH2* hh, Int_t ny = -1, Double_t ymin = -1., Double_t ymax = -1.);

   TH2*  RenormaliseHisto(TH2* hh, Int_t bmin = -1, Int_t bmax = -1, TString axis = "X", Double_t valref = 1);
   TH2*  RenormaliseHisto(TH2* hh, Double_t valmin, Double_t valmax, TString axis = "X", Double_t valref = 1);

   TH1*  CumulatedHisto(TH1* hh, TString direction = "C", Int_t bmin = -1, Int_t bmax = -1, Option_t* norm = "surf");
   TH1*  CumulatedHisto(TH1* hh, Double_t xmin, Double_t xmax, TString direction = "C", Option_t* norm = "surf");
   TH1*  GetDerivative(TH1* hh, Int_t order);

   Double_t GetCorrelationFactor(TH2* hh);
   TGraph* LinkGraphs(TGraph* grx, TGraph* gry);

   TList* Give_ProjectionList(TH2* hh, Double_t MinIntegral = -1, TString axis = "X");

   TH2* PermuteAxis(TH2* hh);
   TGraph* PermuteAxis(TGraph* gr);
   TGraphErrors* MakeGraphFrom(TProfile* pf, Bool_t Error = kTRUE);

   void DefinePattern(TH1* ob, TString titleX = "42 0.08 0.8", TString titleY = "42 0.07 1.2", TString labelX = "42 0.05 0.005", TString labelY = "42 0.05 0.006");
   void DefinePattern(TGraph* ob, TString titleX = "42 0.08 0.8", TString titleY = "42 0.07 1.2", TString labelX = "42 0.05 0.005", TString labelY = "42 0.05 0.006");
   void DefinePattern(TF1* ob, TString titleX = "42 0.08 0.8", TString titleY = "42 0.07 1.2", TString labelX = "42 0.05 0.005", TString labelY = "42 0.05 0.006");
   void DefinePattern(TAxis* ax, TString title = "42 0.08 0.8", TString label = "42 0.05 0.005");

   void DefineLineStyle(TAttLine* ob, TString line);
   void DefineMarkerStyle(TAttMarker* ob, TString marker);
   void DefineStyle(TObject* ob, TString line, TString marker);

   void DefineTitle(TH1* ob, TString xtit, TString ytit);
   void DefineTitle(TGraph* ob, TString xtit, TString ytit);
   void DefineTitle(TF1* ob, TString xtit, TString ytit);

   Double_t GetX(TH1* ob, Double_t val, Double_t eps = 1.e-07, Int_t nmax = 50, Double_t xmin = -1.0, Double_t xmax = -1.0);
   Double_t GetXWithLimits(TH1* ob, Double_t val, Double_t xmin = -1.0, Double_t xmax = -1.0, Double_t eps = 1.e-07, Int_t nmax = 50)
   {
      // See method GetX(): the only difference is the order of the arguments
      return GetX(ob, val, eps, nmax, xmin, xmax);
   }
   TF1* RescaleX(TH1* hist1, TH1* hist2, Int_t degree, Double_t* params,
                 Int_t npoints = -1, const Char_t* direction = "C",
                 Double_t xmin = -1, Double_t xmax = -1, Double_t qmin = 0.05, Double_t qmax = 0.95,
                 Double_t eps = 1.e-07);
   void RescaleX(TH1* hist1, TH1* hist2, TF1* scale_func, Int_t npoints = 2,
                 const Char_t* direction = "C", Double_t xmin = -1, Double_t xmax = -1, Double_t qmin = 0.05, Double_t qmax = 0.95,
                 Double_t eps = 1.e-07);
   TH1* MakeHistoRescaleX(TH1* hist1, TH1* hist2, Int_t degree, Double_t* params,
                          Option_t* opt = "", Int_t npoints = -1, const Char_t* direction = "C",
                          Double_t xmin = -1, Double_t xmax = -1, Double_t qmin = 0.05, Double_t qmax = 0.95,
                          Double_t eps = 1.e-07);
   TH1* MakeHistoRescaleX(TH1* hist1, TH1* hist2, TF1* scale_func, Int_t npoints = 2,
                          Option_t* opt = "", const Char_t* direction = "C",
                          Double_t xmin = -1, Double_t xmax = -1, Double_t qmin = 0.05, Double_t qmax = 0.95,
                          Double_t eps = 1.e-07);

   Double_t GetChisquare(TH1* h1, TF1* f1, Bool_t norm = kTRUE, Bool_t err = kTRUE, Double_t* para = 0);
   Double_t GetLikelihood(TH1* h1, TF1* f1, Bool_t norm = kTRUE, Double_t* para = 0);

   TGraph* DivideGraphs(TGraph* G1, TGraph* G2);
   Double_t* GetLimits(TGraph* G1);
   Double_t* GetLimits(TProfile* G1);
   Double_t* GetLimits(TMultiGraph* mgr);
   Double_t* GetLimits(TSeqCollection* mgr);
   void ApplyCurrentLimitsToAllCanvas(Bool_t AlsoLog = kFALSE);

   ClassDef(CXHistoManipulator, 0) //Propose differentes operations sur les histo
};

//................  global variable
R__EXTERN CXHistoManipulator* gHistoManipulator;

#endif
