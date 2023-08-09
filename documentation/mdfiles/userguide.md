/** @page user-guide TkN user guide

[TOC]

## Introduction

The TkN library gives access to a large amount of nuclear properties as:

* [Elements and nuclei](#_elements)
* [Nuclear levels](#_levels)
* [Nuclear decays](#_decays)

These properties are taken from different official sources [(see the data sources section)](@ref data) and merged in a sqlite3 home-made database.

In the ENSDF database, used for levels and decay properties, more than one dataset can be accessible for a given nucleus (usually one dataset per reaction). This is taken into account in TkN using the [dataset manager](#_dataset)

To easily browse the database, [a database manager](#_dbmanager) is provided. It allows for example to create list of nuclei on given ranges of neutron or proton numbers, or any other selections.

To manage the different data units, [a units manager](#_units) is also provided to convert the data extracted from the database in the requested units.

Finnally, TkN has been developped in thread safe way to allow a [multi-thread access](#_multithread) to the TkN database.

## Properties {#_properties}

All the following used classes are part of the 'tkn' namespace. If you are using the `tkn-root` executable, the namespace is already loaded, but in a c++ macro or project, use the following for an easier access:

```cpp
using namespace tkn;
```

### Elements and nuclei {#_elements}

\include{doc} nucleus.md

### Nuclear levels scheme {#_level_scheme}

For all the nuclei with known data on the ENSDF or XUNDL databases, it is possible to access to a tkn::tklevel_scheme object.

The level scheme is determined for a given dataset [see the dataset section](#_dataset).

For one dataset, the level scheme gives access to the nuclear levels and decay information [see the level section](#_levels) or [the decay section](#_decays)

### Nuclear levels {#_levels}

\include{doc} levels.md

### Nuclear decays {#_decays}

\include{doc} decay.md

## Data manager {#_manager}

### Datasets {#_dataset}

\include{doc} dataset.md

### Database {#_dbmanager}

\include{doc} dbmanager.md

### Measures and units {#_units}

\include{doc} measure.md

\include{doc} units.md

## Multi-thread compatibility {#_multithread}

The TkN library can be used in a multi-threaded code.

In the examples, you can find an example of a multi-thread use of the TkN library, see the [example](@ref tkn-thread-example)
