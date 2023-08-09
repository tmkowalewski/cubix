#ifndef CXBgdFit_H
#define CXBgdFit_H

#include "TObject.h"

#include <vector>

class CXHist1DPlayer;
class TVirtualPad;
class TH1;
class TF1;
class CXArrow;

class CXBgdFit : public TObject
{
private:
    CXHist1DPlayer *fPlayer         = nullptr;
    TList       *fListOfArrows      = nullptr;
    TVirtualPad *fPad               = nullptr;

    TH1         *fHistogram         = nullptr;

    TF1         *fBackFunction      = nullptr;

    std::vector<Double_t> fBackgd;

public:
    CXBgdFit(TH1 *hist, TVirtualPad *pad, CXHist1DPlayer *player);
    ~CXBgdFit();

    void AddArrow(Double_t Energy);
    void RemoveArrow(CXArrow *arrow = nullptr);
    void Update();
    void Fit();

    Double_t FuncBackground(Double_t*xx,Double_t*pp);

    void Clear(TVirtualPad *pad = nullptr);

    ClassDef(CXBgdFit,0);
};

#endif
