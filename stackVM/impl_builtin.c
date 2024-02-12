#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "impl_builtin.h"

#define DEFAULT_LINES 1024

void sleep_(DataConstant seconds) {
    if (seconds.type == Dbl) {
        int time = (int) lround(1000000 * seconds.value.dblVal);
        usleep(time);
    }
    if (seconds.type == Int)
        sleep(seconds.value.intVal);
}

char* at(char* str, int index) {
    if (index >= (int)strlen(str)) {
        fprintf(stderr, "IndexError: String index out of range in function call 'at(\"%s\", %d)'\n", str, index);
        exit(2);
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

bool fileExists(char* filePath) {
    return access(filePath, F_OK) == 0;
}

void createFile(char* filePath) {
    if (fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot create file '%s' because it already exists\n", filePath);
        return; // non-fatal error
    }
    FILE* fp = fopen(filePath, "w");
    fclose(fp);
}

char** readFile(char* filePath) {
    if (!fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot read file '%s' because it does not exist\n", filePath);
        exit(3);
    }
    FILE* fp = fopen(filePath, "r");
    if (fp == NULL || ferror(fp)) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        exit(3);
    }
    char** lines = malloc(sizeof(char*) * DEFAULT_LINES);
    int lineCnt = 0;
    int limit = DEFAULT_LINES;
    char line[DEFAULT_LINES];
    while (fgets(line, DEFAULT_LINES, fp)) {
        lines[lineCnt++] = line;
        if (lineCnt == limit - 1) {
            limit *= 2;
            lines = realloc(lines, sizeof(char*) * limit);
        }
    }
    fclose(fp);
    return lines;
}

void writeToFile(char* filePath, char* content, char* mode) {
    if (!fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot write to file '%s' because it does not exist\n", filePath);
        exit(1);
    }
    FILE* fp = fopen(filePath, mode);
    if (fp == NULL || ferror(fp)) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        exit(3);
    }
    int write = fprintf(fp, "%s", content);
    if (write != 0) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        exit(write);
    }
    fclose(fp);
}

void renameFile(char* filePath, char* newFilePath) {
    if (!fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot rename file '%s' because it does not exist\n", filePath);
        exit(3);
    }
    int mv = rename(filePath, newFilePath);
    if (mv != 0) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        exit(mv);
    }
}

void deleteFile(char* filePath) {
    if (!fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot delete file '%s' because it does not exist\n", filePath);
        exit(3);
    }
    int removal = remove(filePath);
    if (removal != 0) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        exit(removal);
    }
}

void print(DataConstant data, DataConstant* globals, bool newLine) {
    char end = newLine ? '\n' : '\0';
    if (data.type == Int)
        printf("%d%c", data.value.intVal, end);
    if (data.type == Dbl)
        printf("%f%c", data.value.dblVal, end);
    if (data.type == Str)
        printf("%s%c", data.value.strVal, end);
    if (data.type == Bool)
        printf("%s%c",toString(data), end);
    if (data.type == Null)
        printf("null%c", end);
    if (data.type == Addr) {
        printf("[");
        int start = data.value.intVal;
        int stop = start + data.length;
        for (int i = start; i < stop; i++) {
            print(globals[i], globals, false);
            if (i != stop - 1)
                printf(", ");

        }
        printf("]%c", end);
    }
}

char* slice(char* string, int start, int end) {
    if (start > end || start >= (int) strlen(string)) {
        fprintf(stderr, "Invalid start value of slice %d\n", start);
        exit(2);
    }
    char sliced[end - start];
    int index = 0;
    for (int i = start; i < end; i++) {
        sliced[index++] = string[i];
    }
    return strdup(sliced);
}

bool contains(char* str, char* subStr) {
    if (strlen(subStr) == 0)
        return true;
    if (strlen(subStr) > strlen(str))
        return false;
    while (*str) {
         const char* sub = subStr;
         const char* tmp = str;
         while (*tmp++ == *sub++) {
            if (!*sub)
                return true;
         }
         str++;
    }
    return false;
}