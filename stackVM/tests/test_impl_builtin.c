#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/impl_builtin.h"

#define TESTFILE ".temporary_testing_file.txt"

TestSuite(impl_builtin);

typedef struct {
    DataConstant dc;
    char* result;
} getTypeInput;

void free_getTypeInput(struct criterion_test_params* test_params) {
    getTypeInput* params = (getTypeInput*) test_params->params;
    getTypeInput* param;
    for (size_t i = 0; i < test_params->length; i++) {
        param = params + i;
        cr_free(param->result);
    }
    cr_free(test_params->params);
}

ParameterizedTestParameters(impl_builtin, print_non_array) {
    size_t count = 5;
    getTypeInput* values = cr_malloc(sizeof(getTypeInput) * count);
    
    values[0] = (getTypeInput) {createInt(0), cr_strdup("0\n")};
    values[1] = (getTypeInput) {createDouble(1.4), cr_strdup("1.400000\n")};
    values[2] = (getTypeInput) {createBoolean(false), cr_strdup("false\n")};
    values[3] = (getTypeInput) {createNull(), cr_strdup("null\n")};
    values[4] = (getTypeInput) {createString(cr_strdup("whoami")), cr_strdup("whoami\n")};
    return cr_make_param_array(getTypeInput, values, count, free_getTypeInput);
}

/* NOTE: There is a bug with cr_assert_stdout_eq_str that doesn't handle printing without new line characters*/
ParameterizedTest(getTypeInput* input, impl_builtin, print_non_array, .init = cr_redirect_stdout) {
    DataConstant fakeGlobals[0] = {};
    setbuf(stdout, NULL);
    print(input->dc, fakeGlobals, true);
    cr_assert_stdout_eq_str(input->result);
}

Test(impl_builtin, printerr_non_terminating, .init = cr_redirect_stderr, .exit_code = 0) {
    DataConstant message = createString("Could not open socket");
    printerr(message, false, 1);
    cr_assert_stderr_eq_str("Could not open socket\n");
}

Test(impl_builtin, printerr_terminating, .init = cr_redirect_stderr, .exit_code = 1) {
    DataConstant message = createString("Could not open socket");
    printerr(message, true, 1);
    cr_assert_stderr_eq_str("Could not open socket\n");
}

ParameterizedTestParameters(impl_builtin, getType) {
    size_t count = 8;
    getTypeInput* values = cr_malloc(sizeof(getTypeInput) * count);
    
    values[0] = (getTypeInput) {createInt(0), cr_strdup("int")};
    values[1] = (getTypeInput) {createDouble(1.4), cr_strdup("double")};
    values[2] = (getTypeInput) {createBoolean(false), cr_strdup("boolean")};
    values[3] = (getTypeInput) {createString("whoami"), cr_strdup("string")};
    values[4] = (getTypeInput) {createNull(), cr_strdup("null")};
    values[5] = (getTypeInput) {createNone(), cr_strdup("None")};
    values[6] = (getTypeInput) {createAddr(0, 2, 2), cr_strdup("Array<int>")};
    values[7] = (getTypeInput) {(DataConstant) {8, (DataValue){}, 0, 0}, cr_strdup("Unknown")};
    return cr_make_param_array(getTypeInput, values, count, free_getTypeInput);

}

ParameterizedTest(getTypeInput* input, impl_builtin, getType) {
    DataConstant fakeGlobals[2] = {createInt(4), createInt(2)};
    char* result = getType(input->dc, fakeGlobals);
    // cr_log_info("Result: %s\n", result);
    cr_expect_str_eq(result, input->result);
}

Test(impl_builtin, at_invalid, .exit_code = 2, .init = cr_redirect_stderr) {
    char* string = "language";
    at(string, 10);
    cr_assert_stderr_eq_str("IndexError: String index out of range in function call 'at(\"language\", 10)'\n");
}

typedef struct {
    int index;
    char* result;
} atInput;

