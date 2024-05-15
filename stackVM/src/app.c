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
    char* memory = "\t-m, --memory:\tDisplay the amount of memory configured in your configuration file then stop running\n";
    char* help = "\t-h, --help:\tShow this help message\n";
    char* message = "";
    asprintf(&message, "Usage: %s FILE [OPTIONS]\nOPTIONS:\n%s%s%s", prog_name, verbose, memory, help);
    return message;
}

int main(int argc, char** argv) {
    char filename[256];
    bool verbose;
    bool showMemory = false;
    if (argc >= 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printf("%s\n", getUsage(argv[0]));
            return 0;
        }
        else if (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--memory") == 0) {
            showMemory = true;
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
    
    if (showMemory) {
        printf("Estimated memory in configuration file '%s':\n---\n", CONFIG_FILE);
        displayVMConfig(conf);
        return 0;
    }

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