The database used for the levels and decays is extracted from the [ENSDF](https://www.nndc.bnl.gov/ensdf/) database.

For a given nucleus, the database is composed of different validated datasets, corresponding to different ways to produce the nucleus. A global dataset named : "ADOPTED LEVELS, GAMMAS" is used to merge all the different datasets in one. This is the default dataset of any nucleus in TkN.

TkN allows to load the different datasets. For a given nucleus, the datasets and their associated ID can be listed using:

```cpp
tknucleus nuc("132Sn");
nuc.get_level_scheme()->print("dataset")
```
```shell
[ INFO     ] Available datasets are :
[ INFO     ] 132Sn : ADOPTED LEVELS, GAMMAS (12581)
[ INFO     ] 133IN B-N DECAY (165 MS) (12582)
[ INFO     ] 248CM SF DECAY (12583)
[ INFO     ] COULOMB EXCITATION (12584)
[ INFO     ] 132IN B- DECAY (0.200 S) (12585)
[ INFO     ] 132SN IT DECAY (2.080 US) (12586)
[ INFO     ] U(N,F):IS,RADIUS:XUNDL-3 (12587)
[ INFO     ] COULOMB EXCITATION:XUNDL-4 (12588)
[ INFO     ] 133IN B-N DECAY:162 MS:XUNDL-5 (12589)
[ INFO     ] 133IN B-N DECAY:167 MS:XUNDL-6 (12590)
[ COMMENT  ] Current dataset is '132Sn : ADOPTED LEVELS, GAMMAS' (12581)
```

As shown in the above example, TkN also include the non evaluated datasets ([XUNDL](https://www.nndc.bnl.gov/ensdf/xundl/)). These datasets are coming from recently published data, but that have still not been evaluated by the National Nuclear Data Center.

The dataset can be selected as follows (using the Coulomb Excitation dataset ids):

```cpp
tknucleus nuc("132Sn");
nuc.get_level_scheme()->select_dataset(12584);
nuc.get_level_scheme()->print("level");
nuc.get_level_scheme()->select_dataset(12588);
nuc.get_level_scheme()->print("level");
```
```shell
[ INFO     ] dataset 'COULOMB EXCITATION' contains 2 levels:
[ INFO     ] Level energy = 0      keV [no uncertainty] ; Jpi: 0+
[ INFO     ] Level energy = 4040   keV [no uncertainty] ; Jpi: 2+

[ INFO     ] dataset 'COULOMB EXCITATION:XUNDL-4' contains 4 levels:
[ INFO     ] Level energy = 0      keV [no uncertainty] ; Jpi: 0+
[ INFO     ] Level energy = 4041.2 keV [no uncertainty] ; Jpi: 2+   ; lifetime = 2.95    (-0.47  ; +0.70 ) fs
[ INFO     ] Level energy = 4351.9 keV [no uncertainty] ; Jpi: 3-   ; lifetime = 2.4     (-0.6   ; +1.3  ) ps
[ INFO     ] Level energy = 4416   keV [no uncertainty] ; Jpi: 4+   ; lifetime = 3.95    (0.13 ) ns uncertain level tag: S
```
