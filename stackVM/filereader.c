#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "filereader.h" 

bool startsWith(char* in, char chr) {
    for (int i = 0; i < strlen(in); i++) {
        if (in[i] != ' ' &&  in[i] != '\t') {
            return in[i] == chr;
        }
    }
    return false;
}

SourceCode read_file() {
    SourceCode code;
    code.length = 0;
    Function func;
    FILE* fptr;
    fptr = fopen("input.txt", "r");
    StringVector* line;
    StringVector* out = createStringVector();
    const unsigned int MAX_LENGTH = 256;
    char buff[MAX_LENGTH];
    printf("Reading file...\n");
    while (fgets(buff, MAX_LENGTH, fptr)) {
        if (startsWith(buff, ';'))
            continue;
        else if (buff[strlen(buff) - 2] == ':') {
            func.label = getFromSV(split(buff, ":\n"), 0);
        }
        else if (strcmp(buff, "\n") == 0) {
            func.body = out;
            out = createStringVector();
            code.code[code.length++] = func;
        }
        else {
            line = split(buff, " ");
            trimSV(line);
            out = concat(out, line);
            free(line);
        }
    }
    func.body = out;
    code.code[code.length++] = func;
    fclose(fptr);
    return code;
}