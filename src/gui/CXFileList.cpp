#include "CXFileList.h"

#include "Riostream.h"

#include "TGFrame.h"
#include "TGFSContainer.h"
#include "TGMenu.h"
#include "TG3DLine.h"
#include "TGLabel.h"
#include "TGTextEntry.h"
#include "TContextMenu.h"
#include "TSystem.h"
#include "TFile.h"
#include "TKey.h"
#include "TPad.h"
#include "CXGFileBrowser.h"
#include "TFrame.h"
#include "TGListTree.h"
#include "TClass.h"
#include "TCanvas.h"
#include "TCutG.h"
#include "TROOT.h"
#include "TVirtualX.h"

#include "TH1.h"
#include "TF1.h"

#include "CXMainWindow.h"
#include "CXRadReader.h"

#include "CXBashColor.h"

using namespace std;

CXFileList::CXFileList(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    fParentFrame = MotherFrame;

    fMain = new TGVerticalFrame(MotherFrame);
    AddFrame(fMain,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY));

    fH1 = new TGHorizontalFrame(fMain,w,h,kFixedHeight);
    fMain->AddFrame(fH1, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX));

    fH1->Resize(GetWidth(), GetHeight()*0.7);

    fBrowser = new CXGFileBrowser(fH1);
    fH1->AddFrame(fBrowser,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    fBrowser->SetNewBrowser(nullptr);
    fBrowser->AddFSDirectory("/", "/");
    fBrowser->GotoDir(gSystem->pwd());

    TQObject::Connect("TGListTree", "DoubleClicked(TGListTreeItem *, Int_t)", "CXFileList", this, "DoubleClicked(TGListTreeItem *, Int_t)");

    auto *hsplitter = new TGHSplitter(fMain,2,2);
    hsplitter->SetFrame(fH1, kTRUE);
    fMain->AddFrame(hsplitter, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

    fH2 = new TGHorizontalFrame(fMain, 10, 10);
    fMain->AddFrame(fH2, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY));

    auto *fv2 = new TGVerticalFrame(fH2,10,10);
    fH2->AddFrame(fv2,new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY));

    auto* mb = new TGMenuBar(fv2,60,20,kOwnBackground);
    fv2->AddFrame(mb,new TGLayoutHints(kLHintsExpandX, 1, 1, 1, 1) );

    fMenu = mb->AddPopup("&View");
    fMenu->AddEntry("Lar&ge Icons",kLVLargeIcons);
    fMenu->AddEntry("S&mall Icons",kLVSmallIcons);
    fMenu->AddEntry("&List",       kLVList);
    fMenu->AddEntry("&Details",    kLVDetails);
    fMenu->Connect("Activated(Int_t)","CXFileList",this,"DoMenu(Int_t)");

    auto *h3  = new TGHorizontalFrame(fv2,10,10);
    fv2->AddFrame(h3,new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY));

    auto* lv = new TGListView(h3, 10 , 50);
    auto *lo = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
    h3->AddFrame(lv,lo);

    Pixel_t white;
    gClient->GetColorByName("white", white);
    fContents = new TGFileContainer(lv, kSunkenFrame,white);
    fContents->Connect("DoubleClicked(TGFrame*,Int_t)", "CXFileList", this, "OnDoubleClick(TGLVEntry*,Int_t)");
    fContents->Connect("Clicked(TGFrame*, Int_t, Int_t, Int_t)", "CXFileList", this, "OnClick(TGLVEntry *, Int_t, Int_t, Int_t)");

    fContextMenu = new TContextMenu("FileBrowserContextMenu");

    // Set a name to the main frame
    SetWindowName("File List");
    MapSubwindows();
    MapWindow();
    Layout();

    fContents->SetDefaultHeaders();
    fContents->Sort(EFSSortMode::kSortByName);
    fContents->Resize();
    fContents->StopRefreshTimer();   // stop refreshing

    Resize();

    fRadReader = new CXRadReader;

    DoMenu(kLVList);
}

void CXFileList::DoubleClicked(TGListTreeItem *item, Int_t /*a_int*/)
{
    auto *sender = static_cast<TGListTree*>(gTQSender);
    const TGWindow *parent = sender->GetParent()->GetParent()->GetParent();

    fRadCubeMode = false;

    gSystem->ChangeDirectory(fBrowser->DirName(item));

    if(parent == fBrowser){
        auto *obj = static_cast<TObject*>(item->GetUserData());

        if(obj){
            obj->IsA()->SetName("");

//            TString filename = fBrowser->FullPathName(item);
            TString filename = item->GetText();
            if(gSystem->IsFileInIncludePath(filename)) {
                if(filename.EndsWith(".root"))
                    DisplayFile(filename);
                if(filename.EndsWith(".cub"))
                    DisplayRadCube(filename);
                if(filename.EndsWith(".spe"))
                    DisplayRadSpe(filename);
            }
        }
    }
}

