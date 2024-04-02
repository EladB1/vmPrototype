#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/filereader.h"

TestSuite(FileReader);

Test(FileReader, startsWith_true) {
    cr_expect(startsWith("ABC", 'A'));
}

Test(FileReader, startsWith_false) {
    cr_expect(!startsWith("ABC", 'B'));
}

Test(FileReader, startsWith_spaces) {
    cr_expect(startsWith("  \t\tABC", 'A'));
}

Test(FileReader, startsWith_spaces_false) {
    cr_expect(!startsWith("  \t\tABC", 'B'));
}

Test(FileReader, startsWith_newLine) {
    cr_expect(startsWith("\nABC", '\n'));
}


Test(FileReader, read_file_notFound, .init = cr_redirect_stderr, .exit_code = 255) {
    read_file("./tests/notFoundFile");
    cr_expect_stderr_eq_str("Error\nCause: ./tests/notFoundFile\n");
}

Test(FileReader, read_file) {
    SourceCode src = read_file("./tests/sample_source_file.txt");

    cr_expect_eq(src.length, 2);
    cr_expect_str_eq(src.code[0].label, "mult");
    cr_expect_str_eq(src.code[1].label, "_entry");
    cr_expect_eq(src.code[0].jmpCnt, 0);
    cr_expect_eq(src.code[1].jmpCnt, 1);
    cr_expect_str_eq(src.code[1].jumpPoints[0].label, ".stop");
    cr_expect_eq(src.code[1].jumpPoints[0].index, 17);
    
    StringVector* multBody = src.code[0].body;
    StringVector* mainBody = src.code[1].body;

    cr_expect_eq(multBody->length, 6);
    cr_expect_eq(mainBody->length, 19);
}

Test(FileReader, displayCode, .init = cr_redirect_stdout) {
    SourceCode src;
    src.length = 2;
    StringVector* addBody = split("LOAD 0 LOAD 1 ADD RET", " ");
    StringVector* mainBody = split("LOAD_CONST 1 LOAD_CONST 5 LE JPMT .end LOAD_CONST 3 CALL println 1 .end: HALT", " ");
    src.code[0].label = "add";
    src.code[0].body = addBody;
    src.code[0].jmpCnt = 0;
    src.code[1].label = "_entry";
    src.code[1].body = mainBody;
    src.code[1].jumpPoints[0] = (JumpPoint) {".end", 14};
    src.code[1].jmpCnt = 1;
    
    char* expected_output;
    cr_asprintf(
        &expected_output,
        "length: 2\nadd => @%p: [LOAD, 0, LOAD, 1, ADD, RET], length: 6, capacity: 256, Jumps: []\n_entry => @%p: [LOAD_CONST, 1, LOAD_CONST, 5, LE, JPMT, .end, LOAD_CONST, 3, CALL, println, 1, .end:, HALT], length: 14, capacity: 256, Jumps: [{.end : 14} ]\n", 
        addBody, 
        mainBody
    );
    displayCode(src);
    fflush(stdout);
    cr_expect_stdout_eq_str(expected_output);
    
    freeStringVector(addBody);
    freeStringVector(mainBody);
}