#ifndef CXSavedList_H
#define CXSavedList_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TList.h"

using namespace std;

class CXMainWindow;
class TGListBox;
class TGTextEntry;

class CXSavedList : public  TGVerticalFrame
{
    RQ_OBJECT("CXSavedList");

private:

    CXMainWindow *fMainWindow = nullptr;

    TGListBox *fStoredSpectraBox = nullptr;
    TList *fListOfStoredSpectra = nullptr;
    TGTextEntry *fDrawOpt = nullptr;

    TList *fListOfBoxesToUpdate = nullptr;

public:
    CXSavedList(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h);
    ~CXSavedList();

    void SetMainWindow(CXMainWindow *w);

    void AddToStoredList(TObject *o);
    void UpdateStoredList();
    void ClearStoredList();
    void RemoveSelectedEntry();
    void SaveStoredList();
    void ProcessedButtonEvent(Event_t *);
    void DoubleClicked(Int_t id);
    TList *GetListOfStoredSpectra(){return fListOfStoredSpectra;}
    void AddListBox(TGListBox *box);

    void SetListDrawOption(const TString &opt);

    ClassDef(CXSavedList,0);
};

#endif
