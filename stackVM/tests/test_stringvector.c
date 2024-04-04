#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/stringvector.h"

TestSuite(StringVector);

StringVector* sv;

void setupSV() {
    sv = createStringVector();
}

void teardownSV() {
    freeStringVector(sv);
}

Test(StringVector, stringVector_addAndGet, .init = setupSV, .fini = teardownSV) {
    cr_expect_eq(sv->capacity, 256);
    cr_expect_eq(sv->length, 0);
    
    addString(sv, "hello");
    cr_expect_eq(sv->length, 1);
    addString(sv, "world");
    cr_expect_eq(sv->length, 2);

    cr_expect_str_eq(getFromSV(sv, 0), "hello");
    cr_expect_str_eq(getFromSV(sv, 1), "world");
}

Test(StringVector, stringVector_expands, .init = setupSV, .fini = teardownSV) {
    cr_expect_eq(sv->capacity, 256);
    cr_expect_eq(sv->length, 0);
    
    for (int i = 0; i < 256; i++) {
        addString(sv, " ");
    }
    cr_expect_eq(sv->length, 256);
    cr_expect_eq(sv->capacity, 512);
}

Test(StringVector, stringVector_concat, .init = setupSV, .fini = teardownSV) {
    cr_expect_eq(sv->capacity, 256);
    cr_expect_eq(sv->length, 0);

    StringVector* sv2 = createStringVector();
    cr_expect_eq(sv->capacity, 256);
    cr_expect_eq(sv->length, 0);
    
    addString(sv, "hello");
    cr_expect_eq(sv->length, 1);
    addString(sv2, "world");
    cr_expect_eq(sv2->length, 1);

    StringVector* combined = concat(sv, sv2);
    cr_expect_str_eq(getFromSV(combined, 0), "hello");
    cr_expect_str_eq(getFromSV(combined, 1), "world");
    cr_expect_eq(combined->capacity, 256);
    cr_expect_eq(combined->length, 2);

    freeStringVector(sv2);
    freeStringVector(combined);
}

Test(StringVector, printStringVector_empty, .init = cr_redirect_stdout) {
    StringVector* sv = createStringVector();
    char* expected_stdout;
    cr_asprintf(&expected_stdout, "@%p: [], length: 0, capacity: 256\n", sv);
    printStringVector(sv);
    fflush(stdout);
    cr_expect_stdout_eq_str(expected_stdout);
    freeStringVector(sv);
}

Test(StringVector, printStringVector_nonEmpty, .init = cr_redirect_stdout) {
    StringVector* sv = createStringVector();
    addString(sv, "a");
    addString(sv, "b");
    addString(sv, "c");
    char* expected_stdout;
    cr_asprintf(&expected_stdout, "@%p: [a, b, c], length: 3, capacity: 256", sv);
    printStringVector(sv);
    fflush(stdout);
    cr_expect_stdout_eq_str(expected_stdout);
    freeStringVector(sv);
}

Test(StringVector, stringVector_split, .init = setupSV, .fini = teardownSV) {
    sv = split("Hello,World,Space,string,etc.", ",");
    cr_expect_eq(sv->length, 5);
    cr_expect_str_eq(getFromSV(sv, 0), "Hello");
    cr_expect_str_eq(getFromSV(sv, 1), "World");
    cr_expect_str_eq(getFromSV(sv, 2), "Space");
    cr_expect_str_eq(getFromSV(sv, 3), "string");
    cr_expect_str_eq(getFromSV(sv, 4), "etc.");
}

Test(StringVector, stringVector_splitWithQuotes, .init = setupSV, .fini = teardownSV) {
    sv = split("LOAD_CONST \"Hello\" DUP", " ");
    cr_expect_eq(sv->length, 3);
    cr_expect_str_eq(getFromSV(sv, 0), "LOAD_CONST");
    cr_expect_str_eq(getFromSV(sv, 1), "\"Hello\"");
    cr_expect_str_eq(getFromSV(sv, 2), "DUP");
}


Test(StringVector, stringVector_trim, .init = setupSV, .fini = teardownSV) {
    sv = createStringVector();
    addString(sv, "HALT\n");
    addString(sv, "DUP\n");
    addString(sv, "ADD ");
    cr_expect_str_eq(getFromSV(sv, 0), "HALT\n");
    cr_expect_str_eq(getFromSV(sv, 1), "DUP\n");
    cr_expect_str_eq(getFromSV(sv, 2), "ADD ");
    trimSV(sv);
    cr_expect_str_eq(getFromSV(sv, 0), "HALT");
    cr_expect_str_eq(getFromSV(sv, 1), "DUP");
    cr_expect_str_eq(getFromSV(sv, 2), "ADD ");

}