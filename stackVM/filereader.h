#ifndef FILEREADER_H
#define FILEREADER_H

#include "stringvector.h"

typedef struct {
    char* label;
    StringVector* body;
} Function;

typedef struct {
    int length;
    Function code[256];
} SourceCode;

SourceCode read_file(char* filename);
void displayCode(SourceCode src);

#endif