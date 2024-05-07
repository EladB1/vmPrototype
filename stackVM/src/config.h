#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    char* key;
    long value;
} KeyValue;

void displayKeyValue(KeyValue pair);
void displayConfigKeyValues(KeyValue* configPairs);
long processValue(char* value, char* filePath, int line);
KeyValue* read_yaml_file(char* filePath);
long getByKey(KeyValue* configPairs, char* key);
bool validateConfig(KeyValue* configPairs, char* filePath);

#endif