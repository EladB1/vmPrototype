#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/impl_builtin.h"

#define TESTFILE ".temporary_testing_file.txt"
#define BASE_BYTES sizeof(DataConstant)

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
    setbuf(stdout, NULL);
    print(input->dc, true);
    cr_assert_stdout_eq_str(input->result);
}


Test(impl_builtin, print_empty_array, .init = cr_redirect_stdout) {
        DataConstant* fakeLocals = (DataConstant [1]) {createNone()};
        DataConstant addr = createAddr(fakeLocals, 0, 1, 0);

        setbuf(stdout, NULL);
        print(addr, true);
        cr_assert_stdout_eq_str("[]\n");
}

Test(impl_builtin, print_array, .init = cr_redirect_stdout, .disabled = true) {
        // NOTE: this test fails due to a bug with cr_assert_stdout_eq_str so skipping it
        DataConstant* fakeLocals = (DataConstant [1]) {createInt(5)};
        DataConstant addr = createAddr(fakeLocals, 0, 1, 1);

        setbuf(stdout, NULL);
        print(addr, true);
        logStdout(cr_get_redirected_stdout());
        cr_assert_stdout_eq_str("[5]\n");
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
    DataConstant* fakeLocals = cr_malloc(sizeof(DataConstant) * 2);
    fakeLocals[0] = createInt(4);
    fakeLocals[1] = createInt(2);
    size_t count = 8;
    getTypeInput* values = cr_malloc(sizeof(getTypeInput) * count);
    
    values[0] = (getTypeInput) {createInt(0), cr_strdup("int")};
    values[1] = (getTypeInput) {createDouble(1.4), cr_strdup("double")};
    values[2] = (getTypeInput) {createBoolean(false), cr_strdup("boolean")};
    values[3] = (getTypeInput) {createString("whoami"), cr_strdup("string")};
    values[4] = (getTypeInput) {createNull(), cr_strdup("null")};
    values[5] = (getTypeInput) {createNone(), cr_strdup("None")};
    values[6] = (getTypeInput) {createAddr(fakeLocals, 0, 2, 2), cr_strdup("Array<int>")};
    values[7] = (getTypeInput) {(DataConstant) {8, (DataValue){}, 0, 0, 0}, cr_strdup("Unknown")};
    return cr_make_param_array(getTypeInput, values, count, free_getTypeInput);

}

ParameterizedTest(getTypeInput* input, impl_builtin, getType) {
    char* result = getType(input->dc);
    // cr_log_info("Result: %s\n", result);
    cr_expect_str_eq(result, input->result);
}

Test(impl_builtin, at_invalid, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    char* string = "language";
    char* result = at(string, 10, &vmState);
    cr_assert_stderr_eq_str("IndexError: String index out of range in function call 'at(\"language\", 10)'\n");
    cr_expect_str_empty(result);
    cr_expect_eq(vmState, memory_err);
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
    ExitCode vmState = success;
    char* string = "language";
    char* result = at(string, input->index, &vmState);
    // cr_log_info("Result: %s\n", result);
    cr_expect_str_eq(result, input->result);
    cr_expect_eq(vmState, success);
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

Test(impl_builtin, slice_str_error, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    char* result = slice("Ten", 4, 3, &vmState);
    cr_assert_stderr_eq_str("Invalid start value of slice 4\n");
    cr_expect_str_empty(result);
    cr_expect_eq(vmState, memory_err);
}

Test(impl_builtin, slice_str_valid_full) {
    ExitCode vmState = success;
    char* sliced = slice("Ten", 0, 3, &vmState);
    // cr_log_info("%s", sliced);
    cr_expect_str_eq(sliced, "Ten");
}

Test(impl_builtin, slice_str_valid) {
    ExitCode vmState = success;
    char* sliced = slice("What time is it?", 5, 9, &vmState);
    // cr_log_info("%s", sliced);
    cr_expect_str_eq(sliced, "time");
}

Test(impl_builtin, splitString_doesNotContainDelim) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, 640, 0, 0, NULL);
    bool globalsExpanded = false;

    DataConstant result = splitString("a,b,c", ".", vm, frame, &globalsExpanded, false);

    cr_expect_eq(result.type, Addr);
    cr_expect_eq(result.length, 1);
    cr_expect_eq(result.size, 1);
    cr_expect_eq(result.offset, 0);
    cr_expect_eq(result.value.address, frame->locals);

    cr_expect(isEqual(frame->locals[0], createString("a,b,c")));

    cr_expect_not(globalsExpanded);
    cr_expect_not(frame->expandedLocals);
}

