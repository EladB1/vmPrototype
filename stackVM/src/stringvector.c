#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
        printf("@%p: [], length: 0, capacity: %d\n", sv, sv->capacity);
        return;
    }
    printf("@%p: [", sv);
    for (int i = 0; i < sv->length - 1; i++) {
        printf("%s, ", getFromSV(sv, i));
    }
    printf("%s], length: %d, capacity: %d", sv->strings[sv->length - 1], sv->length, sv->capacity);
}

StringVector* splitExceptQuotes(char* line, char* delim) {
    StringVector* strings = createStringVector();
    bool in_word = false;
    bool in_quotes = false;
    bool matching = false;
    char prev, curr;
    char* token = "";
    int k;
    int end = strlen(line) + 1;
    int delimEnd = strlen(delim);
    for (int i = 0; i < end; i++) {
        prev = curr;
        curr = line[i];
        if (!in_word && !matching && curr != delim[0]) {
            in_word = true;
            if (curr == '"')
                in_quotes = true;
            asprintf(&token, "%c", curr);
            continue;
        }
        if (in_word) {
            if ((!in_quotes && curr != delim[0]) || in_quotes)
                asprintf(&token, "%s%c", token, curr);
            if (!in_quotes && curr == delim[0]) {
                matching = true;
                k = i + 1;
                for (int j = 1; j < delimEnd && k < end; j++, k++) {
                    if (delim[j] != line[k]) {
                        matching = false;
                        break;
                    }
                    matching = true;
                }
                if (matching) {
                    addString(strings, token);
                    token = "";
                    in_word = false;
                }
            }
        }
        if (in_quotes && prev != '\\' && curr == '"') {
            addString(strings, token);
            token = "";
            in_quotes = false;
        }
        if (!in_word && matching && curr != delim[0]) {
            in_word = true;
            if (curr == '"')
                in_quotes = true;
            asprintf(&token, "%c", curr);
            matching = false;
        }
    }
    return strings;
} 

StringVector* split(char* line, char* delim) {
    char* copy = strdup(line);
    StringVector* strings = createStringVector();
    if (strchr(copy, '"') != NULL) {
        strings = splitExceptQuotes(line, delim);
    }
    else {
        char* token = strtok(copy, delim);
        while (token != NULL) {
            addString(strings, token);
            if (token[0] == '"') {
                printf("Token: %s\n", token);
                break;
            }
            token = strtok(NULL, delim);
        }
    }
    return strings;
}

void trimSV(StringVector* sv) {
    int len;
    char* string;
    for (int i = 0; i < sv->length; i++) {
        string = strdup(sv->strings[i]);
        len = strlen(string);
        if (string[len - 1] == '\n') {
            string[len - 1] = '\0';
            sv->strings[i] = string;
        }
    }
}