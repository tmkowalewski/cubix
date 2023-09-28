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

#ifndef CXSpreadIntensityMatrix_H
#define CXSpreadIntensityMatrix_H

#include "TNamed.h"

#include "TMatrixDfwd.h"
#include "TMatrix.h"

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