Test(impl_builtin, splitString_containsDelim) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, 640, 0, 0, NULL);
    bool globalsExpanded = false;

    DataConstant result = splitString("a,b,c", ",", vm, frame, &globalsExpanded, false);

    cr_expect_eq(result.type, Addr);
    cr_expect_eq(result.length, 3);
    cr_expect_eq(result.size, 3);
    cr_expect_eq(result.offset, 0);
    cr_expect_eq(result.value.address, frame->locals);

    cr_expect(isEqual(frame->locals[0], createString("a")));
    cr_expect(isEqual(frame->locals[1], createString("b")));
    cr_expect(isEqual(frame->locals[2], createString("c")));

    cr_expect_not(globalsExpanded);
    cr_expect_not(frame->expandedLocals);
}

Test(impl_builtin, splitString_containsDelim_expandLocals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.localsSoftMax = BASE_BYTES * 2;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsSoftMax, 0, 0, NULL);
    bool globalsExpanded = false;

    DataConstant result = splitString("a,b,c", ",", vm, frame, &globalsExpanded, false);

    cr_expect_eq(result.type, Addr);
    cr_expect_eq(result.length, 3);
    cr_expect_eq(result.size, 3);
    cr_expect_eq(result.offset, 0);
    cr_expect_eq(result.value.address, frame->locals);

    cr_expect(isEqual(frame->locals[0], createString("a")));
    cr_expect(isEqual(frame->locals[1], createString("b")));
    cr_expect(isEqual(frame->locals[2], createString("c")));

    cr_expect_not(globalsExpanded);
    cr_expect(frame->expandedLocals);
}

Test(impl_builtin, splitString_containsDelim_useGlobals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.localsHardMax = BASE_BYTES * 2;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    DataConstant result = splitString("a,b,c", ",", vm, frame, &globalsExpanded, false);

    cr_expect_eq(result.type, Addr);
    cr_expect_eq(result.length, 3);
    cr_expect_eq(result.size, 3);
    cr_expect_eq(result.offset, 0);
    cr_expect_eq(result.value.address, vm->globals);

    cr_expect(isEqual(vm->globals[0], createString("a")));
    cr_expect(isEqual(vm->globals[1], createString("b")));
    cr_expect(isEqual(vm->globals[2], createString("c")));

    cr_expect_not(globalsExpanded);
    cr_expect_not(frame->expandedLocals);
}

Test(impl_builtin, splitString_containsDelim_expandGlobals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.localsSoftMax = BASE_BYTES * 2;
    conf.localsHardMax = BASE_BYTES * 2;
    conf.globalsSoftMax = BASE_BYTES * 2;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    DataConstant result = splitString("a,b,c", ",", vm, frame, &globalsExpanded, false);

    cr_expect_eq(result.type, Addr);
    cr_expect_eq(result.length, 3);
    cr_expect_eq(result.size, 3);
    cr_expect_eq(result.offset, 0);
    cr_expect_eq(result.value.address, vm->globals);

    cr_expect(isEqual(vm->globals[0], createString("a")));
    cr_expect(isEqual(vm->globals[1], createString("b")));
    cr_expect(isEqual(vm->globals[2], createString("c")));

    cr_expect(globalsExpanded);
    cr_expect_not(frame->expandedLocals);
}

Test(impl_builtin, splitString_containsDelim_localsError, .init = cr_redirect_stderr) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.useHeapStorageBackup = false;
    conf.localsHardMax = BASE_BYTES;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    DataConstant result = splitString("a,b,c", ",", vm, frame, &globalsExpanded, false);

    cr_expect_eq(result.type, None);

    cr_expect_not(globalsExpanded);
    cr_expect_not(frame->expandedLocals);

    cr_expect_stderr_eq_str("StackOverflow: Exceeded local storage maximum of 1\n");
}

