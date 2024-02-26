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
} readBooleanInput;

void free_readBool_input(struct criterion_test_params* inputs) {
    readBooleanInput* params = (readBooleanInput*) inputs->params;
    readBooleanInput* param;
    for (size_t i = 0; i < inputs->length; i++) {
        param = params + i;
        cr_free(param->str);
    }
    cr_free(inputs->params);
}

ParameterizedTestParameters(DataConstant, readBoolean) {
    size_t count = 2;
    readBooleanInput* values = cr_malloc(sizeof(readBooleanInput) * count);
    values[0] = (readBooleanInput) {cr_strdup("true"), true};
    values[1] = (readBooleanInput) {cr_strdup("false"), false};
    
    return cr_make_param_array(readBooleanInput, values, count, free_readBool_input);
}

ParameterizedTest(readBooleanInput* bools, DataConstant, readBoolean) {
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
} toStringInput;

void free_toString_input(struct criterion_test_params* inputs) {
    toStringInput* params = (toStringInput*) inputs->params;
    toStringInput* param;
    for (size_t i = 0; i < inputs->length; i++) {
        param = params + i;
        cr_free(param->representation);
        cr_free(param);
    }
    cr_free(inputs->params);
}

ParameterizedTestParameters(DataConstant, toString) {
    size_t count = 8;
    toStringInput* values = cr_malloc(sizeof(toStringInput) * count);
    values[0] = (toStringInput) {createInt(10), cr_strdup("10")};
    values[1] = (toStringInput) {createDouble(2.78), cr_strdup("2.780000")};
    values[2] = (toStringInput) {createBoolean(true), cr_strdup("true")};
    values[3] = (toStringInput) {createBoolean(false), cr_strdup("false")};
    values[4] = (toStringInput) {createString(cr_strdup("Hello")), cr_strdup("\"Hello\"")};
    values[5] = (toStringInput) {createAddr(15, 10, 4), cr_strdup("0xf (10)")};
    values[6] = (toStringInput) {createNull(), cr_strdup("null")};
    values[7] = (toStringInput) {createNone(), cr_strdup("None")};
    return cr_make_param_array(toStringInput, values, count, free_toString_input);
}

ParameterizedTest(toStringInput* data, DataConstant, toString) {
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