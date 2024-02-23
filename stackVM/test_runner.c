#include <stdio.h>
#include <stdlib.h>

#include "test_runner.h";

bool run_test(UnitTest test) {
    bool passed = test.test_func();
    printf("Test '%s' %s\n", test.name, passed ? "passed" : "failed");
    return passed;
}

void test_runner(UnitTest* tests, int length) {
    int failures = 0;
    for (int i = 0; i < length; i++) {
        if (!run_test(tests[i]))
            failures++;
    }
    printf("%d/%d tests passed.\n", length - failures, length);
    if (failures != 0)
        exit(1);
}