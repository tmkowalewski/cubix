/** @page install Download and Install

## Prerequisites

### Operating systems

Linux (Ubuntu, Scientific Linux, CentOS, Debian, ...) and MacOS X operating systems are supported. No support for Windows.

### Basic software

You will need :

* [git](http://git-scm.com/) to download the sources

* [cmake](https://cmake.org/) to configure the build (at least version 3.5) 

* a compiler supporting [C++17](https://en.cppreference.com/w/cpp/compiler_support/17)

* a [ROOT](https://root.cern/), version at least version 6.20


### TkN link

Cubix is linked with the [TkN library](https://tkn.in2p3.fr/). Two options are proposed:
1. TkN is already installed and it can be linked with cubix (at least version 1.2) 
2. TkN is not installed and can be built directly with the Cubix package (as a git sub-module)

## Getting the sources

Cubix is available to download and build from source from our [GitLab repository](https://gitlab.in2p3.fr/ip2i_gamma/cubix/cubix).

Lets suppose you plan to install TkN is the following repository:

```shell
cd /path/to/mysofts/Cubix
```

The first time you download the source code, you need to *clone* the repository by typing the following command:

```shell
git clone https://gitlab.in2p3.fr/ip2i_gamma/cubix/cubix cubix-sources
```

This will create a directory called **cubix-sources** containing the source code.
By default, the version of Cubix in this directory will correspond to the last version of the master branch.

To list the available tkn versions, use:

```shell
cd cubix-sources
git tag
```

The release specific tag can be obtained using:

```shell
git checkout -b v1.0 v1.0
```

## Build and install

### cmake configuration

In the folder Cubix (see above section), create the build and install directories:

```shell
mkdir cubix-build cubix-install
cd cubix-build
```

* Option 1: TkN is already installed (and loaded in your system environment)

```shell
cmake -DCMAKE_INSTALL_PREFIX=../cubix-install ../cubix-sources
```

That should end with something similar to:

```
...
Cubix compiled with c++11 standards
Cubix compiled in Release mode !
-- ROOT Found in /Applications/root_v6.26.04
-- ROOT Version: 6.26.04
Looking for TkN:
TkN found in: /Applications/TkN/tkn-lib/install
TkN version:  1.2
Loading libraries
 -> CubixCore
 -> CubixTools
 -> CubixRadwareInterface
 -> CubixGui
Building Cubix executable
 -> cubix
Cubix Configuration
-- Configuring done
-- Generating done
-- Build files have been written to: /Applications/Cubix/build
```

* Option 2: TkN is built directly with the Cubix package

```shell
cmake -DCMAKE_INSTALL_PREFIX=../cubix-install -DBUILTIN_TKN=On ../cubix-sources
```

That should end with something similar to:

```
...
Cubix compiled with c++11 standards
Cubix compiled in Release mode !
-- ROOT Found in /Applications/root_v6.26.04
-- ROOT Version: 6.26.04
TkN compiled with c++17
TkN compiled in Release mode !
Looking for ROOT:
-- ROOT Found in /Applications/root_v6.26.04
-- ROOT Version: 6.26.04
Looking for SQlite:
-- tksqlite3 loaded
Loading libraries
 -> tkbase
 -> tkdb
 -> tkphysics
 -> tkroot
Building users exec:
 -> tkn-print
 -> tkn-db-update
 ROOT execs -> tkn-root
TkN Configuration
TkN database will be downloaded at install step
Loading libraries
 -> CubixCore
 -> CubixTools
 -> CubixRadwareInterface
 -> CubixGui
Building Cubix executable
 -> cubix
Cubix Configuration
-- Configuring done
-- Generating done
-- Build files have been written to: /Applications/Cubix/build
```

### Compile and install

```bash
make -jN install
```

With N being the number of processors available. On a single-core machine, just type `make install`. On a multi-processor machine, the build process can be considerably speeded-up by performing the compilation in parallel. For example, on a quad-core machine, type `make -j4 install` etc...

## Setting up the environment

After installing, you can use one of the following scripts to set up the necessary paths on your system to find the executables and run-time libraries. Choose the right script for your shell:

```shell
source /path/to/mysofts/TkN/tkn-install/bin/thiscubix.csh      [(t)csh shell]
source /path/to/mysofts/TkN/tkn-install/bin/thiscubix.sh       [(ba)sh shell] 
```

After doing this you should be able to do e.g.:

```
cubix-config --version
1.0
```

Launching the cubix program should give the following output

```
cubix
   ______        __     _       | Documentation: https://cubix.in2p3.fr/
  / ____/__  __ / /_   (_)_  __ |
 / /    / / / // __ \ / /| |/_/ | Source: https://gitlab.in2p3.fr/ip2i_gamma/cubix/cubix   
/ /___ / /_/ // /_/ // /_>  <   |
\____/ \____//_____//_//_/|_|   | Version 1.0

[==        ] Loading tkn db...
```

The loading of the TkN database is launched in a separate thread. It is necessary to wait its full loading to use tools that are intereacting with nuclear databases (like the ENSDF reader or the nuclear chart). On standard computers, with a compilation not using debug, it should takes around 10-15 seconds.
