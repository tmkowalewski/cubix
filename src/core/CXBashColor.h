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

#ifndef CXBashColor_H
#define CXBashColor_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

class CXBashColor
{
protected:
    static CXBashColor *g_bash_color;

public:
    //! current state of the FrameIO
    enum BColor
    {
        kBlack = 0,
        kRed = 1,
        kGreen = 2,
        kYellow = 3,
        kBlue = 4,
        kPurple = 5,
        kCyan = 6,
        kWhite = 7
    };
    enum bType
    {
        kRegular = 0,
        kBold = 1,
        kUnderline = 2,
        kBoldUnderline = 3,
        kBackGround = 4,
        kHighIntensity = 5,
        kBoldHighIntensity = 6,
        kHighIntensityBackground = 7
    };

public:
    CXBashColor(){;}
    virtual ~CXBashColor(){;}

    //! singleton
    static CXBashColor *the_bash_color();

    std::string fColor = "";
    std::string fType = "0;3";

    void SetType(bType type);
    void SetColor(BColor col);

    void SetErrorOut();
    void SetWarningOut();
    void SetInfoOut();

    void ProcessColor();
    void ResetColor();

    void ErrorMessage(const char*mess, bool with_prepend=true);
    void WarningMessage(const char*mess, bool with_prepend=true);
    void InfoMessage(const char*mess, bool with_prepend=true);
};

#define gbash_color (CXBashColor::the_bash_color())

#endif
