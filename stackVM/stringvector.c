#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringvector.h"

#define DEFAULT_VECTOR_MAX 256
#define MAX(a, b) (a >= b ? a : b)

StringVector* createStringVector() {
    StringVector* sv = malloc(sizeof(StringVector));
    sv->length = 0;
    sv->capacity = DEFAULT_VECTOR_MAX;
    sv->strings = malloc(sizeof(char*) * sv->capacity);
    return sv;
}

void freeStringVector(StringVector* sv) {
    free(sv->strings);
    free(sv);
}

char* getFromSV(StringVector* sv, int index) {
    //printf("Index: %d, Length: %d\n", index, sv->length);
    return sv->strings[index];
}

void addString(StringVector* sv, char* string) {
    if (sv->length == sv->capacity - 1) {
        sv->capacity *= 2;
        sv->strings = realloc(sv->strings, sv->capacity);
    }
    sv->strings[sv->length] = string;
    sv->length++;
        
}

StringVector* concat(StringVector* sv1, StringVector* sv2) {
    StringVector* sv3 = createStringVector();
    sv3->length = sv1->length + sv2->length;
    sv3->capacity = sv3->length >= sv1->capacity && sv3->length >= sv2->capacity ? sv1->capacity + sv2->capacity : MAX(sv1->capacity, sv2->capacity);
    sv3->length = sv1->length + sv2->length;
    sv3->strings = realloc(sv3->strings, sizeof(char*) * sv3->capacity);
    memcpy(sv3->strings, sv1->strings, sizeof(char*) * sv1->length);
    memcpy(&sv3->strings[sv1->length], sv2->strings, sizeof(char*) * sv2->length);
    return sv3;
}

void printStringVector(StringVector* sv) {
    if (sv->length == 0) {
        printf("@0x%x: [], length: 0, capacity: %d\n", sv, sv->capacity);
    }
    printf("@0x%x: [", sv);
    for (int i = 0; i < sv->length - 1; i++) {
        printf("%s, ", getFromSV(sv, i));
    }
    printf("%s], length: %d, capacity: %d\n", sv->strings[sv->length - 1], sv->length, sv->capacity);
}

StringVector* split(char* line, char* delim) {
    char* copy = strdup(line);
    StringVector* strings = createStringVector();
    char* token = strtok(copy, delim);
    while (token != NULL) {
        addString(strings, token);
        token = strtok(NULL, delim);
    }
    return strings;
}

void trimSV(StringVector* sv) {
    int len;
    char* string;
    for (int i = 0; i < sv->length; i++) {
        string = sv->strings[i];
        len = strlen(string);
        if (string[len - 1] == '\n')
            sv->strings[i][len - 1] = '\0'; // don't use string variable here to overwrite the value in the StringVector
    }
}