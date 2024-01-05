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

#include "CXWSManager.h"

#include <iostream>
#include <fstream>
#include <map>

#include "TGButton.h"
#include "TGListBox.h"
#include "TError.h"
#include "TGFileDialog.h"
#include "TGraphErrors.h"
#include "TSystem.h"
#include "TH1.h"
#include "TF1.h"
#include "TSystemDirectory.h"
#include "TGLabel.h"
#include "TVirtualX.h"
#include "TGFSContainer.h"
#include "TGListView.h"
#include "TGTextEntry.h"

#include "CXMainWindow.h"
#include "CXDialogBox.h"
#include "CXFitFunctions.h"

#include "tklog.h"

using namespace std;

CXWorkspace::CXWorkspace(const char *_name, const char *_dir) : TNamed(_name,_name),  fname(_name), fdirectory(_dir)
{
    if(!gSystem->AccessPathName(Form("%s/%s/%s.conf",_dir,_name,_name))) {
        ReadWS();
    }
    else UpdateWSFile();
}

void CXWorkspace::ReadWS()
{
    ifstream file(Form("%s/%s/%s.conf",fdirectory.Data(),fname.Data(),fname.Data()));
    TString line;
    while(line.ReadLine(file)) {
        line.ReplaceAll("\t", " ");
        TObjArray *arr = line.Tokenize(" ");
        if(arr->GetEntries()<=1) continue;
        TString filename = arr->Last()->GetName();
        if(line.Contains("Calibration graph")) fCalibGraphFileName = filename;
        else if(line.Contains("Calibration function")) fCalibFuncFileName = filename;
        else if(line.Contains("Calibration error")) fCalibErrorFileName = filename;
        else if(line.Contains("Calibration residue")) fCalibResidueFileName = filename;

        else if(line.Contains("FWHM graph")) fFWHMGraphFileName = filename;
        else if(line.Contains("FWHM function")) fFWHMFuncFileName = filename;
        else if(line.Contains("FWHM error")) fFWHMErrorFileName = filename;

        else if(line.Contains("Efficiency graph")) fEfficiencyGraphFileName = filename;
        else if(line.Contains("Efficiency function")) fEfficiencyFuncFileName = filename;
        else if(line.Contains("Efficiency error")) fEfficiencyErrorFileName = filename;
    }

    TString fileName;

    if(fCalibGraphFileName != "None") {
        delete fCalibrationGraph;
        fCalibrationGraph = nullptr;
        fileName = (fCalibGraphFileName.BeginsWith("/")) ? fCalibGraphFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fCalibGraphFileName.Data());
        fCalibrationGraph = new TGraphErrors(fileName);
        if(!fCalibrationGraph || fCalibrationGraph->GetN()==0) {
            glog << tkn::error << "Cannot open or decode energy calibration file: " << fileName << tkn::do_endl;
        }
        else {
            fCalibrationGraph->SetName("CalibrationGraph");
            fCalibrationGraph->SetMarkerColor(kRed);
            fCalibrationGraph->SetMarkerStyle(20);
            fCalibrationGraph->GetXaxis()->SetTitle("Energy (channels)");
            fCalibrationGraph->GetYaxis()->SetTitle("Energy (keV)");
        }
    }
    if(fCalibFuncFileName != "None") {
        delete fCalibFunction;
        fCalibFunction = nullptr;
        fileName = (fCalibFuncFileName.BeginsWith("/")) ? fCalibFuncFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fCalibFuncFileName.Data());
        ifstream filetoread(fileName);
        if(!filetoread) {
            glog << tkn::error << "No energy calibration function file found named:" << fileName << tkn::do_endl;
        }
        else {
            TString line;
            map<TString,double> params;
            vector<double> range;
            while(line.ReadLine(filetoread)) {
                if(line.BeginsWith("#")) continue;
                TObjArray *arr = line.Tokenize(" ");
                if(line.BeginsWith("Range") && arr->GetEntries()==3) {
                    range.push_back(((TString)arr->At(1)->GetName()).Atof());
                    range.push_back(((TString)arr->At(2)->GetName()).Atof());
                }
                if(arr->GetEntries() != 2) continue;
                TString name = arr->First()->GetName();
                params[name] = atof(arr->Last()->GetName());
                delete arr;
            }
            if(!params.count("Order")) {
                glog << tkn::error << "Cannot decode order of calibration in file: " << fileName << tkn::do_endl;
            }
            else if(range.size() !=2 || params.size() != (TMath::Nint(params["Order"])+2)) {
                glog << tkn::error << "Error in decoding file: " << fileName << tkn::do_endl;
            }
            else {
                fCalibFunction = new TF1("ECalibration", &CXFitFunctions::PolynomialFunc, range.at(0), range.at(1), TMath::Nint(params["Order"])+2);
                fCalibFunction->SetLineColor(kBlue);
                fCalibFunction->SetNpx(5000);
                fCalibFunction->SetParName(0,"Order");
                for(int i=0 ; i<=TMath::Nint(params["Order"]) ; i++) fCalibFunction->SetParName(i+1,Form("a%d",i));
                for(const auto &par: params) fCalibFunction->SetParameter(par.first,par.second);
            }
        }
    }
    if(fCalibErrorFileName != "None") {
        delete fCalibrationErrors;
        fCalibrationErrors = nullptr;
        fileName = (fCalibErrorFileName.BeginsWith("/")) ? fCalibErrorFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fCalibErrorFileName.Data());
        ifstream file(fileName);
        if(!file) {
            glog << tkn::error << "Cannot open energy calibration error file: " << fileName << tkn::do_endl;
        }
        else {
            TString line;
            int nbins  = -1;
            double min = -1;
            double max = -1;
            while(line.ReadLine(file)) {
                if(!fCalibrationErrors && nbins>0 && min != 1 && max!=-1) {
                    fCalibrationErrors = new TH1D("CalibrationConfidence95","Calibration 0.95 confidence band", nbins, min, max);
                    fCalibrationErrors->SetLineWidth(0);
                    fCalibrationErrors->SetFillColor(kBlue);
                    fCalibrationErrors->SetFillColorAlpha(kBlue,0.1);
                    fCalibrationErrors->SetFillStyle(1001);
                    fCalibrationErrors->SetStats(false);
                    fCalibrationErrors->SetDirectory(nullptr);
                    if(fCalibrationGraph) {
                        fCalibrationErrors->GetXaxis()->SetTitle(fCalibrationGraph->GetXaxis()->GetTitle());
                        fCalibrationErrors->GetYaxis()->SetTitle(fCalibrationGraph->GetYaxis()->GetTitle());
                    }
                }
                TObjArray *arr = line.Tokenize(" ");
                if(line.BeginsWith("#")) {
                    if(line.Contains("N Bins")) nbins = atoi(arr->Last()->GetName());
                    else if(line.Contains("X min")) min = atof(arr->Last()->GetName());
                    else if(line.Contains("X max")) max = atof(arr->Last()->GetName());
                    else continue;
                }

                if(fCalibrationErrors && arr->GetEntries()==4) {
                    int ibin = atoi(arr->At(0)->GetName());
                    double content = atof(arr->At(2)->GetName());
                    double error = atof(arr->At(3)->GetName());
                    fCalibrationErrors->SetBinContent(ibin,content);
                    fCalibrationErrors->SetBinError(ibin,error);
                }
                delete arr;
            }
        }
    }
    if(fCalibResidueFileName != "None") {
        delete fCalibrationResidue;
        fCalibrationResidue = nullptr;
        fileName = (fCalibResidueFileName.BeginsWith("/")) ? fCalibResidueFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fCalibResidueFileName.Data());
        fCalibrationResidue = new TGraphErrors(fileName);
        if(!fCalibrationResidue || fCalibrationResidue->GetN()==0) {
            glog << tkn::error << "Cannot open or decode energy calibration residue file: " << fileName << tkn::do_endl;
        }
        else {
            fCalibrationResidue->SetName("CalibrationResidue");
            fCalibrationResidue->SetMarkerColor(kRed);
            fCalibrationResidue->SetMarkerStyle(20);
            fCalibrationResidue->GetXaxis()->SetTitle("Energy (keV)");
            fCalibrationResidue->GetYaxis()->SetTitle("Residue (keV)");
        }
    }


    if(fFWHMGraphFileName != "None") {
        delete fFWHMGraph;
        fFWHMGraph = nullptr;
        fileName = (fFWHMGraphFileName.BeginsWith("/")) ? fFWHMGraphFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fFWHMGraphFileName.Data());
        fFWHMGraph = new TGraphErrors(fileName);
        if(!fFWHMGraph || fFWHMGraph->GetN()==0) {
            glog << tkn::error << "Cannot open or decode FWHM file: " << fileName << tkn::do_endl;
        }
        else {
            fFWHMGraph->SetName("FWHMGraph");
            fFWHMGraph->SetMarkerColor(kRed);
            fFWHMGraph->SetMarkerStyle(20);
            fFWHMGraph->GetXaxis()->SetTitle("Energy (keV)");
            fFWHMGraph->GetYaxis()->SetTitle("FWHM (keV)");
        }
    }
    if(fFWHMFuncFileName != "None") {
        delete fFWHMFunction;
        fFWHMFunction = nullptr;
        fileName = (fFWHMFuncFileName.BeginsWith("/")) ? fFWHMFuncFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fFWHMFuncFileName.Data());
        ifstream filetoread(fileName);
        if(!filetoread) {
            glog << tkn::error << "No FWHM function file found named: " << fileName << tkn::do_endl;
        }
        else {
            TString line;
            map<TString,double> params;
            vector<double> range;
            while(line.ReadLine(filetoread)) {
                if(line.BeginsWith("#")) continue;
                TObjArray *arr = line.Tokenize(" ");
                if(line.BeginsWith("Range") && arr->GetEntries()==3) {
                    range.push_back(((TString)arr->At(1)->GetName()).Atof());
                    range.push_back(((TString)arr->At(2)->GetName()).Atof());
                }
                if(arr->GetEntries() != 2) continue;
                TString name = arr->First()->GetName();
                params[name] = atof(arr->Last()->GetName());
                delete arr;
            }
            if(range.size() !=2 || params.size() != 3) {
                glog << tkn::error << "Error in decoding file: " << fileName << tkn::do_endl;
            }
            else {
                fFWHMFunction = new TF1("FWHMFunc", &CXFitFunctions::FWHMFunction, range.at(0), range.at(1), 3);
                fFWHMFunction->SetLineColor(kBlue);
                fFWHMFunction->SetNpx(5000);
                fFWHMFunction->SetParNames("F","G","H");
                for(const auto &par: params) fFWHMFunction->SetParameter(par.first,par.second);
            }
        }
    }
    if(fFWHMErrorFileName != "None") {
        delete fFWHMErrors;
        fFWHMErrors = nullptr;
        fileName = (fFWHMErrorFileName.BeginsWith("/")) ? fFWHMErrorFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fFWHMErrorFileName.Data());
        ifstream file(fileName);
        if(!file) {
            glog << tkn::error << "Cannot open FWHM error file: " << fileName << tkn::do_endl;
        }
        else {
            TString line;
            int nbins  = -1;
            double min = -1;
            double max = -1;
            while(line.ReadLine(file)) {
                if(!fFWHMErrors && nbins>0 && min != 1 && max!=-1) {
                    fFWHMErrors = new TH1D("FWHMConfidence95","FWHM 0.95 confidence band", nbins, min, max);
                    fFWHMErrors->SetLineWidth(0);
                    fFWHMErrors->SetFillColor(kBlue);
                    fFWHMErrors->SetFillColorAlpha(kBlue,0.1);
                    fFWHMErrors->SetFillStyle(1001);
                    fFWHMErrors->SetStats(false);
                    fFWHMErrors->SetDirectory(nullptr);
                    if(fFWHMGraph) {
                        fFWHMErrors->GetXaxis()->SetTitle(fFWHMGraph->GetXaxis()->GetTitle());
                        fFWHMErrors->GetYaxis()->SetTitle(fFWHMGraph->GetYaxis()->GetTitle());
                    }
                }
                TObjArray *arr = line.Tokenize(" ");
                if(line.BeginsWith("#")) {
                    if(line.Contains("N Bins")) nbins = atoi(arr->Last()->GetName());
                    else if(line.Contains("X min")) min = atof(arr->Last()->GetName());
                    else if(line.Contains("X max")) max = atof(arr->Last()->GetName());
                    else continue;
                }

                if(fFWHMErrors && arr->GetEntries()==4) {
                    int ibin = atoi(arr->At(0)->GetName());
                    double content = atof(arr->At(2)->GetName());
                    double error = atof(arr->At(3)->GetName());
                    fFWHMErrors->SetBinContent(ibin,content);
                    fFWHMErrors->SetBinError(ibin,error);
                }
                delete arr;
            }
        }
    }

    if(fEfficiencyGraphFileName != "None") {
        delete fEfficiencyGraph;
        fEfficiencyGraph = nullptr;
        fileName = (fEfficiencyGraphFileName.BeginsWith("/")) ? fEfficiencyGraphFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fEfficiencyGraphFileName.Data());
        fEfficiencyGraph = new TGraphErrors(fileName);
        if(!fEfficiencyGraph || fEfficiencyGraph->GetN()==0) {
            glog << tkn::error << "Cannot open or decode efficiency file: " << fileName << tkn::do_endl;
        }
        else {
            fEfficiencyGraph->SetName("EfficiencyGraph");
            fEfficiencyGraph->SetMarkerColor(kRed);
            fEfficiencyGraph->SetMarkerStyle(20);
            fEfficiencyGraph->GetXaxis()->SetTitle("Energy (keV)");
            fEfficiencyGraph->GetYaxis()->SetTitle("Normalized area (counts)");
        }
    }
    if(fEfficiencyFuncFileName != "None") {
        delete fEfficiencyFunction;
        fEfficiencyFunction = nullptr;
        fileName = (fEfficiencyFuncFileName.BeginsWith("/")) ? fEfficiencyFuncFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fEfficiencyFuncFileName.Data());
        ifstream filetoread(fileName);
        if(!filetoread) {
            glog << tkn::error << "No efficiency function file found named: " << fileName << tkn::do_endl;
        }
        else {
            TString line;
            map<TString,double> params;
            vector<double> range;
            while(line.ReadLine(filetoread)) {
                if(line.BeginsWith("#")) continue;
                TObjArray *arr = line.Tokenize(" ");
                if(line.BeginsWith("Range") && arr->GetEntries()==3) {
                    range.push_back(((TString)arr->At(1)->GetName()).Atof());
                    range.push_back(((TString)arr->At(2)->GetName()).Atof());
                }
                else if(arr->GetEntries()==2) {
                    params[((TString)arr->At(0)->GetName())] = ((TString)arr->At(1)->GetName()).Atof();
                }
                delete arr;
            }
            if(range.size() !=2 || params.size()!=8) {
                glog << tkn::error << "Error in decoding file: " << fileName << tkn::do_endl;
            }
            else {
                fEfficiencyFunction = new TF1("EfficiencyFunc", &CXFitFunctions::EfficiencyFunc, range.at(0), range.at(1), 8);
                fEfficiencyFunction->SetLineColor(kBlue);
                fEfficiencyFunction->SetNpx(5000);
                fEfficiencyFunction->SetParNames("Scale","A","B","C","D","E","F","G");
                for(const auto &par: params) fEfficiencyFunction->SetParameter(par.first,par.second);
            }
        }
    }
    if(fEfficiencyErrorFileName != "None") {
        delete fEfficiencyErrors;
        fEfficiencyErrors = nullptr;
        fileName = (fEfficiencyErrorFileName.BeginsWith("/")) ? fEfficiencyErrorFileName : Form("%s/%s/%s",fdirectory.Data(),fname.Data(),fEfficiencyErrorFileName.Data());
        ifstream file(fileName);
        if(!file) {
            glog << tkn::error << "Cannot open efficiency error file: " << fileName << tkn::do_endl;
        }
        else {
            TString line;
            int nbins  = -1;
            double min = -1;
            double max = -1;
            while(line.ReadLine(file)) {
                if(!fEfficiencyErrors && nbins>0 && min != 1 && max!=-1) {
                    fEfficiencyErrors = new TH1D("EfficiencyConfidence95","Efficiency 0.95 confidence band", nbins, min, max);
                    fEfficiencyErrors->SetLineWidth(0);
                    fEfficiencyErrors->SetFillColor(kBlue);
                    fEfficiencyErrors->SetFillColorAlpha(kBlue,0.1);
                    fEfficiencyErrors->SetFillStyle(1001);
                    fEfficiencyErrors->SetStats(false);
                    fEfficiencyErrors->SetDirectory(nullptr);
                    if(fEfficiencyGraph) {
                        fEfficiencyErrors->GetXaxis()->SetTitle(fEfficiencyGraph->GetXaxis()->GetTitle());
                        fEfficiencyErrors->GetYaxis()->SetTitle(fEfficiencyGraph->GetYaxis()->GetTitle());
                    }
                }
                TObjArray *arr = line.Tokenize(" ");
                if(line.BeginsWith("#")) {
                    if(line.Contains("N Bins")) nbins = atoi(arr->Last()->GetName());
                    else if(line.Contains("X min")) min = atof(arr->Last()->GetName());
                    else if(line.Contains("X max")) max = atof(arr->Last()->GetName());
                    else continue;
                }

                if(fEfficiencyErrors && arr->GetEntries()==4) {
                    int ibin = atoi(arr->At(0)->GetName());
                    double content = atof(arr->At(2)->GetName());
                    double error = atof(arr->At(3)->GetName());
                    fEfficiencyErrors->SetBinContent(ibin,content);
                    fEfficiencyErrors->SetBinError(ibin,error);
                }
                delete arr;
            }
        }
    }
}

