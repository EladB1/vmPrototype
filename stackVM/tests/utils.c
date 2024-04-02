#include <string.h>
#include <criterion/criterion.h>

#include "utils.h"

char* cr_strdup(const char* str) {
    char* string = cr_malloc(strlen(str) + 1);
    if (str)
        strcpy(string, str);
    return string;
}

SourceCode createSource(char** labels, char** bodies, int* jumpCounts, JumpPoint** jumps, int length) {
    SourceCode src;
    src.length = length;
    for (int i = 0; i < length; i++) {
        src.code[i].label = labels[i];
        src.code[i].body = split(bodies[i], " ");
        src.code[i].jmpCnt = jumpCounts[i];
        for (int j = 0; j < jumpCounts[i]; j++) {
            src.code[i].jumpPoints[j] = jumps[i][j];
        }
    }
    return src;
}

void deleteSource(SourceCode src) {
    for (int i = 0; i < src.length; i++) {
        freeStringVector(src.code[i].body);
    }
}