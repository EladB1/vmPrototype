#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "filereader.h" 

bool startsWith(char* in, char chr) {
    int len = (int) strlen(in);
    for (int i = 0; i < len; i++) {
        if (in[i] != ' ' &&  in[i] != '\t') {
            return in[i] == chr;
        }
    }
    return false;
}

SourceCode* read_file(char* filename) {
    SourceCode* code = malloc(sizeof(SourceCode));
    code->length = 0;
    Function func;
    JumpPoint jmp;
    FILE* fp;
    func.jmpCnt = 0;
    int count;
    fp = fopen(filename, "r");
    if (fp == NULL || ferror(fp)) {
        perror("Error");
        fprintf(stderr, "Cause: '%s'\n", filename);
        return NULL;
    }
    StringVector* line;
    StringVector* out = createStringVector();
    const unsigned int MAX_LENGTH = 256;
    bool prevWasBlank = false;
    char buff[MAX_LENGTH];
    bool inJump = false;
    while (fgets(buff, MAX_LENGTH, fp)) {
        if (startsWith(buff, ';'))
            continue;
        else if (!startsWith(buff, '.') && buff[strlen(buff) - 2] == ':') {
            prevWasBlank = false;
            func.label = getFromSV(split(buff, ":\n"), 0);
        }
        else if (startsWith(buff, '.') && buff[strlen(buff) - 2] == ':') {
            inJump = true;
            line = split(buff, ":\n");
            line = split(getFromSV(line, 0), " ");
            trimSV(line);
            jmp.label = getFromSV(line, 0);
            jmp.start = count;
            func.jumpPoints[func.jmpCnt++] = jmp;
        }
        else if (strcmp(buff, "\n") == 0) {
            if (!prevWasBlank) {
                func.body = out;
                out = createStringVector();
                code->code[code->length++] = func;
                count = 0;
                func.jmpCnt = 0;
                prevWasBlank = true;
            }
            continue;
        }
        else {
            line = split(buff, " ");
            trimSV(line);
            if (inJump && strcmp(getFromSV(line, 0), "EJMP") == 0) {
                func.jumpPoints[func.jmpCnt - 1].end = count;
                inJump = false;
            }
            count += line->length;
            out = concat(out, line);
            free(line);
        }
    }
    func.body = out;
    code->code[code->length++] = func;
    fclose(fp);
    return code;
}

void displayCode(SourceCode* src) {
    JumpPoint jmp;
    printf("length: %d\n", src->length);
    for (int i = 0; i < src->length; i++) {
        printf("%s => ", src->code[i].label);
        printStringVector(src->code[i].body);
        printf(", Jumps: [");
        for (int j = 0; j < src->code[i].jmpCnt; j++) {
            jmp = src->code[i].jumpPoints[j];
            printf("{%s : %d - %d} ", jmp.label, jmp.start, jmp.end);
        }
        printf("]\n");
    }
}

void deleteSourceCode(SourceCode* src) {
    for (int i = 0; i < src->length; i++) {
        freeStringVector(src->code[i].body);
    }
    free(src);
}