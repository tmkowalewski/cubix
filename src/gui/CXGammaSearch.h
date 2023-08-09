#ifndef CXGammaSearch_H
#define CXGammaSearch_H

#include "TGFrame.h"
#include "TString.h"
#include "TMatrixD.h"
#include <RQ_OBJECT.h>

#include "tkmanager.h"

class CXLevelSchemePlayer;
class CXSpreadIntensityMatrix;
class CXMainWindow;
class TGListBox;
class TGNumberEntry;
class TGLabel;
class TGCheckButton;
class TGRadioButton;

struct GammaTransition{
    double EGamma;
    double EI;
    double EF;
    TString SpinI;
    TString SpinF;
    TString NucName;
    TString LifeTime;
};

class CXGammaSearch: public TGTransientFrame
{
    enum ECheckCommand
    {
        M_With_Cascade,
        M_WithOut_Cascade
    };

    RQ_OBJECT("CXGammaSearch");

private:

    CXMainWindow *fMainWindow;

    CXSpreadIntensityMatrix *fSp;

    TGLabel *fNNucAnalysed;
    TGLabel *fNGrayAnalysed;

    TGNumberEntry *fEnergies[3];
    TGNumberEntry *fWidths[3];
    TGCheckButton *fCheckGammas[3];

    TGNumberEntry *fZRange[2];
    TGNumberEntry *fARange[2];
    TGNumberEntry *fNRange[2];

    Int_t fNGammas=1;

    TGTextButton *fStartButton,*fLogButton;

    shared_ptr<tkn::tklevel_scheme> fLevelScheme;

    ECheckCommand fCurrentMode;

    TGRadioButton *fCoincMode;
    TGRadioButton *fNoCoincMode;

    TString fDataBaseFolder;

    TGListBox *fResultsBox;

    std::vector < std::vector < GammaTransition > > fListOfGoodGammas;

public:

    CXGammaSearch(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, CXMainWindow *mwin);
    virtual ~CXGammaSearch();

    void HandleButtons();

    void FindGammaRays(Bool_t Bash=false);

    void SetCalMode();

    void PrintInListBox(TString mess, Int_t Type);

protected:

    void FindInDoubleCoincidence(Bool_t Bash = false);
    void FindInTripleCoincidence();

    std::vector<int> Sort_Index(std::vector<int> Index_Nuclei, std::vector<double> Tab_P, std::vector<double> Tab_DeltaE);

    TMatrixD Fill_TS_Matrix(shared_ptr<tkn::tklevel_scheme> fLevelScheme);
    TMatrixD Get_E_Matrix(shared_ptr<tkn::tklevel_scheme> fLevelScheme);

    ClassDef(CXGammaSearch,0)
};


#endif //CXGammaSearch_H

