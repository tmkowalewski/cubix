As for the levels, from the level scheme, it is possible to access to all the known decays of a nucleus. In the current version, only gamma decays are implemented. Particle decays are planned to be added in next TkN version.

Print the known decays as follows:
```cpp
tknucleus nuc("8B");
nuc.get_level_scheme()->print("decay")
```
```shell
[ INFO     ] dataset '8B : ADOPTED LEVELS, GAMMAS' contains 2 decays:
[ INFO     ] gamma decay energy = 769.5   (2.4  ) keV, mult M1
[ INFO     ] gamma decay energy = 2320    (30   ) keV, mult M1
```

It is also possible to print at the same time the levels and its associated decays:
```cpp
tknucleus nuc("8B");
nuc.get_level_scheme()->print("level,decay")
```
```shell
[ INFO     ] dataset '8B : ADOPTED LEVELS, GAMMAS' contains 5 levels
[ INFO     ] dataset '8B : ADOPTED LEVELS, GAMMAS' contains 2 decays
[ INFO     ] Level energy = 0      keV [no uncertainty] ; Jpi: 2+   ; lifetime = 770     (3    ) ms
[ INFO     ] Level energy = 769.5   (2.5  ) keV ; Jpi: 1+   ; lifetime = 35.6    (0.6  ) keV
             -> gamma decay energy = 769.5   (2.4  ) keV, mult M1
[ INFO     ] Level energy = 2320    (20   ) keV ; Jpi: 3+   ; lifetime = 350     (30   ) keV
             -> gamma decay energy = 2320    (30   ) keV, mult M1
[ INFO     ] Level energy = 3500    (500  ) keV ; Jpi: 2-   ; lifetime = 8       (4    ) MeV
[ INFO     ] Level energy = 10619   (9    ) keV ; Jpi: 0+   ; lifetime < 60     keV [limit value]
```

As for the levels, The decay list can be retrieved using:
```cpp
tknucleus nuc("8B");
auto levels = nuc.get_level_scheme()->get_decays();
```
Is is also possible to restrict the decay list to a type of decay:
```cpp
tknucleus nuc("8B");
auto levels = nuc.get_level_scheme()->get_decays<tkgammadecay>();
```
As for the moment only tkn::tkgammadecay are included, it is advised to only use this second option to easily benefit from the tkgammadecay methods on the decay list.

The same can be done using a lambda function to filter the decays (e.g. only decays coming from a level lower than 1 MeV):
```cpp
tknucleus nuc("8B");
auto decays = nuc.get_level_scheme()->get_decays<tkgammadecay>( [](auto dec) {
    return dec->get_level_from()->get_energy_measure()->get_value(tkunit_manager::MeV) < 1.;
});
for(auto &dec: decays) dec->print()
```
```shell
[ INFO     ] gamma decay energy = 769.5   (2.4  ) keV, mult M1
```

A decay can also been accessed directly by its name (e.g. the decay from the first 3⁺ to the first 2⁺ state):
```cpp
tknucleus nuc("8B");
nuc.get_level_scheme()->get_decay<tkgammadecay>("3+1->2+1")->print();
```
```shell
[ INFO     ] gamma decay energy = 2320    (30   ) keV, mult M1
```
, or using its energy (the closest level in energy will be returned):
```cpp
tknucleus nuc("8B");
nuc.get_level_scheme()->get_decay<tkgammadecay>(700.)->print();
```
```shell
[ INFO     ] gamma decay energy = 769.5   (2.4  ) keV, mult M1
```

The get_decay(...) methods return a tkn::tkdecay object, and the get_levels(...) methods return a vector of tkn::tkdecay objects. As explained, if we want to play only with gamma decays, the template access get_decay<tkgammadecay>(...) methods return a tkn::tkgammadecay object, and the get_levels<tkgammadecay>(...) methods return a vector of tkn::tkgammadecay
the tkn::tkgammadecay class inherits from the tkn::tkdecay, so all methods applicable for a tkdecay are also valid for a tkgammadecay.

Here are the main method that can be applied on a [tkdecay](@ref tkn::tkdecay):
<center>
|method name| description |
|---|---|
|[get_energy()](@ref tkn::tkdecay::get_energy())          | to get the energy in keV                                                                       |
|[get_decay_type()](@ref tkn::tkdecay::get_decay_type())  | to get the decay type [kbeta, kec, kalpha, kparticle, kgamma], but only kgamma for the moment  |
|[get_level_from()](@ref tkn::tkdecay::get_level_from())  | returns the parent level                                                                       |
|[get_level_to()](@ref tkn::tkdecay::get_level_to())      | returns the daughter level                                                                     |
|[has_comment()](@ref tkn::tkdecay::has_comment())        | returns true if a comment on this level exists                                                 |
|[get_comment()](@ref tkn::tkdecay::get_comment())        | returns the comment on this level                                                              |
|[is_uncertain()](@ref tkn::tkdecay::is_uncertain())      | returns true if a the level is uncertain                                                       |
|[print()](@ref tkn::tkdecay::print())                    | prints the main level informations (as already used above)                                     |
</center>

Here are the main method that can be applied on a [tkgammadecay](@ref tkn::tkgammadecay):
<center>
|method name| description |
|---|---|
|[get_relative_intensity()](@ref tkn::tkgammadecay::get_relative_intensity())             |  get the gamma relative intensity (100% correspond the the more intence gamma-ray) as a tkn::tkmeasure |
|[get_mixing_ratio()](@ref tkn::tkgammadecay::get_mixing_ratio())                         |  get the mixing ratio|
|[get_conv_coeff()](@ref tkn::tkgammadecay::get_conv_coeff())                             |  get the gamma conversion coefficient|
|[get_trans_prob(bool _elec, int _L, bool _WU)](@ref tkn::tkgammadecay::get_trans_prob()) |  get the transition probability|
|[get_multipolarity()](@ref tkn::tkgammadecay::get_multipolarity())                       |  get the transition multipolarity as a string|
</center>


The above methods giving access to a measure (ex: [get_conv_coeff()](@ref tkn::tkgammadecay::get_conv_coeff()) ), are directly returning the value as a double. Dedicated methods allows to access the tkn::tkmeasure to access to units, uncertainties... etc (see @ref _units).

For exemple, for the lifetime, the method [get_conv_coeff_measure()](@ref tkn::tkgammadecay::get_conv_coeff_measure()) returns the tkmeasure object.