void free_at_input(struct criterion_test_params* test_params) {
    atInput* params = (atInput*) test_params->params;
    atInput* param;
    for (size_t i = 0; i < test_params->length; i++) {
        param = params + i;
        cr_free(param->result);
    }
    cr_free(test_params->params);
}

ParameterizedTestParameters(impl_builtin, at_valid) {
    size_t count = 3;
    atInput* values = cr_malloc(sizeof(atInput) * count);
    
    values[0] = (atInput) {0, cr_strdup("l")};
    values[1] = (atInput) {1, cr_strdup("a")};
    values[2] = (atInput) {7, cr_strdup("e")};
    
    return cr_make_param_array(atInput, values, count, free_at_input);
}

ParameterizedTest(atInput* input, impl_builtin, at_valid) {
    char* string = "language";
    char* result = at(string, input->index);
    // cr_log_info("Result: %s\n", result);
    cr_expect_str_eq(result, input->result);
}

Test(impl_builtin, startsWith_true) {
    // cr_log_info("%s\n", startsWith_("Javascript", "Java") ? "true" : "false");
    cr_expect(startsWith_("Javascript", "Java"));
}

Test(impl_builtin, startsWith_false) {
    cr_expect(!startsWith_("Typescript", "Java"));
}

Test(impl_builtin, endsWith_true) {
    cr_expect(endsWith("Javascript", "script"));
}

Test(impl_builtin, endsWith_false) {
    cr_expect(!endsWith("Typescript", "Java"));
}

Test(impl_builtin, reverse_empty) {
    cr_expect_str_eq(reverse(""), "");
}

Test(impl_builtin, reverse) {
    char* reversed = reverse("what");
    cr_expect_str_eq("tahw", reversed);
}

Test(impl_builtin, contains_empty) {
    cr_expect(contains("hello", ""));
}

Test(impl_builtin, contains_bigger_false) {
    cr_expect(!contains("hello", "hello, world!"));
}

Test(impl_builtin, contains_true) {
    cr_expect(contains("hello", "ll"));
}

Test(impl_builtin, contains_false) {
    cr_expect(!contains("helLo", "ll"));
}

Test(impl_builtin, replace_single_full) {
    char* replaced = replace("Javascript", "Javascript", "TS", false);
    cr_expect_str_eq(replaced, "TS");
}

Test(impl_builtin, replace_single) {
    char* replaced = replace("Good books look cool", "oo", "-", false);
    cr_expect_str_eq(replaced, "G-d books look cool");
}

Test(impl_builtin, replace_multiple) {
    char* replaced = replace("Good books look cool", "oo", "-", true);
    cr_expect_str_eq(replaced, "G-d b-ks l-k c-l");
}

Test(impl_builtin, replace_single_not_found) {
    char* replaced = replace("Kotlin", "ll", "-", false);
    cr_expect_str_eq(replaced, "Kotlin");
}

Test(impl_builtin, replace_multiple_not_found) {
    char* replaced = replace("Kotlin", "ll", "-", true);
    cr_expect_str_eq(replaced, "Kotlin");
}

Test(impl_builtin, slice_str_error, .exit_code = 2, .init = cr_redirect_stderr) {
    slice("Ten", 4, 3);
    cr_assert_stderr_eq_str("Invalid start value of slice 4\n");
}

Test(impl_builtin, slice_str_valid_full) {
    char* sliced = slice("Ten", 0, 3);
    // cr_log_info("%s", sliced);
    cr_expect_str_eq(sliced, "Ten");
}

Test(impl_builtin, slice_str_valid) {
    char* sliced = slice("What time is it?", 5, 9);
    // cr_log_info("%s", sliced);
    cr_expect_str_eq(sliced, "time");
}

// File System functions

