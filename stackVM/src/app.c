#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "filereader.h"
#include "config.h"
#include "vm.h"

#define CONFIG_FILE "build/.bolt_vm_config.yml"

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

    VMConfig conf = readConfigFile(CONFIG_FILE);
    if (!validateVMConfig(conf, CONFIG_FILE))
        return -1;

    SourceCode* src = read_file(filename);
    if (src == NULL)
        return -1;
    if (verbose) {
        displayVMConfig(conf);
        displayCode(src);
    }
    VM* vm = init(src, conf);
    if (vm == NULL)
        return -1;
    ExitCode runStatus = run(vm, verbose);

    destroy(vm);
    deleteSourceCode(src);

    return runStatus;
}