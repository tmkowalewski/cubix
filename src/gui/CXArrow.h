#ifndef CXArrow_H
#define CXArrow_H

#include "TArrow.h"
#include "TBox.h"

class CXFit;
class CXBgdFit;

class TLatex;
class TBox;
class TH1;
class TVirtualPad;

class CXArrowBox;

class CXArrow : public  TArrow
{

private:

    TList *fList = nullptr;
    CXFit *fFit = nullptr;
    CXBgdFit *fBgdFit = nullptr;

    TLatex *fLatex = nullptr;
    CXArrowBox *fBox = nullptr;

    Float_t fTextSize = 0.03;

public:

    CXArrow(CXFit *fit, Double_t E,Double_t y1 ,Double_t y2,Float_t arrowsize=0.05, Float_t textsize = 0.03, Option_t *option=">");
    CXArrow(CXBgdFit *fit, Double_t E,Double_t y1 ,Double_t y2,Float_t arrowsize=0.05, Float_t textsize = 0.03, Option_t *option=">");

    ~CXArrow() = default;

    CXFit *GetFit(){return fFit;}
    CXBgdFit *GetBgdFit(){return fBgdFit;}

    //! Energy
    void SetEnergy(Float_t E); // *MENU* *ARGS={E=>fX1}*
    Double_t GetEnergy() const {return fX1;}

    void Set(Double_t X, Double_t Y1, Double_t Y2);

    void RemoveArrow(); // *MENU*
    void RemoveFit(); // *MENU*

    void SetText(TH1 *hist, const TString &text, const TString &tooltip);

    void ClearPad(TVirtualPad *pad = nullptr, bool refresh = true);

    virtual void Paint(Option_t *option = "");

    //! Sort
    virtual Bool_t  IsSortable() const {return kTRUE;}
    virtual Int_t Compare(const TObject *obj) const;

    ClassDef(CXArrow,0);
};

class CXArrowBox : public  TBox
{
private:
    CXArrow *fArrow = nullptr;

public:
    CXArrowBox(CXArrow *arrow) : TBox() {fArrow = arrow;}
    ~CXArrowBox(){;}

    void Clean(){if(fArrow) {fArrow->ClearPad(); delete fArrow; fArrow = nullptr;}}

    ClassDef(CXArrowBox,0);
};

#endif
