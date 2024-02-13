#ifndef FILEREADER_H
#define FILEREADER_H

#include "stringvector.h"

typedef struct {
    char* label;
    int index;
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
SourceCode read_file(char* filename);
void displayCode(SourceCode src);

#endif