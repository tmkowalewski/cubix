#ifndef CXFit_H
#define CXFit_H

#include "TObject.h"

#include <vector>

class CXHist1DPlayer;
class TVirtualPad;
class TH1;
class TF1;
class CXArrow;

class CXFit : public TObject
{
private:
    CXHist1DPlayer *fPlayer         = nullptr;
    TList       *fListOfArrows      = nullptr;
    TVirtualPad *fPad               = nullptr;

    TH1         *fHistogram         = nullptr;

    TF1         *fFitFunction       = nullptr;
    TF1         *fBackFunction      = nullptr;
    TF1         *fResidue           = nullptr;
    TList       *fListOfPeaks       = nullptr;

    std::vector<Double_t> fEnergies;
    std::vector<Double_t> fBackgd;

public:
    CXFit(TH1 *hist, TVirtualPad *pad, CXHist1DPlayer *player);
    ~CXFit();

    void AddArrow(Double_t Energy);
    void RemoveArrow(CXArrow *arrow = nullptr);
    void Update();
    void Fit();

    Double_t DoubleTailedStepedGaussian(Double_t*xx,Double_t*pp);
    Double_t StepedBackground(Double_t*xx,Double_t*pp);
    Double_t PeakFunction(Double_t*xx,Double_t*pp);
    Double_t Residue(Double_t*xx,Double_t*pp);

    void Clear(TVirtualPad *pad = nullptr);

    ClassDef(CXFit,0);
};

#endif
