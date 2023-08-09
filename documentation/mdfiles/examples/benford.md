/** @page benford benford.C

## Benford's law on lifetime

This example macro tests the benford's law on known excited state lifetime and uncertainty.

The Bendord's law (or first-digit law) states that in many real-life sets of numerical data, the leading digit is likely to be small.
More precisely, the probability that the first digit is equal to n (n=1-9) writes (in base 10) : P(n) = Log<sub>10</sub>(1+1/n). 

This law applies approximately to any set of data covering many order of magnitudes and whose accessible values are not strongly constrained. Under these conditions, 
any strong deviation to the Benford's law can sign a bias in the measurement or a fraud.

Nuclear excited states lifetime are a perfect dataset to test the Benford's law. In this example, we build the first-digit probability distribution P(n) on 
lifetime values (expressed in second and in keV) and lifetime uncertainties (in keV) and compare them to the Benford's law prediction.

Start `tkn-root`, compile and execute this macro :

```
tkn-root
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db

tkn [0] .L benford.C+
tkn [1] benford()
```

It produces the following plots :

![image](images/benford.png "")

## Source code :

\include benford.C
