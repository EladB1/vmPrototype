#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "impl_builtin.h"

#define DEFAULT_LINES 1024

void print(DataConstant data, bool newLine) {
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
        DataConstant* start = getArrayStart(data);
        DataConstant* stop = start + data.length;
        printf("[");
        for (DataConstant* curr = start; curr != stop; curr++) {
            print(*curr, false);
            if (curr != stop - 1)
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

char* getType(DataConstant data) {
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
            char* type = "";
            char* subType = "";
            DataConstant* start = getArrayStart(data);
            DataConstant* end = start + data.length;
            for (DataConstant* curr = start; curr != end; curr++) {
                if (curr->type == Addr && curr->length == 0 && curr < end - 1)
                    continue;
                if (curr->type != Null && curr->type != None) {
                    subType = getType(*curr);
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

char* at(char* str, int index, ExitCode* vmState) {
    if (index < 0 || index >= (int)strlen(str)) {
        fprintf(stderr, "IndexError: String index out of range in function call 'at(\"%s\", %d)'\n", str, index);
        *vmState = memory_err;
        return "";
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

char* slice(char* string, int start, int end, ExitCode* vmState) {
    if (start < 0 || start > end || start >= (int) strlen(string)) {
        fprintf(stderr, "Invalid start value of slice %d\n", start);
        *vmState = memory_err;
        return "";
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

void createFile(char* filePath, ExitCode* vmState) {
    if (fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot create file '%s' because it already exists\n", filePath);
        return; // non-fatal error
    }
    FILE* fp = fopen(filePath, "w");
    if (fp == NULL || ferror(fp)) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        *vmState = file_err;
        return;
    }
    fclose(fp);
}

DataConstant readFile(char* filePath, VM* vm, Frame* frame, bool* globalsExpanded, bool verbose) {
    if (!fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot read file '%s' because it does not exist\n", filePath);
        vm->state = file_err;
        return createNone();
    }
    FILE* fp = fopen(filePath, "r");
    if (fp == NULL || ferror(fp)) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        vm->state = file_err;
        return createNone();
    }
    DataConstant lines;
    lines.type = Addr;
    //lines.value.intVal = *lp + 1;
    lines.length = 0;
    lines.size = 0;
    int max_file_length = (1 << 20) - 1;
    DataConstant content[max_file_length];
    char line[DEFAULT_LINES];
    while (fgets(line, DEFAULT_LINES, fp)) {
        content[lines.length] = createString(strdup(line));
        if (lines.length > max_file_length) {
            fprintf(stderr, "File is too large to read\n");
            vm->state = file_err;
            return createNone();
        }
        lines.length++;
        lines.size++;
    }
    ArrayTarget arrayTarget = checkAndRetrieveArrayValuesTarget(vm, frame, lines.size, globalsExpanded, verbose);
    if (vm->state != success)
        return createNone();
    *frame = *(arrayTarget.frame);
    lines.value.address = arrayTarget.target;
    lines.offset = *arrayTarget.targetp + 1;
    for (int i = 0; i < lines.length; i++) {
        arrayTarget.target[++(*arrayTarget.targetp)] = content[i];
    }
    fclose(fp);
    return lines;
}

void writeToFile(char* filePath, char* content, char* mode, ExitCode* vmState) {
    if (!fileExists(filePath))
        createFile(filePath, vmState);
    FILE* fp = fopen(filePath, mode);
    if (fp == NULL || ferror(fp)) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        *vmState = file_err;
        return;
    }
    int write = fprintf(fp, "%s\n", content);
    if (write < 0) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        *vmState = file_err;
        return;
    }
    fclose(fp);
}

void renameFile(char* filePath, char* newFilePath, ExitCode* vmState) {
    if (!fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot rename file '%s' because it does not exist\n", filePath);
        *vmState = file_err;
        return;
    }
    int mv = rename(filePath, newFilePath);
    if (mv != 0) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        *vmState = file_err;
        return;
    }
}

void deleteFile(char* filePath, ExitCode* vmState) {
    if (!fileExists(filePath)) {
        fprintf(stderr, "FileError: Cannot delete file '%s' because it does not exist\n", filePath);
        *vmState = file_err;
        return;
    }
    int removal = remove(filePath);
    if (removal != 0) {
        perror("FileError");
        fprintf(stderr, "Cause: '%s'\n", filePath);
        *vmState = file_err;
        return;
    }
}

void reverseArr(DataConstant array) {
    if (array.length <= 1)
        return;
    DataConstant* start = getArrayStart(array);
    DataConstant temp;
    int half = array.length / 2;
    DataConstant* mid;
    for (int i = 0; i < half; i++) {
        mid = start + (array.length - i - 1);
        temp = *(start + i);
        *(start + i) = *mid;
        *mid = temp;
    }
}

DataConstant sliceArr(DataConstant array, int start, int end, VM* vm, Frame* frame, bool* globalsExpanded, bool verbose) {
    if (start < 0 || start > end || start >= array.length || end > array.length) {
        fprintf(stderr, "Array index out of bounds in call to slice. start: %d, end: %d\n", start, end);
        vm->state = memory_err;
        return createNone();
    }
    int len = end - start;
    ArrayTarget arrayTarget = checkAndRetrieveArrayValuesTarget(vm, frame, array.size, globalsExpanded, verbose);
    *frame = *(arrayTarget.frame);
    return partialCopyAddr(array, start, len, arrayTarget.targetp, &arrayTarget.target);
}

bool arrayContains(DataConstant array, DataConstant element) {
    if (array.length != 0) {
        DataConstant* start = getArrayStart(array);
        DataConstant* stop = start + array.length;
        for (DataConstant* curr = start; curr != stop; curr++) {
            if (isEqual(*curr, element))
                return true;
        }
    }
    return false;
}

int indexOf(DataConstant array, DataConstant element) {
    if (array.length != 0) {
        DataConstant* start = getArrayStart(array);
        DataConstant* stop = start + array.length;
        int i = 0;
        for (DataConstant* curr = start; curr != stop; curr++) {
            if (isEqual(*curr, element))
                return i;
            i++;
        }
    }
    return -1;
}

char* join(DataConstant array, char* delim) {
    if (array.length == 0)
        return "";
    DataConstant* start = getArrayStart(array);
    DataConstant* stop = start + array.length;
    char* result = start->value.strVal;
    for (DataConstant* curr = start + 1; curr != stop; curr++) {
        asprintf(&result, "%s%s%s", result, delim, curr->value.strVal);
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

void sort(DataConstant array) {
    DataConstant* start = getArrayStart(array);
    qsort(start, array.length, sizeof(DataConstant), comparator);
}

void removeByIndex(DataConstant* array, int index, ExitCode* vmState) {
    if (index < 0 || index > array->size) {
        fprintf(stderr, "Array index out of bounds\n");
        *vmState = memory_err;
        return;
    }
    DataConstant* start = getArrayStart(*array);
    memmove(start + index, start + index + 1, sizeof(DataConstant) * (array->length - index - 1));
    array->length--;
    *(start + array->length) = createNone();
}

void append(DataConstant* array, DataConstant elem, ExitCode* vmState) {
    if (array->length == array->size) {
        fprintf(stderr, "Array size limit %d reached. Cannot insert into array.\n" , array->size);
        *vmState = memory_err;
        return;
    }
    DataConstant* start = getArrayStart(*array);
    *(start + array->length) = elem;
    array->length++; 
}

void prepend(DataConstant* array, DataConstant elem, ExitCode* vmState) {
    if (array->length == array->size) {
        fprintf(stderr, "Array size limit %d reached. Cannot insert into array.\n" , array->size);
        *vmState = memory_err;
        return;
    }
    DataConstant* start = getArrayStart(*array);
    memmove(start+1, start, sizeof(DataConstant) * (array->length));
    *start = elem;
    array->length++; 
}

void insert(DataConstant* array, DataConstant elem, int index, ExitCode* vmState) {
    if (array->length == array->size) {
        fprintf(stderr, "Array size limit %d reached. Cannot insert into array.\n" , array->size);
        *vmState = memory_err;
        return;
    }
    if (index < 0 || index > array->length) {
        fprintf(stderr, "Array index %d out of range %d\n", index, array->length);
        *vmState = memory_err;
        return;
    }
    if (index == 0) {
        prepend(array, elem, vmState);
        return;
    }
    if (index == array->length) {
        append(array, elem, vmState);
        return;
    }
    DataConstant* start = getArrayStart(*array) + index;
    memmove(start+1, start, sizeof(DataConstant) * (array->length - index));
    *start = elem;
    array->length++; 
}