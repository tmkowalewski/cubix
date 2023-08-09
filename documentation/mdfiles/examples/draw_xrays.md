/** @page draw_xrays draw_xrays.C

## X-Rays energy plot

This example macro loops on the different elements of the periodic table and plot the different x-rays energy

Start `tkn-root`, compile and execute this macro :
```
tkn-root
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db


tkn [0] .L draw_xrays.C+
tkn [1] draw_xrays()
```

It produces the following plot:

![image](images/draw_xrays.png "")

## Source code :

\include draw_xrays.C
