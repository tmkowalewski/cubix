/** @page link Use TkN in your project

<!--This folder contains several examples of TkN use/integration.-->

## ROOT macros using TkN

Type `tkn-root` to launch the interactive C++ interpreter of ROOT with all the basic libraries of the toolkit already linked in and all necessary 
paths required for compilation of your code predefined:

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

You can then compile any code written using the TkN toolkit in the command line 
of a `tkn-root` interactive session, just like you would with ROOT. An example is given in [macro_explore.C](@ref macro_explore):

```
tkn [0] .L macro_explore.C+
Info in <TUnixSystem::ACLiC>: creating shared library tkn-install/examples/./macro_explore_C.so
tkn [1] macro_explore()
*******************************************
* Excited states on Carbon isotopic chain *
*******************************************
* Ground state spin parity                *
*-----------------------------------------*
8C   0+
9C   3/2-
10C  0+
11C  3/2-
12C  0+
13C  1/2-
14C  0+
15C  1/2+
16C  0+
17C  3/2+
18C  0+
19C  (1/2+)
20C  0+
21C  
22C  0+
23C  
*-----------------------------------------*
* Energy of second 0+ state               *
*-----------------------------------------*
8C   unkown
10C  unkown
12C  Level energy = 7654.07 (0.19 ) keV
14C  Level energy = 6589.4  (0.2  ) keV
16C  unkown
18C  unkown
20C  unkown
22C  unkown
*-----------------------------------------*
* All positive parity states              *
*-----------------------------------------*
8C   0+   
9C   5/2+ 
10C  0+   2+   
11C  1/2+ 5/2+ 3/2+ 3/2+ 7/2+ 5/2+ 5/2+ 7/2+ 9/2+ 
12C  0+   2+   0+   2+   0+   1+   4+   4+   1+   2+   0+   2+   1+   2+   2+   2+   0+   
13C  1/2+ 5/2+ 5/2+ 3/2+ 3/2+ 9/2+ 1/2+ 7/2+ 5/2+ 3/2+ 1/2+ 9/2+ 
14C  0+   0+   2+   2+   0+   2+   >= 1 4+   1+   
15C  1/2+ 5/2+ 3/2+ 
16C  0+   2+   4+   (2+,3-,4+)
17C  3/2+ 1/2+ 5/2+ 7/2+ 9/2+ 
18C  0+   2+   
19C  5/2+ 
20C  0+   2+   
21C  
22C  0+   
23C  
```

## Compile a TkN executable

If you need to compile code using the toolkit in stand-alone mode, you can use the `tkn-config` tool in order to obtain the necessary installation-dependent paths to header files and libraries. 
An example is given in [tkn-exec.cpp](@ref tkn-exec):

\include tkn-exec.cpp

This program can be compiled and linked into an executable using the following command:

```shell
g++ tkn-exec.cpp -o tkn-exec `tkn-config --cflags --linklibs`
```

Note that the above commands include also the required paths and libraries of ROOT, if ROOT has been linked to the TkN installation.

If the TkN database has been previously downloaded, you should then be able to execute `tkn-exec` with the following output:

```
./tkn-exec 
**************************************************
* Ground state spin parity in C isotopic chain   *
**************************************************

8C   0+
9C   3/2-
10C  0+
11C  3/2-
12C  0+
13C  1/2-
14C  0+
15C  1/2+
16C  0+
17C  3/2+
18C  0+
19C  (1/2+)
20C  0+
21C  
22C  0+
23C 
```

## Using TkN in a CMake project

The above method is unwieldy for anything more than a simple executable. 
It cannot be used to make shared libraries containing your own classes, 
for which you would also need to generate the associated ROOT dictionaries, etc... 
Therefore we provide modules which enable TkN to be used with the [CMake](https://cmake.org/) build system.

An example of simple CMake project using TkN is given [here](https://gitlab.in2p3.fr/tkn/tkn-cmake-link-example)

It allows to compile two programs, with and without ROOT link (needs that TkN has been compiled with ROOT for the former)

The important lines to be added in your cmake are the following:

``` cmake
# to link the tkn sqlite library used to access the database
find_package(tksqlite3 REQUIRED)

# to link the tkn libraries
find_package(TkN REQUIRED)

# if compiled with ROOT, to link with ROOT libraries
find_package(ROOT 6.20 CONFIG)

# To define the tkn classes include files
include_directories(${TKN_INCLUDE_DIRS})
```

from this project example, the configuration of the build then proceeds like this (note that we strongly discourage in-source builds, the following is just a simplified example):

```shell
cmake .
-- The CXX compiler identification is AppleClang 13.1.6.13160021
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /Library/Developer/CommandLineTools/usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
TkN found in: /Users/dudouet/Softs/TkN/test/tkn-install
TkN version:  1.0
Looking for ROOT:
-- ROOT Found in /Applications/root_v6.26.04
-- ROOT Version: 6.26.04 
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/dudouet/Softs/TkN/test/tkn-install/examples/tkn-cmake-link-example
```

After which your code can be compiled like so:

```shell
make
[ 25%] Building CXX object CMakeFiles/my_prog.dir/src/my_prog.cpp.o
[ 50%] Linking CXX executable my_prog
[ 50%] Built target my_prog
[ 75%] Building CXX object CMakeFiles/my_prog_ROOT.dir/src/my_prog_ROOT.cpp.o
[100%] Linking CXX executable my_prog_ROOT
[100%] Built target my_prog_ROOT
```

The `my_prog` executable will then print this output:

```
./my_prog
**************************************************
* Ground state spin parity in C isotopic chain   *
**************************************************

8C   0+
9C   3/2-
10C  0+
11C  3/2-
12C  0+
13C  1/2-
14C  0+
15C  1/2+
16C  0+
17C  3/2+
18C  0+
19C  (1/2+)
20C  0+
21C  
22C  0+
23C 
```

The `my_prog_ROOT` executable (if TkN is linked with ROOT), will give this output:

```
./my_prog_ROOT 
**************************************************
* Plotting the energies of the first 2+ state    *
**************************************************

[==========] Reading the database 
[ INFO     ] First 2+ states ploted in "my_prog_ROOT.root"
```

It produces a ROOT files named `my_prog_ROOT.root` containing a 2D histogram representing the first 2+ states energies.

## Using TkN as a git submodule

A third possibility for linking the TkN library with an external project is to use it directly as a git [submodule](https://git-scm.com/docs/git-submodule).

The git project: [TkN-submod-example](https://gitlab.in2p3.fr/tkn/tkn-submod-example) gives an example of how to add TkN as a submodule in an external project.

To add TkN as a submodule, use the following command in your project:

```shell
git submodule add https://gitlab.in2p3.fr/tkn/tkn-lib.git
```

This will create in your repository the *tkn-lib* folder. Then, you need to add the following lines in your *CMakeLists.txt*, as done in the [TkN-submod-example](https://gitlab.in2p3.fr/tkn/tkn-submod-example):

```cmake
SET(DB_DOWNLOAD ON CACHE BOOL "Download the TkN database at install")
add_subdirectory(tkn-lib)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/tkn-lib)
if(USE_ROOT)
    find_package(ROOT 6.20 CONFIG)
endif()
```

Once your package will have been compiled and install, you will need to source the file: *install/bin/thistkn.sh* to load the TkN environment and gives access your code to the TkN database.
