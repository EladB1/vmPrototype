#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

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
    DataConstant* locals = (DataConstant[]) {createBoolean(true)};
    DataConstant data = createAddr(locals, 1, 12, 10);
    cr_expect_eq(data.type, Addr);
    cr_expect_eq((DataConstant *) data.value.address, locals);
    cr_expect_eq(data.offset, 1);
    cr_expect_eq(data.size, 12);
    cr_expect_eq(data.length, 10);
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
    DataConstant* locals = (DataConstant[]) {};
    char* arrayToString;
    cr_asprintf(&arrayToString, "%p (10)", locals + 15);
    size_t count = 8;
    toStringInput* values = cr_malloc(sizeof(toStringInput) * count);

    values[0] = (toStringInput) {createInt(10), cr_strdup("10")};
    values[1] = (toStringInput) {createDouble(2.78), cr_strdup("2.780000")};
    values[2] = (toStringInput) {createBoolean(true), cr_strdup("true")};
    values[3] = (toStringInput) {createBoolean(false), cr_strdup("false")};
    values[4] = (toStringInput) {createString(cr_strdup("Hello")), cr_strdup("\"Hello\"")};
    values[5] = (toStringInput) {createNull(), cr_strdup("null")};
    values[6] = (toStringInput) {createNone(), cr_strdup("None")};
    values[7] = (toStringInput) {createAddr(locals, 15, 10, 1), cr_strdup(arrayToString)};
    cr_free(arrayToString);
    return cr_make_param_array(toStringInput, values, count, free_toString_input);
}