Test(impl_builtin, createAndDeleteFile, .init = cr_redirect_stderr, .exit_code = 0) {
    cr_expect_not(fileExists(TESTFILE));
    createFile(TESTFILE);
    cr_expect(fileExists(TESTFILE));
    createFile(TESTFILE);
    cr_expect_stderr_eq_str("FileError: Cannot create file '.temporary_testing_file.txt' because it already exists\n");
    deleteFile(TESTFILE);
    cr_expect_not(fileExists(TESTFILE));
}

Test(impl_builtin, deleteFile_nonExistant, .init = cr_redirect_stderr, .exit_code = 3) {
    cr_expect_not(fileExists(TESTFILE));
    deleteFile(TESTFILE);
    cr_expect_stderr_eq_str("FileError: Cannot delete file '.temporary_testing_file.txt' because it does not exist\n");
}

Test(impl_builtin, renameFile_nonExistant, .init = cr_redirect_stderr, .exit_code = 3) {
    cr_expect_not(fileExists(TESTFILE));
    renameFile(TESTFILE, ".new_name.txt");
    cr_expect_stderr_eq_str("FileError: Cannot delete file '.temporary_testing_file.txt' because it does not exist\n");
}

Test(impl_builtin, readFile_nonExistant, .init = cr_redirect_stderr, .exit_code = 3) {
    cr_expect_not(fileExists(TESTFILE));
    int globCount = 0;
    DataConstant* globals = (DataConstant[]) {};
    readFile(TESTFILE, &globCount, &globals);
    cr_expect_stderr_eq_str("FileError: Cannot read file '.temporary_testing_file.txt' because it does not exist\n");
}

Test(impl_builtin, writeAppendReadDeleteFile) {
    cr_expect_not(fileExists(TESTFILE));
    int globCount = -1;
    DataConstant* globals = (DataConstant[]) {createNone(), createNone(), createNone(), createNone()}; // need to have a none value in there for test to pass
    
    writeToFile(TESTFILE, "hello", "w");
    cr_expect(fileExists(TESTFILE));
    DataConstant read1 = readFile(TESTFILE, &globCount, &globals);
    cr_expect_eq(read1.length, 1);
    cr_expect_eq(read1.value.intVal, 0);
    cr_expect_eq(globCount, 0);
    cr_expect_str_eq(globals[0].value.strVal, "hello\n");
    
    writeToFile(TESTFILE, "hello", "w"); // should overwrite file contents
    DataConstant read2 = readFile(TESTFILE, &globCount, &globals);
    cr_expect_eq(read2.length, 1);
    cr_expect_eq(read2.value.intVal, 1);
    cr_expect_eq(globCount, 1);
    cr_expect_str_eq(globals[1].value.strVal, "hello\n");
    
    writeToFile(TESTFILE, "world", "a"); // should not overwrite file contents
    DataConstant read3 = readFile(TESTFILE, &globCount, &globals);
    cr_expect_eq(read3.length, 2);
    cr_expect_eq(read3.value.intVal, 2);
    cr_expect_eq(globCount, 3);
    cr_expect_str_eq(globals[2].value.strVal, "hello\n");
    cr_expect_str_eq(globals[3].value.strVal, "world\n");
    
    deleteFile(TESTFILE);
    cr_expect_not(fileExists(TESTFILE));
}

Test(impl_builtin, createRenameDeleteFile) {
    char* newName = ".temp_test_file.2.txt";
    cr_expect_not(fileExists(TESTFILE));
    cr_expect_not(fileExists(newName));
    createFile(TESTFILE);
    cr_expect(fileExists(TESTFILE));
    cr_expect_not(fileExists(newName));
    renameFile(TESTFILE, newName);
    cr_expect_not(fileExists(TESTFILE));
    cr_expect(fileExists(newName));
    deleteFile(newName);
    cr_expect_not(fileExists(newName));
}

// Array functions

Test(impl_builtin, reverseArr) {
    DataConstant* globals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(0, 3, 3);
    reverseArr(array, &globals);
    cr_expect_eq(globals[0].value.intVal, -1);
    cr_expect_eq(globals[1].value.intVal, 0);
    cr_expect_eq(globals[2].value.intVal, 2);
}

