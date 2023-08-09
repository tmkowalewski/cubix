/** @page programs List of TkN programs

## tkn-config
```
tkn-config 
Usage: tkn-config [--version] [--gitinfos]  [--bindir] [--libdir] [--incdir] [--srcdir] [--builddir]
                  [--examples] [--dbdir] [--dbfile] [--libs] [--cflags] [--linklibs] [--help]
```

+ `--version` TkN version number 
+ `--gitinfos` info on git repository
+ `--bindir` directory containing binary files
+ `--libdir` directory containing shared libraries
+ `--incdir` directory containing include files
+ `--srcdir` source directory
+ `--builddir` build directory
+ `--examples` directory containing examples
+ `--dbdir` directory containing the sqlite database
+ `--dbfile` name of the current database file
+ `--libs` list of TkN libraries
+ `--cflags` include files
+ `--linklibs` lib flags for compilation
+ `--help` print help message

## tkn-print

Prints level or decay data for a given nucleus.

```
tkn-print
 ***********************************
 ***    tkn-print  user-guide    ***
 ***********************************

this utility allows to print nuclear properties:

required argument:
[SYMBOL]         select the nucleus
optional arguments:
 --levels [-l]         print the list of nuclear levels
 --decays [-d]         print the list of nuclear decays
 --with-dataset id     print data for the selected dataset (use show-datasets to have the list of available datasets)
display arguments:
 --table [-t]    print as a table (not available when levels and decay are on)
 --comments [-c] print comments
 --show-datasets [-s] show the available datasets for this nucleus

examples of use:
tkn-print 4He -lc
tkn-print 12C -dtc
```


## tkn-root

Launch the interactive C++ interpreter of ROOT with all the basic libraries of the toolkit already linked 
in and all necessary paths required for compilation of your code predefined.

```
tkn-root
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db

tkn [0]
```

## tkn-db-update

Update or download the latest database available for the installed TkN version.

```
tkn-db-update 
 **************************************
 ***    tkn-db-update user-guide    ***
 **************************************

this utility allows to download the TkN nuclear database

supported options:
 --show         to show the available databases corresponding to the current TkN version
 --file name    to select the name of the database to download (default is the latest)
 --download     to download the database
 --help         to print this help
 ```

<!--## tkn-ensdf-update

Compiled only if `USE_DEV_MODE = ON`.

`tkn-ensdf-update`

## tkn-create
`tkn-create`

## tkn-explore
`tkn-explore`-->
