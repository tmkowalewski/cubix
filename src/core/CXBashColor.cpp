/***************************************************************************
 *   Copyright (c) IPNL, IN2P3, CNRS                                       *
 *   Contibutor(s) :                                                       *
 *      Jeremie Dudouet  dudouet(AT)ipnl.in2p3.fr [2014]                   *
 *      Olivier Stezowski stezow(AT)ipnl.in2p3.fr [2014]                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
