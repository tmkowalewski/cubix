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
