To handle the fact that a measure is always associated to a specific unit and an uncertainty, that can be in some cases assymetric, a dedicated class called tkn::tkmeasure has been created.

In addition, a tkmeasure gives some properties on the measure, like if the measure is uncertain, is coming from a systematic strudy or from calculations. It can also handle the fact that some measures are given as a limit value.

The standard way to access a tkmeasure is using the [tkproperty_list::get()](@ref tkn::tkproperty_list::get()) method:

```cpp
tknucleus nuc("14C");
auto measure = nuc.get("lifetime");
cout << measure->get_value() << " " << measure->get_unit() <<  " +- " << measure->get_error() << endl;
```
```shell
5700 y +- 30
```

To list the accessible tkmeasure properties, use:

```cpp
tknucleus nuc("14C");
nuc.list_data_properties()
```
```shell
[ INFO     ] nucleus data properties:
[ INFO     ] Qalpha                             Qalpha = -12012.500 (0.081) keV
[ INFO     ] QbetaMinus                         QbetaMinus = 156.4760 (0.0037) keV
[ INFO     ] QbetaMinusOneNeutronEmission       QbetaMinusOneNeutronEmission = -10396.9000 (0.2696) keV
[ INFO     ] QbetaMinusTwoNeutronEmission       QbetaMinusTwoNeutronEmission = -30460.80000000 (0.00537582) keV
[ INFO     ] QdeltaAlpha                        QdeltaAlpha = 2892.4400000 (0.0405445) keV
[ INFO     ] QdoubleBetaMinus                   QdoubleBetaMinus = -4987.8900 (0.0254) keV
[ INFO     ] QdoubleElectronCapture             QdoubleElectronCapture = -36934.600 (132.245) keV
[ INFO     ] QelectronCapture                   QelectronCapture = -20643.8000 (21.2133) keV
[ INFO     ] QelectronCaptureOneProtonEmission  QelectronCaptureOneProtonEmission = -37928.2000 (10.1805) keV
[ INFO     ] QpositronEmission                  QpositronEmission = -21665.8000 (21.2133) keV
[ INFO     ] XRay_Kalpha1                       XRay_Kalpha1 = 0.277  keV
[ INFO     ] atomic_mass                        atomic_mass = 12.011 u
[ INFO     ] atomic_radius_van_der_Waals        atomic_radius_van_der_Waals = 170    pm
[ INFO     ] binding_energy_ldm_fit_overA       binding_energy_ldm_fit_overA = -37.0681 (0.0004) keV
[ INFO     ] binding_energy_overA               binding_energy_overA = 7520.3200 (0.0004) keV
[ INFO     ] boiling_point                      boiling_point = 4098   K
[ INFO     ] density                            density = 2.267
[ INFO     ] ionization_energy                  ionization_energy = 11.26  eV
[ INFO     ] lifetime                           lifetime = 5700    (30   ) y
[ INFO     ] mass_excess                        mass_excess = 3019.89000 (0.00375) keV
[ INFO     ] melting_point                      melting_point = 3823   K
[ INFO     ] neutronSeparationEnergy            neutronSeparationEnergy = 8176.4300 (0.0038) keV
[ INFO     ] pairingGap                         pairingGap = 3479.180000 (0.399772) keV
[ INFO     ] protonSeparationEnergy             protonSeparationEnergy = 20831.0000 (1.0001) keV
[ INFO     ] quadrupoleDeformation              quadrupoleDeformation = 0.3593840 (0.0279568)
[ INFO     ] twoNeutronSeparationEnergy         twoNeutronSeparationEnergy = 13122.7000 (0.0039) keV
[ INFO     ] twoProtonSeparationEnergy          twoProtonSeparationEnergy = 36635.8000 (1.9086) keV
```

The same applies for a tklevel or a tkdecay:
```cpp
tknucleus nuc("132Sn");
auto level = nuc.get_level_scheme()->get_level("2+1");
level->list_data_properties();
```
```shell
[ INFO     ] data properties:
[ INFO     ] energy                             Level energy = 4041.20 (0.15 ) keV
[ INFO     ] lifetime                           lifetime = 2.4     (-0.5   ; +0.9  ) fs
```

In this case, the lifetime uncertainty is asymetric, we can access it as follows:

```cpp
tknucleus nuc("132Sn");
auto level = nuc.get_level_scheme()->get_level("2+1");
auto lifetime =  level->get("lifetime");
cout << lifetime->get_value() << " " << lifetime->get_unit() <<  " - " << lifetime->get_error_low() <<  " + " << lifetime->get_error_high() << endl;
```
```shell
2.4 fs - 0.5 + 0.9
```
