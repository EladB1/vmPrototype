#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    bool dynamicResourceExpansionEnabled;
    bool useHeapStorageBackup;
    short framesSoftMax;
    short framesHardMax;
    long globalsSoftMax;
    long globalsHardMax;
    long stackSizeSoftMax;
    long stackSizeHardMax;
    long localsSoftMax;
    long localsHardMax;
} VMConfig;

long processValue(char* value, char* filePath, int line);
VMConfig getDefaultConfig();
VMConfig readConfigFile(char* filePath);
void displayVMConfig(VMConfig conf);
bool validateVMConfig(VMConfig conf, char* filePath);

#endif