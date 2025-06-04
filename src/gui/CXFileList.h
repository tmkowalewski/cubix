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

#ifndef CXFileList_H
#define CXFileList_H

#include "TGFrame.h"
#include "RQ_OBJECT.h"
#include "CXMainWindow.h"

class TGTransientFrame;
class TGFileContainer;
class TGPopupMenu;
class TGLVEntry;
class TGTextEntry;
class TList;
class TFile;
class TGListTreeItem;
class CXGFileBrowser;
class TDirectoryFile;
class CXRadReader;

class CXFileList : public  TGVerticalFrame
{

    RQ_OBJECT("CXFileList");

protected:

    CXMainWindow      *fMainWindow = nullptr;

    const TGCompositeFrame *fParentFrame;
    TGVerticalFrame  *fMain;
    TGHorizontalFrame *fH1;
    TGHorizontalFrame *fH2;

    TGFileContainer  *fContents;
    TGPopupMenu      *fMenu;
    TGTextEntry      *fDrawOption = nullptr;

    std::vector<TObject*> fFolders;

    TObject          *fCurrent = nullptr;
    TObject          *fParent = nullptr;
    TFile            *fCurrentFile = nullptr;

    CXGFileBrowser   *fBrowser = nullptr;

    TContextMenu     *fContextMenu = nullptr;


    ///Radware Cube stuffs

    Bool_t fRadCubeMode     = false;
    TString fCubeFileName   = "None";

    Bool_t fConfFileLoaded  =  false;
    TString fConfFileName   = "None";

    TString f2DProjName     = "None";

    TString fLUTFileName    = "None";
    TString fECalFileName   = "None";
    TString fEffFileName    = "None";
    TString fTotalProjFile  = "None";
    TString fBackgroundFile = "None";

    Float_t fCompFactor       = 1;

    CXRadReader *fRadReader = nullptr;

public:
    CXFileList(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    virtual ~CXFileList();

    // slots
    void OnClick(TGLVEntry *f, Int_t btn, Int_t x, Int_t y);
    void OnDoubleClick(TGLVEntry*,Int_t);

    void DoMenu(Int_t);
    void CloseWindow();

    void DisplayFile(const TString &fname);

    void DoubleClicked(TGListTreeItem *item, Int_t);
    void SetMainWindow(CXMainWindow *w){fMainWindow = w;}
private:

    int DisplayRadCube(const TString &fname);
    void DisplayRadSpe(const TString &fname);
    void DisplayASCII(const TString &fname);

    void DisplayObject(const TString& fname,const TString& name);
    void DisplayList(TList *list);

    Bool_t ReadConfFile(const char *_dirname=".");

    ClassDef(CXFileList,0)
};

#endif
