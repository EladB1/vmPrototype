#include <string.h>

#include <criterion/criterion.h>
#include <criterion/parameterized.h>

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

ParameterizedTestParameters(DataConstant, toString) {
    size_t count = 8;
    toString_input* values = cr_malloc(sizeof(toString_input) * count);
    values[0].data = createInt(10);
    values[0].representation = "10";
    values[1].data = createDouble(2.78);
    values[1].representation = "2.780000";
    values[2].data = createBoolean(true);
    values[2].representation = "true";
    values[3].data = createBoolean(false);
    values[3].representation = "false";
    values[4].data = createString("Hello");
    values[4].representation = "\"Hello\"";
    values[5].data = createAddr(15, 10, 4);
    values[5].representation = "0xf (10)";
    values[6].data = createNull(); 
    values[6].representation = "null";
    values[7].data = createNone();
    values[7].representation = "None";
    return cr_make_param_array(toString_input, values, count, (void *)cr_free);
}

ParameterizedTest(toString_input* data, DataConstant, toString) {
    char* repr = (*data).representation;
    char* stringValue = toString((*data).data);
    // cr_log_info("%s, %s\n", repr, stringValue);
    cr_expect(strcmp(stringValue, repr) == 0);
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