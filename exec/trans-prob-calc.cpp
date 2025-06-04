#include <TString.h>
#include <iostream>
#include <cmath>
#include <iomanip>

// Constants
const double HBAR = 6.582119569509067e-22;       // MeV.s
const double HBAR_C = 197.3269804685045;         // MeV.fm
const double LN2 = std::log(2.0);
const double e2 = 1.439964546;                   // MeV.fm

using namespace std;

long long oddDoubleFactorial(int n) {
    if (n < 0) throw std::invalid_argument("L must be ≥ 0");
    long long result = 1;
    for (int k = n; k > 1; k -= 2) {
        result *= k;
    }
    return result;
}

// Compute Weisskopf unit for electric or magnetic transition
// E: true for electric, false for magnetic
// Returns Weisskopf unit in e^2.fm^{2L} or mu_N^2.fm^{2L-2}
double weisskopf_unit(bool E, int L, int A) {
    if(E) return pow(1.2,2.*L)/(4.*M_PI)*3./(L+3.)*3./(L+3.)*pow(A,2.*L/3.);
    return 10./M_PI*pow(1.2,2.*L-2.)*3./(L+3.)*3./(L+3.)*pow(A,(2.*L-2.)/3.);
}

// Calculate coefficient from theory:
// Gamma = coeff * E^{2L+1} * B(XL)
double theoretical_prefactor(bool E, int L) {
    double num = 8.0 * M_PI * (L + 1) * e2;
    double den = L * std::pow(oddDoubleFactorial(2 * L + 1), 2);
    double prefactor = num / den;
    return prefactor;
}

// Compute B(XL) from lifetime and energy
// E_MeV: energy in MeV, halflife_s: lifetime in seconds
// Returns B(XL) in e^2.fm^{2L} or mu_N^2.fm^{2L-2}
double compute_BXL(bool E, int L, double E_MeV, double halflife_s) {
    double coeff = theoretical_prefactor(E, L);
    double E_over_hbarc = E_MeV / HBAR_C;
    double tau_s = halflife_s / LN2;
    double Gamma = HBAR / tau_s;          // s-1
    double BXL = Gamma / (coeff * std::pow(E_over_hbarc, 2 * L + 1));
    return BXL;
}

// Compute half-life from B(XL) and energy
// E_MeV: energy in MeV, B(XL) in e^2.fm^{2L} or mu_N^2.fm^{2L-2}
// Returns half-life in s
double compute_halflife(bool E, int L, double E_MeV, double BXL) {
    double coeff = theoretical_prefactor(E, L);
    double E_over_hbarc = E_MeV / HBAR_C;
    double Gamma = BXL * (coeff * std::pow(E_over_hbarc, 2 * L + 1));
    double tau_s = HBAR / Gamma;
    double halflife_s = tau_s*LN2;

    return halflife_s;
}

double E_keV=0., dE_keV=0., t_halflife=0., dt_halflife=0.;
int A=0, L=0;
double BXL=0.;
TString type="";
bool isElectric;

int main(int argc, char **argv)
{
    // Parse command-line options:
    for(int i=0 ; i<argc ; i++) {
        if(((TString)argv[i]).Contains("-de") && ++i<argc) dE_keV = atof(argv[i]);
        else if(((TString)argv[i]).Contains("-e") && ++i<argc) E_keV = atof(argv[i]);
        else if(((TString)argv[i]).Contains("-dt") && ++i<argc) dt_halflife = atof(argv[i]);
        else if(((TString)argv[i]).Contains("-t") && ++i<argc) t_halflife = atof(argv[i]);
        else if(((TString)argv[i]).Contains("-a") && ++i<argc) A = atoi(argv[i]);
        else if(((TString)argv[i]).Contains("-b") && ++i<argc) BXL = atof(argv[i]);
        else if(((TString)argv[i]).Contains("-m") && ++i<argc) {
            TString mult = argv[i];
            type = mult(0,1);
            L = ((TString)(mult(1,1))).Atoi();
            isElectric = (type == "E" || type == "e");
        }
    }

    std::cout << "=== B(XL) Transition Probability Calculator ===" << std::endl;

    if(type == "" || L==0) {
        cout << "missing multipolarity type and/or L" << endl;
        return EXIT_FAILURE;
    }
    isElectric = (type == "E" || type == "e");

    if(E_keV>0) cout << "Energy: " << E_keV;if(dE_keV) cout << " ± " << dE_keV; cout << " keV" << endl;
    if(t_halflife>0) cout << "half-life value: " << t_halflife;if(dt_halflife) cout << " ± " << dt_halflife; cout << " s" << endl;
    if(BXL>0) cout << "transition probability: " << BXL << " W.u." << endl;
    if(A>0) cout << "mass number: " << A << endl;
    cout << "multipolarity: " << type << L << endl;

    if(t_halflife>0. && E_keV>0. && BXL==0.) {

        double E_MeV = E_keV / 1000.0;
        double BXL = compute_BXL(isElectric, L, E_MeV, t_halflife);
        double BWU = BXL / weisskopf_unit(isElectric, L, A);

        double rel_unc_tau = dt_halflife / t_halflife;
        double rel_unc_E = dE_keV / E_keV;

        double rel_unc_B = std::sqrt( std::pow(rel_unc_tau, 2) + std::pow((2 * L + 1) * rel_unc_E, 2));
        double dBXL = BXL * rel_unc_B;
        double dB_WU = dBXL / weisskopf_unit(isElectric, L, A);

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "\n--- Results ---\n";
        std::cout << "Transition        : " << (isElectric ? "E" : "M") << L << std::endl;
        std::cout << "Energy            : " << E_keV << " keV" << std::endl;
        std::cout << "Halflife          : " << std::scientific << t_halflife << " s" << std::endl;

        std::cout << "B(" << (isElectric ? "E" : "M") << L << ") : " << std::scientific << BXL;
        if(dBXL>0.) cout << " ± " << dBXL; cout << " [e^2.fm^{" << 2*L <<  "}]" << endl;

        std::cout << "B(" << (isElectric ? "E" : "M") << L << ") : " << std::scientific << BWU;
        if(dB_WU>0.) cout << " ± " << dBXL; cout << " [W.u.]" << endl;
    }
    else if(t_halflife==0. && E_keV>0. && BXL>0.) {


        double E_MeV = E_keV / 1000.0;

        double WU = weisskopf_unit(isElectric, L, A);
        double B = BXL*WU;
        double halflife_s = compute_halflife(isElectric, L, E_MeV, B);

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "\n--- Results ---\n";
        std::cout << "Transition        : " << (isElectric ? "E" : "M") << L << std::endl;
        std::cout << "Energy            : " << E_keV << " keV" << std::endl;
        std::cout << "B(" << (isElectric ? "E" : "M") << L << ") [W.u.]      : " << std::scientific << BXL << std::endl;
        std::cout << "B(" << (isElectric ? "E" : "M") << L << ") [e^2.fm^{" << 2*L <<  "}]: " << std::scientific << B << std::endl;
        std::cout << "Halflife (s)      : " << std::scientific << halflife_s << std::endl;
    }

    return 0;
}
