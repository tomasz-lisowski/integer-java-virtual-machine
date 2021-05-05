# Integer Java Virtual Machine
This project contains both a virtual machine capable of running an extended IJVM ISA as well as a
debugger called IJDB. 

# Documentation
The ```docs``` directory contains most of the technical documentation and in-depth discussion of the
inner workings of the virtual machine and debugger. The rest of this document explains compilation
steps.

# Compiling
Run `build.sh` to build the IJVM and IJDB binaries. `build_afl.sh` builds an instrumented IJVM
binary for use with the american fuzzy lop (AFL) fuzzer. Note that `CC` environmental variable 
must be set to the desired compiler before running the build scripts.

Enable the debug print `dprintf` found in `include/util.h` by setting the `DEBUG` environmental variable.

With the debug flag set, the IJVM will print out human-readable information regarding the executed
instructions and how they affected the state of the virtual machine. Below is an example:
```
[LOAD OK]
[VM START]
OPC: 0    OP: LDC_W         PC:3     SP:1     FP:1     LV:0     NV:1     S[ 2130706433 ]    LV[ 0 ]    AR[ ]    NR[ ]
OPC: 3    OP: LDC_W         PC:6     SP:2     FP:1     LV:0     NV:1     S[ 2130706433 5555 ]    LV[ 0 ]    AR[ ]    NR[ ]
OPC: 6    OP: NETCONNECT    PC:7     SP:1     FP:1     LV:0     NV:1     S[ -872415220 ]    LV[ 0 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 7    OP: DUP           PC:8     SP:2     FP:1     LV:0     NV:1     S[ -872415220 -872415220 ]    LV[ 0 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 8    OP: IFEQ          PC:11    SP:1     FP:1     LV:0     NV:1     S[ -872415220 ]    LV[ 0 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 11   OP: ISTORE        PC:13    SP:0     FP:1     LV:0     NV:1     S[ ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 13   OP: ILOAD         PC:15    SP:1     FP:1     LV:0     NV:1     S[ -872415220 ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 15   OP: NETIN         PC:16    SP:1     FP:1     LV:0     NV:1     S[ 66 ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 16   OP: ILOAD         PC:18    SP:2     FP:1     LV:0     NV:1     S[ 66 -872415220 ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 18   OP: NETIN         PC:19    SP:2     FP:1     LV:0     NV:1     S[ 66 65 ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 19   OP: ILOAD         PC:21    SP:3     FP:1     LV:0     NV:1     S[ 66 65 -872415220 ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 21   OP: NETOUT        PC:22    SP:1     FP:1     LV:0     NV:1     S[ 66 ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 22   OP: ILOAD         PC:24    SP:2     FP:1     LV:0     NV:1     S[ 66 -872415220 ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 24   OP: NETOUT        PC:25    SP:0     FP:1     LV:0     NV:1     S[ ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 25   OP: ILOAD         PC:27    SP:1     FP:1     LV:0     NV:1     S[ -872415220 ]    LV[ -872415220 ]    AR[ ]    NR[ 0xCC00000C ]
OPC: 27   OP: NETCLOSE      PC:28    SP:0     FP:1     LV:0     NV:1     S[ ]    LV[ -872415220 ]    AR[ ]    NR[ ]
OPC: 28   OP: HALT          PC:29    SP:0     FP:1     LV:0     NV:1     S[ ]    LV[ -872415220 ]    AR[ ]    NR[ ]
[HALT FLAG]
[VM STOP]
[DESTROY IJVM]
[Took 0.015625 seconds]
```
From the left to right, first comes the old program counter ('OPC') which is the program counter
before the instruction got executed, next are the op-code ('OP'), current program counter ('PC'),
stack pointer ('SP'), frame pointer ('FP'), local variable base pointer ('LV'), number of local
variables ('NV'), stack contents ('S[...]'), active array references ('AR[...]'), and active network
references ('NR[...]').

# Tools
## Compiler
Compilation requires a C11 compiler like GCC or CLANG, both of which are included in the
`build-essential` package (installed by running `sudo apt install build-essential`). The VM works on
Linux and macOS but Windows users need to install a Linux VM or the Windows subsystem for Linux
([WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10)).

## AFL
The `afl-clang` compiler is required to build an instrumented IJVM binary which is preferred when
fuzzing. A [fuzzer](https://en.wikipedia.org/wiki/Fuzzing) like, e.g. AFL, will try to automatically
find bugs in the program by generating input files based on patterns found in a given corpus then
passing the input through a program to determine if it causes crashes or hangs. The inputs that
crash/hang the program will be saved for later analysis which should help in finding and fixing
bugs.

## Libraries
Compiling the debugger (a.k.a. IJDB) requires the readline library which can be installed as
follows:
1. Download sources from the [GNU website](https://tiswww.case.edu/php/chet/readline/rltop.html).
2. Follow the instructions provided in the `INSTALL` file. An alternative would be to run `sudo apt
   install libreadline-dev` but the installed library will most likely be outdated.

Compiling in general also requires `glibc` which can be installed by running `sudo apt install
build-essential`. 
