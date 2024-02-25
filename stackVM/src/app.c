#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "filereader.h"
#include "vm.h"

int main(int argc, char** argv) {
    bool verbose = argc >= 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0);
    SourceCode src = read_file("input.txt");
    if (verbose)
        displayCode(src);
    VM* vm = init(src);
    run(vm, verbose);
    destroy(vm);
    return 0;
}