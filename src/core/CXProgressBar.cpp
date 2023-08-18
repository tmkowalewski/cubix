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

#include "CXProgressBar.h"

#include <iostream>
#include <fstream>
#include <iomanip>

#include "TString.h"

using namespace std;

CXProgressBar::CXProgressBar(ULong64_t Nevts) :
    fWatch(new TStopwatch()),
    fNEvts(Nevts),
    fCurrentEntry(0),
    fLastValidEntry(0),
    fLastValidTime(0.)
{
    fWatch->Start();
}

CXProgressBar::~CXProgressBar()
{
    fWatch->Stop();

    Double_t  realt = fWatch->RealTime();
    Double_t  cput  = fWatch->CpuTime();

    Int_t  hours = Int_t(realt / 3600);
    realt -= hours * 3600;
    Int_t  min   = Int_t(realt / 60);
    realt -= min * 60;
    Int_t  sec   = Int_t(realt);

    if (cput  < 0) cput  = 0;

    Printf("Real time %d:%02d:%02d, CP time %.3f", hours, min, sec, cput);

    delete fWatch;
}

void CXProgressBar::UpdateStatus()
{
    Int_t Step = (Int_t)(((Double_t)fCurrentEntry)/(((Double_t)fNEvts)/20.));

    Int_t RefreshRate = (Int_t)(((Float_t)fNEvts)/1e5); // Pour 1 affichage par seconde a un rate de 50evts/s
    if(RefreshRate == 0) RefreshRate = 1;

    if(((Int_t)(fNEvts/RefreshRate)) == 0  || ((Int_t)fCurrentEntry)%((Int_t)(fNEvts/RefreshRate))==0)
    {
        Float_t Frac = ((Double_t)fCurrentEntry)/((Double_t)fNEvts);

        clog<<"\rTotal "<<fNEvts<<" events |";
        for(int it=0 ; it<Step; it++)
            clog<<"=";
        clog<<">";
        for(int it=Step+1 ; it<20 ; it++)
            clog<<".";
        clog<<"| "<<setw(6)<<Form("%.2f%%",Frac*100.);
        fWatch->Stop();

        Float_t Time = fWatch->CpuTime();
        Float_t TimeLeftTotSec = ((1-Frac)*Time)/Frac;
        UInt_t TimeLeftInHour = (UInt_t)(TimeLeftTotSec/3600.);
        UInt_t TimeLeftInMin = (UInt_t)((TimeLeftTotSec - TimeLeftInHour*3600.)/60.);
        UInt_t TimeLeftInSec = (UInt_t)(TimeLeftTotSec - TimeLeftInHour*3600. - TimeLeftInMin*60.);

        Float_t Rate = ((Float_t)(fCurrentEntry-fLastValidEntry))/((Float_t)(Time-fLastValidTime));

        //Float_t Rate = ((Float_t)(fCurrentEntry))/((Float_t)(Time));
        if(isnan(Rate)) Rate=0.;
        if(isinf(Rate)) Rate=0.;

        fLastValidEntry = fCurrentEntry;
        fLastValidTime = Time;

        clog<<" [ "<<setw(5)<<Form("%.1f",Rate)<<" evts/s, time left: "<<Form("%3dh%.2dmin%.2ds ]",TimeLeftInHour,TimeLeftInMin,TimeLeftInSec);
        fWatch->Start(false);
        clog<<flush;
    }
}
