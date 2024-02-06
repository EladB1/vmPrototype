#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "impl_builtin.h"

DataConstant sleep_(DataConstant seconds) {
    if (seconds.type == Dbl)
        usleep(1000000 * seconds.value.dblVal);
    if (seconds.type == Int)
        sleep(seconds.value.intVal);
    return createNone();
}

DataConstant at(char* str, int index) {
    if (index >= strlen(str)) {
        printf("IndexError: String index out of range in function call 'at(\"%s\", %d)'\n", str, index);
        exit(1);
    }
    char result[2] = {str[index], '\0'};
    return createString(strdup(result));
}

DataConstant startsWith(char* string, char* prefix) {
    int prefix_len = strlen(prefix);
    int len = strlen(string);
    if (len < prefix_len)
        return createBoolean("false");
    if (len == prefix_len)
        return createBoolean(strcmp(string, prefix) == 0);
    return createBoolean(strncmp(string, prefix, prefix_len) == 0);
}

DataConstant endsWith(char* string, char* suffix) {
    int suffix_len = strlen(suffix);
    int len = strlen(string);
    if (len < suffix_len)
        return createBoolean("false");
    if (len == suffix_len)
        return createBoolean(strcmp(string, suffix) == 0);
    int j = len - 1;
    for (int i = suffix_len - 1; i >= 0; i--) {
        if (string[j] != suffix[i])
            return createBoolean("false");
        j--;
    }
    return createBoolean("true");
}