ParameterizedTest(toStringInput* data, DataConstant, toString) {
    // cr_log_info("%s, %s\n", toString(data->data), data->representation);
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

typedef struct {
    DataConstant lhs;
    DataConstant rhs;
    bool result;
} EqualityComparison;

ParameterizedTestParameters(DataConstant, isEqual) {
    size_t count = 9;
    EqualityComparison* values = cr_malloc(sizeof(EqualityComparison) * count);
    values[0] = (EqualityComparison) {createInt(0), createInt(0), true};
    values[1] = (EqualityComparison) {createInt(1), createInt(-1), false};
    values[2] = (EqualityComparison) {createInt(10), createDouble(10.000), true};
    values[3] = (EqualityComparison) {createDouble(5.0), createInt(5), true};
    values[4] = (EqualityComparison) {createInt(0), createNull(), false};
    values[5] = (EqualityComparison) {createString(cr_strdup("hello")), createString(cr_strdup("hello")), true};
    values[6] = (EqualityComparison) {createString(cr_strdup("hello")), createString(cr_strdup("hello, world")), false};
    values[7] = (EqualityComparison) {createString(cr_strdup("hello")), createString(cr_strdup("h3llo")), false};
    values[8] = (EqualityComparison) {createNull(), createNull(), true};
    return cr_make_param_array(EqualityComparison, values, count);
}

ParameterizedTest(EqualityComparison* compare, DataConstant, isEqual) {
    cr_expect_eq(isEqual(compare->lhs, compare->rhs), compare->result);
}

typedef struct {
    DataConstant lhs;
    DataConstant rhs;
    char* operator;
    bool result;
} Comparison;

ParameterizedTestParameters(DataConstant, compareData) {
    size_t count = 10;
    Comparison* values = cr_malloc(sizeof(Comparison) * count);
    values[0] = (Comparison) {createInt(0), createInt(0), cr_strdup("=="), true};
    values[1] = (Comparison) {createInt(1), createInt(-1), cr_strdup("=="), false};
    values[2] = (Comparison) {createInt(1), createInt(-1), cr_strdup("!="), true};
    values[3] = (Comparison) {createInt(0), createInt(0), cr_strdup("!="), false};
    values[4] = (Comparison) {createInt(10), createDouble(10.000), cr_strdup("<="), true};
    values[5] = (Comparison) {createDouble(5.0), createInt(5), cr_strdup(">="), true};
    values[6] = (Comparison) {createDouble(5.01), createInt(5), cr_strdup(">"), true};
    values[7] = (Comparison) {createDouble(5.01), createDouble(5.02), cr_strdup(">"), false};
    values[8] = (Comparison) {createInt(10), createInt(11), cr_strdup("<"), true};
    values[9] = (Comparison) {createInt(12), createInt(11), cr_strdup("<"), false};
    return cr_make_param_array(Comparison, values, count);
}

ParameterizedTest(Comparison* compare, DataConstant, compareData) {
    DataConstant result = compareData(compare->lhs, compare->rhs, compare->operator);
    char* logMessage = "compareData(%s, %s, \"%s\") Result: %s; Expected result: %s\n";
    char* expectedResult = compare->result ? "true" : "false";
    cr_log_info(logMessage, toString(compare->lhs),toString(compare->rhs), compare->operator, toString(result), expectedResult);
    cr_expect_eq(result.value.boolVal, compare->result);
}

Test(DataConstant, getMax_LHS) {
    DataConstant lhs = createInt(4);
    DataConstant rhs = createDouble(3.99);
    DataConstant result = getMax(lhs, rhs);
    cr_expect_eq(lhs.type, result.type);
    cr_expect_eq(lhs.value.intVal, result.value.intVal);
}

Test(DataConstant, getMax_RHS) {
    DataConstant lhs = createDouble(3.98);
    DataConstant rhs = createDouble(3.99);
    DataConstant result = getMax(lhs, rhs);
    cr_expect_eq(rhs.type, result.type);
    cr_expect_eq(rhs.value.dblVal, result.value.dblVal);
}

Test(DataConstant, getMin_LHS) {
    DataConstant lhs = createInt(4);
    DataConstant rhs = createInt(4);
    DataConstant result = getMin(lhs, rhs);
    cr_expect_eq(lhs.type, result.type);
    cr_expect_eq(lhs.value.intVal, result.value.intVal);
}

Test(DataConstant, getMin_RHS) {
    DataConstant lhs = createDouble(3.98);
    DataConstant rhs = createDouble(3.97);
    DataConstant result = getMin(lhs, rhs);
    cr_expect_eq(rhs.type, result.type);
    cr_expect_eq(rhs.value.dblVal, result.value.dblVal);
}

typedef struct {
    DataConstant lhs;
    DataConstant rhs;
    char* operator;
    DataConstant result;
} Operation;

ParameterizedTestParameters(DataConstant, binaryArithmeticOperation) {
    size_t count = 12;
    Operation* values = cr_malloc(sizeof(Operation) * count);
    values[0] = (Operation) {createInt(0), createInt(0), cr_strdup("+"), createInt(0)};
    values[1] = (Operation) {createInt(1), createDouble(-1.0), cr_strdup("+"), createDouble(0)};
    values[2] = (Operation) {createInt(1), createInt(-1.0), cr_strdup("-"), createInt(2)};
    values[3] = (Operation) {createInt(0), createInt(5), cr_strdup("-"), createInt(-5)};
    values[4] = (Operation) {createInt(10), createDouble(10.000), cr_strdup("*"), createDouble(100.0)};
    values[5] = (Operation) {createDouble(5.0), createInt(3), cr_strdup("*"), createDouble(15.0)};
    values[6] = (Operation) {createInt(6), createInt(5), cr_strdup("/"), createInt(1)};
    values[7] = (Operation) {createDouble(6.0), createDouble(5), cr_strdup("/"), createDouble(1.2)};
    values[8] = (Operation) {createInt(6), createInt(5), cr_strdup("mod"), createInt(1)};
    values[9] = (Operation) {createInt(6), createDouble(11.0), cr_strdup("mod"), createDouble(6.0)};
    values[10] = (Operation) {createInt(3), createInt(-1), cr_strdup("exp"), createInt(0)};
    values[11] = (Operation) {createDouble(3.0), createInt(-2), cr_strdup("exp"), createDouble(1.0 / 9.0)};
    return cr_make_param_array(Operation, values, count);
}

ParameterizedTest(Operation* operation, DataConstant, binaryArithmeticOperation) {
    DataConstant result = binaryArithmeticOperation(operation->lhs, operation->rhs, operation->operator);
    char* logMessage = "binaryArithmeticOperation(%s, %s, \"%s\") Result: %s; Expected result: %s\n";
    cr_log_info(logMessage, toString(operation->lhs),toString(operation->rhs), operation->operator, toString(result), toString(operation->result));
    cr_expect_eq(result.type, operation->result.type);
    cr_expect(isEqual(result, operation->result));
}

Test(DataConstant, binaryArithmeticOperation_divByZero, .init = cr_redirect_stderr) {
    DataConstant result = binaryArithmeticOperation(createInt(1), createInt(0), "/");
    cr_expect_stderr_eq_str("Error: Division by zero\n");
    cr_expect_eq(result.type, None);
}

Test(DataConstant, binaryArithmeticOperation_modByZero, .init = cr_redirect_stderr) {
    DataConstant result = binaryArithmeticOperation(createInt(1), createInt(0), "mod");
    cr_expect_stderr_eq_str("Error: Division by zero\n");
    cr_expect_eq(result.type, None);
}

Test(DataConstant, binaryArithmeticOperation_expZeroByNegative, .init = cr_redirect_stderr) {
    DataConstant result = binaryArithmeticOperation(createInt(0), createInt(-1), "exp");
    cr_expect_stderr_eq_str("Error: Zero cannot be raised to a negative power\n");
    cr_expect_eq(result.type, None);
}

Test(DataConstant, copyAddr) {
    DataConstant* globals = (DataConstant[4]){createInt(1), createInt(5)};
    int globIndex = 1;
    DataConstant addr = createAddr(globals, 0, 2, 2);
    DataConstant copy = copyAddr(addr, &globIndex, &globals);
    cr_expect_eq(copy.type, Addr);
    cr_expect_eq(copy.size, 2);
    cr_expect_eq(copy.length, 2);
    cr_expect_eq((DataConstant *) copy.value.address, globals);
    cr_expect_eq(copy.offset, 2);
    cr_expect(isEqual(globals[2], globals[0]));
    cr_expect(isEqual(globals[3], globals[1]));
    cr_expect_eq(globIndex, 3);
}

Test(DataConstant, partialCopyAddr) {
    DataConstant* globals = (DataConstant[4]){createInt(1), createInt(5)};
    int globIndex = 1;
    DataConstant addr = createAddr(globals, 0, 2, 2);
    DataConstant copy = partialCopyAddr(addr, 0, 1, &globIndex, &globals);
    cr_expect_eq(copy.type, Addr);
    cr_expect_eq(copy.size, 2);
    cr_expect_eq(copy.length, 1);
    cr_expect_eq((DataConstant *) copy.value.address, globals);
    cr_expect_eq(copy.offset, 2);
    cr_expect(isEqual(globals[2], globals[0]));
    cr_expect_eq(globals[3].type, None);
    cr_expect_eq(globIndex, 3);
}

Test(DataConstant, expandExistingAddr) {
    DataConstant* globals = (DataConstant[6]){createInt(1), createInt(5)};
    int globIndex = 1;
    DataConstant addr = createAddr(globals, 0, 2, 2);
    DataConstant copy = expandExistingAddr(addr, 4, &globIndex, &globals);
    cr_expect_eq(copy.type, Addr);
    cr_expect_eq(copy.size, 4);
    cr_expect_eq(copy.length, 2);
    cr_expect_eq((DataConstant *) copy.value.address, globals);
    cr_expect_eq(copy.offset, 2);
    cr_expect(isEqual(globals[2], globals[0]));
    cr_expect(isEqual(globals[3], globals[1]));
    cr_expect_eq(globals[4].type, None);
    cr_expect_eq(globals[5].type, None);
    cr_expect_eq(globIndex, 5);
}