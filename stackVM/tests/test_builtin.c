#include <stdbool.h>
#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/builtin.h"

TestSuite(builtin);

typedef struct {
    char* functionName;
    bool result;
} isBuiltinInput;

ParameterizedTestParameters(builtin, isBuiltinFunction) {
    size_t count = 2;
    isBuiltinInput* values = cr_malloc(sizeof(isBuiltinInput) * count);

    values[0] = (isBuiltinInput) {cr_strdup("asdf"), false};
    values[1] = (isBuiltinInput) {cr_strdup("print"), true};
    values[2] = (isBuiltinInput) {cr_strdup("_toInt_s"), true};

    return cr_make_param_array(isBuiltinInput, values, count);
}

ParameterizedTest(isBuiltinInput* input, builtin, isBuiltinFunction) {
    bool actual = isBuiltinFunction(input->functionName);
    cr_assert_eq(actual, input->result);
}

// Test any builtins that aren't just calls to functions in impl_builtin.c
// Or any branching calls

// printerr
Test(builtin, non_terminating_printerr, .exit_code = 0, .init = cr_redirect_stderr) {
    DataConstant string = createString("An error occured");
    callBuiltinFunction("printerr", 1, &string, (int *) 0, NULL);
    cr_assert_stderr_eq_str("An error occured\n");
}

Test(builtin, terminating_printerr, .exit_code = 5, .init = cr_redirect_stderr) {
    DataConstant params[3] = {createString("An error occured"), createBoolean(true), createInt(5)};
    callBuiltinFunction("printerr", 3, params, (int *) 0, NULL);
    cr_assert_stderr_eq_str("An error occured\n");
}

// _length_s
Test(builtin, string_length) {
    DataConstant string = createString("Hello");
    DataConstant length = callBuiltinFunction("_length_s", 1, &string, (int *) 0, NULL);
    cr_expect_eq(length.value.intVal, 5);
}

// slice
Test(builtin, slice_string_two_params) {
    DataConstant params[2] = {createString("Hello"), createInt(1)};
    DataConstant result = callBuiltinFunction("_slice_s", 2, params, (int *) 0, NULL);
    cr_expect_str_eq(result.value.strVal, "ello");
}

Test(builtin, slice_string_three_params) {
    DataConstant params[3] = {createString("Hello"), createInt(1), createInt(3)};
    DataConstant result = callBuiltinFunction("_slice_s", 3, params, (int *) 0, NULL);
    cr_expect_str_eq(result.value.strVal, "el");
}

Test(builtin, slice_array_two_params) {
    DataConstant* globals = (DataConstant[]) {createInt(4), createInt(2), createInt(1)};
    int globCount = 3;
    DataConstant params[2] = {createAddr(0, globCount, globCount), createInt(1)};
    DataConstant result = callBuiltinFunction("_slice_a", 2, params, &globCount, &globals);
    cr_expect_eq(result.length, 2);
    cr_expect_eq(result.value.intVal, 4);
}

Test(builtin, slice_array_three_params) {
    DataConstant* globals = (DataConstant[]) {createInt(8), createInt(4), createInt(2), createInt(1)};
    int globCount = 4;
    DataConstant params[3] = {createAddr(0, globCount, globCount), createInt(1), createInt(3)};
    DataConstant result = callBuiltinFunction("_slice_a", 3, params, &globCount, &globals);
    cr_expect_eq(result.length, 2);
    cr_expect_eq(result.value.intVal, 5);
}

// _remove_val_a
Test(builtin, remove_value_array_not_found) {
    int globCount = 2;
    DataConstant* globals = (DataConstant[]) {createDouble(3.14), createDouble(2.718)};
    DataConstant params[2] = {createAddr(0, 2, 2), createDouble(-9.8)};
    DataConstant result = callBuiltinFunction("_remove_val_a", 2, params, &globCount, &globals);
    cr_expect_eq(result.length, params[0].length); // nothing happens
}

Test(builtin, remove_value_array_found) {
    int globCount = 2;
    DataConstant* globals = (DataConstant[]) {createDouble(3.14), createDouble(2.718)};
    DataConstant params[2] = {createAddr(0, 2, 2), createDouble(2.718)};
    DataConstant result = callBuiltinFunction("_remove_val_a", 2, params, &globCount, &globals);
    cr_expect_eq(result.length, 1);
}

// _remove_all_val_a
Test(builtin, remove_all_values_array_found) {
    DataConstant math_e = createDouble(2.718);
    int globCount = 5;
    DataConstant* globals = (DataConstant[]) {math_e, createDouble(3.14), math_e, math_e, createDouble(-9.8)};
    DataConstant params[2] = {createAddr(0, 5, 5), math_e};
    DataConstant result = callBuiltinFunction("_remove_all_val_a", 2, params, &globCount, &globals);
    cr_expect_eq(result.length, 2);
    cr_expect_eq(globals[0].type, Dbl);
    cr_expect_eq(globals[0].value.dblVal, 3.14);
    cr_expect_eq(globals[1].type, Dbl);
    cr_expect_eq(globals[1].value.dblVal, -9.8);
    cr_expect_eq(globals[2].type, None);
}

// join
Test(builtin, join_single_param) {
    int globCount = 3;
    DataConstant* globals = (DataConstant[]) {createString("a"), createString("b"), createString("c")};
    DataConstant params[1] = {createAddr(0, globCount, globCount)};
    DataConstant result = callBuiltinFunction("join", 1, params, &globCount, &globals);
    cr_expect_str_eq(result.value.strVal, "abc");
}

Test(builtin, join_multiple_params) {
    int globCount = 3;
    DataConstant* globals = (DataConstant[]) {createString("a"), createString("b"), createString("c")};
    DataConstant params[2] = {createAddr(0, globCount, globCount), createString(",")};
    DataConstant result = callBuiltinFunction("join", 2, params, &globCount, &globals);
    cr_expect_str_eq(result.value.strVal, "a,b,c");
}

// exit
Test(builtin, exit_no_params, .exit_code = 0) {
    callBuiltinFunction("exit", 0, NULL, 0, NULL);
}

Test(builtin, exit__with_param, .exit_code = 1) {
    DataConstant exitCode = createInt(1);
    callBuiltinFunction("exit", 1, &exitCode, 0, NULL);
}
