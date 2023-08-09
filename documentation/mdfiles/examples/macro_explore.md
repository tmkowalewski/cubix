/** @page macro_explore macro_explore.C

## Simple ROOT macro using TkN

`macro_explore.C` provides a simple example ROOT macro using TkN to explore some properties of the Carbon isotopic chain.

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

You can then compile and execute `macro_explore.C`:

```
tkn [0] .L macro_explore.C+
Info in <TUnixSystem::ACLiC>: creating shared library tkn-install/examples/./macro_explore_C.so
tkn [1] macro_explore()
```

## Source code :

\include macro_explore.C