Test(impl_builtin, splitString_containsDelim_globalsError, .init = cr_redirect_stderr) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.localsHardMax = BASE_BYTES;
    conf.globalsHardMax = BASE_BYTES * 2;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    DataConstant result = splitString("a,b,c", ",", vm, frame, &globalsExpanded, false);

    cr_expect_eq(result.type, None);

    cr_expect_not(globalsExpanded);
    cr_expect_not(frame->expandedLocals);

    cr_expect_stderr_eq_str("HeapOverflow: Exceeded local storage maximum of 1 and global storage maximum of 2\n");
}

// File System functions

Test(impl_builtin, createAndDeleteFile, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    cr_expect_not(fileExists(TESTFILE));
    createFile(TESTFILE, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect(fileExists(TESTFILE));
    createFile(TESTFILE, &vmState);
    cr_expect_stderr_eq_str("FileError: Cannot create file '.temporary_testing_file.txt' because it already exists\n");
    cr_expect_eq(vmState, success);
    deleteFile(TESTFILE, &vmState);
    cr_expect_not(fileExists(TESTFILE));
    cr_expect_eq(vmState, success);
}

Test(impl_builtin, deleteFile_nonExistant, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    cr_expect_not(fileExists(TESTFILE));
    deleteFile(TESTFILE, &vmState);
    cr_expect_stderr_eq_str("FileError: Cannot delete file '.temporary_testing_file.txt' because it does not exist\n");
    cr_expect_eq(vmState, file_err);
}

Test(impl_builtin, renameFile_nonExistant, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    cr_expect_not(fileExists(TESTFILE));
    renameFile(TESTFILE, ".new_name.txt", &vmState);
    cr_expect_stderr_eq_str("FileError: Cannot rename file '.temporary_testing_file.txt' because it does not exist\n");
    cr_expect_eq(vmState, file_err);
}

Test(impl_builtin, readFile_nonExistant, .init = cr_redirect_stderr) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VM* vm = init(src, getDefaultConfig());
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, 640, 0, 0, NULL);
    bool globalsExpanded = false;

    cr_expect_not(fileExists(TESTFILE));
    DataConstant read = readFile(TESTFILE, vm, frame, &globalsExpanded, false);
    cr_expect_eq(read.type, None);
    cr_expect_stderr_eq_str("FileError: Cannot read file '.temporary_testing_file.txt' because it does not exist\n");
    cr_expect_eq(vm->state, file_err);
}

Test(impl_builtin, writeAppendReadDeleteFile) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VM* vm = init(src, getDefaultConfig());
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, 640, 0, 0, NULL);
    bool globalsExpanded = false;

    ExitCode vmState = success;
    cr_expect_not(fileExists(TESTFILE));
    
    writeToFile(TESTFILE, "hello", "w", &vmState);
    cr_expect_eq(vmState, success);
    cr_expect(fileExists(TESTFILE));
    DataConstant read1 = readFile(TESTFILE, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read1.length, 1);
    cr_expect_eq(read1.value.address, frame->locals);
    cr_expect_eq(read1.offset, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_str_eq(frame->locals[0].value.strVal, "hello\n");
    
    writeToFile(TESTFILE, "hello", "w", &vmState); // should overwrite file contents
    cr_expect_eq(vmState, success);
    DataConstant read2 = readFile(TESTFILE, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read2.length, 1);
    cr_expect_eq(read2.value.address, frame->locals);
    cr_expect_eq(read2.offset, 1);
    cr_expect_eq(frame->lp, 1);
    cr_expect_str_eq(frame->locals[1].value.strVal, "hello\n");
    
    writeToFile(TESTFILE, "world", "a", &vmState); // should not overwrite file contents
    cr_expect_eq(vmState, success);
    DataConstant read3 = readFile(TESTFILE, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read3.length, 2);
    cr_expect_eq(read3.value.address, frame->locals);
    cr_expect_eq(read3.offset, 2);
    cr_expect_eq(frame->lp, 3);
    cr_expect_str_eq(frame->locals[2].value.strVal, "hello\n");
    cr_expect_str_eq(frame->locals[3].value.strVal, "world\n");
    
    deleteFile(TESTFILE, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect_not(fileExists(TESTFILE));
}

Test(impl_builtin, writeAppendReadDeleteFile_expandLocals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.localsSoftMax = BASE_BYTES * 3;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsSoftMax, 0, 0, NULL);
    bool globalsExpanded = false;

    char* filename = ".tempfile_le.txt";

    ExitCode vmState = success;
    cr_expect_not(fileExists(filename));

    writeToFile(filename, "hello", "w", &vmState);
    cr_expect_eq(vmState, success);
    cr_expect(fileExists(filename));
    DataConstant read1 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read1.length, 1);
    cr_expect_eq(read1.value.address, frame->locals);
    cr_expect_eq(read1.offset, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_str_eq(frame->locals[0].value.strVal, "hello\n");
    cr_expect_not(frame->expandedLocals);

    writeToFile(filename, "world", "a", &vmState); // should not overwrite file contents
    cr_expect_eq(vmState, success);
    DataConstant read2 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read2.length, 2);
    cr_expect_eq(read2.value.address, frame->locals);
    cr_expect_eq(read2.offset, 1);
    cr_expect_eq(frame->lp, 2);
    cr_expect_str_eq(frame->locals[1].value.strVal, "hello\n");
    cr_expect_str_eq(frame->locals[2].value.strVal, "world\n");
    cr_expect(frame->expandedLocals);

    deleteFile(filename, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect_not(fileExists(filename));
}

