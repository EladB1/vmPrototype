This repo is a prototype of a VM for the backend of a compiler/interpreter

### Byte code

Whitespace does not matter, but it's a good idea to keep code organized with new lines and proper indentation.

Any lines starting with a semicolon are ignored.

Any line that does not start with a dot but ends with a colon, is considered a function definition. When the function is called, the location of the function will be found, the program counter (pc) will move to the start of the function, and a new frame will be loaded onto the call stack. Each frame contains its own stack, local variables, jump points, pc, and return address (pc of the caller). All frames share global variables, the call stack, and the frame pointer (fp). All functions except for the program entry point must end with a return; if they're void, they should return `None` (not the same as `null`). When a function returns, it's popped off the call stack, the fp is decremented, and the program returns to the return address of that frame. If the function is not void, the top of the stack will be popped and pushed onto the caller's stack. 

Builtin functions do not create a frame or do most of the steps listed above. Instead, they just run C code and return a value which is pushed onto the stack (if not void).

Any line that starts with a dot and ends with a colon will be used as a jump point. Jump points are used by jump instructions (`JMP`, `JMPT`, `JMPF`, `SJMPT`, `SJMPF`) to move the pc non-sequentially. All jumps must be explicitly defined otherwise, the pc will move to the next instruction it sees. The VM ignores jump labels and move to the next instruction right away.

### Exit codes

- 0 - Successful execution
- 1 - Improper operation (division by zero)
- 2 - Improper memory access (array bounds error)
- 3 - File operation error
- 254 - Unknown bytecode 
- 255 - Error running VM

> Any other exit codes will come from C functions or C itself