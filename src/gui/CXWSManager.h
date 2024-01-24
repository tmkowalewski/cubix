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

#ifndef CXWSManager_H
#define CXWSManager_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TNamed.h"

using namespace std;

class CXMainWindow;
class TGListBox;
class TGTextEntry;
class TGLBEntry;
class TF1;
class TH1;
class TList;
class TGLabel;
class TGraphErrors;
class TGraph;
class TGFileContainer;
class TGListView;
class TGLVEntry;

class CXWorkspace : public TNamed
{
public:
    TString fname="";
    TString fdirectory="";

    TString fCalibGraphFileName = "None";
    TString fCalibFuncFileName  = "None";
    TString fCalibErrorFileName = "None";
    TString fCalibResidueFileName = "None";
    TString fQAngCorrQiFileName = "None";

    TF1 *fCalibFunction = nullptr;
    TGraph *fCalibrationGraph = nullptr;
    TH1 *fCalibrationErrors = nullptr;
    TGraph *fCalibrationResidue = nullptr;

    TString fFWHMGraphFileName = "None";
    TString fFWHMFuncFileName  = "None";
    TString fFWHMErrorFileName = "None";

    TF1 *fFWHMFunction = nullptr;
    TGraph *fFWHMGraph = nullptr;
    TH1 *fFWHMErrors = nullptr;

    TString fEfficiencyGraphFileName = "None";
    TString fEfficiencyFuncFileName  = "None";
    TString fEfficiencyErrorFileName = "None";

    TF1 *fEfficiencyFunction = nullptr;
    TH1 *fEfficiencyErrors = nullptr;
    TGraph *fEfficiencyGraph = nullptr;

    array< array<double,3>, 2> fAngCorrQis;

public:
    CXWorkspace(const char *_name, const char *_dir);
    ~CXWorkspace();


    void SetEfficiency(TGraph *_graph, TF1 *_func, TH1 *_error);
    void SetCalibration(TGraph *_graph, TF1 *_func, TH1 *_error, TGraph *_residue);
    void SetFWHM(TGraph *_graph, TF1 *_func, TH1 *_error);

    void SetAngCorrQis(double _Q2, double _Q2low, double _Q2high, double _Q4, double _Q4low, double _Q4high);

    void ReadWS();
    void UpdateWSFile();

private:
    ClassDef(CXWorkspace,0);
};

class CXWSManager : public  TGVerticalFrame
{
    RQ_OBJECT("CXWSManager");

private:

    TGListView *fListView = nullptr;

    CXMainWindow *fMainWindow = nullptr;

    TGGroupFrame *fContentGroupFrame = nullptr;

    TGListBox *fWSListBox = nullptr;
    TGFileContainer *fWSContentBox = nullptr;
    TGTextEntry *fDrawOptions = nullptr;

    TString fWorkspaceDirectory = "";

    TGLabel *fActiveWSLabel = nullptr;
    TString fActiveWorkspaceName = "None";
    CXWorkspace *fCurrentWorkspace = nullptr;
    CXWorkspace *fActiveWorkspace = nullptr;

    TList *fWSList = nullptr;

    Int_t fSelectedEntry = -1;
    Int_t fLastSelectedEntry = -1;

    TGLBEntry *fLastSelected = nullptr;

public:
    CXWSManager(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXWSManager();

    void SetMainWindow(CXMainWindow *w);

    void RefreshWS();
    void NewWS();

    TString GetActiveWSName() {return fActiveWorkspaceName;}

    void SetWSDirectory(TString _ws_dir) {fWorkspaceDirectory = _ws_dir;}
    TString GetWSDirectory(){return fWorkspaceDirectory;}

    void LoadWS(TString _ws_dir="");
    void SelectionChanged();

    void DoubleClicked(Int_t id);
    void ProcessedButtonEvent(Event_t *event);

    CXWorkspace *GetActiveWorkspace() {return fActiveWorkspace;}
    CXWorkspace *GetWorkspace(const char* _name) {return dynamic_cast<CXWorkspace*>(fWSList->FindObject(_name));}

    void OnDoubleClick(TGLVEntry *f, Int_t btn);

private:

    ClassDef(CXWSManager,0);
};

#endif
