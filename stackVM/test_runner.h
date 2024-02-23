#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include <stdbool.h>

typedef struct {
    char* name;
    bool (*test_func)();
} UnitTest;

bool run_test(UnitTest test);
void test_runner();

#endif