Test(impl_builtin, sliceArr_valid) {
    int globCount = 3;
    DataConstant* globals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(0, globCount, globCount);
    DataConstant result = sliceArr(array, 1, 3, &globCount, &globals);
    cr_expect_eq(result.length, 2);
    cr_expect_eq(result.size, array.size);
    cr_expect_eq(result.value.intVal, 4);
    cr_expect_eq(globals[4].value.intVal, 0);
    cr_expect_eq(globals[5].value.intVal, -1);
}

Test(impl_builtin, sliceArr_invalid, .init = cr_redirect_stderr, .exit_code = 2) {
    int globCount = 3;
    DataConstant* globals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(0, globCount, globCount);
    sliceArr(array, 4, 3, &globCount, &globals);
    cr_expect_stderr_eq_str("Array index out of bounds in call to slice. start: 4, end: 3\n");
}

Test(impl_builtin, arrayContains_true) {
    DataConstant* globals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(0, 3, 3);
    cr_expect(arrayContains(array, createInt(0), globals));
}

Test(impl_builtin, arrayContains_false) {
    DataConstant* globals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(0, 3, 3);
    cr_expect_not(arrayContains(array, createInt(5), globals));
}

Test(impl_builtin, join_empty) {
    DataConstant globals[0] = {};
    DataConstant array = createAddr(0, 0, 0);
    char* result = join(array, "", globals);
    cr_expect_str_eq(result, "");
}

Test(impl_builtin, join_non_empty) {
    DataConstant globals[2] = {createString("hello"), createString("world")};
    DataConstant array = createAddr(0, 2, 2);
    char* result = join(array, ", ", globals);
    cr_expect_str_eq(result, "hello, world");
}

bool arraysEqual(DataConstant* array1, DataConstant* array2, int length) {
    for (int i = 0; i < length; i++) {
        if (!isEqual(array1[i], array2[i]))
            return false;
    }
    return true;
}

Test(impl_builtin, sort_strs) {
    DataConstant* globals = (DataConstant[]) {createString("world"), createString("a"), createString("hello")};
    DataConstant* expected = (DataConstant[]) {createString("a"), createString("hello"), createString("world")};
    cr_expect_not(arraysEqual(expected, globals, 3));
    DataConstant array = createAddr(0, 3, 3);
    sort(array, globals);
    cr_expect(arraysEqual(expected, globals, 3));
}

Test(impl_builtin, sort_bools) {
    DataConstant* globals = (DataConstant[]) {createBoolean(false), createBoolean(true), createBoolean(true), createNull(), createBoolean(false)};
    DataConstant* expected = (DataConstant[]) {createNull(), createBoolean(false), createBoolean(false), createBoolean(true), createBoolean(true)};
    cr_expect_not(arraysEqual(expected, globals, 5));
    DataConstant array = createAddr(0, 5, 5);
    sort(array, globals);
    cr_expect(arraysEqual(expected, globals, 5));
}

Test(impl_builtin, sort_ints) {
    DataConstant* globals = (DataConstant[]) {createInt(9), createInt(-5), createInt(0), createInt(0)};
    DataConstant* expected = (DataConstant[]) {createInt(-5), createInt(0), createInt(0), createInt(9)};
    cr_expect_not(arraysEqual(expected, globals, 4));
    DataConstant array = createAddr(0, 4, 4);
    sort(array, globals);
    cr_expect(arraysEqual(expected, globals, 4));
}

Test(impl_builtin, sort_doubles) {
    DataConstant* globals = (DataConstant[]) {createDouble(0.9), createDouble(-0.0001), createDouble(-0.000099), createDouble(0.00001)};
    DataConstant* expected = (DataConstant[]) {createDouble(-0.0001), createDouble(-0.000099), createDouble(0.00001), createDouble(0.9)};
    cr_expect_not(arraysEqual(expected, globals, 4));
    DataConstant array = createAddr(0, 4, 4);
    sort(array, globals);
    cr_expect(arraysEqual(expected, globals, 4));
}