void CXWorkspace::UpdateWSFile()
{
    ofstream file(Form("%s/%s/%s.conf",fdirectory.Data(),fname.Data(),fname.Data()));

    TString header = "## Workspace " + fname + " ##";
    for(int i=0 ; i<header.Length() ; i++) {file << "#";} file << endl;
    file << header << endl;
    for(int i=0 ; i<header.Length() ; i++) {file << "#";} file << endl << endl;

    file << "Calibration graph   : " << fCalibGraphFileName << endl;
    file << "Calibration function: " << fCalibFuncFileName << endl;
    file << "Calibration error   : " << fCalibErrorFileName << endl;
    file << "Calibration residue : " << fCalibResidueFileName << endl;

    file << endl;
    file << "FWHM graph          : " << fFWHMGraphFileName << endl;
    file << "FWHM function       : " << fFWHMFuncFileName << endl;
    file << "FWHM error          : " << fFWHMErrorFileName << endl;

    file << endl;
    file << "Efficiency graph    : " << fEfficiencyGraphFileName << endl;
    file << "Efficiency function : " << fEfficiencyFuncFileName << endl;
    file << "Efficiency error    : " << fEfficiencyErrorFileName << endl;
    file.close();

    glog << tkn::info << "Updating Workspace: " << fname << " in " << fdirectory << tkn::do_endl;
}

