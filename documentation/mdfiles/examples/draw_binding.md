/** @page draw_binding draw_binding.C

## Explore binding energies

This example macro fills the nuclear chart with binding energy per nucleon.

Start `tkn-root`, compile and execute this macro :
```
tkn-root
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db


tkn [0] .L draw_binding.C+
tkn [1] draw_binding()
```

It produces the following plot:

![image](images/draw_binding.png "")

## Source code :

\include draw_binding.C
