/** @page list_BE2_WU list_BE2_WU.C

## Explore B(E2) values

This example macro prints all known B(E2) in Weisskopf unit from ADOPTED and XUNDL datasets.

Start `tkn-root`, compile and execute this macro :
```
tkn-root
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db


tkn [0] .L list_BE2_WU.C+
tkn [1] list_BE2_WU()
10Be   [ENSDF] 3.4e+03        E2    2+   (3368.0) --> 0+ (0.0)  BE2W = 8.00    (0.76 ) Weisskopf
10Be   [XUNDL] 3370.00        E2    2+   (3370.0) --> 0+ (0.0)  BE2W = 0.000718848 (0.000023441) Weisskopf [converted]
12Be   [ENSDF] 2109.00        E2    2+   (2109.0) --> 0+ (0.0)  BE2W = 8.5     (1.7  ) Weisskopf
10C    [ENSDF] 3353.60        E2    2+   (3353.7) --> 0+ (0.0)  BE2W = 9.5     (1.5  ) Weisskopf
10C    [XUNDL] 3354.00        E2    2+   (3354.0) --> 0+ (0.0)  BE2W = 0.000687593 (0.000023441) Weisskopf [converted]
12C    [ENSDF] 4438.94        E2    2+   (4439.8) --> 0+ (0.0)  BE2W = 4.65    (0.26 ) Weisskopf
14C    [ENSDF] 7010.00        [E2]  2+   (7012.0) --> 0+ (0.0)  BE2W = 1.8     (0.3  ) Weisskopf
16C    [XUNDL] 1766.00        E2    2+   (1766.0) --> 0+ (0.0)  BE2W = 1.08    (0.30 ) Weisskopf
16C    [XUNDL] 1758.00        [E2]  2+   (1758.0) --> 0+ (0.0)  BE2W = 1.73    (0.30 ) Weisskopf
20C    [ENSDF] 1618.00        E2    2+   (1618.0) --> 0+ (0.0)  BE2W = 0.000232561 (-0.000062016 ; +0.000099226) Weisskopf [converted]
16O    [ENSDF] 6915.50        [E2]  2+   (6917.1) --> 0+ (0.0)  BE2W = 3.1     (0.1  ) Weisskopf
18O    [ENSDF] 1982.00        E2    2+   (1982.1) --> 0+ (0.0)  BE2W = 3.32    (0.09 ) Weisskopf
20O    [ENSDF] 1673.60        [E2]  2+   (1673.7) --> 0+ (0.0)  BE2W = 1.80    (0.07 ) Weisskopf
22O    [ENSDF] 3199.00        E2    2+   (3199.0) --> 0+ (0.0)  BE2W = 1.2     (0.5  ) Weisskopf
18Ne   [ENSDF] 1887.00        E2    2+   (1887.3) --> 0+ (0.0)  BE2W = 17.7    (1.8  ) Weisskopf
20Ne   [ENSDF] 1633.60        [E2]  2+   (1633.7) --> 0+ (0.0)  BE2W = 20.3    (1.0  ) Weisskopf
[...]
```

It also produces the following plot:

![image](images/list_BE2_WU.png "")

## Source code :

\include list_BE2_WU.C