Test(impl_builtin, writeAppendReadDeleteFile_useGlobals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.localsHardMax = BASE_BYTES;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    char* filename = ".tempfile_g.txt";

    ExitCode vmState = success;
    cr_expect_not(fileExists(filename));

    writeToFile(filename, "hello", "w", &vmState);
    cr_expect_eq(vmState, success);
    cr_expect(fileExists(filename));
    DataConstant read1 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read1.length, 1);
    cr_expect_eq(read1.value.address, frame->locals);
    cr_expect_eq(read1.offset, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_str_eq(frame->locals[0].value.strVal, "hello\n");
    cr_expect_not(frame->expandedLocals);
    cr_expect_not(globalsExpanded);

    writeToFile(filename, "world", "a", &vmState); // should not overwrite file contents
    cr_expect_eq(vmState, success);
    DataConstant read2 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read2.length, 2);
    cr_expect_eq(read2.value.address, vm->globals);
    cr_expect_eq(read2.offset, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_eq(vm->gp, 1);
    cr_expect_str_eq(vm->globals[0].value.strVal, "hello\n");
    cr_expect_str_eq(vm->globals[1].value.strVal, "world\n");
    cr_expect_not(frame->expandedLocals);
    cr_expect_not(globalsExpanded);

    deleteFile(filename, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect_not(fileExists(filename));
}

Test(impl_builtin, writeAppendReadDeleteFile_expandGlobals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.localsSoftMax = BASE_BYTES;
    conf.localsHardMax = BASE_BYTES;
    conf.globalsSoftMax = BASE_BYTES;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsSoftMax, 0, 0, NULL);
    bool globalsExpanded = false;

    char* filename = ".tempfile_ge.txt";

    ExitCode vmState = success;
    cr_expect_not(fileExists(filename));

    writeToFile(filename, "hello", "w", &vmState);
    cr_expect_eq(vmState, success);
    cr_expect(fileExists(filename));
    DataConstant read1 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read1.length, 1);
    cr_expect_eq(read1.value.address, frame->locals);
    cr_expect_eq(read1.offset, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_str_eq(frame->locals[0].value.strVal, "hello\n");
    cr_expect_not(frame->expandedLocals);
    cr_expect_not(globalsExpanded);

    writeToFile(filename, "world", "a", &vmState); // should not overwrite file contents
    writeToFile(filename, "from file", "a", &vmState); // should not overwrite file contents
    cr_expect_eq(vmState, success);
    DataConstant read2 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read2.length, 3);
    cr_expect_eq(read2.value.address, vm->globals);
    cr_expect_eq(read2.offset, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_eq(vm->gp, 2);
    cr_expect_str_eq(vm->globals[0].value.strVal, "hello\n");
    cr_expect_str_eq(vm->globals[1].value.strVal, "world\n");
    cr_expect_str_eq(vm->globals[2].value.strVal, "from file\n");
    cr_expect_not(frame->expandedLocals);
    cr_expect(globalsExpanded);

    deleteFile(filename, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect_not(fileExists(filename));
}

Test(impl_builtin, writeAppendReadDeleteFile_localsError, .init = cr_redirect_stderr) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.useHeapStorageBackup = false;
    conf.localsHardMax = BASE_BYTES;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    char* filename = ".tempfile_error_l.txt";

    ExitCode vmState = success;
    cr_expect_not(fileExists(filename));

    writeToFile(filename, "hello", "w", &vmState);
    cr_expect_eq(vmState, success);
    cr_expect(fileExists(filename));
    DataConstant read1 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read1.length, 1);
    cr_expect_eq(read1.value.address, frame->locals);
    cr_expect_eq(read1.offset, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_str_eq(frame->locals[0].value.strVal, "hello\n");
    cr_expect_not(frame->expandedLocals);
    cr_expect_not(globalsExpanded);

    writeToFile(filename, "world", "a", &vmState); // should not overwrite file contents
    cr_expect_eq(vmState, success);
    DataConstant read2 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, memory_err);
    cr_expect_eq(read2.type, None);
    cr_expect_stderr_eq_str("StackOverflow: Exceeded local storage maximum of 1\n");
    cr_expect_not(frame->expandedLocals);
    cr_expect_not(globalsExpanded);

    deleteFile(filename, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect_not(fileExists(filename));
}

