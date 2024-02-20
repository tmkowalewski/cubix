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

#include <iostream>
#include <fstream>

#include "TRint.h"
#include "TString.h"
#include "TEnv.h"
#include "TGClient.h"
#include "TSysEvtHandler.h"
#include "TSystem.h"

#include "cubix_config.h"

#include "CXMainWindow.h"

#include "tklog.h"

using namespace std;

void print_splash_screen();

TString CXWorkspace="";
TString Default_WS="None";

CXMainWindow *fCXMainWindow;

class TInterruptHandler : public TSignalHandler {
   public:
      TInterruptHandler() : TSignalHandler(kSigInterrupt, kFALSE) { }

      Bool_t Notify() override
      {
         // do anything

          fCXMainWindow->CloseWindow();

          return kTRUE;
      }
};

bool ReadCubixrc()
{
    TString confile=gSystem->ExpandPathName("${HOME}/.cubixrc");

    // if not in home, use the installed one
    if(gSystem->AccessPathName(confile.Data())) {
        confile = gSystem->ExpandPathName(Form("%s/conf/cubixrc",getenv("CUBIX_SYS")));
        if(gSystem->AccessPathName(confile.Data())) {
            return false;
        }
    }

    ifstream file(confile.Data());
    TString line;
    while(line.ReadLine(file)) {
        if(line.BeginsWith("*")) continue;
        if(line.BeginsWith("#")) continue;
        line.ReplaceAll("\t"," ");
        TObjArray *arr = line.Tokenize(" ");
        TString Key = arr->First()->GetName();
        if(Key.EqualTo("CXWorkspace",TString::kIgnoreCase) && arr->GetEntries()==2) {
            CXWorkspace = gSystem->ExpandPathName(arr->Last()->GetName());
        }
        if(Key.EqualTo("Default_WS",TString::kIgnoreCase) && arr->GetEntries()==2) {
            Default_WS = arr->Last()->GetName();
        }
        delete arr;
    }

    if(CXWorkspace !="" || Default_WS != "None") {
        glog << tkn::info << "Loading cubix configuration from: " << confile.Data() << tkn::do_endl;
        if(CXWorkspace !="") glog << tkn::info << "Cubix Workspace directory -> " << CXWorkspace.Data() << tkn::do_endl;
        if(Default_WS != "None") glog << tkn::info << "Default workspace loaded  -> " << Default_WS << tkn::do_endl;
    }

    return true;
}

void Terminate()
{
    cout << "totototototo" << endl;
}

int main(int argc, char **argv)
{
    if(getenv("CUBIX_SYS") == nullptr) {
        cout<<"Environment variable: CUBIX_SYS needs to be define ==> EXIT"<<endl;
        return 1;
    }
    gEnv->SetValue("Gui.IconPath",Form("%s/icons:%s/icons",getenv("ROOTSYS"),getenv("CUBIX_SYS")));
    gEnv->SetValue("X11.UseXft","yes");
    gEnv->SetValue("Canvas.ShowGuideLines","false");
    gEnv->SetValue("Unix.*.Root.UseTTFonts","true");

    glog.set_warnings(false);

    auto *fxInterruptHandler = new TInterruptHandler();
    fxInterruptHandler->Add();

    TString file="";
    if(argc==2) file = argv[1];

    argc=1;

    auto Cubix_App = new TRint("App", &argc, argv,nullptr,0,kTRUE);

    print_splash_screen();

    Cubix_App->SetPrompt("\u001b[1;32mCubix [%d]\u001b[1;0m ");

    // make sure that the Gpad and GUI libs are loaded
    TRint::NeedGraphicsLibs();
    TApplication::NeedGraphicsLibs();
    gApplication->InitializeGraphics();

    ReadCubixrc();

    fCXMainWindow = new CXMainWindow(gClient->GetRoot(), 1500, 700 );
    gApplication->Connect("Terminate(Int_t)", "CXMainWindow", fCXMainWindow, "CloseWindow()");

    if(CXWorkspace !="") fCXMainWindow->SetWorkSpaceDirectory(CXWorkspace);
    if(Default_WS != "None") fCXMainWindow->LoadWorkSpace(Default_WS);

    if(!gSystem->AccessPathName(file)) fCXMainWindow->OpenFile(file);
    
    Cubix_App->Run();

    if (fCXMainWindow) fCXMainWindow->CloseWindow();

    return EXIT_SUCCESS;
}

void print_splash_screen()
{
    TString cubix_version = Form("%d.%d",Cubix_Version_Major,Cubix_Version_Minor);

    cout << "\u001b[1;31m" << R"(   ______)" << "\u001b[1;33m" << R"(        )" << "\u001b[1;32m" << R"(__     )" << "\u001b[1;34m" << R"(_      )" << "\u001b[0;33m" << R"()" << "\u001b[1;0m | Documentation: https://cubix.in2p3.fr/" <<endl;
    cout << "\u001b[1;31m" << R"(  / ____/)" << "\u001b[1;33m" << R"(__  __ )" << "\u001b[1;32m" << R"(/ /_   )" << "\u001b[1;34m" << R"((_))" << "\u001b[0;33m" << R"(_  __)" << "\u001b[1;0m |" <<endl;
    cout << "\u001b[1;31m" << R"( / /    )" << "\u001b[1;33m" << R"(/ / / /)" << "\u001b[1;32m" << R"(/ __ \ )" << "\u001b[1;34m" << R"(/ /)" << "\u001b[0;33m" << R"(| |/_/)" << "\u001b[1;0m | Source: https://gitlab.in2p3.fr/ip2igamma/cubix/cubix    " <<endl;
    cout << "\u001b[1;31m" << R"(/ /___ )" << "\u001b[1;33m" << R"(/ /_/ /)" << "\u001b[1;32m" << R"(/ /_/ /)" << "\u001b[1;34m" << R"(/ /)" << "\u001b[0;33m" << R"(_>  <  )" << "\u001b[1;0m |" <<endl;
    cout << "\u001b[1;31m" << R"(\____/)" << "\u001b[1;33m" << R"( \____/)" << "\u001b[1;32m" << R"(/_____/)" << "\u001b[1;34m" << R"(/_/)" << "\u001b[0;33m" << R"(/_/|_|)" << "\u001b[1;0m   | Version " << cubix_version <<endl;
    cout << endl;
}
