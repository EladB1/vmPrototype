#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "config.h"
#include "dataconstant.h"

// manually process instead of using regex
long processValue(char* value, char* filePath, int line) {
    char* digits = "";
    char endLetter = '\0';
    int len = strlen(value);
    bool lastCharLetter = value[len - 1] == 'K' || value[len - 1] == 'M' || value[len - 1] == 'G';
    for (int i = 0; i < len; i++) {
        if (!isdigit(value[i])) {
            if (i == len - 1 && lastCharLetter) {
                endLetter = value[i];
            }
            else {
                fprintf(stderr, "ConfigError: Invalid character '%c' found\nFile: '%s', Line: %d, Index: %d\n", value[i], filePath, line, i);
                return -1;
            }
        }
        else
            asprintf(&digits, "%s%c", digits, value[i]);
    }
    long number = atol(digits);
    if (endLetter != '\0') {
        if (endLetter == 'K')
            number *= 1024;
        else if (endLetter == 'M')
            number *= 1 << 20;
        else
            number *= 1 << 30;
    }
    return number;
}

VMConfig getDefaultConfig() {
    VMConfig conf;
    conf.dynamicResourceExpansionEnabled = true;
    conf.useHeapStorageBackup = true;
    conf.framesSoftMax = 1 << 9;
    conf.framesHardMax = 1 << 10;
    conf.stackSizeSoftMax = 1 << 10;
    conf.stackSizeHardMax = 10 * (1 << 10);
    conf.localsSoftMax = 1 << 16;
    conf.localsHardMax = 1 << 20;
    conf.globalsSoftMax = 1 << 20;
    conf.globalsHardMax = 1 << 29;
    return conf;
}

VMConfig readConfigFile(char* filePath) {
    VMConfig conf = getDefaultConfig();
    FILE* fp = fopen(filePath, "r");
    if (fp == NULL || ferror(fp)) {
        perror("Warning");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        return conf;
    }
    char buff[1024];
    int line = 1;
    bool started = false;
    bool inKey = false;
    bool inValue = false;
    char* key = "";
    char* value = "";
    size_t length;
    while (fgets(buff, 1024, fp)) {
        if (strcmp(buff, "---\n") == 0) {
            line++;
            continue;
        }
        // printf("Line %d: %s", line, buff);
        length = strlen(buff);
        for (size_t j = 0; j < length; j++) {
            if (!started) {
                if (buff[j] == '#')
                    break;
                else if (buff[j] == '-')
                    started = true;
            }
            else {
                if (buff[j] != ' ' && !inKey && !inValue)
                    inKey = true;
                if (inKey && buff[j] != ' ' && buff[j] != ':')
                    asprintf(&key, "%s%c", key, buff[j]);
                if (inKey && buff[j] == ':') {
                    inKey = false;
                    inValue = true;
                }
                if (inValue && buff[j] != ' ' && buff[j] != ':' && j < length - 1)
                    asprintf(&value, "%s%c", value, buff[j]);
                if (j == length - 1) {
                    if (buff[j] != '\n')
                        asprintf(&value, "%s%c", value, buff[j]);
                    inValue = false;
                    if (strcmp(key, "DynamicResourceExpansion") == 0) {
                        conf.dynamicResourceExpansionEnabled = (strcmp(value, "disabled") != 0);
                    }
                    if (strcmp(key, "HeapStorageBackup") == 0) {
                        conf.useHeapStorageBackup = (strcmp(value, "disabled") != 0);
                    }
                    if (strcmp(key, "frames_soft_max") == 0) {
                        conf.framesSoftMax = processValue(value, filePath, line);
                    }
                    if (strcmp(key, "frames_hard_max") == 0) {
                        conf.framesHardMax = processValue(value, filePath, line);
                    }
                    if (strcmp(key, "stack_size_soft_max") == 0) {
                        conf.stackSizeSoftMax = processValue(value, filePath, line);
                    }
                    if (strcmp(key, "stack_size_hard_max") == 0) {
                        conf.stackSizeHardMax = processValue(value, filePath, line);
                    }
                    if (strcmp(key, "locals_soft_max") == 0) {
                        conf.localsSoftMax = processValue(value, filePath, line);
                    }
                    if (strcmp(key, "locals_hard_max") == 0) {
                        conf.localsHardMax = processValue(value, filePath, line);
                    }
                    if (strcmp(key, "globals_soft_max") == 0) {
                        conf.globalsSoftMax = processValue(value, filePath, line);
                    }
                    if (strcmp(key, "globals_hard_max") == 0) {
                        conf.globalsHardMax = processValue(value, filePath, line);
                    }
                    key = "";
                    value = "";
                }
            }
      }
      started = false;
      line++;

    }
    fclose(fp);
    return conf;
}

char* getHumanReadableStorageSpace(long space) {
    char* result;
    char* suffix = "B";
    double storage = space;
    if (space >= 1024 && space < 1 << 20) {
        suffix = "KB";
        storage /= 1024;
    }
    else if (space >= 1 << 20 && space < 1 << 30) {
        suffix = "MB";
        storage /= (1 << 20);
    }
    else if (space >= 1 << 30) {
        suffix = "GB";
        storage /= (1 << 30);
    }
    asprintf(&result, "%.2f %s", storage, suffix);
    return result;
}