Test(impl_builtin, writeAppendReadDeleteFile_globalsError, .init = cr_redirect_stderr) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.localsHardMax = BASE_BYTES;
    conf.globalsHardMax = BASE_BYTES;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    char* filename = ".tempfile_error_g.txt";

    ExitCode vmState = success;
    cr_expect_not(fileExists(filename));

    writeToFile(filename, "hello", "w", &vmState);
    cr_expect_eq(vmState, success);
    cr_expect(fileExists(filename));
    DataConstant read1 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, success);
    cr_expect_eq(read1.length, 1);
    cr_expect_eq(read1.value.address, frame->locals);
    cr_expect_eq(read1.offset, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_str_eq(frame->locals[0].value.strVal, "hello\n");
    cr_expect_not(frame->expandedLocals);
    cr_expect_not(globalsExpanded);

    writeToFile(filename, "world", "a", &vmState); // should not overwrite file contents
    cr_expect_eq(vmState, success);
    DataConstant read2 = readFile(filename, vm, frame, &globalsExpanded, false);
    cr_expect_eq(vm->state, memory_err);
    cr_expect_eq(read2.type, None);
    cr_expect_stderr_eq_str("HeapOverflow: Exceeded local storage maximum of 1 and global storage maximum of 1\n");
    cr_expect_not(frame->expandedLocals);
    cr_expect_not(globalsExpanded);

    deleteFile(filename, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect_not(fileExists(filename));
}

Test(impl_builtin, createRenameDeleteFile) {
    ExitCode vmState = success;
    char* newName = ".temp_test_file.2.txt";
    cr_expect_not(fileExists(TESTFILE));
    cr_expect_not(fileExists(newName));
    createFile(TESTFILE, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect(fileExists(TESTFILE));
    cr_expect_not(fileExists(newName));
    renameFile(TESTFILE, newName, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect_not(fileExists(TESTFILE));
    cr_expect(fileExists(newName));
    deleteFile(newName, &vmState);
    cr_expect_eq(vmState, success);
    cr_expect_not(fileExists(newName));
}

// Array functions

Test(impl_builtin, reverseArr) {
    DataConstant* locals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(locals, 0, 3, 3);
    reverseArr(array);
    cr_expect_eq(locals[0].value.intVal, -1);
    cr_expect_eq(locals[1].value.intVal, 0);
    cr_expect_eq(locals[2].value.intVal, 2);
}

Test(impl_builtin, sliceArr_valid) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VM* vm = init(src, getDefaultConfig());
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, 640, 0, 0, NULL);
    bool globalsExpanded = false;

    frame->lp = 2;
    frame->locals[0] = createInt(2);
    frame->locals[1] = createInt(0);
    frame->locals[2] = createInt(-1);
    DataConstant array = createAddr(frame->locals, 0, 3, 3);
    DataConstant result = sliceArr(array, 1, 3, vm, frame, &globalsExpanded, false);
    cr_expect_neq(result.type, None);
    cr_expect_eq(result.length, 2);
    cr_expect_eq(result.size, array.size);
    cr_expect_eq((DataConstant *) result.value.address, frame->locals);
    cr_expect_eq(result.offset, 3);
    cr_expect_eq(frame->locals[3].value.intVal, 0);
    cr_expect_eq(frame->locals[4].value.intVal, -1);
}

Test(impl_builtin, sliceArr_invalid, .init = cr_redirect_stderr) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VM* vm = init(src, getDefaultConfig());
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 32000, 64000, 0, 0, NULL);
    bool globalsExpanded = false;

    frame->lp = 2;
    frame->locals[0] = createInt(2);
    frame->locals[1] = createInt(0);
    frame->locals[2] = createInt(-1);
    DataConstant array = createAddr(frame->locals, 0, 3, 3);
    DataConstant sliced = sliceArr(array, 4, 3, vm, frame, &globalsExpanded, false);
    cr_expect_eq(sliced.type, None);
    cr_expect_stderr_eq_str("Array index out of bounds in call to slice. start: 4, end: 3\n");
    cr_expect_eq(vm->state, memory_err);
}

