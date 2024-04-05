#ifndef EXITCODE_H
#define EXITCODE_H

typedef enum {
    success = 0,
    operation_err = 1,
    memory_err = 2,
    file_err = 3,
    unknown_bytecode = 254,
    vm_err = 255
} ExitCode;

#endif