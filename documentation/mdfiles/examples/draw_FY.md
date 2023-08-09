/** @page draw_FY draw_FY.C

## Explore Fission Yields

This example macro fills the nuclear chart with fission yields.

Start `tkn-root`, compile and execute this macro :
```
tkn-root
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db

tkn [0] .L draw_FY.C+
tkn [1] draw_FY252Cf()
tkn [2] draw_FY239Pu()
```

It produces the following plot for <sup>252</sup>Cf:

![image](images/draw_FY252Cf.png "")

and the following plot for <sup>239</sup>Pu:

![image](images/draw_FY239Pu.png "")

## Source code :

\include draw_FY.C