CXWorkspace::~CXWorkspace()
{
    delete fCalibFunction;
    delete fCalibrationGraph;
    delete fCalibrationErrors;
    delete fCalibrationResidue;

    delete fFWHMFunction;
    delete fFWHMGraph;
    delete fFWHMErrors;

    delete fEfficiencyGraph;
    delete fEfficiencyFunction;
    delete fEfficiencyErrors;
}

void CXWorkspace::SetEfficiency(TGraph *_graph, TF1 *_func, TH1 *_error)
{
    TString filename = Form("%s/%s/%s_efficiency.dat",fdirectory.Data(),fname.Data(),fname.Data());
    if(!gSystem->AccessPathName(filename)) {
        glog << tkn::error << " file: " << filename << " already existing, plese delete it manually before saving workspace" << tkn::do_endl;
        return;
    }
    if(_graph) {
        ofstream outfile(filename);
        outfile<<"# "<< _graph->GetName() << " " << _graph->GetTitle() <<endl;
        outfile<<"# X axis : "<< _graph->GetXaxis()->GetTitle() <<endl;
        outfile<<"# Y axis : "<< _graph->GetYaxis()->GetTitle() <<endl;
        outfile<<"# N points : "<< _graph->GetN() <<endl;
        outfile<<"# X value     Y value       X error       Y error" <<endl;
        for(int ipoint = 0 ; ipoint < _graph->GetN() ; ipoint++) {
            outfile << left << setw(14) << _graph->GetX()[ipoint] << setw(14) << _graph->GetY()[ipoint] << setw(14) << _graph->GetEX()[ipoint] << setw(14) << _graph->GetEY()[ipoint] <<endl;
        }
        outfile.close();

        fEfficiencyGraphFileName = Form("%s_efficiency.dat",fname.Data());
    }
    if(_error) {
        ofstream outfile(filename.Copy().ReplaceAll(".dat",".err"));
        outfile<<"# "<< _error->GetName() << " " << _error->GetTitle() <<endl;
        outfile<<"# X axis : "<< _error->GetXaxis()->GetTitle() <<endl;
        outfile<<"# Y axis : "<< _error->GetYaxis()->GetTitle() <<endl;
        outfile<<"# N Bins : "<< _error->GetXaxis()->GetNbins() <<endl;
        outfile<<"# X min  : "<< _error->GetXaxis()->GetXmin() <<endl;
        outfile<<"# X max  : "<< _error->GetXaxis()->GetXmax() <<endl;
        outfile<<"# Bin       BinCenter     BinContent    BinError" <<endl;
        for(int ibin = 1 ; ibin<=_error->GetXaxis()->GetNbins() ; ibin++) {
            outfile << left << setw(12) << ibin << setw(14) << _error->GetBinCenter(ibin) << setw(14) << _error->GetBinContent(ibin) << setw(14) << _error->GetBinError(ibin) <<endl;
        }
        outfile.close();

        fEfficiencyErrorFileName = Form("%s_efficiency.err",fname.Data());
    }
    if(_func) {
        ofstream file(filename.Copy().ReplaceAll(".dat",".func"));
        TString header = "## Workspace " + fname + ": efficiency function ##";
        for(int i=0 ; i<header.Length() ; i++) {file << "#";} file << endl;
        file << header << endl;
        for(int i=0 ; i<header.Length() ; i++) {file << "#";} file << endl << endl;

        file << "# Function adapted from radware: https://radware.phy.ornl.gov/gf3/" << endl;
        file << "# Seven parameters used: Scale, A, B, C, D, E, G G" << endl;
        file << "# eff = Scale * exp{ [ (A+B*x+C*x*x)**(-G) + (D+E*y+F*y*y)**(-G) ]**(-1/G) }" << endl;
        file << left << setw(10) << "Range" << _func->GetX(1e-10) << " " << _func->GetXmax() << endl;
        for(int i=0 ; i<_func->GetNpar() ; i++) {
            file << left << setw(10) << _func->GetParName(i) << _func->GetParameter(i) << endl;
        }
        file.close();
        fEfficiencyFuncFileName = Form("%s_efficiency.func",fname.Data());
    }

    UpdateWSFile();
}

