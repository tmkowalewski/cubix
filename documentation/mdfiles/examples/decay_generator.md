/** @page decay_generator decay_generator.C

## Simulate a randomly distributed nuclear chart and process its decay along time

This example macro takes all the known nuclei from the nuclear chart, and generates randomly a initial distribution of nuclei that are represented on a tkn::tknuclear_chart.
The user can then simulate a time evolution that will make the nuclei decay corresponding to their main decay mode and lifetime.

This is not a perfect representation of the decay process, but just a simple view. For example, for a time evolution of 1 second, the program actually process 1000 successive decays of 1ms.

Start `tkn-root`, compile and execute this macro :
```
tkn-root
 _____    _   _   |  Documentation: https://tkn.in2p3.fr/
(_   _)  | \ | |  |  Source: https://gitlab.in2p3.fr/tkn/tkn-lib
  | |_  _|  \| |  |
  | | |/ /     |  |  Version 1.0
  | |   <| |\  |  |
  |_|_|\_\_| \_|  |  Database: TkN_ensdf_221101_xundl_210701_v1.0.db


tkn [0] .L decay_generator.C++O
tkn [1] init(1e6)
tkn [2] elapse_time_in_seconds(1)
```

a fonction is already prepared to simulate the earth age time evolution using adapted time scales to have a global overview of the different decay steps.

```
tkn [0] .L decay_generator.C++O
tkn [1] decay_generator_animation()
```

It produces the following animated gif:

![image](images/decay_generator.gif "")

## Source code :

\include decay_generator.C