CXFileList::~CXFileList()
{
    // Cleanup.
    delete fContents;
    fMain->DeleteWindow();  // deletes fMain

    delete fContextMenu;
}

void CXFileList::DoMenu(Int_t mode)
{
    // Switch view mode.
    if (mode<10)
        fContents->SetViewMode((EListViewMode)mode);
    else
        delete this;
}

void CXFileList::DisplayFile(const TString &fname)
{
    //Close the last opened file
    if(fCurrentFile){
        fCurrentFile->Close();
        fCurrentFile = nullptr;
        fFolders.clear();
    }

    // Display content of ROOT file.
    TFile *file = TFile::Open(fname);
    fContents->RemoveAll();
    //    fContents->AddFile(gSystem->WorkingDirectory());
    fContents->SetPagePosition(0,0);
    fContents->SetColHeaders("Name","Title");

    TIter next(file->GetListOfKeys());
    TKey *key;

    while ((key=dynamic_cast<TKey*>(next()))) {
        TString cname = key->GetClassName();
        TString name = key->GetName();
        auto *entry = new TGLVEntry(fContents,name,cname);
        entry->SetSubnames(key->GetTitle());

        if(cname=="TList" || cname=="TDirectoryFile")
            entry->SetPictures(gClient->GetPicture("folder_s.xpm"),gClient->GetPicture("folder_t.xpm"));
        if(cname=="TCutG")
            entry->SetPictures(gClient->GetPicture("bld_cut.png"),gClient->GetPicture("bld_cut.png"));
        if(cname.BeginsWith("TGraph"))
            entry->SetPictures(gClient->GetPicture("graph.xpm"),gClient->GetPicture("graph.xpm"));

        fContents->AddItem(entry);

        // user data is a filename
        entry->SetUserData((void*)StrDup(fname));
    }
    fMain->Resize();
    fCurrentFile = file;
    fFolders.push_back(file);

    fContents->SetViewMode(EListViewMode::kLVList);
}