Test(impl_builtin, sliceArr_expandLocals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.localsSoftMax = BASE_BYTES * 4;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsSoftMax, 0, 0, NULL);
    bool globalsExpanded = false;

    frame->lp = 2;
    frame->locals[0] = createInt(2);
    frame->locals[1] = createInt(0);
    frame->locals[2] = createInt(-1);
    DataConstant array = createAddr(frame->locals, 0, 3, 3);
    DataConstant result = sliceArr(array, 1, 3, vm, frame, &globalsExpanded, false);
    cr_expect_neq(result.type, None);
    cr_expect_eq(result.length, 2);
    cr_expect_eq(result.size, array.size);
    cr_expect_eq((DataConstant *) result.value.address, frame->locals);
    cr_expect_eq(result.offset, 3);
    cr_expect_eq(frame->lp, 5);
    cr_expect_eq(frame->locals[3].value.intVal, 0);
    cr_expect_eq(frame->locals[4].value.intVal, -1);
    cr_expect_eq(frame->locals[5].type, None);
    cr_expect(frame->expandedLocals);
    cr_expect_not(globalsExpanded);
    cr_expect_eq(vm->state, success);
}

Test(impl_builtin, sliceArr_useGlobals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.localsHardMax = BASE_BYTES * 4;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    frame->lp = 2;
    frame->locals[0] = createInt(2);
    frame->locals[1] = createInt(0);
    frame->locals[2] = createInt(-1);
    DataConstant array = createAddr(frame->locals, 0, 3, 3);
    DataConstant result = sliceArr(array, 1, 3, vm, frame, &globalsExpanded, false);
    cr_expect_neq(result.type, None);
    cr_expect_eq(result.length, 2);
    cr_expect_eq(result.size, array.size);
    cr_expect_eq((DataConstant *) result.value.address, vm->globals);
    cr_expect_eq(result.offset, 0);
    cr_expect_eq(frame->lp, 2);
    cr_expect_eq(vm->gp, 2);
    cr_expect_eq(vm->globals[0].value.intVal, 0);
    cr_expect_eq(vm->globals[1].value.intVal, -1);
    cr_expect_eq(vm->globals[2].type, None);
    cr_expect_not(frame->expandedLocals);
    cr_expect_not(globalsExpanded);
    cr_expect_eq(vm->state, success);
}

Test(impl_builtin, sliceArr_expandGlobals) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.localsSoftMax = BASE_BYTES * 3;
    conf.localsHardMax = BASE_BYTES * 3;
    conf.globalsSoftMax = BASE_BYTES * 4;
    conf.globalsHardMax = BASE_BYTES * 10;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsSoftMax, 0, 0, NULL);
    bool globalsExpanded = false;

    frame->lp = 2;
    frame->locals[0] = createInt(2);
    frame->locals[1] = createInt(0);
    frame->locals[2] = createInt(-1);
    DataConstant array = createAddr(frame->locals, 0, 3, 3);
    DataConstant result = sliceArr(array, 1, 3, vm, frame, &globalsExpanded, false);
    cr_expect_neq(result.type, None);
    cr_expect_eq(result.length, 2);
    cr_expect_eq(result.size, array.size);
    cr_expect_eq((DataConstant *) result.value.address, vm->globals);
    cr_expect_eq(result.offset, 0);
    cr_expect_eq(frame->lp, 2);
    cr_expect_eq(vm->gp, 2);
    cr_expect_eq(vm->globals[0].value.intVal, 0);
    cr_expect_eq(vm->globals[1].value.intVal, -1);
    cr_expect_eq(vm->globals[2].type, None);
    cr_expect_not(frame->expandedLocals);
    cr_expect(globalsExpanded);
    cr_expect_eq(vm->state, success);
}

