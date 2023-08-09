/** @page data Data sources

## Elements

The JSON file containing most of the information on chemical elements is downloaded from the 
[PubChem](https://pubchem.ncbi.nlm.nih.gov) website :
https://pubchem.ncbi.nlm.nih.gov/rest/pug/periodictable/JSON/?response_type=save&response_basename=PubChemElements_all

Units were added manually and column names have been modified.

You can access their interactive periodic table and find all relevent references here : https://pubchem.ncbi.nlm.nih.gov/periodic-table/#view=table.

### X-Ray data

X-ray data are extracted from the [X-RAY DATA BOOKLET](https://xdb.lbl.gov): 
https://xdb.lbl.gov/Section1/Table_1-2.pdf and then converted in JSON format.

References : 
+ J. A. Bearden, “X-Ray Wavelengths,” Rev. Mod. Phys. 39, 78 (1967).
+ M. O. Krause and J. H. Oliver, “Natural Widths of Atomic K and L Levels, Ka X-Ray Lines and Several KLL Auger Lines,” J. Phys. Chem. Ref. Data 8, 329 (1979).

## Isotopes

Most information on isotopes are extracted from [NUDAT3](https://www.nndc.bnl.gov/nudat3/) export: 
https://www.nndc.bnl.gov/nudat3/data/output.json.

## Levels and decays

Excited states and decay information are extracted from [ENSDF](https://www.nndc.bnl.gov/ensdf/) 
and [XUNDL](https://www.nndc.bnl.gov/ensdf/xundl/) databases.

Contributions to [ENSDF](https://www.nndc.bnl.gov/ensdf/) are made by members of [NSDD](https://www-nds.iaea.org/nsdd/), the international network of Nuclear Structure and Decay Data evaluators.

## Data update

The TkN database is automatically updated every month (the 15th) to take into account every new data releases from NUDAT3 and ENSDF/XUNDL
