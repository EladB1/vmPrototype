#include <stdio.h>

#include "../test_runner.h"
#include "../dataconstant.h"

bool testAdd_twoInts() {
    DataConstant result = binaryArithmeticOperation(createInt(3), createInt(4), "+");
    return isEqual(result, createInt(7));
}

bool testAdd_twoFloats() {
    DataConstant result = binaryArithmeticOperation(createDouble(3.01), createDouble(4.59), "+");
    return isEqual(result, createDouble(7.6));
}

int main(int argc, char** argv) {
    UnitTest tests[] = {
        {"testAdd(1)", testAdd_twoInts},
        {"testAdd(2)", testAdd_twoFloats}
    };
    test_runner(tests, 2);
    return 0;
}