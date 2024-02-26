#include <string.h>
#include <criterion/criterion.h>

#include "utils.h"

char* cr_strdup(const char* str) {
    char* string = cr_malloc(strlen(str) + 1);
    if (str)
        strcpy(string, str);
    return string;
}