#include "tkmanager.h"

#include "TH2F.h"
#include "TFile.h"

using namespace tkn;

int main(int /*argc*/, char **/*argv*/)
{
    glog.set_log_level(tklog::kerror);

    std::cout << "**************************************************" << std::endl;
    std::cout << "* Plotting the energies of the first 2+ state    *" << std::endl;
    std::cout << "**************************************************" << std::endl;
    std::cout<<endl;

    TH2F *myhist = new TH2F("First2PlusState","First2PlusState",90,0,180,60,0,120);
    myhist->GetXaxis()->SetTitle("N");
    myhist->GetYaxis()->SetTitle("Z");

    int inuc=0;
    tkstring message = "Reading the database";
    auto fullsize=gmanager->get_nuclei().size();

    for(auto nuc : gmanager->get_nuclei()) {
        glog.progress_bar(fullsize,inuc,message.data());
        auto lvl = nuc->get_level_scheme()->get_level("2+1",false);
        if(lvl != nullptr) {
            myhist->Fill(nuc->get_n(),nuc->get_z(),lvl->get_energy());
        }
        inuc++;
    }

    TFile *fout = new TFile("my_prog_ROOT.root","recreate");
    myhist->Write();
    fout->Close();
    cout<<endl;
    glog.set_log_level(tklog::kinfo);
    glog << info << "First 2+ states ploted in \"my_prog_ROOT.root\"" << do_endl;
}
