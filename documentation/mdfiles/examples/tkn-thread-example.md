/** @page tkn-thread-example tkn-thread-example.cpp

## Simple example using TkN with multi-threading

TkN can be used in a multi-threaded program, this example shows such. In this example, a large number of nuclei are randomly produced, and for each nucleus, a state is randomly choosen.
If this example is linked with ROOT, it produces some histograms according to these information.

To optimize the capabilities of the multi-thread, it is possible to pre_load all the level schemes in the memory using the command:

```cpp
 gmanager->preload_level_schemes(true);
```

But be aware that this will only preload the datasets corresponding to "ADOPTED LEVELS" from the ENSDF.

This table summerize the CPU time for processing 10<SUP>8</SUP> events in this example, with and without level schemes preload. The preload time is of around 13s.

<center>
| number of threads  | time with preload (s) | time without preload (s)  |
| :----:             |    :----:             |        :----:             |
| 1                  | 212                   | 225                       |
| 2                  | 108                   | 120                       |
| 4                  | 63                    | 59                        |
| 8                  | 44                    | 42                        |
| 16                 | 30                    | 28                        |
</center>

## user-guide

To compile this example, use:

```shell
g++ tkn-thread-example.cpp -o tkn-thread-example `tkn-config --cflags --linklibs` -lpthread
```
use `tkn-thread-example --help`

```shell
*******************************************
***    tkn-thread-example user-guide    ***
*******************************************

this utility allows to test the multi thread compatibility

supported options:
 --evts N     number of random nuclei to process (default 1e6)
 --workers N  number of workers to be used (default 1)
```

For example, for lauching 10<SUP>8</SUP> events on 4 threads, use:

```shell
./tkn-thread-example --workers 4 --evts 1e8
```

```
***********************************************************************************
* Randomly produce a nucleus and extract a random excited state in multi thread   *
***********************************************************************************

Nevents  : 100000000
N workers: 8
[==========] ... pre-loading
Analysis progress :  99 %[ INFO     ] tkn-thread.root created with content:
TFile**		tkn-thread.root
 TFile*		tkn-thread.root
  KEY: TH2F	NucChart;1	NucChart
  KEY: TH2F	NucChartBetaminus;1	NucChartBetaminus
  KEY: TH2F	NucChartBetaplus;1	NucChartBetaplus
  KEY: TH2F	NucChartStable;1	NucChartStable
  KEY: TH1F	RandExcitedState;1	RandExcitedState

CPU time to load the full db  : 13.7668 s
CPU time to process the events: 29.7347 s
Total CPU time                : 43.5015 s
```

As written, some histograms are stored in the ROOT file tkn-thread.root

## Source code

\include tkn-thread-example.cpp
