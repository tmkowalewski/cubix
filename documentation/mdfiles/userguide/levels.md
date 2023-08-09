For any nucleus, it is possible to access to its level scheme composed of the known levels and decays. Print the known levels as follows:
```cpp
tknucleus nuc("8B");
nuc.get_level_scheme()->print("level")
```
```shell
[ INFO     ] dataset '8B : ADOPTED LEVELS, GAMMAS' contains 5 levels:
[ INFO     ] Level energy = 0      keV [no uncertainty] ; Jpi: 2+   ; lifetime = 770     (3    ) ms
[ INFO     ] Level energy = 769.5   (2.5  ) keV ; Jpi: 1+   ; lifetime = 35.6    (0.6  ) keV
[ INFO     ] Level energy = 2320    (20   ) keV ; Jpi: 3+   ; lifetime = 350     (30   ) keV
[ INFO     ] Level energy = 3500    (500  ) keV ; Jpi: 2-   ; lifetime = 8       (4    ) MeV
[ INFO     ] Level energy = 10619   (9    ) keV ; Jpi: 0+   ; lifetime < 60     keV [limit value]
```

The level list can be retrieved using:
```cpp
tknucleus nuc("8B");
auto levels = nuc.get_level_scheme()->get_levels();
```
or using a lambda function to filter the levels (e.g. all the positive parity states in):
```cpp
tknucleus nuc("8B");
auto levels = nuc.get_level_scheme()->get_levels( [](auto lvl) {
    return lvl->get_spin_parity()->get_parity().is_parity(tkparity::kParityPlus);
});
for(auto &lev: levels) lev->print()
```
```shell
[ INFO     ] Level energy = 0      keV [no uncertainty] ; Jpi: 2+   ; lifetime = 770     (3    ) ms
[ INFO     ] Level energy = 769.5   (2.5  ) keV ; Jpi: 1+   ; lifetime = 35.6    (0.6  ) keV
[ INFO     ] Level energy = 2320    (20   ) keV ; Jpi: 3+   ; lifetime = 350     (30   ) keV
[ INFO     ] Level energy = 10619   (9    ) keV ; Jpi: 0+   ; lifetime < 60     keV [limit value]
```

A level can also been accessed directly by its name (e.g. the first 3⁺ state):
```cpp
tknucleus nuc("8B");
nuc.get_level_scheme()->get_level("3+1")->print();
```
```shell
[ INFO     ] Level energy = 2320    (20   ) keV ; Jpi: 3+   ; lifetime = 350     (30   ) keV
```
, or using its energy (the closest level in energy will be returned):
```cpp
tknucleus nuc("8B");
nuc.get_level_scheme()->get_level(3000.)->print();
```
```shell
[ INFO     ] Level energy = 3500    (500  ) keV ; Jpi: 2-   ; lifetime = 8       (4    ) MeV
```

The get_level(...) methods return a tkn::tklevel object, and the get_levels(...) methods return a vector of tkn::tklevel objects.

Here are the main method that can be applied on a [tklevel](@ref tkn::tklevel):

<center>
|method name| description |
|---|---|
| [get_energy()](@ref tkn::tklevel::get_energy())                   | to get the energy in keV                                    |
| [get_lifetime()](@ref tkn::tklevel::get_lifetime())               | to get the lifetime in seconds                              |
| [get_spin_parity()](@ref tkn::tklevel::get_spin_parity())         | to get the spin and parity as a tkn::tkspin_parity object   |
| [get_spin_parity_str()](@ref tkn::tklevel::get_spin_parity_str()) | to get the spin and parity as a string                      |
| [is_stable()](@ref tkn::tklevel::is_stable())                     | returns true is the level is stable                         |
| [is_isomer()](@ref tkn::tklevel::is_isomer())                     | returns true is the level is an isomer                      |
| [get_isomer_level()](@ref tkn::tklevel::get_isomer_level())       | returns the isomer level (1st, second,...)                  |
| [is_yrast()](@ref tkn::tklevel::is_yrast())                       | returns true is the level yrast                             |
| [get_decays_up()](@ref tkn::tklevel::get_decays_up())             | returns the list of decays that populate this level         |
| [get_decays_down()](@ref tkn::tklevel::get_decays_down())         | returns the list of decays that depopulate this level       |
| [has_comment()](@ref tkn::tklevel::has_comment())                 | returns true if a comment on this level exists              |
| [get_comment()](@ref tkn::tklevel::get_comment())                 | returns the comment on this level                           |
| [is_uncertain()](@ref tkn::tklevel::is_uncertain())               | returns true if a the level is uncertain                    |
| [print()](@ref tkn::tklevel::print())                             | prints the main level informations (as already used above)  |
</center>

The above methods giving access to a measure (ex: [get_lifetime()](@ref tkn::tklevel::get_lifetime()) ), are directly returning the value as a double. Dedicated methods allows to access the tkn::tkmeasure to access to units, uncertainties... etc (see @ref _units).

For exemple, for the lifetime, the method [get_lifetime_measure()](@ref tkn::tklevel::get_lifetime_measure()) returns the tkmeasure object.
