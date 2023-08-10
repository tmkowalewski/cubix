To access any nuclear property, a @ref tkn::tknucleus needs to be created. it can be created using its symbol, or its proton and mass number:

```{cpp}
tknucleus nuc("235U");
tknucleus nuc2(92,235);
```

A nucleus can also be create only using it's element name or charge number.
In such case, the most stable isotope will be returned, and the most abundant in case of multiple stable nuclei:

```shell
tknucleus nuc("U");
tknucleus nuc2(92);
```

The main properties of a nucleus can be printed with:

```shell
tknucleus nuc1("12C");
nuc1.print()
[ INFO     ] 12C (Z=6, N=6) properties:
[ INFO     ] Ground state configuration: 0+
[ INFO     ] Stable nucleus, abundance: 98.93 %

tknucleus nuc2("U");
nuc2.print()
[ INFO     ] 238U (Z=92, N=146) properties:
[ INFO     ] Ground state configuration: 0+
[ INFO     ] Radioactive nucleus:
[ INFO     ] lifetime = 4e+09   (6e+06) y
[ INFO     ] Decay: [A;100.000000][SF;0.000054]
```

For each element, the following properties **can be** available:

- charge
- symbol
- name
- group block
- standard state
- atomic mass (u)
- electronic configuration
- atomic radius (van der Waals) (pm)
- ionization energy (eV)
- melting point (K)
- boiling point (K)
- density (g/cm³)
- year discovered
- list of X-rays (keV)

Then, for each isotope, the following properties **can be** available:

- abundance (%)
- binding energy per nucleon (keV)
- binding energy minus Liquid Drop model fit per nucleon (keV)
- cross sections:
  - thermal neutron capture cross section at 300K (barns)
  - thermal neutron fission cross section at 300K (barns)
- decay mode (returned as a string)
- fission yields:
  - thermal neutron-induced fission yields for 235-Uranium
  - thermal neutron-induced fission yields for 239-Plutonium
  - spontaneous fission yields for 252-Californium
- ground state lifetime
- ground state spin-parity
- mass excess (keV)
- pairing gap (keV)
- Q-values (keV):
  - α
  - ∆α: 0.5 x ( Qα(Z+2,N+2) - Qα(Z,N) )
  - ß⁻
  - double ß⁻
  - ß⁺
  - electron capture
  - double electron capture
  - ß⁻ delayed neutron emission
  - ß⁻ delayed double neutron emission
  - electron capture followed by proton emission
- quadrupole deformation ß²
- separation energies(keV):
  - neutron separation energy
  - two neutron separation energy
  - proton separation energy
  - two proton separation energy

to see the available properties for a given nucleus, use the [tknucleus::list_properties()](@ref tkn::tknucleus::list_properties()) method:

```shell
tknucleus nuc("12C")
nuc.list_properties()
[ INFO     ] nucleus properties:
[ INFO     ] Qalpha                             -7366.590000 keV                   [FLOAT]
[ INFO     ] QbetaMinus                         -17338.100000 keV                  [FLOAT]
[ INFO     ] QbetaMinusOneNeutronEmission       -32437.000000 keV                  [FLOAT]
[ INFO     ] QbetaMinusTwoNeutronEmission       -54942.700000 keV                  [FLOAT]
[ INFO     ] QdeltaAlpha                        102.335000 keV                     [FLOAT]
[ INFO     ] QdoubleBetaMinus                   -32013.300000 keV                  [FLOAT]
[ INFO     ] QdoubleElectronCapture             -25077.800000 keV                  [FLOAT]
[ INFO     ] QelectronCapture                   -13369.400000 keV                  [FLOAT]
[ INFO     ] QelectronCaptureOneProtonEmission  -27466.100000 keV                  [FLOAT]
[ INFO     ] QpositronEmission                  -14391.400000 keV                  [FLOAT]
[ INFO     ] XRay_Kalpha1                       0.277000 keV                       [FLOAT]
[ INFO     ] abundance                          98.930000 %                        [FLOAT]
[ INFO     ] atomic_mass                        12.011000 u                        [FLOAT]
[ INFO     ] atomic_radius_van_der_Waals        170.000000 pm                      [FLOAT]
[ INFO     ] binding_energy_ldm_fit_overA       -210.981000 keV                    [FLOAT]
[ INFO     ] binding_energy_overA               7680.140000 keV                    [FLOAT]
[ INFO     ] boiling_point                      4098.000000 K                      [FLOAT]
[ INFO     ] charge                             6                                  [TEXT]
[ INFO     ] decay_modes                                                           [TEXT]
[ INFO     ] density                            2.267000                           [FLOAT]
[ INFO     ] electronic_configuration           [He]2s2 2p2                        [TEXT]
[ INFO     ] group_block                        Nonmetal                           [TEXT]
[ INFO     ] ionization_energy                  11.260000 eV                       [FLOAT]
[ INFO     ] lifetime                           STABLE                             [FLOAT]
[ INFO     ] mass_excess                        0.000000 keV                       [FLOAT]
[ INFO     ] melting_point                      3823.000000 K                      [FLOAT]
[ INFO     ] name                               Carbon                             [TEXT]
[ INFO     ] neutronSeparationEnergy            18720.700000 keV                   [FLOAT]
[ INFO     ] pairingGap                         6887.200000 keV                    [FLOAT]
[ INFO     ] protonSeparationEnergy             15956.700000 keV                   [FLOAT]
[ INFO     ] quadrupoleDeformation              0.576601                           [FLOAT]
[ INFO     ] spin_parity                        0+                                 [TEXT]
[ INFO     ] state                              Solid                              [TEXT]
[ INFO     ] symbol                             C                                  [TEXT]
[ INFO     ] twoNeutronSeparationEnergy         31841.300000 keV                   [FLOAT]
[ INFO     ] twoProtonSeparationEnergy          27185.400000 keV                   [FLOAT]
[ INFO     ] year_discovered                    Ancient                            [TEXT]
```

