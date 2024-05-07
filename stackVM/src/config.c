#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"

#define MB 1048576
#define GB 1073741824

#define CONF_KV_PAIRS 8 // number of key, value pairs in the config file

void displayKeyValue(KeyValue pair) {
    printf("%s: %ld\n", pair.key, pair.value);
}

void displayConfigKeyValues(KeyValue* configPairs) {
    for (int i = 0; i < CONF_KV_PAIRS; i++) {
        displayKeyValue(configPairs[i]);
    }
}

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
            number *= MB;
        else
            number *= GB;
    }
    return number;
}

KeyValue* read_yaml_file(char* filePath) {
    FILE* fp = fopen(filePath, "r");
    if (fp == NULL || ferror(fp)) {
        perror("Error");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        return NULL;
    }
    char buff[1024];
    int i = 1;
    bool started = false;
    bool inKey = false;
    bool inValue = false;
    char* key = "";
    char* value = "";
    size_t length;
    int endIndex = 0;
    KeyValue* pairs = malloc(sizeof(KeyValue) * CONF_KV_PAIRS);
    while (fgets(buff, 1024, fp)) {
      if (i == 1) {
          i++;
          continue;
      }
      // printf("Line %d: %s", i, buff);
      length = strlen(buff);
      for (size_t j = 0; j < length; j++) {
        if (!started) {
            if (buff[j] == '#')
                break;
            else if (buff[j] == '-')
                started = true;
        }
        else {
            if (buff[j] != ' ' && !inKey && !inValue) {
                inKey = true;
                
            }
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
                pairs[endIndex].key = strdup(key);
                pairs[endIndex].value = processValue(value, filePath, i);
                //displayKeyValue(pairs[endIndex]);
                if (pairs[endIndex].value == -1) { // handle errors
                    return NULL;
                }
                endIndex++;
                key = "";
                value = "";
            }
        }
      }
      started = false;
      i++;
    }
    fclose(fp);
    return pairs;
}

long getByKey(KeyValue* configPairs, char* key) {
    for (int i = 0; i < CONF_KV_PAIRS; i++) {
        if (configPairs[i].key != NULL && strcmp(key, configPairs[i].key) == 0)
            return configPairs[i].value;
    }
    return -1;
}

bool validateConfig(KeyValue* configPairs, char* filePath) {
    if (configPairs == NULL)
        return false;
    long fsm = getByKey(configPairs, "frames_soft_max"),
    fhm = getByKey(configPairs, "frames_hard_max"),
    sssm = getByKey(configPairs, "stack_size_soft_max"),
    sshm = getByKey(configPairs, "stack_size_hard_max"),
    lsm = getByKey(configPairs, "locals_soft_max"),
    lhm = getByKey(configPairs, "locals_hard_max"),
    gsm = getByKey(configPairs, "globals_soft_max"),
    ghm = getByKey(configPairs, "globals_hard_max");
    bool valid = true;
    char* missingValue = "ConfigError: '%s' was expected but could not be found in config file: '%s'\n";
    if (fsm == -1) {
        fprintf(stderr, missingValue, "frames_soft_max", filePath);
        valid = false;
    }
    if (fhm == -1) {
        fprintf(stderr, missingValue, "frames_hard_max", filePath);
        valid = false;
    }
    if (fsm == -1) {
        fprintf(stderr, missingValue, "frames_soft_max", filePath);
        valid = false;
    }
    if (sssm == -1) {
        fprintf(stderr, missingValue, "stack_size_soft_max", filePath);
        valid = false;
    }
    if (sshm == -1) {
        fprintf(stderr, missingValue, "stack_size_hard_max", filePath);
        valid = false;
    }
    if (lsm == -1) {
        fprintf(stderr, missingValue, "locals_soft_max", filePath);
        valid = false;
    }
    if (lhm == -1) {
        fprintf(stderr, missingValue, "locals_hard_max", filePath);
        valid = false;
    }
    if (gsm == -1) {
        fprintf(stderr, missingValue, "globals_soft_max", filePath);
        valid = false;
    }
    if (ghm == -1) {
        fprintf(stderr, missingValue, "globals_hard_max", filePath);
        valid = false;
    }
    char* improperValue = "ConfigError: '%s' must be less than or equal to '%s' in config file: '%s'\n";
    if (fsm > fhm) {
        fprintf(stderr, improperValue, "frames_soft_max", "frames_hard_max", filePath);
        valid = false;
    }
    if (sssm > sshm) {
        fprintf(stderr, improperValue, "stack_size_soft_max", "stack_size_hard_max", filePath);
        valid = false;
    }
    if (lsm > lhm) {
        fprintf(stderr, improperValue, "locals_soft_max", "locals_hard_max", filePath);
        valid = false;
    }
    if (gsm > ghm) {
        fprintf(stderr, improperValue, "globals_soft_max", "globals_hard_max", filePath);
        valid = false;
    }
    return valid;
}