This repo is a prototype of a VM for the backend of a compiler/interpreter

### Vitrual Machine Internals

This is a stack based VM which takes some inspiration from the JVM and CPython VM. It is written in C and designed to be the backend for [this compiler](https://github.com/EladB1/boltc).

> May eventually merge this repo into the compiler repo

The VM is made up of the following parts:
- **Source Code:** The source byte code you read in from a file
- **Call Stack:** A stack made up of frames.
    - **Frame:** Represents the execution of a function. When a non-builtin function is called, load a new frame.
        - **Instructions:** byte code for the currently running function
        - **Program Counter (pc):** The index of the current instruction being executed
        - **Return Address:** pc of the caller (previous frame)
        - **Jumps:** Index of labels used to make jumps/branches/gotos more convenient
        - **Jump Counter (jc):**  Index of the latest jump label that's defined
        - **Locals:** Store paramters and local variables here
        - **Local Counter (lc):** Index of the last local stored; `-1` if empty
        - **Stack:** Execution stack for the frame
        - **Stack Pointer (sp):** Index of the top of the stack; `-1` if empty
- **Frame pointer (fp):** Index of top of the call stack
- **Globals:** Array containing global variables
  - It also is treated as heap memory to store array literals
    - Arrays are pointers to first element in array literal
    - There isn't a memory management strategy for the heap yet but will work on that
- **Global Counter (gc):** Index of the last element stored in globals; `-1` if empty

### Funtion calls and returns

As stated above, when a non-built in function is called, a new frame is loaded and the pc of the current frame is saved as the return address of that frame. 

Functions are called as `CALL [function name] [argc]` where `[function name]` is replaced by a function name and `[argc]` is replaced by the number of parameters your function takes (both without square brakets). The number of arguments tells the VM how many values to pop off the stack; these popped values will be treated as the function parameters. Since a stack is "Last in first out", you must push your parameters in reverse order. The parameters are added as local variables of the new frame.

When a function returns, the top of the stack is popped, the frame is popped off the call stack, and the PC is set to the return address of the frame. The return value is pushed onto the top of the stack, unless it's of type `None` (indicating void return).

Built-in functions are somewhat similar in behavior. The only difference is they don't create a frame; instead they pop parameters, run some C code, and push the return value onto the stack (if not `None`).

Example:

This byte code

```
plus_one:
    LOAD 0
    LOAD_CONST 1
    ADD
    RET

caller:
    LOAD_CONST 1
    CALL plus_one 1
    CALL println 1
    HALT
```

is equivalent to this source code

```
    fn plus_one(int num): int {
        return num + 1;
    }

    fn caller() {
        println(plus_one(1));
        exit();
    }
```

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