# Integer Java Virtual Machine
The IJVM instruction set architecture (ISA) is a subset of the Java ISA (JVM), which operates only
on a single type of data, namely 32-bit integers. Contained in this project is also a debugger
called IJDB (Integer Java Debugger) which uses the IJVM virtual machine (or IJVM for short) to help
debug IJVM programs.

## Memory Architecture
There are several different places to store data by a running program:
- **Stack**: Used for storing temporary operands and stack frames when a function is called.
- **Code memory**: Holds the instructions which make up a program as well as arguments for some
  instructions (more on this in the next section). This memory is write-protected.
- **Constant memory**: Every program can define constants which can be accessed from anywhere in the
  program but which are stored in write-protected memory.
- **Arrays**: These provide a persistent read and write memory for use across different functions.
  The program may request that an array is allocated and the stack stores a reference to this array
  called the "array reference" which is used to read and write from the array.
- **Network**: It's not memory but the VM allows a program to create sockets that receive/send data
  over the network. The stack stores a reference to each connection which is called the "network
  reference" which is used to uniquely identify connections.

## ISA
IJVM instructions are variable-length although the op-code is always stored in a single byte.
Running programs on the VM is done as follows:
1. Compile IJVM assembler (or compatible Java ASembler (JAS)) programs down to an IJVM bytecode
   using e.g. goJASM.
2. Pass the IJVM bytecode into the VM.

A holistic description of the implemented IJVM ISA can be found in ```docs/isa.md```.

## How it works
Unlike Intel's x86 ISA which uses a register machine, IJVM and JVM both use a stack machine which
means that temporary data is stored in a first-in last-out (FILO) data structure; the only exception
are arrays which are allocated on the heap. The simplified architecture of this IJVM implementation
is as follows:
```
                                                       +------------------------------------------+
                               +-----------+           |                                          |
                     +-------->|   Code    |---+       v                                          |
                     |         +-----------+   |    +-----+     +-------+                         |
                     |                         +--->| CPU |<--->| Stack |                         |
                 +--------+    +-----------+   |    +-----+     +-------+                         |
                 | Loader |--->| Constants |---+       ^                                          |
                 +--------+    +-----------+           |         +-------+-----------+            |
                     ^                                 |         |       |           |            |
                     |                                 v         |       v           v            |
 +---------+     +-------------+                +-------------+  |  +---------+  +--------+       |
 | Program |---->| Initializer |--------------->| Interpreter |  |  | Network |  | Arrays |       |
 +---------+     +-------------+                +-------------+  |  +---------+  +--------+       |
                                                       |         |                   ^            |
                                                       |         |                   |            |
                                                       v         v                   v            |
                                          +----------------------------+   +-------------------+  |
                                          | "Execute Instruction 0x??" |-->| Garbage Collector |  |
                                          +----------------------------+   +-------------------+  |
                                                       ^                                          |
                                                       |                                          |
                                                       +------------------------------------------+
```
The IJDB adds two more components namely the `Debug Data Loader` which loads debug symbols from the
program binaries, and the `Debugger` itself which decodes and executes commands provided by the user
and takes care of tracking function calls, breakpoints, and other debugging information. 
