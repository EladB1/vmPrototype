#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "impl_builtin.h"

#define DEFAULT_LINES 1024

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

void printerr(DataConstant data, bool terminates, int exitCode) {
    fprintf(stderr, "%s\n", data.value.strVal);
    if (terminates)
        exit(exitCode);
}

char* getType(DataConstant data, DataConstant* globals) {
    switch (data.type) {
        case Int:
            return "int";
        case Dbl:
            return "double";
        case Bool:
            return "boolean";
        case Str:
            return "string";
        case Null:
            return "null";
        case None:
            return "None";
        case Addr:
            if (data.length == 0)
                return "Array<>";
            DataConstant elem;
            char* type = "";
            char* subType = "";
            int start = data.value.intVal;
            int end = start + data.length;
            for (int i = start; i < end; i++) {
                elem = globals[i];
                if (elem.type == Addr && elem.length == 0 && i < end - 1)
                    continue;
                if (elem.type != Null && elem.type != None) {
                    subType = getType(elem, globals);
                    break;
                }
            }
            asprintf(&type, "Array<%s>", subType);
            return type;
        default:
            return "Unknown";
    }
}

void sleep_(DataConstant seconds) {
    if (seconds.type == Dbl) {
        int time = (int) lround(1000000 * seconds.value.dblVal);
        usleep(time);
    }
    if (seconds.type == Int)
        sleep(seconds.value.intVal);
}

