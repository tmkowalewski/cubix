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

#include "CXSpreadIntensityMatrix.h"

#include <iostream>
#include <iomanip>

using namespace std;

CXSpreadIntensityMatrix::CXSpreadIntensityMatrix() = default;

CXSpreadIntensityMatrix::~CXSpreadIntensityMatrix() = default;

CXSpreadIntensityMatrix::CXSpreadIntensityMatrix(const char* name, const char *title) : TNamed (name,title)
{
    fCorr_Above=1.;
    fSGate=1.;
}

Float_t CXSpreadIntensityMatrix::Get_Corr_Above()
{
    return fCorr_Above;
}
Float_t CXSpreadIntensityMatrix::Get_S_Gate()
{
    return fSGate;
}

Bool_t CXSpreadIntensityMatrix::IsDet(const TMatrixD &Mat_TS_Brute,const Int_t Size)
{
    Bool_t Inversible=true;
    TMatrixD I(Size,Size);
    I.UnitMatrix();

    TMatrixD P1(I,TMatrixD::kMinus,Mat_TS_Brute);

    if(P1.Determinant()==0)Inversible=false;

    return Inversible;
}



TMatrixD CXSpreadIntensityMatrix::Get_Total_P(const TMatrixD &Mat_TS_Brute,const Int_t Size)
{

    TMatrixD I(Size,Size);
    I.UnitMatrix();

    TMatrixD P1(I,TMatrixD::kMinus,Mat_TS_Brute);

    TMatrixD P(Size,Size);
    P.Zero();


    P=P1.Invert()-I;



    return P;

}

TMatrixD CXSpreadIntensityMatrix::Get_Final_M(TMatrixD P,TMatrix Fill_S,const Int_t Size)
{

    TMatrixD Final(Size,Size);


    for(Int_t i=0;i<Size;i++)
    {
        for(Int_t j=0;j<Size;j++)
        {

            Final[i][j]=Fill_S[i][0]*P[i][j];
        }

    }


    return Final;

}

TMatrixD CXSpreadIntensityMatrix::Get_S_And_E(TMatrixD Final_Matrix,TMatrixD E_Matrix,Float_t Gate,const Int_t Size)
{

    TMatrixD Final_S_E(Size,2);


    Int_t k=0;
    for(Int_t i=0;i<Size;i++)
    {
        if(Gate==E_Matrix[i][0])break;
        k++;
    }

    if(k!=Size)

    {

        for(Int_t i=0;i<Size;i++)
        {
            Final_S_E[i][0]=E_Matrix[i][0];//the ernergy
            Final_S_E[i][1]=Final_Matrix[i][k]+Final_Matrix[k][i];//the strength
        }

    }

    return Final_S_E;

}


TMatrixD CXSpreadIntensityMatrix::Get_S_And_E_2(TMatrixD Mat_S,TMatrixD Mat_P,TMatrixD Final_Matrix,TMatrixD E_Matrix,Float_t Gate1,Float_t Gate2,const Int_t Size)
{

    TMatrixD Final_S_E(Size,2);//Final Matrix and below the lowest gate
    TMatrixD IB(Size,2);//matrix between
    TMatrixD IH(Size,2);//Above the higher gate

    Int_t k=0;
    for(Int_t i=0;i<Size;i++)
    {
        if(Gate1==E_Matrix[i][0])break;
        k++;
    }
    Int_t l =0;
    for(Int_t i=0;i<Size;i++)
    {
        if(Gate2==E_Matrix[i][0])break;
        l++;
    }

    //-------------------------------- COMMENTS ABOUT THE COMPUTATION------------------------------//

    //---Mat_S[k][0]->Raw Strength of the highest gate
    //---Mat_S[l][0]->Raw Strength of the Lowest gate
    //--Mat_P[k][l]-> Probility to make a decay Gate1->Gate2 (index of Gate1==k)(index of Gate2==l)
    //--Mat_P[i][l]-> Probability to decay to Gate2
    //--Mat_P[k][i]-> Probability to decay make a decay gate1->i



    if(k!=Size && l!=Size)

    {

        for(Int_t i=0;i<Size;i++)
        {
            Final_S_E[i][0]=E_Matrix[i][0];//the ernergy
            IB[i][0]=E_Matrix[i][0];


            //            //*******************************************************OTHER METHOD*************************************************

            //            // Below the lowest gate
            //            if((Final_Matrix[i][l]+Final_Matrix[l][i])!=0) Final_S_E[i][1]=(Final_Matrix[l][i]);

            //            //just between the first and the second gate

            //            if(Final_Matrix[i][l] !=0 /*&& Mat_P[k][i]!=0*/){ IB[i][1]=Mat_P[i][l]*((Mat_P[k][i])*Mat_S[l][0])/Mat_P[k][l];}


            //            //just above the second gate

            //            if(Final_Matrix[i][k] !=0) IH[i][1]=(Final_Matrix[i][k])*Mat_S[l][0]/Mat_S[k][0];

            //             //******************************************************* END OTHER METHOD*************************************************


            // Below the lowest gate
            if((Final_Matrix[i][l]+Final_Matrix[l][i])!=0) {Final_S_E[i][1]=(Mat_P[l][i]*Mat_S[l][0]*Mat_S[k][0]);}



            //just between the first and the second gate

            if(Final_Matrix[i][l] !=0 ){ IB[i][1]=(Mat_P[i][l]*Mat_P[k][i]*Mat_S[l][0]*Mat_S[k][0])/Mat_P[k][l];}



            //just above the second gate

            if(Final_Matrix[i][k] !=0) IH[i][1]=(Mat_P[i][k])*Mat_S[l][0]*Mat_S[i][0];


            //Add All

            Final_S_E[i][1]+=IB[i][1]+IH[i][1];

        }

    }

    return Final_S_E;

}



TMatrixD CXSpreadIntensityMatrix::Get_S_And_E_3(TMatrixD Mat_P,TMatrixD Final_Matrix,TMatrixD E_Matrix,Float_t Gate1,Float_t Gate2,Float_t Gate3,const Int_t Size)
{



    //-------------------------------- TO BE CONTINUED------------------------------//
    //--------------------------------- DO NOT WORK--------------------------------//

    TMatrixD Final_S_E(Size,2);

    Int_t k=0;
    for(Int_t i=0;i<Size;i++)
    {
        if(Gate1==E_Matrix[i][0])break;
        k++;
    }
    Int_t l =0;
    for(Int_t i=0;i<Size;i++)
    {
        if(Gate2==E_Matrix[i][0])break;
        l++;
    }
    Int_t m =0;
    for(Int_t i=0;i<Size;i++)
    {
        if(Gate3==E_Matrix[i][0])break;
        m++;
    }

    if(k!=Size && l!=Size && m!=Size)

    {

        for(Int_t i=0;i<Size;i++)
        {
            Final_S_E[i][0]=E_Matrix[i][0];//the ernergy
            if((Final_Matrix[i][l]+Final_Matrix[l][i])!=0 && (Final_Matrix[i][m]+Final_Matrix[m][i])!=0) Final_S_E[i][1]=(Final_Matrix[i][k]+Final_Matrix[k][i])*Mat_P[k][l]*Mat_P[k][m];//the strength
        }

    }


    return Final_S_E;

}

ClassImp(CXSpreadIntensityMatrix);
