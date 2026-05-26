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

#include "CXGammaSource.h"

#include <fstream>

#include "TSystem.h"
#include "TObjArray.h"

CXGammaSource::CXGammaSource(const char *name) : TNamed(name,name)
{
    Form("%s/databases/Sources",getenv("CUBIX_SYS"));

    // trying to find a real source data (with intensities): .sou
    TString FileName = Form("%s/databases/Sources/%s.sou",getenv("CUBIX_SYS"),name);
    if(gSystem->IsFileInIncludePath(FileName)) {
        ifstream file(FileName);
        string line;
        TString Buffer;
        while(true) {
            getline(file,line);
            if(!file) break;
            Buffer = line;
            if(Buffer.BeginsWith("#") || Buffer.BeginsWith("A")) continue;
            Buffer.ReplaceAll("\t"," ");
            TObjArray *arr = Buffer.Tokenize(" ");
            if(arr->GetEntries() != 6) {
                continue;
                delete arr;
            }
            CXGammaSourceDecay decay;
            decay.DecayMode = arr->At(0)->GetName();
            decay.Daughter = arr->At(1)->GetName();
            TString ener = arr->At(2)->GetName();
            TString ener_err = arr->At(3)->GetName();
            if(ener.Contains("*")) {
                decay.is_ref = true;
                ener.ReplaceAll("*","");
            }
            decay.energy.set(ener.Atof(),"keV");
            decay.energy.set_error(ener_err.Atof());
            decay.energy.set_info(tkn::tkproperty::kKnown);
            decay.energy.set_type("E");
            TString intensity = arr->At(4)->GetName();
            TString intensity_err = arr->At(5)->GetName();
            decay.intensity.set(intensity.Atof(),"%");
            decay.intensity.set_error(intensity_err.Atof());
            decay.intensity.set_info(tkn::tkproperty::kKnown);
            decay.intensity.set_type("I");
            fdecays.push_back(decay);
            delete arr;
        }
        fis_known = true;
        fis_source = true;
    }
    // if not existing trying to find a calibration data (without intensities): .cal
    else {
        FileName = Form("%s/databases/Sources/%s.cal",getenv("CUBIX_SYS"),name);
        if(gSystem->IsFileInIncludePath(FileName)) {
            ifstream file(FileName);
            string line;
            TString Buffer;
            while(true) {
                getline(file,line);
                if(!file) break;
                Buffer = line;
                if(Buffer.BeginsWith("#")) continue;
                Buffer.ReplaceAll("\t"," ");
                TObjArray *arr = Buffer.Tokenize(" ");
                if(arr->GetEntries() == 0) {
                    continue;
                    delete arr;
                }
                CXGammaSourceDecay decay;
                TString textline = (TString)arr->First()->GetName();
                if(textline.Contains("*")) {
                    textline.ReplaceAll("*","");
                    decay.is_ref = true;
                }
                Double_t ener = textline.Atof();
                decay.energy.set(ener,"keV");
                decay.energy.set_error(0.);
                fdecays.push_back(decay);
                delete arr;
            }
            fis_known = true;
            fis_source = false;
        }
    }
}
