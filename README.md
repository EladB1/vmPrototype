This repo is a prototype of a VM for the backend of a compiler/interpreter



### Exit codes

- 1 - Improper operation (division by zero)
- 2 - Improper memory access (array bounds error)
- 3 - File operation error
- 254 - Unknown bytecode 
- 255 - Error running VM

> Any other exit codes will come from C functions or C itself