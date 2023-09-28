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

#include "CXBashColor.h"

CXBashColor *CXBashColor::g_bash_color = nullptr;

CXBashColor *CXBashColor::the_bash_color()
{
    if ( g_bash_color == nullptr ) {
        g_bash_color = new CXBashColor();
    }

    return g_bash_color;
}

void CXBashColor::SetType(bType type)
{
    switch (type) {
    case kRegular:
        fType = "0;3";
        break;
    case kBold:
        fType = "1;3";
        break;
    case kUnderline:
        fType = "4;3";
        break;
    case kBoldUnderline:
        fType = "1;4;3";
        break;
    case kBackGround:
        fType = "0;4";
        break;
    case kHighIntensity:
        fType = "0;9";
        break;
    case kBoldHighIntensity:
        fType = "1;9";
        break;
    case kHighIntensityBackground:
        fType = "0;10";
        break;
    }
    ProcessColor();
}

void CXBashColor::SetColor(BColor col)
{
    std::ostringstream oss;
    oss<<col;
    fColor = oss.str();

    ProcessColor();
}

void CXBashColor::ProcessColor()
{
    std::string s = "\e[" + fType + fColor + "m";
    std::cout<<s;
}

void CXBashColor::ResetColor()
{
    fColor = "";
    fType = "0";

    ProcessColor();

    fColor = "";
    fType = "0;3";
}


void CXBashColor::SetErrorOut()
{
    SetColor(kRed);
    SetType(kBoldHighIntensity);
}

void CXBashColor::SetWarningOut()
{
    SetColor(kYellow);
    SetType(kBoldUnderline);
}

void CXBashColor::SetInfoOut()
{
    SetColor(kGreen);
    SetType(kBoldHighIntensity);
}

void CXBashColor::ErrorMessage(const char* mess, bool with_prepend)
{
    SetErrorOut();
    if(with_prepend) cout << " -- ERROR   : ";
    cout<<mess<<endl;
    ResetColor();
}

void CXBashColor::WarningMessage(const char*mess, bool with_prepend)
{
    SetWarningOut();
    if(with_prepend) cout << " -- WARNNING: ";
    cout<<mess<<endl;
    ResetColor();
}

void CXBashColor::InfoMessage(const char*mess, bool with_prepend)
{
    SetInfoOut();
    if(with_prepend) cout << " -- INFO    : ";
    cout<<mess<<endl;
    ResetColor();
}