void CXFileList::DisplayRadCube(const TString &fname)
{
    TGLVEntry *entry;
    fFolders.clear();

    fRadCubeMode = true;

    Int_t Status=0;

    fContents->RemoveAll();
    fContents->SetColHeaders("DataType","File");

//    fCubeFileName = fname.Copy().ReplaceAll(Form("%s/",gSystem->pwd()),"");
    fCubeFileName = fname.Copy();

    entry = new TGLVEntry(fContents,"Cube File","TH3F");
    entry->SetSubnames(fCubeFileName);
    fContents->AddItem(entry);

//    gSystem->ChangeDirectory(gSystem->DirName(fCubeFileName));

    // Read Conf file
    fConfFileName = fCubeFileName.Copy().ReplaceAll(".cub",".conf");
    fConfFileLoaded = false;
    entry = new TGLVEntry(fContents,"Conf File loaded","conf");
    entry->SetPictures(gClient->GetPicture("about.xpm"),gClient->GetPicture("about.xpm"));
    fContents->AddItem(entry);
    entry->SetSubnames("No");
    if(gSystem->IsFileInIncludePath(fConfFileName) && ReadConfFile()) {
        fConfFileLoaded = true;
        entry->SetSubnames("Yes");
    }
    else {
        f2DProjName     = fCubeFileName.Copy().ReplaceAll(".cub",".2dp");
        if(!gSystem->IsFileInIncludePath(f2DProjName)) f2DProjName="None";
        fLUTFileName    = fCubeFileName.Copy().ReplaceAll(".cub",".tab");
        if(!gSystem->IsFileInIncludePath(fLUTFileName)) fLUTFileName="None";
        fECalFileName   = fCubeFileName.Copy().ReplaceAll(".cub",".aca");
        if(!gSystem->IsFileInIncludePath(fECalFileName)) fECalFileName="None";
        fEffFileName    = fCubeFileName.Copy().ReplaceAll(".cub",".aef");
        if(!gSystem->IsFileInIncludePath(fEffFileName)) fEffFileName="None";
        fTotalProjFile  = fCubeFileName.Copy().ReplaceAll(".cub",".spe");
        if(!gSystem->IsFileInIncludePath(fTotalProjFile)) fTotalProjFile="None";
        fBackgroundFile = fCubeFileName.Copy().ReplaceAll(".cub",".bg.spe");
        if(!gSystem->IsFileInIncludePath(fBackgroundFile)) fBackgroundFile="None";
    }

    // Read Cube
    Status += fRadReader->ReadCube(fCubeFileName);
    if(Status>0) {gbash_color->WarningMessage("Errors occured in reading the cube, ignored");return;}

    // Read LUT
    Status += fRadReader->ReadTabFile(fLUTFileName);
    if(Status>0) {gbash_color->WarningMessage("Errors occured in reading the cube, ignored");return;}

    // Read calibrations
    Status += fRadReader->ReadCalibs(fECalFileName, fEffFileName, fCompFactor);

    // Read Tot proj
    Status += fRadReader->ReadTotProj(fTotalProjFile);
    if(Status>0) {gbash_color->WarningMessage("Errors occured in reading the cube, ignored");return;}

    // Read Background
    Status += fRadReader->ReadBackground(fBackgroundFile);
    if(Status>0) {gbash_color->WarningMessage("Errors occured in reading the cube, ignored");return;}

    // Read 2D projection
    Status += fRadReader->Read2DProj(f2DProjName);
    if(Status>0) {gbash_color->WarningMessage("Errors occured in reading the cube, ignored");return;}

    if(Status>0){
        gbash_color->WarningMessage("Errors occured in reading the cube, ignored");
    }
    else{
        fRadReader->BuildHistos();

        entry = new TGLVEntry(fContents,"LUT","TH1F");entry->SetSubnames(fLUTFileName);fContents->AddItem(entry);
        entry = new TGLVEntry(fContents,"Total Projection","TH1F");entry->SetSubnames(fTotalProjFile);fContents->AddItem(entry);
        entry = new TGLVEntry(fContents,"Background","TH1F");entry->SetSubnames(fBackgroundFile);fContents->AddItem(entry);
        entry = new TGLVEntry(fContents,"2D Proj","TH2F");entry->SetSubnames(f2DProjName);fContents->AddItem(entry);
        entry = new TGLVEntry(fContents,"Energy calibration","TF1");entry->SetSubnames(fECalFileName);fContents->AddItem(entry);
        entry = new TGLVEntry(fContents,"Efficiency","TF1");entry->SetSubnames(fEffFileName);fContents->AddItem(entry);
    }

    fMain->Resize();
    fContents->SetViewMode(EListViewMode::kLVDetails);
    fContents->GetListView()->AdjustHeaders();
}

void CXFileList::DisplayRadSpe(const TString &fname)
{
    TString CanvasName = fMainWindow->GetCanvas()->GetName();

    if(CanvasName.BeginsWith("GxG") || CanvasName.BeginsWith("RadGG") ){
        cout<<" cannot plot in a Tab dedicated to Gamma Gamma projections" << endl;
        return;
    }
    if(CanvasName.BeginsWith("RadCube")){
        cout<<" cannot plot in a Tab dedicated to Cube projections" << endl;
        return;
    }

    TString filanem = fname.Copy().ReplaceAll(Form("%s/",gSystem->pwd()),"");

    TH1 *hist = fRadReader->GetHistFromSpeFile(filanem);

    fMainWindow->DoDraw(hist,fBrowser->GetDrawOption());
}

