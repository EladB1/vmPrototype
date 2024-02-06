#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "impl_builtin.h"

void sleep_(DataConstant seconds) {
    if (seconds.type == Dbl)
        usleep(1000000 * seconds.value.dblVal);
    if (seconds.type == Int)
        sleep(seconds.value.intVal);
}

char* at(char* str, int index) {
    if (index >= strlen(str)) {
        printf("IndexError: String index out of range in function call 'at(\"%s\", %d)'\n", str, index);
        exit(1);
    }
    char result[2] = {str[index], '\0'};
    return strdup(result);
}

bool startsWith_(char* string, char* prefix) {
    int prefix_len = strlen(prefix);
    int len = strlen(string);
    if (len < prefix_len)
        return false;
    if (len == prefix_len)
        return strcmp(string, prefix) == 0;
    return strncmp(string, prefix, prefix_len) == 0;
}

bool endsWith(char* string, char* suffix) {
    int suffix_len = strlen(suffix);
    int len = strlen(string);
    if (len < suffix_len)
        return false;
    if (len == suffix_len)
        return strcmp(string, suffix) == 0;
    int j = len - 1;
    for (int i = suffix_len - 1; i >= 0; i--) {
        if (string[j] != suffix[i])
            return false;
        j--;
    }
    return true;
}

char* reverse(char* string) {
    int len = strlen(string);
    char out[len + 1];
    for (int i = len - 1; i >= 0; i--) {
        out[i] = string[len - i - 1];
    }
    out[len] = '\0';
    return strdup(out);
}