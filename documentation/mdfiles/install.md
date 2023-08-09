/** @page install Download and Install

## Prerequisites

### Operating systems

Linux (Ubuntu, Scientific Linux, CentOS, Debian, ...) and MacOS X operating systems are supported. No support for Windows yet.

### Basic software

You will need :

* [git](http://git-scm.com/) to download the sources

* [cmake](https://cmake.org/) to configure the build (at least version 3.5) 

* a compiler supporting [C++17](https://en.cppreference.com/w/cpp/compiler_support/17)

* if linked with [ROOT](https://root.cern/), a ROOT version at least version 6.20

## Getting the sources

TkN is available to download and build from source from our [GitLab repository](https://gitlab.in2p3.fr/tkn/tkn-lib).

Lets suppose you plan to install TkN is the following repository:

```shell
cd /path/to/mysofts/TkN
```

The first time you download the source code, you need to *clone* the repository by typing the following command:

```shell
git clone https://gitlab.in2p3.fr/tkn/tkn-lib tkn-sources
```

This will create a directory called **tkn-sources** containing the source code.
By default, the version of TkN in this directory will correspond to the last version of the master branch.

To list the available tkn versions, use:

```shell
cd tkn-sources
git tag
```

The release specific tag can be obtained using:

```shell
git checkout -b v1.1 v1.1
```

## Build and install

### cmake configuration

In the folder TkN (see above section), create the build and install directories:

```shell
mkdir tkn-build tkn-install
cd tkn-build
cmake -DCMAKE_INSTALL_PREFIX=../tkn-install ../tkn-sources
```

That should end with something similar to:

```
...
--   + tksqlite3
-- Looking for ROOT:
-- ROOT Found in /Applications/root_v6.26.04
-- ROOT Version: 6.26.04 
--   + base
--   + builder
-- Building exec:
tkn-print
tkn-create
tkn-ensdf-update
tkn-db-builder
-- Configuring done
-- Generating done
-- Build files have been written to: TkN/test/tkn-build
```

### Advanced options

The following options can be given at the cmake configuration level (with default values):

```cmake
option(USE_ROOT       "compile TkN using ROOT (if installed)"   ON)
option(DB_DOWNLOAD    "Download the SQL database at install"    OFF)
option(USE_CXX11      "Compile TkN using C++11"                 OFF)
```

For example, to not link TkN with ROOT, and install the sqlite database at the installation step, use:

```bash
cmake -DCMAKE_INSTALL_PREFIX=../tkn-install -DUSE_ROOT=OFF -DDB_DOWNLOAD=ON ../tkn-sources
```

### Compile and install

```bash
make -jN install
```

With N being the number of processors available. On a single-core machine, just type `make install`. On a multi-processor machine, the build process can be considerably speeded-up by performing the compilation in parallel. For example, on a quad-core machine, type `make -j4 install` etc...

### Possible compilation issues

In case you compiler does not support c++17, or if ROOT has been compiled with c++11, a patch can be applied to compile TkN with C++11:

```bash
cmake -DCMAKE_INSTALL_PREFIX=../tkn-install -DUSE_CXX11=ON ../tkn-sources
```

Be aware that in the future, possible developments could no more be c++11 compatible. We do not assure c++11 backward compatibility

## Setting up the environment

After installing, you can use one of the following scripts to set up the necessary paths on your system to find the executables and run-time libraries. Choose the right script for your shell:

```shell
source /path/to/mysofts/TkN/tkn-install/bin/thistkn.csh      [(t)csh shell]
source /path/to/mysofts/TkN/tkn-install/bin/thistkn.sh       [(ba)sh shell] 
```

After doing this you should be able to do e.g.:

```
tkn-config --version
1.0
```

## Downloading the database

The `tkn-db-update` executable allows you to download or update the sqlite database:

```shell
tkn-db-update --download
--2022-08-10 10:36:21--  https://projets.ip2i.in2p3.fr/TkN/TkN_ensdf_220701_xundl_210701_v1.0.db
Résolution de projets.ip2i.in2p3.fr (projets.ip2i.in2p3.fr)… 134.158.81.213
Connexion à projets.ip2i.in2p3.fr (projets.ip2i.in2p3.fr)|134.158.81.213|:443… connecté.
requête HTTP transmise, en attente de la réponse… 200 OK
Taille : 171507712 (164M) [application/octet-stream]
Enregistre : ‘tkn-install/databases/TkN_ensdf_220701_xundl_210701_v1.0.db’

TkN_ensdf_220701_xundl_210701_v1.0.db     100%[====================================================================================>] 163,56M  3,53MB/s    ds 44s     

2022-08-10 10:37:06 (3,72 MB/s) - ‘tkn-install/databases/TkN_ensdf_220701_xundl_210701_v1.0.db’ enregistré [171507712/171507712]

[ INFO     ] TkN_ensdf_220701_xundl_210701_v1.0.db installed in tkn-install/databases/
```

This step is not needed if the database has been downloaded during the install process.
The database is updated twice a year, following the ENSDF updates.

## Interactive session

If TkN has been compiled linked with root, you should be able to do:

```
tkn-root 
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db
```

If the database has not been downloaded at the `cmake` step using the option `-DCMAKE_DB_DOWNLOAD` or using `tkn-db-update`, 
you will obtain the following message:

```
tkn-root 
[ ERROR    ] database::open(/Users/dudouet/Softs/TkN/test/tkn-install/databases/TkN.db,OPEN)
             can't open database: 'unable to open database file'
[ INFO     ] use tkn-db-update to download the database
```

In this case, as indicated, you need tu use the `tkn-db-update` program to download the database.