void CXFileList::DisplayList(TList *list)
{
    // Display content of ROOT file.
    TString Name = fCurrentFile->GetName();

    TObjArray *arr = Name.Tokenize("/");
    Name = arr->Last()->GetName();
    delete arr;

    fContents->RemoveAll();
    fContents->SetPagePosition(0,0);
    //    fContents->SetColHeaders("Name","Title");

    TObject *Folder = fFolders.back();
    TGLVEntry *entry = nullptr;

    if(fFolders.size()>1){
        entry = new TGLVEntry(fContents,"..",Folder->ClassName());
        entry->SetUserData((void*)StrDup(Folder->GetName()));
        entry->SetPictures(gClient->GetPicture("folder_s.xpm"),gClient->GetPicture("folder_t.xpm"));
        fContents->AddItem(entry);
    }

    TIter next(list);
    TObject *key;

    while ((key=(TObject*)next())) {

        if(Folder->InheritsFrom(TDirectoryFile::Class_Name())){
            key = dynamic_cast<TDirectoryFile*>(Folder)->Get(key->GetName());
        }

        TString cname = key->ClassName();
        TString name = key->GetName();
        auto *entry = new TGLVEntry(fContents,name,cname);
        entry->SetSubnames(key->GetTitle());
        fContents->AddItem(entry);

        if(cname==TList::Class_Name() || cname==TDirectoryFile::Class_Name())
            entry->SetPictures(gClient->GetPicture("folder_s.xpm"),gClient->GetPicture("folder_t.xpm"));
        if(cname==TCutG::Class_Name())
            entry->SetPictures(gClient->GetPicture("bld_cut.png"),gClient->GetPicture("bld_cut.png"));
        if(cname.BeginsWith(TGraph::Class_Name()))
            entry->SetPictures(gClient->GetPicture("graph.xpm"),gClient->GetPicture("graph.xpm"));

        // user data is a filename
        entry->SetUserData((void*)StrDup(list->GetName()));
    }
    fMain->Resize();
}

void CXFileList::DisplayObject(const TString& /*fname*/,const TString& name)
{
    // Browse object located in file.
    TDirectory *sav = gDirectory;
    TObject* obj = nullptr;

    if(fFolders.empty()) return;

    TObject *CurrentFolder = fFolders.back();

    if(name==".."){
        fFolders.pop_back();
        obj=fFolders.back();
        fFolders.pop_back();
    }
    else if(CurrentFolder->InheritsFrom(TList::Class_Name()))
        obj = dynamic_cast<TList*>(CurrentFolder)->FindObject(name);
    else if(CurrentFolder->InheritsFrom(TDirectoryFile::Class()))
        obj = dynamic_cast<TDirectoryFile*>(CurrentFolder)->Get(name);

    if(obj == nullptr){
        DisplayFile(fCurrentFile->GetName());
        return;
    }

    if (obj)
    {
        if (!obj->IsFolder()){

            TString CanvasName = fMainWindow->GetCanvas()->GetName();

            if(CanvasName.BeginsWith("GxG") || CanvasName.BeginsWith("RadGG") ){
                cout<<" cannot plot in a Tab dedicated to Gamma Gamma projections" << endl;
                return;
            }
            if(CanvasName.BeginsWith("RadCube")){
                cout<<" cannot plot in a Tab dedicated to Cube projections" << endl;
                return;
            }

            fMainWindow->DoDraw(obj,fBrowser->GetDrawOption());
        }
        else if(obj->InheritsFrom(TList::Class()) || obj->InheritsFrom(TDirectoryFile::Class())){
            fFolders.push_back(obj);
            if(obj->InheritsFrom(TList::Class()))
                DisplayList(dynamic_cast<TList*>(obj));
            if(obj->InheritsFrom(TDirectoryFile::Class()))
                DisplayList(dynamic_cast<TDirectoryFile*>(obj)->GetListOfKeys());
        }
    }

    gDirectory = sav;
}

void CXFileList::OnClick(TGLVEntry *f, Int_t btn, Int_t x, Int_t y)
{
    if (btn == kButton3) {
        if(!fFolders.empty()) {
            TObject *CurrentFolder = fFolders.back();

            TString name(f->GetTitle());

            TObject* obj = nullptr;

            if(CurrentFolder->InheritsFrom(TDirectoryFile::Class_Name()))
                obj = dynamic_cast<TDirectoryFile*>(CurrentFolder)->Get(name);
            else if(CurrentFolder->InheritsFrom(TList::Class_Name()))
                obj = CurrentFolder->FindObject(name);

            if(obj == nullptr)
                return;

            fContextMenu->Popup(x, y, obj, fBrowser->GetBrowser());
        }
        else if(fRadCubeMode) {
            TString name(f->GetTitle());
            TObject *obj = nullptr;
            if(name=="2D Proj") obj = fRadReader->Get2DProj();
            if(obj == nullptr)
                return;

            fContextMenu->Popup(x, y, obj, fBrowser->GetBrowser());
        }
    }
}