The results can be filtered using a regular expression:

```shell
tknucleus nuc("12C")
nuc.list_properties("Q*")
[ INFO     ] nucleus properties:
[ INFO     ] Qalpha                             -7366.590000 keV                   [FLOAT]
[ INFO     ] QbetaMinus                         -17338.100000 keV                  [FLOAT]
[ INFO     ] QbetaMinusOneNeutronEmission       -32437.000000 keV                  [FLOAT]
[ INFO     ] QbetaMinusTwoNeutronEmission       -54942.700000 keV                  [FLOAT]
[ INFO     ] QdeltaAlpha                        102.335000 keV                     [FLOAT]
[ INFO     ] QdoubleBetaMinus                   -32013.300000 keV                  [FLOAT]
[ INFO     ] QdoubleElectronCapture             -25077.800000 keV                  [FLOAT]
[ INFO     ] QelectronCapture                   -13369.400000 keV                  [FLOAT]
[ INFO     ] QelectronCaptureOneProtonEmission  -27466.100000 keV                  [FLOAT]
[ INFO     ] QpositronEmission                  -14391.400000 keV                  [FLOAT]
```

To test if a property is available, use the use the [tknucleus::has_property()](@ref tkn::tknucleus::has_property()) method:

- If the requested property is available and is a measured property (with a value and an associated error) get it using: [tkproperty_list::get()](@ref tkn::tkproperty_list::get())
```cpp
tknucleus nuc("12C");
if(nuc.has_property("abundance")) {
    auto value = nuc.get("abundance");
    cout << nuc.get_symbol() << " abundance: " << value->get_value() << " +- " << value->get_error() << " " << value->get_unit() << endl;
}
```
```shell
12C abundance: 98.93 +- 0.08 %
```
The returned value is a tkn::tkmeasure, see the dedicated [section](#_units)

- If the requested property is available and is refering to a string value (e.g. drounf state spin parity), get it using
[tknucleus::get_property()](@ref tkn::tknucleus::get_property())
```cpp
tknucleus nuc("13C");
value = nuc.get_property("spin_parity");
cout << nuc.get_symbol() << " GS spin parity: " << value << endl;
```
```shell
13C GS spin parity: 1/2-
```

For a faster access to the most used properties, few specific methods have been added to tknucleus objects:

<center>
|method name| description |
|---|---|
|[get_a()](@ref tkn::tknucleus::get_a())                                         | nucleon number  |
|[get_n()](@ref tkn::tknucleus::get_n())                                         | neutron number  |
|[get_z()](@ref tkn::tknucleus::get_z())                                         | proton number  |
|[get_symbol()](@ref tkn::tknucleus::get_symbol())                               | nucleus name (ex: 12C) |
|[get_element_name()](@ref tkn::tknucleus::get_element_name())                   | element name (ex: Carbon) |
|[get_element_symbol()](@ref tkn::tknucleus::get_element_symbol())               | element symbol (ex: C) |
|is_z_magic()](@ref tkn::tknucleus::is_z_magic())                                | is Z a magic number |
|is_n_magic()](@ref tkn::tknucleus::is_n_magic())                                | is N a magic number |
|is_doubly_magic()](@ref tkn::tknucleus::is_doubly_magic())                      | is nucleus doubly magic |
|[get_abundance()](@ref tkn::tknucleus::get_abundance())                         | abundance for stable nuclei (in %) |
|[get_binding_energy_over_a()](@ref tkn::tknucleus::get_binding_energy_over_a()) | binding energy per nucleon (MeV) (unit as parameter) |
|[get_lifetime()](@ref tkn::tknucleus::get_lifetime())                           | ground state lifetime in seconds (unit as parameter) |
|[get_mass_excess()](@ref tkn::tknucleus::get_mass_excess())                     | mass excess (MeV) (unit as parameter) |
|[get_jpi()](@ref tkn::tknucleus::get_jpi())                                     | ground state spin and parity as a string |
|[get_xrays()](@ref tkn::tknucleus::get_xrays())                                 | vector of the known xray energies |
|[get_decay_mode_str()](@ref tkn::tknucleus::get_decay_mode_str())               | decay modes as a string |
|[get_decay_modes()](@ref tkn::tknucleus::get_decay_modes())                     | decay modes as a vector of decay and branching ratio |
|[get_ground_state()](@ref tkn::tknucleus::get_ground_state())                   | access the ground state tkn::tklevel|
|[get_level_scheme()](@ref tkn::tknucleus::get_level_scheme())                   | access the tkn::tklevel_scheme|
</center>

The above methods giving access to a measure (ex: [get_lifetime()](@ref tkn::tknucleus::get_lifetime()) ), are directly returning the value as a double. Dedicated methods allows to access the tkn::tkmeasure to access to units, uncertainties... etc (see @ref _units).

For exemple, for the lifetime, the method [get_lifetime_measure()](@ref tkn::tknucleus::get_lifetime_measure()) returns the tkmeasure object.
