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

#ifndef CXFitFunctions_h
#define CXFitFunctions_h

#include <cmath>

#include "TMath.h"

#include "cubix_config.h"

#ifdef HAS_MATHMORE
#include "Math/SpecFuncMathMore.h"
#endif

using namespace std;

class CXFitFunctions
{
public:
    CXFitFunctions() = default;
    virtual ~CXFitFunctions() = default;

    static double EfficiencyFunc(double*x,double*p) {
        double EG = x[0];
        double eff=0;

        double Scale=p[0];

        double A=p[1];
        double B=p[2];
        double C=p[3];

        //high energy
        double D=p[4];
        double E=p[5];
        double F=p[6];

        // interaction parameter between the two regions
        // the larger G is, the sharper will be the turnover at the top, between the two curves.
        // If the efficiency turns over gently, G will be small.
        double G=p[7];

        double E1=100.; //100 keV
        double E2=1000.; //1000 keV

        double x1 = log(EG / E1);
        double x2 = log(EG / E2);

        double f1 = A + B*x1 + C*x1*x1;
        double f2 = D + E*x2 + F*x2*x2;

        if (f1 < 0. || f2 < 0.)
            eff = 0.0;
        else {
            double x3 = exp(-G * log(f1)) + exp(-G * log(f2));

            if (x3 <= 0.) eff = 0.0;
            else eff = exp(exp(-log(x3) / G));
        }
        if(eff<0. || isnan(eff)) eff=0.;

        return Scale*eff;
    }

    static double PolynomialFunc(double*x,double*p) {
        Double_t value=0;
        int caliborder = p[0];

        for(int i=0 ; i<=caliborder ; i++) {
            value += p[i+1]*TMath::Power(x[0],i);
        }
        return value;
    }

    static double FWHMFunction(double*x,double*p) {

        double E = x[0];
        double F = p[0];
        double G = p[1];
        double H = p[2];

        return sqrt(F*F + G*G*E + H*H*E*E);
    }

    static double AngCorrFunction(double *x, double *p) {

        double theta = x[0];

        double A0  = p[0];
        double A2  = p[1];
        double A4  = p[2];

        double total=0.;

#ifdef HAS_MATHMORE
        total = A0*(1+A2*ROOT::Math::legendre(2,cos(theta*TMath::DegToRad())) + A4*ROOT::Math::legendre(4,cos(theta*TMath::DegToRad())));
#endif

        return total;
    }

    ClassDef(CXFitFunctions,0)
};

#endif // CXSpreadIntensityMatrix_H
