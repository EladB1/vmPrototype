#include <string.h>

#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "../src/dataconstant.h"

Test(DataConstant, readInt) {
    DataConstant data = readInt("4");
    cr_expect(data.type == Int);
    cr_expect(data.value.intVal == 4);
}

Test(DataConstant, createInt) {
    DataConstant data = createInt(4);
    cr_expect(data.type == Int);
    cr_expect(data.value.intVal == 4);
}

Test(DataConstant, readDouble) {
    DataConstant data = readDouble("4.5");
    cr_expect(data.type == Dbl);
    cr_expect(data.value.dblVal == 4.5);
}

ParameterizedTestParameters(DataConstant, createDouble) {
    static double values[] = {4, -4.01};
    size_t count = sizeof(values) / sizeof(double);

    return cr_make_param_array(double, values, count);
}

ParameterizedTest(double* dbl, DataConstant, createDouble) {
    DataConstant data = createDouble(*dbl);
    cr_expect(data.type == Dbl);
    cr_expect(data.value.dblVal == *dbl);
}

typedef struct {
    char* str;
    bool value;
} readBoolean_input;

ParameterizedTestParameters(DataConstant, readBoolean) {
    static readBoolean_input values[] = {{"true", true}, {"false", false}};
    size_t count = sizeof(values) / sizeof(readBoolean_input);

    return cr_make_param_array(readBoolean_input, values, count);
}

ParameterizedTest(readBoolean_input* bools, DataConstant, readBoolean) {
    DataConstant data = readBoolean((*bools).str);
    cr_expect(data.type == Bool);
    cr_expect(data.value.boolVal == (*bools).value);
}

Test(DataConstant, createBoolean) {
    DataConstant data = createBoolean(true);
    cr_expect(data.type == Bool);
    cr_expect(data.value.boolVal);
}

Test(DataConstant, createString) {
    DataConstant data = createString("Hello, world!");
    cr_expect(data.type == Str);
    cr_expect(strcmp(data.value.strVal, "Hello, world!") == 0);
}

Test(DataConstant, createNull) {
    DataConstant data = createNull();
    cr_expect(data.type == Null);
}

Test(DataConstant, createNone) {
    DataConstant data = createNone();
    cr_expect(data.type == None);
}