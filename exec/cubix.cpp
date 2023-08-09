#include <iostream>
#include <iomanip>

#include "TRint.h"
#include "TString.h"
#include "TEnv.h"
#include "TGClient.h"

#include "cubix_config.h"

#include "CXMainWindow.h"

#include "tkmanager.h"

using namespace std;

void print_splash_screen();

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

    auto Cubix_App = new TRint("App", &argc, argv,nullptr,0,kTRUE);

    print_splash_screen();

    Cubix_App->SetPrompt("\u001b[1;32mCubix [%d]\u001b[1;0m ");

    // make sure that the Gpad and GUI libs are loaded
    TRint::NeedGraphicsLibs();
    TApplication::NeedGraphicsLibs();
    gApplication->InitializeGraphics();

    fCXMainWindow = new CXMainWindow(gClient->GetRoot(), 1300, 600 );
    if(!gSystem->AccessPathName(file) && file.EndsWith(".root"))
        fCXMainWindow->OpenFile(file);

    Cubix_App->Run();

    if (fCXMainWindow) fCXMainWindow->CloseWindow();

    return EXIT_SUCCESS;
}

void print_splash_screen()
{
    TString cubix_version = Form("%d.%d",Cubix_Version_Major,Cubix_Version_Minor);

    cout << "\u001b[1;31m" << R"(   ______)" << "\u001b[1;33m" << R"(        )" << "\u001b[1;32m" << R"(__     )" << "\u001b[1;34m" << R"(_      )" << "\u001b[0;33m" << R"()" << "\u001b[1;0m | Documentation: https://cubix.in2p3.fr/" <<endl;
    cout << "\u001b[1;31m" << R"(  / ____/)" << "\u001b[1;33m" << R"(__  __ )" << "\u001b[1;32m" << R"(/ /_   )" << "\u001b[1;34m" << R"((_))" << "\u001b[0;33m" << R"(_  __)" << "\u001b[1;0m |" <<endl;
    cout << "\u001b[1;31m" << R"( / /    )" << "\u001b[1;33m" << R"(/ / / /)" << "\u001b[1;32m" << R"(/ __ \ )" << "\u001b[1;34m" << R"(/ /)" << "\u001b[0;33m" << R"(| |/_/)" << "\u001b[1;0m | Source: https://gitlab.in2p3.fr/ip2i_gamma/cubix/cubix   " <<endl;
    cout << "\u001b[1;31m" << R"(/ /___ )" << "\u001b[1;33m" << R"(/ /_/ /)" << "\u001b[1;32m" << R"(/ /_/ /)" << "\u001b[1;34m" << R"(/ /)" << "\u001b[0;33m" << R"(_>  <  )" << "\u001b[1;0m |" <<endl;
    cout << "\u001b[1;31m" << R"(\____/)" << "\u001b[1;33m" << R"( \____/)" << "\u001b[1;32m" << R"(/_____/)" << "\u001b[1;34m" << R"(/_/)" << "\u001b[0;33m" << R"(/_/|_|)" << "\u001b[1;0m   | Version " << cubix_version <<endl;
    cout << endl;
}
