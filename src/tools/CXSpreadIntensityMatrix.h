#ifndef CXSpreadIntensityMatrix_H
#define CXSpreadIntensityMatrix_H

#include "TNamed.h"

#include "TMatrixD.h"
#include "TMatrix.h"
#include "TArrayD.h"

class CXSpreadIntensityMatrix : public TNamed
{

private:
    Float_t fCorr_Above{};
    Float_t fSGate{};

public:
    CXSpreadIntensityMatrix();
    CXSpreadIntensityMatrix(const char* name, const char *title);
    virtual ~CXSpreadIntensityMatrix();

    TMatrixD Get_Total_P(const TMatrixD &Mat_TS_Brute, const Int_t Size);
    TMatrixD Get_Final_M(TMatrixD P,TMatrix Fill_S,const Int_t Size);
    TMatrixD Get_S_And_E(TMatrixD Final_Matrix,TMatrixD E_Matrix,Float_t Gate,const Int_t Size);
    TMatrixD Get_S_And_E_2(TMatrixD Mat_S,TMatrixD Mat_P,TMatrixD Final_Matrix,TMatrixD E_Matrix,Float_t Gate,Float_t Gate2,const Int_t Size);
    TMatrixD Get_S_And_E_3(TMatrixD Mat_P,TMatrixD Final_Matrix,TMatrixD E_Matrix,Float_t Gate1,Float_t Gate2,Float_t Gate3,const Int_t Size);

    Bool_t IsDet(const TMatrixD &Mat_TS_Brute, const Int_t Size);

    Float_t Get_Corr_Above();
    Float_t Get_S_Gate();

    ClassDef(CXSpreadIntensityMatrix,0)
};

#endif // CXSpreadIntensityMatrix_H
