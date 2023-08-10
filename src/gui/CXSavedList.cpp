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

#include "CXSavedList.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "TGButton.h"
#include "TGTextEntry.h"
#include "TFile.h"
#include "TGLabel.h"
#include "TGListBox.h"
#include "TError.h"
#include "TGFileDialog.h"
#include "Tsystem.h"

#include "CXMainWindow.h"

using namespace std;

CXSavedList::CXSavedList(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    auto *fSubGroupFrame = new TGGroupFrame(MotherFrame, "Stored spectra", kVerticalFrame);
    fSubGroupFrame->SetTextColor(CXblue);
    fSubGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fSubGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY | kLHintsExpandX, 3, 3, 0, 0));

    auto *fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Draw Options:"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 5, 0, 0));
    fDrawOpt = new TGTextEntry(fHorizontalFrame, "hist");
    fDrawOpt->SetToolTipText("hist, same, norm, add, add(fact), div(fact), mult(fact)",300);
    fHorizontalFrame->AddFrame(fDrawOpt,new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsCenterY ,0,0,0,0));
    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,5));

    fStoredSpectraBox = new TGListBox(fSubGroupFrame);
    fStoredSpectraBox->GetContainer()->RemoveInput(kButtonReleaseMask | kButtonMotionMask);
    fSubGroupFrame->AddFrame(fStoredSpectraBox, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,-10,-10,0,0));
    fStoredSpectraBox->Connect("DoubleClicked(Int_t)", "CXSavedList", this, "DoubleClicked(Int_t)");
    fStoredSpectraBox->GetContainer()->Connect("ProcessedEvent(Event_t*)", "CXSavedList", this, "ProcessedButtonEvent(Event_t*)");

    fHorizontalFrame = new TGCompositeFrame(fSubGroupFrame, 60, 20, kHorizontalFrame);
    auto *Button = new TGTextButton(fHorizontalFrame, "Remove");
    Button->Connect("Clicked()", "CXSavedList", this, "RemoveSelectedEntry()");
    fHorizontalFrame->AddFrame(Button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    Button = new TGTextButton(fHorizontalFrame, "Clear");
    Button->Connect("Clicked()", "CXSavedList", this, "ClearStoredList()");
    fHorizontalFrame->AddFrame(Button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    Button = new TGTextButton(fHorizontalFrame, "Save");
    Button->Connect("Clicked()", "CXSavedList", this, "SaveStoredList()");
    fHorizontalFrame->AddFrame(Button,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    fSubGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fListOfBoxesToUpdate = new TList;

    AddListBox(fStoredSpectraBox);

    fListOfStoredSpectra = new TList;
    fListOfStoredSpectra->SetOwner();
}

CXSavedList::~CXSavedList() = default;

void CXSavedList::SetMainWindow(CXMainWindow *w)
{
    fMainWindow = w;
}

void CXSavedList::AddToStoredList(TObject *o){

    fListOfStoredSpectra->Remove(fListOfStoredSpectra->FindObject(o->GetName()));
    fListOfStoredSpectra->Add(o);

    UpdateStoredList();
}

void CXSavedList::UpdateStoredList(){

    gErrorIgnoreLevel = kError;

    for(int i=0 ; i<fListOfBoxesToUpdate->GetEntries() ; i++) {
        auto *box = dynamic_cast<TGListBox*>(fListOfBoxesToUpdate->At(i));

        box->RemoveAll();

        for(int i=0 ; i<fListOfStoredSpectra->GetEntries() ; i++){
            auto *entry = new TGTextLBEntry(box->GetContainer(), new TGString(fListOfStoredSpectra->At(i)->GetName()), box->GetNumberOfEntries()+1);
            entry->SetBackgroundColor((Pixel_t)0x90f269);
            box->AddEntry((TGLBEntry *)entry, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX));
        }

        box->Layout();
    }

    gErrorIgnoreLevel = kPrint;
}

void CXSavedList::RemoveSelectedEntry(){
    if(fStoredSpectraBox->GetSelectedEntry()){
        fListOfStoredSpectra->Remove(fListOfStoredSpectra->FindObject(fStoredSpectraBox->GetSelectedEntry()->GetTitle()));
        UpdateStoredList();
    }
    else
        cout<<"No selected entry"<<endl;
}

void CXSavedList::SaveStoredList(){
    if(fListOfStoredSpectra->GetEntries()==0){
        cout<<"No histogram to save"<<endl;
    }

    const char* SaveAsTypes[] = {
        "ROOT files",   "*.root",
        nullptr,              nullptr
    };

    TString workdir = gSystem->WorkingDirectory();
    static TString dir(".");
    static Int_t typeidx = 0;
    static Bool_t overwr = kFALSE;
    TGFileInfo fi;
    fi.fFileTypes   = SaveAsTypes;
    fi.fIniDir      = StrDup(dir);
    fi.fFileTypeIdx = typeidx;
    fi.fOverwrite   = overwr;
    new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), kFDSave, &fi);
    gSystem->ChangeDirectory(workdir.Data());
    if (!fi.fFilename) return;
    TString fn = fi.fFilename;
    dir     = fi.fIniDir;
    typeidx = fi.fFileTypeIdx;
    overwr  = fi.fOverwrite;

    auto *FileOut = new TFile(fn.Data(),"recreate");
    for(int i=0 ; i<fListOfStoredSpectra->GetEntries() ; i++)
        fListOfStoredSpectra->At(i)->Write();
    FileOut->Close();
}

void CXSavedList::ClearStoredList(){
    fListOfStoredSpectra->Clear();
    UpdateStoredList();
}

void CXSavedList::ProcessedButtonEvent(Event_t */*event*/)
{
    //    char input[10];
    //    UInt_t keysym;

    //    gVirtualX->LookupString(event, input, sizeof(input), keysym);

    //    //        std::cout << "event : " << event->fCode << " " << event->fState <<" ; "<< event->fType  << "; " << keysym << std::endl;

    //    //    if(event->fType == kGKeyPress && keysym == kKey_Control)
    //    //        fCtrl_On = true;

    //    if(event->fType == kButtonPress){

    //        if(fLastSelected){
    //            fLastSelected->SetBackgroundColor((Pixel_t)0x90f269);
    //            fLastSelected = nullptr;
    //            fStoredSpectraBox->Layout();
    //        }

    //        if(fStoredSpectraBox->GetSelectedEntry()){
    //            fStoredSpectraBox->GetSelectedEntry()->SetBackgroundColor((Pixel_t) 0x87a7d2);
    //            fLastSelected = fStoredSpectraBox->GetSelectedEntry();
    //            fStoredSpectraBox->Select(fStoredSpectraBox->GetSelected(),false);
    //            fStoredSpectraBox->Layout();
    //        }
    //    }
}

void CXSavedList::DoubleClicked(Int_t id)
{
    TObject *o = fListOfStoredSpectra->At(id-1);

    if(fMainWindow->GetCanvas()->GetUniqueID() == 666) {
        cout<<"Cannot draw on this Canvas"<<endl;
        return;
    }

    gErrorIgnoreLevel = kError;
    TObject *clone = o->Clone();
    gErrorIgnoreLevel = kPrint;

    fMainWindow->DoDraw(clone,fDrawOpt->GetText());
}

void CXSavedList::AddListBox(TGListBox *box)
{
    fListOfBoxesToUpdate->Add(box);
}


void CXSavedList::SetListDrawOption(const TString &opt)
{
    fDrawOpt->SetTitle(opt);
}


ClassImp(CXSavedList)
