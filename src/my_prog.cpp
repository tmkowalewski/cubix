#include "tknucleus.h"
#include "tkmanager.h"

using namespace tkn;

int main(int /*argc*/, char **/*argv*/)
{
    std::cout << "**************************************************" << std::endl;
    std::cout << "* Ground state spin parity in C isotopic chain   *" << std::endl;
    std::cout << "**************************************************" << std::endl;
    std::cout<<endl;

    for(auto nuc : gmanager->get_nuclei([](auto nuc) {return (nuc->get_z()==6);})) {
        auto lvls = nuc->get_level_scheme()->get_levels([](auto lvl) {return (lvl->get_energy()==0);});

        std::cout << std::setw(5) << std::left << nuc->get_symbol();
        if(lvls.size()) std::cout << lvls.at(0)->get_spin_parity()->get_jpi_str();
        std::cout << std::endl;
    }
}
