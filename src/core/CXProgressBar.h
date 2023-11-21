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

#ifndef CXProgressBar_H
#define CXProgressBar_H

#include <iostream>
#include <thread>

#include "TStopwatch.h"

class CXProgressBar
{
public:

protected:

private:
    TStopwatch *fWatch;
    ULong64_t fNEvts;
    ULong64_t fCurrentEntry;

    int frefreshtime_in_msec = 200;
    std::thread print_thread;

    bool fdo_stop=false;

public:
    CXProgressBar(ULong64_t Nevts=0);
    virtual ~CXProgressBar();
    void SetN(ULong64_t _nevts){fNEvts = _nevts;}

    CXProgressBar &operator++(){
        fCurrentEntry++;
        return *this;
    }
    CXProgressBar& operator+=(ULong64_t n) {
        fCurrentEntry += n;
        return *this;
    }
    CXProgressBar &operator++(int) {
        (*this)+=1;
        return *this;
    }


protected:

private:

    void UpdateStatus();
};

#endif // CXProgressBar_H