void CXWorkspace::SetCalibration(TGraph *_graph, TF1 *_func, TH1 *_error, TGraph *_residue)
{
    TString filename = Form("%s/%s/%s_calibration.dat",fdirectory.Data(),fname.Data(),fname.Data());
    if(!gSystem->AccessPathName(filename)) {
        glog << tkn::error << " file: " << filename << " already existing, plese delete it manually before saving workspace" << tkn::do_endl;
        return;
    }
    if(_graph) {
        ofstream outfile(filename);
        outfile<<"# "<< _graph->GetName() << " " << _graph->GetTitle() <<endl;
        outfile<<"# X axis : "<< _graph->GetXaxis()->GetTitle() <<endl;
        outfile<<"# Y axis : "<< _graph->GetYaxis()->GetTitle() <<endl;
        outfile<<"# N points : "<< _graph->GetN() <<endl;
        outfile<<"# X value     Y value       X error       Y error" <<endl;
        for(int ipoint = 0 ; ipoint < _graph->GetN() ; ipoint++) {
            outfile << left << setw(14) << _graph->GetX()[ipoint] << setw(14) << _graph->GetY()[ipoint] << setw(14) << _graph->GetEX()[ipoint] << setw(14) << _graph->GetEY()[ipoint] <<endl;
        }
        outfile.close();

        fCalibGraphFileName = Form("%s_calibration.dat",fname.Data());
    }
    if(_error) {
        ofstream outfile(filename.Copy().ReplaceAll(".dat",".err"));
        outfile<<"# "<< _error->GetName() << " " << _error->GetTitle() <<endl;
        outfile<<"# X axis : "<< _error->GetXaxis()->GetTitle() <<endl;
        outfile<<"# Y axis : "<< _error->GetYaxis()->GetTitle() <<endl;
        outfile<<"# N Bins : "<< _error->GetXaxis()->GetNbins() <<endl;
        outfile<<"# X min  : "<< _error->GetXaxis()->GetXmin() <<endl;
        outfile<<"# X max  : "<< _error->GetXaxis()->GetXmax() <<endl;
        outfile<<"# Bin       BinCenter     BinContent    BinError" <<endl;
        for(int ibin = 1 ; ibin<=_error->GetXaxis()->GetNbins() ; ibin++) {
            outfile << left << setw(12) << ibin << setw(14) << _error->GetBinCenter(ibin) << setw(14) << _error->GetBinContent(ibin) << setw(14) << _error->GetBinError(ibin) <<endl;
        }
        outfile.close();

        fCalibErrorFileName = Form("%s_calibration.err",fname.Data());
    }
    if(_func) {
        ofstream file(filename.Copy().ReplaceAll(".dat",".func"));
        TString header = "## Workspace " + fname + ": calibration function ##";
        for(int i=0 ; i<header.Length() ; i++) {file << "#";} file << endl;
        file << header << endl;
        for(int i=0 ; i<header.Length() ; i++) {file << "#";} file << endl << endl;
        file << "# Polynomial function of order N" << endl;
        file << left << setw(10) << "Range" << _func->GetXmin() << " " << _func->GetXmax() << endl;
        for(int i=0 ; i<_func->GetNpar() ; i++) {
            file << left << setw(10) << _func->GetParName(i) << _func->GetParameter(i) << endl;
        }
        file.close();
        fCalibFuncFileName = Form("%s_calibration.func",fname.Data());
    }
    if(_residue) {
        ofstream outfile(filename.Copy().ReplaceAll(".dat",".res"));
        outfile<<"# "<< _residue->GetName() << " " << _residue->GetTitle() <<endl;
        outfile<<"# X axis : "<< _residue->GetXaxis()->GetTitle() <<endl;
        outfile<<"# Y axis : "<< _residue->GetYaxis()->GetTitle() <<endl;
        outfile<<"# N points : "<< _residue->GetN() <<endl;
        outfile<<"# X value     Y value       X error       Y error" <<endl;
        for(int ipoint = 0 ; ipoint < _residue->GetN() ; ipoint++) {
            outfile << left << setw(14) << _residue->GetX()[ipoint] << setw(14) << _residue->GetY()[ipoint] << setw(14) << _residue->GetEX()[ipoint] << setw(14) << _residue->GetEY()[ipoint] <<endl;
        }
        outfile.close();

        fCalibResidueFileName = Form("%s_calibration.res",fname.Data());
    }

    UpdateWSFile();
}

