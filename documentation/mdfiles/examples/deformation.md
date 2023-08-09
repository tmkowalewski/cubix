/** @page deformation deformation.C

## Explore nuclear deformation

This example macro produces the correlation between normalized transition strength B(E2)/A expressed in Weisskopf unit and the ratio between the energy of the first 4$^+$ and 2$^+$ states for all even-even nuclei. 
The color scale and the shape of the points reflect the quadrupole deformation $\beta_2$ of the ground state while the size of each point in proportional to A$^{2/3}$.

Start `tkn-root`, compile and execute this macro :
```
tkn-root
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db


tkn [0] .L deformation.C+
tkn [1] draw_deformation()
```

It produces the following plot:

![image](images/deformation.png "")

## Source code :

\include deformation.C
