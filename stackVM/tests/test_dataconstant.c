#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "utils.h"
#include "../src/dataconstant.h"

TestSuite(DataConstant);

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

void free_readBool_input(struct criterion_test_params* inputs) {
    readBoolean_input* params = (readBoolean_input*) inputs->params;
    readBoolean_input* param;
    for (size_t i = 0; i < inputs->length; i++) {
        param = params + i;
        cr_free(param->str);
    }
    cr_free(inputs->params);
}

ParameterizedTestParameters(DataConstant, readBoolean) {
    size_t count = 2;
    readBoolean_input* values = cr_malloc(sizeof(readBoolean_input) * count);
    values[0] = (readBoolean_input) {cr_strdup("true"), true};
    values[1] = (readBoolean_input) {cr_strdup("false"), false};
    
    return cr_make_param_array(readBoolean_input, values, count, free_readBool_input);
}

ParameterizedTest(readBoolean_input* bools, DataConstant, readBoolean) {
    DataConstant data = readBoolean(bools->str);
    cr_expect(data.type == Bool);
    cr_expect(data.value.boolVal == bools->value);
}

Test(DataConstant, createBoolean) {
    DataConstant data = createBoolean(true);
    cr_expect(data.type == Bool);
    cr_expect(data.value.boolVal);
}

Test(DataConstant, createString) {
    DataConstant data = createString("Hello, world!");
    cr_expect(data.type == Str);
    cr_expect_str_eq(data.value.strVal, "Hello, world!");
}

Test(DataConstant, createNull) {
    DataConstant data = createNull();
    cr_expect(data.type == Null);
}

Test(DataConstant, createNone) {
    DataConstant data = createNone();
    cr_expect(data.type == None);
}

Test(DataConstant, createAddr) {
    DataConstant data = createAddr(0, 12, 10);
    cr_expect(data.type == Addr);
    cr_expect(data.size == 12);
    cr_expect(data.length == 10);
}

typedef struct {
    DataConstant data;
    char* representation;
} toString_input;

void free_toString_input(struct criterion_test_params* inputs) {
    toString_input* params = (toString_input*) inputs->params;
    toString_input* param;
    for (size_t i = 0; i < inputs->length; i++) {
        param = params + i;
        cr_free(param->representation);
        cr_free(param);
    }
    cr_free(inputs->params);
}

ParameterizedTestParameters(DataConstant, toString) {
    size_t count = 8;
    toString_input* values = cr_malloc(sizeof(toString_input) * count);
    values[0] = (toString_input) {createInt(10), cr_strdup("10")};
    values[1] = (toString_input) {createDouble(2.78), cr_strdup("2.780000")};
    values[2] = (toString_input) {createBoolean(true), cr_strdup("true")};
    values[3] = (toString_input) {createBoolean(false), cr_strdup("false")};
    values[4] = (toString_input) {createString(cr_strdup("Hello")), cr_strdup("\"Hello\"")};
    values[5] = (toString_input) {createAddr(15, 10, 4), cr_strdup("0xf (10)")};
    values[6] = (toString_input) {createNull(), cr_strdup("null")};
    values[7] = (toString_input) {createNone(), cr_strdup("None")};
    return cr_make_param_array(toString_input, values, count, free_toString_input);
}

ParameterizedTest(toString_input* data, DataConstant, toString) {
    cr_log_info("%s, %s\n", toString(data->data), data->representation);
    cr_expect_str_eq(toString(data->data), data->representation);
}

ParameterizedTestParameters(DataConstant, isZero) {
    size_t count = 3;
    DataConstant* values = cr_malloc(sizeof(DataConstant) * count);
    values[0] = createInt(0);
    values[1] = createDouble(0);
    values[2] = createDouble(0.000);
    return cr_make_param_array(DataConstant, values, count, (void *)cr_free);
}

ParameterizedTest(DataConstant* data, DataConstant, isZero) {
    DataConstant literal = *data;
    cr_expect(isZero(literal));
}

ParameterizedTestParameters(DataConstant, isZeroFalse) {
    size_t count = 2;
    DataConstant* values = cr_malloc(sizeof(DataConstant) * count);
    values[0] = createInt(1);
    values[1] = createDouble(0.0001);
    return cr_make_param_array(DataConstant, values, count, (void *)cr_free);
}

ParameterizedTest(DataConstant* data, DataConstant, isZeroFalse) {
    DataConstant literal = *data;
    cr_expect(!isZero(literal));
}