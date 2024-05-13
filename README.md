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
        - **Locals Pointer (lp):** Index of the last local stored; `-1` if empty
        - **Stack:** Execution stack for the frame
        - **Stack Pointer (sp):** Index of the top of the stack; `-1` if empty
- **Frame pointer (fp):** Index of top of the call stack
- **Globals:** Array containing global variables
- **Globals Pointer (gp):** Index of the last element stored in globals; `-1` if empty

### Arrays and memory managment

Arrays are references to a list of values (either some frame's locals or VM globals). When an array is created, the destination for the values is stored, as well as the offset ( value of `lp`) to the destination with the array size (total amount of memory allocated) and length (number of actual elements in the array).

By default, arrays are stored in the local frame. When the frame returns, all values in the locals are deallocated anyway, so the array memory gets cleaned up. Frames that return arrays, copy the memory to the caller frame then get deallocated. When you store an array reference globally, its values get copied to the VM globals, the pointer gets changed to point to the globals, and the offset changes to the value of `gp`. 

There is a configuration file `.bolt_vm_config.yml` that lets you adjust the behavior of arrays. There is a setting `DynamicResourceExpansion` which gives you the option of using less memory upfront and expanding as needed; this is on by default. If `DynamicResourceExpansion` is off, the soft maximums will be ignored. The `HeapStorageBackup` setting gives you the option of storing the array values in globals if the array is too big for locals; on by default. You can adjust the number of frames, size of the VM globals, frame locals, and frame stack in this file.

Going over the configured hard limits, will result in the program crashing with an out of memory error. Setting the amount of allocated too high or too low, will also result in a memory error.

| Memory Region | Minimum | Maximum |
|---------------|---------|---------|
| frames        |    1    | 16,384  |      
| stack         | `sizeof(DataContant)` | 64 KB |
| locals        | `sizeof(DataContant)` | 1 MB |
| globals       | `sizeof(DataContant)` | 32 GB |

>`sizeof(DataConstant)` is 32 B on a 64-bit machine, so the minimum value would be 32. May vary on other instruction sizes.

### Funtion calls and returns

As stated above, when a non-built in function is called, a new frame is loaded and the pc of the current frame is saved as the return address of that frame. 

Functions are called as `CALL [function name] [argc]` where `[function name]` is replaced by a function name and `[argc]` is replaced by the number of parameters your function takes (both without square brakets). The number of arguments tells the VM how many values to pop off the stack; these popped values will be treated as the function parameters. Since a stack is "Last in first out", you must push your parameters in reverse order. The parameters are added as local variables of the new frame.

When a function returns, the top of the stack is popped, the frame is popped off the call stack, and the PC is set to the return address of the frame. The return value is pushed onto the top of the stack, unless it's of type `None` (indicating void return).

Built-in functions are somewhat similar in behavior. The only difference is they don't create a frame; instead they pop parameters, run some C code, and push the return value onto the stack (if not `None`).

When a function call happens, the VM first check if the name matches any of the built-in function names. If not, the VM searches the source code for defined functions and returns the index of the chunk of source code representing that function. Functions must be separated by new lines. The name of the function must be followed by a semicolon then the instructions.

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

> `println` is a built-in function

### Byte code

White space is used to denote the end of a function; each function must be separated by one or more new lines. Other than that, white space is not consequential, but it's a good idea to use spacing and indentation.

Any lines starting with a semicolon are ignored.

Any line that starts with a dot and ends with a colon will be used as a jump point. Jump points are used by jump instructions (`JMP`, `JMPT`, `JMPF`, `SJMPT`, `SJMPF`) to move the pc non-sequentially. All jumps must be explicitly defined otherwise, the pc will move to the next instruction it sees. The VM ignores jump labels and move to the next instruction right away.

### Exit codes

- 0 - Successful execution
- 1 - Improper operation (division by zero)
- 2 - Improper memory access (array bounds error)
- 3 - File operation error
- 254 - Unknown bytecode 
- 255 - Error running VM

> Any other exit codes will come from C functions or C itself

### Dependencies

 - C compiler that supports `c11` or greater
 - make
 - [criterion](https://github.com/Snaipe/Criterion)
    - Only needed if running `make test`