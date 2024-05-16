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
    char* config = "\t-c, --config [CONFIG_FILE_PATH]: Use your own custom configuration file for memory limits; the default configuration will be used if your file is missing or has improper values\n";
    char* help = "\t-h, --help:\tShow this help message\n";
    char* message = "";
    asprintf(&message, "Usage: %s FILE [OPTIONS]\nOPTIONS:\n%s%s%s%s", prog_name, verbose, memory, config, help);
    return message;
}

int main(int argc, char** argv) {
    char filename[256];
    bool verbose = false;
    bool showMemory = false;
    char config_file[256];
    switch(argc) {
        case 2:
            if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
                printf("%s\n", getUsage(argv[0]));
                return 0;
            }
            else if (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--memory") == 0) {
                showMemory = true;
            }
            else
                strncpy(filename, argv[1], strlen(argv[1]) + 1);
            break;
        case 3:
            strncpy(filename, argv[1], strlen(argv[1]) + 1);
            verbose = (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0);
            break;
        case 4:
            if (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--memory") == 0) {
                showMemory = true;
                if (strcmp(argv[2], "-c") == 0 || strcmp(argv[2], "--config") == 0)
                    strncpy(config_file, argv[3], strlen(argv[3]) + 1);
            }
            else if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--config") == 0) {
                strncpy(config_file, argv[2], strlen(argv[2]) + 1);
                showMemory = strcmp(argv[3], "-m") == 0 || strcmp(argv[3], "--memory") == 0;

            }
            else {
                strncpy(filename, argv[1], strlen(argv[1]) + 1);
                if (strcmp(argv[2], "-c") == 0 || strcmp(argv[2], "--config") == 0)
                    strncpy(config_file, argv[3], strlen(argv[3]) + 1);
            }
            break;
        case 5:
            strncpy(filename, argv[1], strlen(argv[1]) + 1);
            if (strcmp(argv[2], "-c") == 0 || strcmp(argv[2], "--config") == 0) {
                strncpy(config_file, argv[3], strlen(argv[3]) + 1);
                verbose = (strcmp(argv[4], "-v") == 0 || strcmp(argv[4], "--verbose") == 0);
            }
            else if (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0) {
                verbose = true;
                if (strcmp(argv[3], "-c") == 0 || strcmp(argv[3], "--config") == 0)
                    strncpy(config_file, argv[4], strlen(argv[4]) + 1);
            }
            break;
        default:
            fprintf(stderr, "Error: No input file provided\n%s\n", getUsage(argv[0]));
            return -1;
    }
    if (strlen(config_file) == 0)
        strcpy(config_file, CONFIG_FILE); // use the default if it's empty
    VMConfig conf = readConfigFile(config_file);
    if (!validateVMConfig(conf, config_file))
        return -1;
    
    if (showMemory) {
        printf("Estimated memory in configuration file '%s':\n---\n", config_file);
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