void CXFileList::OnDoubleClick(TGLVEntry *f, Int_t btn)
{
    // Handle double click.

    if (btn != kButton1) return;

    // set kWatch cursor
    ULong_t cur = gVirtualX->CreateCursor(kWatch);
    gVirtualX->SetCursor(fContents->GetId(), cur);

    TString name(f->GetTitle());
    const char* fname = static_cast<const char*>(f->GetUserData());

    if(fRadCubeMode) {

        TObject *obj = nullptr;

        TString CanvasName = fMainWindow->GetCanvas()->GetName();

        if(CanvasName.BeginsWith("GxG") || CanvasName.BeginsWith("RadGG") ){
            cout<<" cannot plot in a Tab dedicated to Gamma Gamma projections" << endl;
            return;
        }
        if(CanvasName.BeginsWith("RadCube")){
            cout<<" cannot plot in a Tab dedicated to Cube projections" << endl;
            return;
        }

        if(name=="LUT") obj = fRadReader->GetLUTSpectrum();
        else if(name=="Total Projection") obj = fRadReader->GetTotalProjection();
        else if(name=="Background") obj = fRadReader->GetBackground();
        else if(name=="2D Proj") obj = fRadReader->Get2DProj();
        else if(name=="Energy calibration") obj = fRadReader->GetECalibFunc();
        else if(name=="Efficiency") obj = fRadReader->GetEEffFunc();
        else if(name=="Cube File") fMainWindow->InitRadCubePlayer(fRadReader);

        fMainWindow->DoDraw(obj,fBrowser->GetDrawOption());
    }
    else {
        DisplayObject(fname, name);
    }

    // set kPointer cursor
    cur = gVirtualX->CreateCursor(kPointer);
    gVirtualX->SetCursor(fContents->GetId(), cur);
}

void CXFileList::CloseWindow()
{
    delete this;
}

Bool_t CXFileList::ReadConfFile()
{
    cout<<"\e[0;3;32m"<<endl;
    cout<<"***********************************"<<endl;
    cout<<"* Reading Cube Configuration File *"<<endl;
    cout<<"***********************************"<<endl;
    cout<<"Conf file: "<<fConfFileName<<endl;
    cout<<endl;gbash_color->ResetColor();

    std::ifstream FileConf;
    FileConf.open(fConfFileName.Data());

    if(!FileConf){
        gbash_color->ErrorMessage(fConfFileName + " not found ==> EXIT");
        return false;
    }

    string line;
    TString Buffer;

    while(FileConf)
    {
        getline(FileConf,line);
        Buffer = line;

        if(Buffer.BeginsWith("#")) continue;
        if(Buffer.Copy().ReplaceAll(" ","").ReplaceAll("\t","") == "") continue;
        if(Buffer.BeginsWith("CubeFileName")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fCubeFileName = ((TString)loa->At(1)->GetName());
            delete loa;
            gbash_color->InfoMessage("Cube Filename: " + fCubeFileName);
        }
        else if(Buffer.BeginsWith("2DProj")){
            TObjArray *loa=Buffer.Tokenize(" ");
            f2DProjName = ((TString)loa->At(1)->GetName());
            delete loa;
            gbash_color->InfoMessage("2D projection file: " + f2DProjName);
        }
        else if(Buffer.BeginsWith("LUT")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fLUTFileName = ((TString)loa->At(1)->GetName());
            delete loa;
            gbash_color->InfoMessage("LUT file: " + fLUTFileName);
        }
        else if(Buffer.BeginsWith("CalFile")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fECalFileName = ((TString)loa->At(1)->GetName());
            delete loa;
            gbash_color->InfoMessage("Energy calibration file: " + fECalFileName);
        }
        else if(Buffer.BeginsWith("TotalProj")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fTotalProjFile = ((TString)loa->At(1)->GetName());
            delete loa;
            gbash_color->InfoMessage("Total projection file: " + fTotalProjFile);
        }
        else if(Buffer.BeginsWith("BackgroudFile")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fBackgroundFile = ((TString)loa->At(1)->GetName());
            delete loa;
            gbash_color->InfoMessage("BackgroudFile file: " + fBackgroundFile);
        }
        else if(Buffer.BeginsWith("EffFile")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fEffFileName = ((TString)loa->At(1)->GetName());
            delete loa;
            gbash_color->InfoMessage("Efficiency file: " + fEffFileName);
        }
        else if(Buffer.BeginsWith("CompressFact")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fCompFactor = ((TString)loa->At(1)->GetName()).Atof();
            delete loa;
            gbash_color->InfoMessage(Form("Compression factor: %f",fCompFactor));
        }
        else{
            gbash_color->InfoMessage("Unkown keyword: " + Buffer);
            cout<<endl;
            return false;
        }
    }

    cout<<endl;

    return true;
}


ClassImp(CXFileList);