void printEstimatedMemory(VMConfig conf) {
    long space;
    if (conf.dynamicResourceExpansionEnabled) {
        space = conf.globalsSoftMax + (conf.framesSoftMax * (conf.stackSizeSoftMax + conf.localsSoftMax));
        char* softSpace = getHumanReadableStorageSpace(space);
        space = conf.globalsHardMax + (conf.framesHardMax * (conf.stackSizeHardMax + conf.localsHardMax));
        char* hardSpace = getHumanReadableStorageSpace(space);
        printf("Estimated VM memory usage: %s (soft limits) - %s (hard limits)\n", softSpace, hardSpace);
    }
    else {
        space = conf.globalsHardMax + (conf.framesHardMax * (conf.stackSizeHardMax + conf.localsHardMax));
        printf("Estimated VM memory usage: %s\n", getHumanReadableStorageSpace(space));
    }
}

void displayVMConfig(VMConfig conf) {
    printf("DynamicResourceExpansion: %s\n", conf.dynamicResourceExpansionEnabled ? "enabled" : "disabled");
    printf("HeapStorageBackup: %s\n", conf.useHeapStorageBackup ? "enabled" : "disabled");
    printf("frames_soft_max: %ld frames\n", conf.framesSoftMax);
    printf("frames_hard_max: %ld frames\n", conf.framesHardMax);
    printf("stack_size_soft_max: %ld B (%ld values)\n", conf.stackSizeSoftMax, conf.stackSizeSoftMax / sizeof(DataConstant));
    printf("stack_size_hard_max: %ld B (%ld values)\n", conf.stackSizeHardMax, conf.stackSizeHardMax / sizeof(DataConstant));
    printf("locals_soft_max: %ld B (%ld values)\n", conf.localsSoftMax, conf.localsSoftMax / sizeof(DataConstant));
    printf("locals_hard_max: %ld B (%ld values)\n", conf.localsHardMax, conf.localsHardMax / sizeof(DataConstant));
    printf("globals_soft_max: %ld B (%ld values)\n", conf.globalsSoftMax, conf.globalsSoftMax / sizeof(DataConstant));
    printf("globals_hard_max: %ld B (%ld values)\n", conf.globalsHardMax, conf.globalsHardMax / sizeof(DataConstant));
    printEstimatedMemory(conf);
}

bool validateVMConfig(VMConfig conf, char* filePath) {
    bool valid = true;
    long valueSize = (long) sizeof(DataConstant);
    char* valueError = "ConfigError: '%s' must be between %ld and %ld in config file: '%s'\n";
    char* improperValue = "ConfigError: '%s' must be less than or equal to '%s' in config file: '%s'\n";
    if (conf.dynamicResourceExpansionEnabled) {
        if (conf.framesSoftMax < 1 || conf.framesSoftMax > LONG_MAX) {
            fprintf(stderr, valueError, "frames_soft_max", 1, LONG_MAX, filePath);
            valid = false;
        }
        if (conf.stackSizeSoftMax < valueSize || conf.stackSizeSoftMax > LONG_MAX) {
            fprintf(stderr, valueError, "stack_size_soft_max", valueSize, LONG_MAX, filePath);
            valid = false;
        }
        if (conf.localsSoftMax < valueSize || conf.localsSoftMax > LONG_MAX) {
            fprintf(stderr, valueError, "locals_soft_max", valueSize, LONG_MAX, filePath);
            valid = false;
        }
        if (conf.globalsSoftMax < valueSize || conf.globalsSoftMax > LONG_MAX) {
            fprintf(stderr, valueError, "globals_soft_max", valueSize, LONG_MAX, filePath);
            valid = false;
        }
        if (conf.framesSoftMax > conf.framesHardMax) {
            fprintf(stderr, improperValue, "frames_soft_max", "frames_hard_max", filePath);
            valid = false;
        }

        if (conf.stackSizeSoftMax > conf.stackSizeHardMax) {
            fprintf(stderr, improperValue, "stack_size_soft_max", "stack_size_hard_max", filePath);
            valid = false;
        }

        if (conf.localsSoftMax > conf.localsHardMax) {
            fprintf(stderr, improperValue, "locals_soft_max", "locals_hard_max", filePath);
            valid = false;
        }

        if (conf.globalsSoftMax > conf.globalsHardMax) {
            fprintf(stderr, improperValue, "globals_soft_max", "globals_hard_max", filePath);
            valid = false;
        }
    }
    if (conf.framesHardMax < 1 || conf.framesHardMax > LONG_MAX) {
        fprintf(stderr, valueError, "frames_hard_max", 1, LONG_MAX, filePath);
        valid = false;
    }
    if (conf.stackSizeHardMax < valueSize || conf.stackSizeHardMax > LONG_MAX) {
        fprintf(stderr, valueError, "stack_size_hard_max", valueSize, LONG_MAX, filePath);
        valid = false;
    }
    if (conf.localsHardMax < valueSize || conf.localsHardMax > LONG_MAX) {
        fprintf(stderr, valueError, "locals_hard_max", valueSize, LONG_MAX, filePath);
        valid = false;
    }
    if (conf.globalsHardMax < valueSize || conf.globalsHardMax > LONG_MAX) {
        fprintf(stderr, valueError, "globals_hard_max", valueSize, LONG_MAX, filePath);
        valid = false;
    }
    return valid;
}