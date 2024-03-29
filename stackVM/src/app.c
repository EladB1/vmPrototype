#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "filereader.h"
#include "vm.h"

char* getUsage(char* prog_name) {
    char* verbose = "\t-v, --verbose:\tDisplay the internal VM state at each execution cycle\n";
    char* help = "\t-h, --help:\tShow this help message\n";
    char* message = "";
    asprintf(&message, "Usage: %s FILE [OPTIONS]\nOPTIONS:\n%s%s", prog_name, verbose, help);
    return message;
}

int main(int argc, char** argv) {
    char filename[256];
    bool verbose;
    if (argc >= 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printf("%s\n", getUsage(argv[0]));
            return 0;
        }
        strncpy(filename, argv[1], strlen(argv[1]) + 1);
        verbose = (argc >= 3) && (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0);
    }
    else {
        fprintf(stderr, "Error: No input file provided\n%s\n", getUsage(argv[0]));
        return -1;
    }
    SourceCode src = read_file(filename);
    if (verbose)
        displayCode(src);
    VM* vm = init(src);
    run(vm, verbose);
    destroy(vm);
    return 0;
}