# Debugger
The debugger provides an interface that is similar although much simpler than that provided by GDB.
The following is the help message provided by the debugger which explains all of its functionality.
```
List of commands:

file <path/to/binary> -- Loads a program into the IJVM.
input <file> -- Sets the IJVM standard input to the specified file.
break <offset> -- Sets a breakpoint at a given offset from the first instruction (decimal or hexadecimal with the '0x' prefix).
start -- Starts running the program.
step -- Executes the next instruction.
run -- Re-start the program and run it until a breakpoint is reached or until it is finished.
continue -- Executes program and stops at the first breakpoint.
info frame -- Shows contents of the current frame (operand stack and local variables).
info break -- Shows a list of all active breakpoints.
backtrace -- Shows a call-stack of all frames including arguments.

* Note that running "file <path/to/binary>" when a program is running,
will terminate the current program and load the new one into a fresh VM.
* Frame information will be printed out in the following format: "[bottom ... top]".
* Breakpoint information will be printed in the following format: "Breakpoint <id>, in <func_addr> <func_name> ( <arg1>, <arg2>, ... )"
but other information may also be appended to the end.
* It is possible to use a symbol name as a breakpoint address.
The label should be provided in the following format "function#section" or "function".
* Backtrace information will be printed in the following format: "#<id>  <func_addr> <func_name> ( <arg1>, <arg2>, ... )".
```
## Architecture
Below is a diagram representing the major building blocks of the debugger.
```
                  +------+       +----------------------+      +------------+
                  | IJVM |<----->| "Execute Command..." |<---->| Call Stack |
                  +------+       +----------------------+      +------------+
                                         ^   ^
                                         |   |
                                         |   +---------+
                                         |             |
+---------+     +----------+     +-----------------+   |
| Program |---->| Debugger |<--->| Command Decoder |   |
+---------+     +----------+     +-----------------+   |
    |                ^                                 |
    |                |                                 |
    |                v                                 |
    |       +-------------------+                      |
    +------>| Debug Data Loader |<---------------------+
            +-------------------+
```

## Breakpoints
A user can create a breakpoint both at an address (hex or decimal) and at a label loaded by the
```Debug Data Loader```. Each label address is resolved by reading a debug symbol table created when
loading the program and if a match is found, that becomes the breakpoint address. The label string
itself is discarded and every time a user requests that a list of breakpoints is printed, labels are
re-fetched from the symbol table.

## Backtrace
Every time that an ```INVOKEVIRTUAL``` or ```IRETURN``` instruction occurs, the debugger will store
or remove (respectively) a function call from its internally held ```Call Stack```. Each entry in
the call stack contains a function address and values of all arguments from back when the function
was called. When the user requests a backtrace, it is simply a matter of iterating over the entire
call stack and formatting the data into a human-readable format. 
