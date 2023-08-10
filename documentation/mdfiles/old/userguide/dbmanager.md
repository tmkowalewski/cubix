The [TkN data manager](@ref tkn::tkmanager) is key part of the TkN library. It reads the TkN sqlite3 database, build the C++ structures in memory and make the links between these structures and any object created by the user. This way, the level scheme of a nucleus is only built once, even if the user create the corresponding nucleus millions times.

Is also allows the user to easily browse the full database via the use of the `gmanager` singleton. For example, we can loop on all the existing nuclei using:

```cpp
#include "tkmanager.h"
for(const auto &nuc : gmanager->get_nuclei()) {
...
}
```

As seen previously with the levels and decays, we can also use a lambda expression to filter the list of nuclei, e.g. the carbon isotopes:
```cpp
#include "tkmanager.h"
for(const auto &nuc : gmanager->get_nuclei( [](auto nuc) {
    return (nuc->get_z()==6 && !nuc->get_level_scheme()->get_levels().empty());
    })) {
cout << nuc->get_symbol() << " ";
} cout << endl;
```
```shell
8C 9C 10C 11C 12C 13C 14C 15C 16C 17C 18C 19C 20C 22C
```

Some tools are also provided to easily loop on the Z or N known ranges:

```cpp
#include "tkmanager.h"
for(const auto &znucs : gmanager->get_map_of_nuclei_per_z()) {
    cout<<znucs.first<<" ";
}
```

```shell
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118
```

```cpp
#include "tkmanager.h"
auto CarbonIsotopes = gmanager->get_map_of_nuclei_per_z().at(6);
for(auto &nuc : CarbonIsotopes) {
    cout<<nuc->get_symbol()<<" ";
}
```

```shell
8C 9C 10C 11C 12C 13C 14C 15C 16C 17C 18C 19C 20C 21C 22C 23C
```
