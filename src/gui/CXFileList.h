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

    void DisplayRadCube(const TString &fname);
    void DisplayRadSpe(const TString &fname);

    void DisplayObject(const TString& fname,const TString& name);
    void DisplayList(TList *list);

    Bool_t ReadConfFile();

    ClassDef(CXFileList,0)
};

#endif
