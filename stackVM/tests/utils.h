#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "../src/filereader.h"

char* cr_strdup(const char* str);
SourceCode* createSource(char** labels, char** bodies, int* jumpCounts, JumpPoint** jumps, int length);
void logStdout(FILE* stdout);
void logStderr(FILE* stderr);

#endif