Test(impl_builtin, sliceArr_localsError, .init = cr_redirect_stderr) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.useHeapStorageBackup = false;
    conf.localsHardMax = BASE_BYTES * 4;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    frame->lp = 2;
    frame->locals[0] = createInt(2);
    frame->locals[1] = createInt(0);
    frame->locals[2] = createInt(-1);
    DataConstant array = createAddr(frame->locals, 0, 3, 3);
    DataConstant result = sliceArr(array, 1, 3, vm, frame, &globalsExpanded, false);
    cr_expect_eq(result.type, None);
    cr_expect_eq(vm->state, memory_err);
    cr_expect_stderr_eq_str("StackOverflow: Exceeded local storage maximum of 4\n");
}

Test(impl_builtin, sliceArr_globalsError, .init = cr_redirect_stderr) {
    JumpPoint** jumps = {(JumpPoint* [0]) {}};
    SourceCode* src = createSource((char* [1]) {"_entry"}, (char* [1]) {"HALT"}, (int[1]) {0}, jumps, 1);
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    conf.useHeapStorageBackup = true;
    conf.localsHardMax = BASE_BYTES * 3;
    conf.globalsHardMax = BASE_BYTES * 2;
    VM* vm = init(src, conf);
    Frame* frame = loadFrame(createStringVector(), *jumps, 0, 320, conf.localsHardMax, 0, 0, NULL);
    bool globalsExpanded = false;

    frame->lp = 2;
    frame->locals[0] = createInt(2);
    frame->locals[1] = createInt(0);
    frame->locals[2] = createInt(-1);
    DataConstant array = createAddr(frame->locals, 0, 3, 3);
    DataConstant result = sliceArr(array, 1, 3, vm, frame, &globalsExpanded, false);
    cr_expect_eq(result.type, None);
    cr_expect_eq(vm->state, memory_err);
    cr_expect_stderr_eq_str("HeapOverflow: Exceeded local storage maximum of 3 and global storage maximum of 2\n");
}

Test(impl_builtin, arrayContains_true) {
    DataConstant* locals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(locals, 0, 3, 3);
    cr_expect(arrayContains(array, createInt(0)));
}

Test(impl_builtin, arrayContains_false) {
    DataConstant* locals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(locals, 0, 3, 3);
    cr_expect_not(arrayContains(array, createInt(5)));
}

Test(impl_builtin, join_empty) {
    DataConstant locals[0] = {};
    DataConstant array = createAddr(locals, 0, 0, 0);
    char* result = join(array, "");
    cr_expect_str_eq(result, "");
}

Test(impl_builtin, join_non_empty) {
    DataConstant* locals = (DataConstant[2]) {createString("hello"), createString("world")};
    DataConstant array = createAddr(locals, 0, 2, 2);
    char* result = join(array, ", ");
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
    DataConstant* locals = (DataConstant[]) {createString("world"), createString("a"), createString("hello")};
    DataConstant* expected = (DataConstant[]) {createString("a"), createString("hello"), createString("world")};
    cr_expect_not(arraysEqual(expected, locals, 3));
    DataConstant array = createAddr(locals, 0, 3, 3);
    sort(array);
    cr_expect(arraysEqual(expected, locals, 3));
}

Test(impl_builtin, sort_bools) {
    DataConstant* locals = (DataConstant[]) {createBoolean(false), createBoolean(true), createBoolean(true), createNull(), createBoolean(false)};
    DataConstant* expected = (DataConstant[]) {createNull(), createBoolean(false), createBoolean(false), createBoolean(true), createBoolean(true)};
    cr_expect_not(arraysEqual(expected, locals, 5));
    DataConstant array = createAddr(locals, 0, 5, 5);
    sort(array);
    cr_expect(arraysEqual(expected, locals, 5));
}

Test(impl_builtin, sort_ints) {
    DataConstant* locals = (DataConstant[]) {createInt(9), createInt(-5), createInt(0), createInt(0)};
    DataConstant* expected = (DataConstant[]) {createInt(-5), createInt(0), createInt(0), createInt(9)};
    cr_expect_not(arraysEqual(expected, locals, 4));
    DataConstant array = createAddr(locals, 0, 4, 4);
    sort(array);
    cr_expect(arraysEqual(expected, locals, 4));
}

