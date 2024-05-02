#include <string.h>
#include <criterion/criterion.h>

#include "utils.h"

char* cr_strdup(const char* str) {
    char* string = cr_malloc(strlen(str) + 1);
    if (str)
        strcpy(string, str);
    return string;
}

SourceCode* createSource(char** labels, char** bodies, int* jumpCounts, JumpPoint** jumps, int length) {
    SourceCode* src = cr_malloc(sizeof(SourceCode));
    SourceCode code;
    code.length = length;

    for (int i = 0; i < length; i++) {
        code.code[i].label = labels[i];
        code.code[i].body = split(bodies[i], " ");
        code.code[i].jmpCnt = jumpCounts[i];
        for (int j = 0; j < jumpCounts[i]; j++) {
            code.code[i].jumpPoints[j] = jumps[i][j];
        }
    }
    src = &code;
    return src;
}

void logStdout(FILE* stdout) {
    char buff[1024];
    while(fgets(buff, 1024, stdout)) {
        cr_log_info("STDOUT: %s", buff);
    }
}

void logStderr(FILE* stderr) {
    char buff[1024];
    while(fgets(buff, 1024, stderr)) {
        cr_log_info("STDERR: %s", buff);
    }
}