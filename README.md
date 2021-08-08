# Polybugger
The goal of this project is to create a functional debugger which can debug C and C++ programs. For now, it is only limited to debugging x86 assembly code.
The debugger makes use of the ptrace() system call in Linux to trace the execution of programs that are being debugged. The project still requires parsing of
the DWARF format which will be implemented in a later version of the debugger.

### Resources
Eli Bendersky: https://github.com/eliben/code-for-blog/tree/master/2011/debuggers_part2_code  
Liz Rice: https://medium.com/@lizrice/a-debugger-from-scratch-part-1-7f55417bc85f  

### Compilation instructions:
- On Linux: ```make```

### Usage
- `objdump -d helloWorldAsm`
- `./debugger.e helloWorldAsm`

### Commands
- break <Address> - Sets a break point at the specified address.
- step - Executes a line of assembly code.
- continue - Continues to the next breakpoint, or completes execution of program if breakpoint not present.
- read <Address> - Reads the value at the address specified.

### Special Instructions
Since this debugger is primitive, you are required to use the machine code you get from running objdump.
There should be a label called <_start> after you execute objdump. Ignore <message> and <message2>.
All the address listed under <_start> are the ones you can use to set breakpoints and read memory. To
step through code, you can just execute the step command.

### Note
This debugger can only debug assembly code.  