Test(impl_builtin, sort_doubles) {
    DataConstant* locals = (DataConstant[]) {createDouble(0.9), createDouble(-0.0001), createDouble(-0.000099), createDouble(0.00001)};
    DataConstant* expected = (DataConstant[]) {createDouble(-0.0001), createDouble(-0.000099), createDouble(0.00001), createDouble(0.9)};
    cr_expect_not(arraysEqual(expected, locals, 4));
    DataConstant array = createAddr(locals, 0, 4, 4);
    sort(array);
    cr_expect(arraysEqual(expected, locals, 4));
}

Test(impl_builtin, removeByIndex_valid) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(locals, 0, 3, 3);
    removeByIndex(&array, 0, &vmState);
    cr_expect_eq(array.length, 2);
    cr_expect_eq(array.size, 3);
    cr_expect_eq(locals[0].value.intVal, 0);
    cr_expect_eq(locals[1].value.intVal, -1);
    cr_expect_eq(locals[2].type, None);
}

Test(impl_builtin, removeByIndex_invalid, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createInt(2), createInt(0), createInt(-1)};
    DataConstant array = createAddr(locals, 0, 3, 3);
    removeByIndex(&array, 4, &vmState);
    cr_expect_stderr_eq_str("Array index out of bounds\n");
    cr_expect_eq(vmState, memory_err);
}

Test(impl_builtin, append_valid) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createBoolean(false), createNone()};
    DataConstant array = createAddr(locals, 0, 2, 1);
    append(&array, createBoolean(true), &vmState);
    cr_expect_eq(array.size, 2);
    cr_expect_eq(array.length, 2);
    cr_expect_eq(locals[1].type, Bool);
    cr_expect_eq(locals[1].value.boolVal, true);
}

Test(impl_builtin, append_full, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createBoolean(false)};
    DataConstant array = createAddr(locals, 0, 1, 1);
    append(&array, createBoolean(true), &vmState);
    cr_expect_stderr_eq_str("Array size limit 1 reached. Cannot insert into array.\n");
    cr_expect_eq(vmState, memory_err);
}

Test(impl_builtin, prepend_valid) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createBoolean(false), createNone()};
    DataConstant array = createAddr(locals, 0, 2, 1);
    prepend(&array, createBoolean(true), &vmState);
    cr_expect_eq(array.size, 2);
    cr_expect_eq(array.length, 2);
    cr_expect_eq(locals[0].type, Bool);
    cr_expect_eq(locals[0].value.boolVal, true);
}

Test(impl_builtin, prepend_full, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createBoolean(false)};
    DataConstant array = createAddr(locals, 0, 1, 1);
    prepend(&array, createBoolean(true), &vmState);
    cr_expect_stderr_eq_str("Array size limit 1 reached. Cannot insert into array.\n");
    cr_expect_eq(vmState, memory_err);
}

Test(impl_builtin, insert_valid) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createBoolean(false), createBoolean(false), createNone()};
    DataConstant array = createAddr(locals, 0, 3, 2);
    insert(&array, createBoolean(true), 1, &vmState);
    cr_expect_eq(array.size, 3);
    cr_expect_eq(array.length, 3);
    cr_expect_eq(locals[1].type, Bool);
    cr_expect_eq(locals[1].value.boolVal, true);
}

Test(impl_builtin, insert_out_of_range, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createBoolean(false), createBoolean(false), createNone()};
    DataConstant array = createAddr(locals, 0, 3, 2);
    insert(&array, createBoolean(true), 3, &vmState);
    cr_expect_stderr_eq_str("Array index 3 out of range 2\n");
    cr_expect_eq(vmState, memory_err);

}

Test(impl_builtin, insert_full, .init = cr_redirect_stderr) {
    ExitCode vmState = success;
    DataConstant* locals = (DataConstant[]) {createBoolean(false)};
    DataConstant array = createAddr(locals, 0, 1, 1);
    insert(&array, createBoolean(true), 0, &vmState);
    cr_expect_stderr_eq_str("Array size limit 1 reached. Cannot insert into array.\n");
    cr_expect_eq(vmState, memory_err);
}