#ifndef FILEREADER_H
#define FILEREADER_H

#include <stdbool.h>

#include "stringvector.h"

typedef struct {
    char* label;
    int start;
    int end;
} JumpPoint;

typedef struct {
    char* label;
    StringVector* body;
    JumpPoint jumpPoints[16];
    int jmpCnt;
} Function;

typedef struct {
    int length;
    Function code[256];
} SourceCode;

bool startsWith(char* in, char chr);
SourceCode* read_file(char* filename);
void displayCode(SourceCode* src);
void deleteSourceCode(SourceCode* src);

#endif