void CXWorkspace::SetFWHM(TGraph *_graph, TF1 *_func, TH1 *_error)
{
    TString filename = Form("%s/%s/%s_FWHM.dat",fdirectory.Data(),fname.Data(),fname.Data());
    if(!gSystem->AccessPathName(filename)) {
        glog << tkn::error << " file: " << filename << " already existing, plese delete it manually before saving workspace" << tkn::do_endl;
        return;
    }
    if(_graph) {
        ofstream outfile(filename);
        outfile<<"# "<< _graph->GetName() << " " << _graph->GetTitle() <<endl;
        outfile<<"# X axis : "<< _graph->GetXaxis()->GetTitle() <<endl;
        outfile<<"# Y axis : "<< _graph->GetYaxis()->GetTitle() <<endl;
        outfile<<"# N points : "<< _graph->GetN() <<endl;
        outfile<<"# X value     Y value       X error       Y error" <<endl;
        for(int ipoint = 0 ; ipoint < _graph->GetN() ; ipoint++) {
            outfile << left << setw(14) << _graph->GetX()[ipoint] << setw(14) << _graph->GetY()[ipoint] << setw(14) << _graph->GetEX()[ipoint] << setw(14) << _graph->GetEY()[ipoint] <<endl;
        }
        outfile.close();

        fFWHMGraphFileName = Form("%s_FWHM.dat",fname.Data());
    }
    if(_error) {
        ofstream outfile(filename.Copy().ReplaceAll(".dat",".err"));
        outfile<<"# "<< _error->GetName() << " " << _error->GetTitle() <<endl;
        outfile<<"# X axis : "<< _error->GetXaxis()->GetTitle() <<endl;
        outfile<<"# Y axis : "<< _error->GetYaxis()->GetTitle() <<endl;
        outfile<<"# N Bins : "<< _error->GetXaxis()->GetNbins() <<endl;
        outfile<<"# X min  : "<< _error->GetXaxis()->GetXmin() <<endl;
        outfile<<"# X max  : "<< _error->GetXaxis()->GetXmax() <<endl;
        outfile<<"# Bin       BinCenter     BinContent    BinError" <<endl;
        for(int ibin = 1 ; ibin<=_error->GetXaxis()->GetNbins() ; ibin++) {
            outfile << left << setw(12) << ibin << setw(14) << _error->GetBinCenter(ibin) << setw(14) << _error->GetBinContent(ibin) << setw(14) << _error->GetBinError(ibin) <<endl;
        }
        outfile.close();

        fFWHMErrorFileName = Form("%s_FWHM.err",fname.Data());
    }
    if(_func) {
        ofstream file(filename.Copy().ReplaceAll(".dat",".func"));
        TString header = "## Workspace " + fname + ": FWHM function ##";
        for(int i=0 ; i<header.Length() ; i++) {file << "#";} file << endl;
        file << header << endl;
        for(int i=0 ; i<header.Length() ; i++) {file << "#";} file << endl << endl;
        file << "# FWHM = sqrt(F*F + G*G*x + H*H*x*x)" << endl;
        file << left << setw(10) << "Range" << _func->GetXmin() << " " << _func->GetXmax() << endl;
        for(int i=0 ; i<_func->GetNpar() ; i++) {
            file << left << setw(10) << _func->GetParName(i) << _func->GetParameter(i) << endl;
        }
        file.close();
        fFWHMFuncFileName = Form("%s_FWHM.func",fname.Data());
    }

    UpdateWSFile();
}

