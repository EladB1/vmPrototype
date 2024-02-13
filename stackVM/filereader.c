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

SourceCode read_file(char* filename) {
    SourceCode code;
    code.length = 0;
    Function func;
    JumpPoint jmp;
    FILE* fp;
    func.jmpCnt = 0;
    int count;
    fp = fopen(filename, "r");
    if (fp == NULL || ferror(fp)) {
        char msg[128];
        sprintf(msg, "Input Error('%s')", filename);
        perror(msg);
        exit(-1);
    }
    StringVector* line;
    StringVector* out = createStringVector();
    const unsigned int MAX_LENGTH = 256;
    char buff[MAX_LENGTH];
    while (fgets(buff, MAX_LENGTH, fp)) {
        if (startsWith(buff, ';'))
            continue;
        else if (!startsWith(buff, '.') && buff[strlen(buff) - 2] == ':') {
            func.label = getFromSV(split(buff, ":\n"), 0);
        }
        else if (startsWith(buff, '.') && buff[strlen(buff) - 2] == ':') {
            line = split(buff, ":\n");
            line = split(getFromSV(line, 0), " ");
            trimSV(line);
            jmp.label = getFromSV(line, 0);
            jmp.index = count;
            func.jumpPoints[func.jmpCnt++] = jmp;
        }
        else if (strcmp(buff, "\n") == 0) {
            func.body = out;
            out = createStringVector();
            code.code[code.length++] = func;
            count = 0;
            func.jmpCnt = 0;
            continue;
        }
        else {
            line = split(buff, " ");
            trimSV(line);
            count += line->length;
            out = concat(out, line);
            free(line);
        }
    }
    func.body = out;
    code.code[code.length++] = func;
    fclose(fp);
    return code;
}

void displayCode(SourceCode src) {
    JumpPoint jmp;
    printf("length: %d\n", src.length);
    for (int i = 0; i < src.length; i++) {
        printf("%s => ", src.code[i].label);
        printStringVector(src.code[i].body);
        printf("Jumps: [");
        for (int j = 0; j < src.code[i].jmpCnt; j++) {
            jmp = src.code[i].jumpPoints[j];
            printf("{%s : %d} ", jmp.label, jmp.index);
        }
        printf("]\n");
    }
}