Test(impl_builtin, removeByIndex_valid) {
    DataConstant* globals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(0, 3, 3);
    removeByIndex(&array, 0, &globals);
    cr_expect_eq(array.length, 2);
    cr_expect_eq(array.size, 3);
    cr_expect_eq(globals[0].value.intVal, 0);
    cr_expect_eq(globals[1].value.intVal, -1);
    cr_expect_eq(globals[2].type, None);
}

Test(impl_builtin, removeByIndex_invalid, .init = cr_redirect_stderr, .exit_code = 2) {
    DataConstant* globals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(0, 3, 3);
    removeByIndex(&array, 4, &globals);
    cr_expect_stderr_eq_str("Array index out of bounds\n");
}

Test(impl_builtin, append_valid) {
    DataConstant* globals = (DataConstant[]) {createBoolean(false), createNone()};
    DataConstant array = createAddr(0, 2, 1);
    append(&array, createBoolean(true), &globals);
    cr_expect_eq(array.size, 2);
    cr_expect_eq(array.length, 2);
    cr_expect_eq(globals[1].type, Bool);
    cr_expect_eq(globals[1].value.boolVal, true);
}

Test(impl_builtin, append_full, .init = cr_redirect_stderr, .exit_code = 2) {
    DataConstant* globals = (DataConstant[]) {createBoolean(false)};
    DataConstant array = createAddr(0, 1, 1);
    append(&array, createBoolean(true), &globals);
    cr_expect_stderr_eq_str("Array size limit 1 reached. Cannot insert into array.\n");
}

Test(impl_builtin, prepend_valid) {
    DataConstant* globals = (DataConstant[]) {createBoolean(false), createNone()};
    DataConstant array = createAddr(0, 2, 1);
    prepend(&array, createBoolean(true), &globals);
    cr_expect_eq(array.size, 2);
    cr_expect_eq(array.length, 2);
    cr_expect_eq(globals[0].type, Bool);
    cr_expect_eq(globals[0].value.boolVal, true);
}

Test(impl_builtin, prepend_full, .init = cr_redirect_stderr, .exit_code = 2) {
    DataConstant* globals = (DataConstant[]) {createBoolean(false)};
    DataConstant array = createAddr(0, 1, 1);
    prepend(&array, createBoolean(true), &globals);
    cr_expect_stderr_eq_str("Array size limit 1 reached. Cannot insert into array.\n");
}

Test(impl_builtin, insert_valid) {
    DataConstant* globals = (DataConstant[]) {createBoolean(false), createBoolean(false), createNone()};
    DataConstant array = createAddr(0, 3, 2);
    insert(&array, createBoolean(true), 1, &globals);
    cr_expect_eq(array.size, 3);
    cr_expect_eq(array.length, 3);
    cr_expect_eq(globals[1].type, Bool);
    cr_expect_eq(globals[1].value.boolVal, true);
}

Test(impl_builtin, insert_out_of_range, .init = cr_redirect_stderr, .exit_code = 2) {
    DataConstant* globals = (DataConstant[]) {createBoolean(false), createBoolean(false), createNone()};
    DataConstant array = createAddr(0, 3, 2);
    insert(&array, createBoolean(true), 3, &globals);
    cr_expect_stderr_eq_str("Array index 3 out of range 3.\n");

}

Test(impl_builtin, insert_full, .init = cr_redirect_stderr, .exit_code = 2) {
    DataConstant* globals = (DataConstant[]) {createBoolean(false)};
    DataConstant array = createAddr(0, 1, 1);
    insert(&array, createBoolean(true), 0, &globals);
    cr_expect_stderr_eq_str("Array size limit 1 reached. Cannot insert into array.\n");
}