//************************************************************************

CXWSManager::CXWSManager(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    auto *fGroupFrame = new TGGroupFrame(MotherFrame, "Workspace manager", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY | kLHintsExpandX, 3, 3, 0, 0));

    auto *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    auto *button = new TGTextButton(fHorizontalFrame, "Refresh");
    button->SetTextColor(CXred);
    button->Connect("Clicked()", "CXWSManager", this, "RefreshWS()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,10,0,0));
    button = new TGTextButton(fHorizontalFrame, "New workspace");
    button->SetTextColor(CXred);
    button->Connect("Clicked()", "CXWSManager", this, "NewWS()");
    fHorizontalFrame->AddFrame(button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,20,20,10,10));

    fWSListBox = new TGListBox(fGroupFrame);
    fWSListBox->GetContainer()->RemoveInput(kButtonReleaseMask | kButtonMotionMask);
    fGroupFrame->AddFrame(fWSListBox, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,-10,-10,0,0));
    fWSListBox->GetContainer()->Connect("ProcessedEvent(Event_t*)", "CXWSManager", this, "ProcessedButtonEvent(Event_t*)");

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGLabel *label;
    fHorizontalFrame->AddFrame(label = new TGLabel(fHorizontalFrame, "Current Workspace: "),new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 5, 0, 0));
    label->SetTextColor(CXblue);
    fHorizontalFrame->AddFrame(fActiveWSLabel = new TGLabel(fHorizontalFrame, fActiveWorkspaceName.Data()),new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 0, 0));
    fActiveWSLabel->SetTextColor(CXred);
    fActiveWSLabel->SetTextJustify(kTextLeft);
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,10,5));

    fContentGroupFrame = new TGGroupFrame(MotherFrame, "Workspace content", kVerticalFrame);
    fContentGroupFrame->SetTextColor(CXblue);
    fContentGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fContentGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fContentGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *Load = new TGTextButton(fHorizontalFrame, " Load ");
    Load->SetTextColor(CXred);
    Load->Connect("Clicked()", "CXWSManager", this, "LoadWS()");
    fHorizontalFrame->AddFrame(Load,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,5,5,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Draw Options:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 10, 5, 0, 0));
    fDrawOptions = new TGTextEntry(fHorizontalFrame, "");
    fHorizontalFrame->AddFrame(fDrawOptions,new TGLayoutHints(kLHintsLeft | kLHintsCenterY | kLHintsExpandX ,5,5,0,0));
    fContentGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,20,20,10,10));

    fListView = new TGListView(fContentGroupFrame, 10 , 50);
    fContentGroupFrame->AddFrame(fListView,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,-10,-10,10,10));

    Pixel_t white;
    gClient->GetColorByName("white", white);
    fWSContentBox = new TGFileContainer(fListView, kSunkenFrame,white);
    fWSContentBox->Connect("DoubleClicked(TGFrame*,Int_t)", "CXWSManager", this, "OnDoubleClick(TGLVEntry*,Int_t)");

    fWSContentBox->SetDefaultHeaders();
    fWSContentBox->Resize();
    fWSContentBox->StopRefreshTimer();   // stop refreshing

    fWSList = new TList;
}

CXWSManager::~CXWSManager()
{
    fWSList->Delete();
    delete fWSList;
}

void CXWSManager::DoubleClicked(Int_t id)
{
}

void CXWSManager::ProcessedButtonEvent(Event_t *event)
{
    // char input[10];
    // UInt_t keysym;

    // gVirtualX->LookupString(event, input, sizeof(input), keysym);

    // std::cout << "event : " << event->fCode << " " << event->fState <<" ; "<< event->fType  << "; " << keysym << " " << input << std::endl;

    if(fWSListBox && (fWSListBox->GetSelected()>0)) {
        if(fLastSelectedEntry >=0 && fWSListBox->GetEntry(fSelectedEntry)) {
            fWSListBox->GetEntry(fSelectedEntry)->SetBackgroundColor((Pixel_t)0x90f269);
        }
        fSelectedEntry = fWSListBox->GetSelected();
        fWSListBox->GetEntry(fSelectedEntry)->SetBackgroundColor((Pixel_t)0x87a7d2);
        fWSListBox->Select(fSelectedEntry,false);
        fWSListBox->Layout();

        if(fLastSelectedEntry!=fSelectedEntry) SelectionChanged();

        fLastSelectedEntry = fSelectedEntry;
    }
}