char* at(char* str, int index) {
    if (index < 0 || index >= (int)strlen(str)) {
        fprintf(stderr, "IndexError: String index out of range in function call 'at(\"%s\", %d)'\n", str, index);
        exit(2);
    }
    char* result = "";
    asprintf(&result, "%c", str[index]);
    return result;
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

char* replace(char* string, char* old, char* new, bool multiple) {
    size_t len = strlen(string);
    size_t old_len = strlen(old);
    size_t new_len = strlen(new);
    size_t end;
    if (len <= old_len && strstr(old, string))
        return new;
    char* replaced = malloc(len - old_len + new_len + 1);
    char* temp = strstr(string, old);
    if (temp == NULL)
        return string;
    end = len - strlen(temp);
    strncpy(replaced, string, end);
    replaced[end] = '\0';
    sprintf(&replaced[end], "%s%s", new, temp + old_len);
    while (multiple && strstr(replaced, old)) {
        replaced = replace(replaced, old, new, multiple);
    }
    return replaced;
}

char* slice(char* string, int start, int end) {
    if (start < 0 || start > end || start >= (int) strlen(string)) {
        fprintf(stderr, "Invalid start value of slice %d\n", start);
        exit(2);
    }
    if (start == 0 && end == (int) strlen(string))
        return string;
    char sliced[end - start];
    int index = 0;
    for (int i = start; i < end; i++) {
        sliced[index++] = string[i];
    }
    sliced[index] = '\0';
    return strdup(sliced);
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

DataConstant readFile(char* filePath, int* globCount, DataConstant** globals) {
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
    DataConstant lines;
    lines.type = Addr;
    lines.value.intVal = *globCount + 1;
    lines.length = 0;
    lines.size = 0;
    char line[DEFAULT_LINES];
    while (fgets(line, DEFAULT_LINES, fp)) {
        (*globals)[++(*globCount)] = createString(strdup(line));
        lines.length++;
        lines.size++;
    }
    fclose(fp);
    return lines;
}

void writeToFile(char* filePath, char* content, char* mode) {
    if (!fileExists(filePath))
        createFile(filePath);
    FILE* fp = fopen(filePath, mode);
    if (fp == NULL || ferror(fp)) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        exit(3);
    }
    int write = fprintf(fp, "%s\n", content);
    if (write < 0) {
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

void reverseArr(DataConstant array, DataConstant** globals) {
    if (array.length <= 1)
        return;
    DataConstant temp;
    int j, addr, half = array.length / 2;
    DataConstant* globs = *globals;
    for (int i = 0; i < half; i++) {
        j = array.value.intVal + (array.length - i - 1);
        addr = array.value.intVal + i;
        temp = globs[addr];
        globs[addr] = globs[j];
        globs[j] = temp;
    }
}

DataConstant sliceArr(DataConstant array, int start, int end, int* globCount, DataConstant** globals) {
    if (start < 0 || start > end || start >= array.length || end > array.length) {
        fprintf(stderr, "Array index out of bounds in call to slice. start: %d, end: %d\n", start, end);
        exit(2);
    }
    int addr = array.value.intVal + start;
    int len = array.value.intVal + end - addr;
    return partialCopyAddr(array, addr, len, globCount, globals);
}

bool arrayContains(DataConstant array, DataConstant element, DataConstant* globals) {
    if (array.length == 0)
        return false;
    int start = array.value.intVal;
    int end = start + array.length;
    for (int i = start; i < end; i++) {
        if (isEqual(globals[i], element))
            return true;
    }
    return false;
}

int indexOf(DataConstant array, DataConstant element, DataConstant* globals) {
    if (array.length != 0) {
        int start = array.value.intVal;
        int end = start + array.length;
        for (int i = start; i < end; i++) {
            if (isEqual(globals[i], element))
                return i;
        }
    }
    return -1;
}

char* join(DataConstant array, char* delim, DataConstant* globals) {
    if (array.length == 0)
        return "";
    int start = array.value.intVal;
    int end = start + array.length;
    char* result = globals[start].value.strVal;
    for (int i = start+1; i < end; i++) {
        asprintf(&result, "%s%s%s", result, delim, globals[i].value.strVal);
    }
    return result;
}

int comparator(const void* a, const void* b) {
    DataConstant lhs = *(DataConstant*)a;
    DataConstant rhs = *(DataConstant*)b;
    if (lhs.type != rhs.type) {
        if (lhs.type == Null || rhs.type == Null)
            return lhs.type == Null ? -1 : 1; // null values come first
    }
    if (lhs.type == Str)
        return strcmp(lhs.value.strVal, rhs.value.strVal);
    else if (lhs.type == Bool)
        return lhs.value.boolVal - rhs.value.boolVal;
    else if (lhs.type == Int)
        return lhs.value.intVal - rhs.value.intVal;
    else if (lhs.type == Dbl) {
        if (lhs.value.dblVal == rhs.value.dblVal)
            return 0;
        return lhs.value.dblVal < rhs.value.dblVal ? -1 : 1;
    }
    // preserve order by default
    return 0;
}

void sort(DataConstant array, DataConstant* globals) {
    int addr = array.value.intVal;
    DataConstant* start = &globals[addr];
    qsort(start, array.length, sizeof(DataConstant), comparator);
}

void removeByIndex(DataConstant* array, int index, DataConstant** globals) {
    if (index < 0 || index > array->size) {
        fprintf(stderr, "Array index out of bounds\n");
        exit(2);
    }
    int addr = array->value.intVal + index;
    memmove(&globals[0][addr], &globals[0][addr + 1], sizeof(DataConstant) * (array->length - index - 1));
    array->length--;
    (*globals)[array->value.intVal + array->length] = createNone();
}

void append(DataConstant* array, DataConstant elem, DataConstant** globals) {
    if (array->length == array->size) {
        fprintf(stderr, "Array size limit %d reached. Cannot insert into array.\n" , array->size);
        exit(2);
    }
    int addr = array->value.intVal + array->length;
    (*globals)[addr] = elem;
    array->length++; 
}

void prepend(DataConstant* array, DataConstant elem, DataConstant** globals) {
    if (array->length == array->size) {
        fprintf(stderr, "Array size limit %d reached. Cannot insert into array.\n" , array->size);
        exit(2);
    }
    int addr = array->value.intVal;
    memmove(&globals[addr + 1], &globals[addr], sizeof(DataConstant) * (array->length + 1));
    (*globals)[addr] = elem;
    array->length++; 
}

void insert(DataConstant* array, DataConstant elem, int index, DataConstant** globals) {
    if (array->length == array->size) {
        fprintf(stderr, "Array size limit %d reached. Cannot insert into array.\n" , array->size);
        exit(2);
    }
    if (index < 0 || index > array->length) {
        fprintf(stderr, "Array index %d out of range %d\n", index, array->length);
        exit(2);
    }
    if (index == 0) {
        prepend(array, elem, globals);
        return;
    }
    if (index == array->length) {
        append(array, elem, globals);
        return;
    }
    int addr = array->value.intVal + index;
    memmove(&globals[addr + 1], &globals[addr], sizeof(DataConstant) * (array->length - index + 1));
    (*globals)[addr] = elem;
    array->length++; 
}