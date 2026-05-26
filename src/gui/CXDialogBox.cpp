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

#include "CXDialogBox.h"

#include <iostream>

#include "TMath.h"
#include "TGLabel.h"
#include "TGTextEntry.h"
#include "TGButton.h"
#include "TVirtualX.h"

using namespace std;

extern TGTextEntry *gBlinkingEntry;

CXDialogBox::CXDialogBox(const TGWindow *main, const char *title) : TGTransientFrame(gClient->GetRoot(), main, 200, 100)
{
    SetWindowName(title);
    SetIconName(title);
    SetEditDisabled(kEditDisable);

    fWidgets = new TList;

    fL1 = new TGLayoutHints(kLHintsTop | kLHintsCenterX, 0, 0, 5, 0);
    fL2 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

    AddInput(kKeyPressMask | kEnterWindowMask | kLeaveWindowMask);
}

CXDialogBox::~CXDialogBox()
{
    fWidgets->Delete();
    delete fWidgets;

    delete fL1;
    delete fL2;
}

void CXDialogBox::Add(const TString &argname, TString &value)
{
    if(fMappingOfReturnValues.count(argname)) {
        cout << argname << " already present in the arguments, skippde" << endl;
        return;
    }

   TGLabel      *l = new TGLabel(this, argname);
   TString svalue(value);

   TGTextBuffer *b = new TGTextBuffer(20); b->AddText(0, svalue.Data());
   TGTextEntry  *t = new TGTextEntry(this, b);
   t->SetName(argname);

   t->Connect("TabPressed()", "CXDialogBox", this, "TabPressed()");
   t->Connect("ReturnPressed()", "CXDialogBox", this, "ReturnPressed()");
   t->Resize(260, t->GetDefaultHeight());
   AddFrame(l, fL1);
   AddFrame(t, fL2);

   fWidgets->Add(l);
   fWidgets->Add(t);   // TGTextBuffer will be deleted by TGTextEntry

   fMappingOfReturnValues[argname] = &value;
}

void CXDialogBox::Popup()
{
    TGHorizontalFrame *hf = new TGHorizontalFrame(this, 60, 20, kFixedWidth);
    TGLayoutHints     *l1 = new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 5, 5, 0, 0);

    // put hf as last in the list to be deleted
    fWidgets->Add(l1);

    UInt_t  nb = 0, width = 0, height = 0;

    TGTextButton *b = new TGTextButton(hf, "OK", kOK);
    b->Connect("Clicked()", "CXDialogBox", this, "HandleButtons()");
    fWidgets->Add(b);
    hf->AddFrame(b, l1);
    width  = TMath::Max(width, b->GetDefaultWidth()); ++nb;

    b = new TGTextButton(hf, "Cancel", kCancel);
    b->Connect("Clicked()", "CXDialogBox", this, "HandleButtons()");
    fWidgets->Add(b);
    hf->AddFrame(b, l1);
    height = b->GetDefaultHeight();
    width  = TMath::Max(width, b->GetDefaultWidth()); ++nb;

    // place buttons at the bottom
    l1 = new TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5);
    fWidgets->Add(l1);
    fWidgets->Add(hf);

    AddFrame(hf, l1);

    // keep the buttons centered and with the same width
    hf->Resize((width + 20) * nb, height);

    // map all widgets and calculate size of dialog
    MapSubwindows();

    width  = GetDefaultWidth();
    height = GetDefaultHeight();

    Resize(width, height);

    // position relative to the parent's window
    CenterOnParent();

    // make the message box non-resizable
    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    SetMWMHints(kMWMDecorAll | kMWMDecorResizeH  | kMWMDecorMaximize | kMWMDecorMinimize | kMWMDecorMenu,
                kMWMFuncAll  | kMWMFuncResize    | kMWMFuncMaximize | kMWMFuncMinimize, kMWMInputModeless);
    MapWindow();
    fClient->WaitFor(this);
}

void CXDialogBox::TabPressed()
{
    Bool_t setNext = kFALSE;
    TGTextEntry *entry;
    TIter next(fWidgets);

    while ( TObject* obj = next() ) {
        if ( obj->IsA() == TGTextEntry::Class() ) {
            entry = (TGTextEntry*) obj;
            if ( entry == gBlinkingEntry ) {
                setNext = kTRUE;
            } else if ( setNext ) {
                entry->SetFocus();
                entry->End();
                return;
            }
        }
    }

    next.Reset();
    while ( TObject* obj = next() ) {
        if ( obj->IsA() == TGTextEntry::Class() ) {
            entry = (TGTextEntry*) obj;
            entry->SetFocus();
            entry->End();
            return;
        }
    }
}

void CXDialogBox::HandleButtons(int id)
{
    // Handle different buttons
    auto *btn = static_cast<TGButton*>(gTQSender);
    if (id == -1) id = btn->WidgetId();

    if(id == kOK) {
        TIter next(fWidgets);
        TString label_name="";
        while ( TObject* obj = next() ) {
            if ( obj->IsA() == TGLabel::Class() ) {
                TGLabel *label = (TGLabel*) obj;
                label_name = label->GetText()->GetString();
            }
            if ( label_name != "" && obj->IsA() == TGTextEntry::Class() ) {
                TGTextEntry *entry = (TGTextEntry*) obj;
                *fMappingOfReturnValues[label_name] = entry->GetText();
            }
        }
        this->CloseWindow();
    }
    if(id == kCancel) this->CloseWindow();
}

ClassImp(CXDialogBox)