void CXWSManager::SelectionChanged()
{
    if(fSelectedEntry<0) return;

    if(fWSListBox->GetEntry(fSelectedEntry) == nullptr) {
        fWSContentBox->RemoveAll();
        fWSContentBox->SetDefaultHeaders();
        return;
    }

    fCurrentWorkspace = dynamic_cast<CXWorkspace*>(fWSList->FindObject(fWSListBox->GetEntry(fSelectedEntry)->GetTitle()));
    if(fCurrentWorkspace == nullptr) return;

    fContentGroupFrame->SetTitle(Form("Workspace content: %s",fCurrentWorkspace->GetName()));

    fWSContentBox->RemoveAll();
    fWSContentBox->SetColHeaders("DataType","File");

    TGLVEntry *entry;
    TString subname;

    entry = new TGLVEntry(fWSContentBox,"Calibration graph","TGraph");
    entry->SetPictures(gClient->GetPicture("graph.xpm"),gClient->GetPicture("graph.xpm"));
    subname = (fCurrentWorkspace->fCalibrationGraph) ? fCurrentWorkspace->fCalibrationGraph->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    entry = new TGLVEntry(fWSContentBox,"Calibration function","TF1");
    subname = (fCurrentWorkspace->fCalibFunction) ? fCurrentWorkspace->fCalibFunction->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    entry = new TGLVEntry(fWSContentBox,"Calibration error","TH1F");
    subname = (fCurrentWorkspace->fCalibrationErrors) ? fCurrentWorkspace->fCalibrationErrors->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    entry = new TGLVEntry(fWSContentBox,"Calibration residue","TGraph");
    entry->SetPictures(gClient->GetPicture("graph.xpm"),gClient->GetPicture("graph.xpm"));
    subname = (fCurrentWorkspace->fCalibrationResidue) ? fCurrentWorkspace->fCalibrationResidue->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    entry = new TGLVEntry(fWSContentBox,"Efficiency graph","TGraph");
    entry->SetPictures(gClient->GetPicture("graph.xpm"),gClient->GetPicture("graph.xpm"));
    subname = (fCurrentWorkspace->fEfficiencyGraph) ? fCurrentWorkspace->fEfficiencyGraph->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    entry = new TGLVEntry(fWSContentBox,"Efficiency function","TF1");
    subname = (fCurrentWorkspace->fEfficiencyFunction) ? fCurrentWorkspace->fEfficiencyFunction->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    entry = new TGLVEntry(fWSContentBox,"Efficiency error","TH1F");
    subname = (fCurrentWorkspace->fEfficiencyErrors) ? fCurrentWorkspace->fEfficiencyErrors->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);


    entry = new TGLVEntry(fWSContentBox,"FWHM graph","TGraph");
    entry->SetPictures(gClient->GetPicture("graph.xpm"),gClient->GetPicture("graph.xpm"));
    subname = (fCurrentWorkspace->fFWHMGraph) ? fCurrentWorkspace->fFWHMGraph->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    entry = new TGLVEntry(fWSContentBox,"FWHM function","TF1");
    subname = (fCurrentWorkspace->fFWHMFunction) ? fCurrentWorkspace->fFWHMFunction->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    entry = new TGLVEntry(fWSContentBox,"FWHM error","TH1F");
    subname = (fCurrentWorkspace->fFWHMErrors) ? fCurrentWorkspace->fFWHMErrors->GetName() : "None";
    entry->SetSubnames(subname); fWSContentBox->AddItem(entry);

    fListView->Resize();
    fWSContentBox->SetViewMode(EListViewMode::kLVDetails);
    fWSContentBox->GetListView()->AdjustHeaders();
}

void CXWSManager::NewWS()
{
    TString WSName = "MyWorkspace";

    CXDialogBox *diag = new CXDialogBox(this->GetMainFrame(),"New workspace");
    diag->Add("Workspace name",WSName);
    diag->Popup();

    WSName.ReplaceAll(" ","_");

    if(!gSystem->AccessPathName(Form("%s/%s/%s.conf",fWorkspaceDirectory.Data(),WSName.Data(),WSName.Data()))) {
        glog << tkn::error << "Workspace: " << WSName << " already existing in Workspace directory: " << fWorkspaceDirectory << tkn::do_endl;
        return;
    }

    int ok = gSystem->mkdir(Form("%s/%s",fWorkspaceDirectory.Data(),WSName.Data()),true);
    if(ok != 0) {
        glog << tkn::error << "Workspace: " << WSName << " cannot be created in Workspace directory: " << fWorkspaceDirectory << tkn::do_endl;
        return;
    }

    CXWorkspace *WS = new CXWorkspace(WSName,fWorkspaceDirectory);
    fWSList->Add(WS);

    RefreshWS();
    SelectionChanged();
}

void CXWSManager::LoadWS(TString _ws_dir)
{
    if(_ws_dir == "" && fCurrentWorkspace) {
        fActiveWorkspace = fCurrentWorkspace;
    }
    else {
        fActiveWorkspace = dynamic_cast<CXWorkspace*>(fWSList->FindObject(_ws_dir));
        if(fActiveWorkspace == nullptr) {
            glog << tkn::error << "No Workspace named: " << _ws_dir << "found, Ignored" << tkn::do_endl;
        }
    }

    if(fActiveWorkspace) {
        fActiveWorkspaceName = fActiveWorkspace->GetName();
        fActiveWSLabel->SetText(fActiveWorkspaceName);
        auto entry = fWSListBox->FindEntry(fActiveWorkspace->GetName());
        fWSListBox->Select(entry->EntryId());
    }
}

void CXWSManager::RefreshWS()
{
    fWSList->RemoveAll();
    fCurrentWorkspace = nullptr;
    fWSContentBox->RemoveAll();

    fContentGroupFrame->SetTitle("Workspace content: None");

    TSystemDirectory dir(fWorkspaceDirectory.Data(),fWorkspaceDirectory.Data());
    TIter next(dir.GetListOfFiles());
    while ( TObject* obj = next() ) {
        if ( obj->IsA() == TSystemDirectory::Class() ) {
            TString WSName = Form("%s/%s/%s.conf",fWorkspaceDirectory.Data(),obj->GetName(),obj->GetName());
            if(!gSystem->AccessPathName(WSName.Data())) {
                CXWorkspace *WS = new CXWorkspace(obj->GetName(),fWorkspaceDirectory);
                fWSList->Add(WS);
            }
        }
    }

    gErrorIgnoreLevel = kError;

    fWSListBox->RemoveAll();

    for(auto WS: *fWSList) {
        auto *entry = new TGTextLBEntry(fWSListBox->GetContainer(), new TGString(WS->GetName()), fWSListBox->GetNumberOfEntries()+1);
        entry->SetBackgroundColor((Pixel_t)0x90f269);
        fWSListBox->AddEntry((TGLBEntry *)entry, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX));
    }
    fWSListBox->Layout();

    gErrorIgnoreLevel = kPrint;

    SelectionChanged();
}

