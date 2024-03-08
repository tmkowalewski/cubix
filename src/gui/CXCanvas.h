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

#ifndef __CXCanvas_H
#define __CXCanvas_H

#include "TCanvas.h"
#include "TH2.h"
#include "TH1.h"
#include "TGFrame.h"
#include "TList.h"

class TGraph;
class CXMainWindow;
class CXCanvas : public TCanvas {

protected:

   CXMainWindow *fMainWindow = nullptr;

   Double_t xmin, xmax, ymin, ymax;
   Double_t oldx, oldy;
   Double_t xinit, yinit;
   //! variables for pan & scan
   Int_t X0, Y0;           //! coordinates of initial click in pad pixels
   Int_t NdisXbins, NdisYbins;      //! number of displayed bins on X & Y
   Int_t NXbins, Xfirst0, Xlast0;   //! number of bins on x-axis, initial displayed bins
   Int_t NYbins, Yfirst0, Ylast0;   //! number of bins on y-axis, initial displayed bins
   TAxis* theXaxis, *theYaxis;   //! the axes of the histogram
   Double_t XbinPixel, YbinPixel;   //! size of bins in pixels
   Int_t Xf1, Xl1, Yf1, Yl1;  //! last modification to axis limits

   Bool_t fkey_Left = false;
   Bool_t fkey_Right = false;
   Bool_t fkey_Down = false;
   Bool_t fkey_Up = false;

   Bool_t   moved1D = false;
   Bool_t   moved2D = false;
   Bool_t   button1double = false;
   Bool_t   fPPressed;
   Bool_t   fJPressed;
   Bool_t   fAgeOfEmpireMode;
   Bool_t   fVenerMode;
   Bool_t   fHasDisabledClasses;
   TString  fDisabledClasses;
   Bool_t   fHasDisabledObject;

   TList    fDisabledObjects;
   TList    fShortCuts;
   Int_t    fEnabledShortcuts;
   TString  fSavedAs;

   TGFrame* fKeyHandler;         //! handler for keys
   Bool_t   fFreezed;

   TBox *fZoomBox = nullptr;

   Int_t fLastX;
   Int_t fLastY;
   Int_t fLastEvent;
   TObject *fLastSelected;

   TH1 *fSelectedHisto;
   Float_t ScaleFact;

   Int_t fNPads;

public:
   CXCanvas();
//   using TCanvas::TCanvas;
   CXCanvas(const char* name, const char* title, Int_t ww, Int_t wh, Bool_t keyHandler = kTRUE);
   CXCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid);
   virtual ~CXCanvas();

   void HandleInput(EEventType event, Int_t px, Int_t py);
   void EventProcessed(Event_t *ev, Window_t);

   Bool_t IsLogz();
   Bool_t IsLogy();
   Bool_t IsLogx();

   void DisableClass(const char* className);
   void DisableObject(TObject* obj);
   void ResetDisabledClass();
   void ResetDisabledObject();

   void SaveCanvasAs();
   TCanvas *GetTCanvas();

   void SetMainWindow(CXMainWindow *w);

   TH1 *GetSelectedHisto(){return fSelectedHisto;}

   Int_t GetNPads(){return fNPads;}
   void SetNPads(Int_t npads) {fNPads = npads;}

   void FreezCavans(Bool_t freez) { fFreezed = freez;}

   void ShowShortcutsInfos(); // *MENU*

   void SetAgeOfEmpireMode(Int_t value = 1); // *TOGGLE*
   void SetVenerMode(Int_t value = 1); // *TOGGLE*

   Int_t GetVenerMode() { return fVenerMode; }

   Int_t GetAgeOfEmpireMode() {return fAgeOfEmpireMode;}

   void SetEnabledShortcuts(Int_t value = 1); // *TOGGLE*
   Int_t GetEnabledShortcuts() { return fEnabledShortcuts; }

   TH1* FindHisto(TVirtualPad *pad = nullptr);
   TGraph* FindGraph(TVirtualPad *pad = nullptr);
   TAxis* GetAxis(TVirtualPad *pad = nullptr, int _iaxis=0);

   void SaveHistToAsciiFile();

   void CopyObject(TObject *obj);
   void CutObject(TObject *obj);
   void UndrawObject(TObject *obj);
   void Paste(); // *MENU*

protected:

//   virtual Bool_t HandleKey(Event_t* /*event*/){return kTRUE;}
   virtual Bool_t HandleKey(Int_t px, Int_t py);

   void DynamicZoom(Int_t Sign, Int_t px, Int_t py);
   void DynamicZoom1D(TAxis *axis, Int_t Sign);
   void RunAutoExec();
   void DrawEventStatus(Int_t event, Int_t px, Int_t py, TObject* selected);
   void ZoomSelected(TH2* TheHisto);

   void MoveAxis(TAxis* ax, Int_t sign);
   void ProfileX(TH2* hh);
   void ProfileY(TH2* hh);
   void ProjectionX(TH2* hh);
   void ProjectionY(TH2* hh);

   Bool_t ExpandFunctionRange();

   void InitInfos();
   void AddShortcutsInfo(const char* cut, const char* desc);

   ClassDef(CXCanvas, 1) //TCanvas with mouse-controlled dynamic zoom and pan & scan
};

//................  global variable
R__EXTERN TObject* gCopyObject;
R__EXTERN TString gDrawOptions;

#endif