void CXWSManager::OnDoubleClick(TGLVEntry *f, Int_t btn)
{
    // Handle double click.

    if (btn != kButton1) return;

    // set kWatch cursor
    ULong_t cur = gVirtualX->CreateCursor(kWatch);
    gVirtualX->SetCursor(fWSContentBox->GetId(), cur);

    TString name(f->GetTitle());
    TString DrawOpt = fDrawOptions->GetText();
    if(DrawOpt.Copy().ReplaceAll(" ","").Length()==0) DrawOpt="";

    if(name == "Calibration graph" && fCurrentWorkspace->fCalibrationGraph) {
        if(!DrawOpt.Length()) {
            fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationGraph,"ape");
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationGraph,DrawOpt);
        }
    }
    if(name == "Calibration function" && fCurrentWorkspace->fCalibFunction) {
        if(!DrawOpt.Length()) {
            if(fCurrentWorkspace->fCalibrationGraph) {
                fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationGraph,"ape");
                fMainWindow->DoDraw(fCurrentWorkspace->fCalibFunction,"same");
            }
            else {
                fMainWindow->DoDraw(fCurrentWorkspace->fCalibFunction,"");
            }
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fCalibFunction,DrawOpt);
        }
    }
    if(name == "Calibration error" && fCurrentWorkspace->fCalibrationErrors) {
        if(!DrawOpt.Length()) {
            if(fCurrentWorkspace->fCalibrationGraph) {
                fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationGraph,"ape");
                if(fCurrentWorkspace->fCalibFunction) {
                    fMainWindow->DoDraw(fCurrentWorkspace->fCalibFunction,"same");
                    fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationErrors,"e3 same");
                }
                else {
                    fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationErrors,"e3 same");
                }
            }
            else {
                if(fCurrentWorkspace->fCalibFunction) {
                    fMainWindow->DoDraw(fCurrentWorkspace->fCalibFunction,"");
                    fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationErrors,"e3 same");
                }
                else {
                    fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationErrors,"e3");
                }
            }
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationErrors,DrawOpt);
        }
    }
    if(name == "Calibration residue" && fCurrentWorkspace->fCalibrationResidue) {
        fCurrentWorkspace->fCalibrationResidue->Print("all");
        if(!DrawOpt.Length()) fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationResidue,"ape");
        else fMainWindow->DoDraw(fCurrentWorkspace->fCalibrationResidue,DrawOpt);
    }

    if(name == "Efficiency graph" && fCurrentWorkspace->fEfficiencyGraph) {
        if(!DrawOpt.Length()) {
            fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyGraph,"ape");
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyGraph,DrawOpt);
        }
    }
    if(name == "Efficiency function" && fCurrentWorkspace->fEfficiencyFunction) {
        if(!DrawOpt.Length()) {
            if(fCurrentWorkspace->fEfficiencyGraph) {
                fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyGraph,"ape");
                fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyFunction,"same");
            }
            else {
                fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyFunction,"");
            }
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyFunction,DrawOpt);
        }
    }
    if(name == "Efficiency error" && fCurrentWorkspace->fEfficiencyErrors) {
        if(!DrawOpt.Length()) {
            if(fCurrentWorkspace->fEfficiencyGraph) {
                fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyGraph,"ape");
                if(fCurrentWorkspace->fEfficiencyFunction) {
                    fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyFunction,"same");
                    fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyErrors,"e3 same");
                }
                else {
                    fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyErrors,"e3 same");
                }
            }
            else {
                if(fCurrentWorkspace->fEfficiencyFunction) {
                    fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyFunction,"");
                    fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyErrors,"e3 same");
                }
                else {
                    fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyErrors,"e3");
                }
            }
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fEfficiencyErrors,DrawOpt);
        }
    }

    if(name == "FWHM graph" && fCurrentWorkspace->fFWHMGraph) {
        if(!DrawOpt.Length()) {
            fMainWindow->DoDraw(fCurrentWorkspace->fFWHMGraph,"ape");
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fFWHMGraph,DrawOpt);
        }
    }
    if(name == "FWHM function" && fCurrentWorkspace->fFWHMFunction) {
        if(!DrawOpt.Length()) {
            if(fCurrentWorkspace->fFWHMGraph) {
                fMainWindow->DoDraw(fCurrentWorkspace->fFWHMGraph,"ape");
                fMainWindow->DoDraw(fCurrentWorkspace->fFWHMFunction,"same");
            }
            else {
                fMainWindow->DoDraw(fCurrentWorkspace->fFWHMFunction,"");
            }
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fFWHMFunction,DrawOpt);
        }
    }
    if(name == "FWHM error" && fCurrentWorkspace->fFWHMErrors) {
        if(!DrawOpt.Length()) {
            if(fCurrentWorkspace->fFWHMGraph) {
                fMainWindow->DoDraw(fCurrentWorkspace->fFWHMGraph,"ape");
                if(fCurrentWorkspace->fFWHMFunction) {
                    fMainWindow->DoDraw(fCurrentWorkspace->fFWHMFunction,"same");
                    fMainWindow->DoDraw(fCurrentWorkspace->fFWHMErrors,"e3 same");
                }
                else {
                    fMainWindow->DoDraw(fCurrentWorkspace->fFWHMErrors,"e3 same");
                }
            }
            else {
                if(fCurrentWorkspace->fFWHMFunction) {
                    fMainWindow->DoDraw(fCurrentWorkspace->fFWHMFunction,"");
                    fMainWindow->DoDraw(fCurrentWorkspace->fFWHMErrors,"e3 same");
                }
                else {
                    fMainWindow->DoDraw(fCurrentWorkspace->fFWHMErrors,"e3");
                }
            }
        }
        else {
            fMainWindow->DoDraw(fCurrentWorkspace->fFWHMErrors,DrawOpt);
        }
    }


    // set kPointer cursor
    cur = gVirtualX->CreateCursor(kPointer);
    gVirtualX->SetCursor(fWSContentBox->GetId(), cur);
}

void CXWSManager::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

ClassImp(CXWSManager)
    ClassImp(CXWorkspace)
