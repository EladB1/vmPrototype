#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/config.h"
#include "../src/dataconstant.h"

TestSuite(Config);

typedef struct {
    char* inputValue;
    long outputValue;
} ProcessValueValidInput;

ParameterizedTestParameters(Config, processValue_valid) {
    size_t count = 4;
    ProcessValueValidInput* values = cr_malloc(sizeof(ProcessValueValidInput) * count);
    values[0] = (ProcessValueValidInput) {cr_strdup("100"), 100};
    values[1] = (ProcessValueValidInput) {cr_strdup("2K"), 2048};
    values[2] = (ProcessValueValidInput) {cr_strdup("4M"), 1 << 22};
    values[3] = (ProcessValueValidInput) {cr_strdup("1G"), 1 << 30};
    return cr_make_param_array(ProcessValueValidInput, values, count);

}

ParameterizedTest(ProcessValueValidInput* values, Config, processValue_valid) {
    cr_expect_eq(processValue(values->inputValue, "file.txt", 1), values->outputValue);
}

typedef struct {
    char* inputValue;
    char badChar;
    int badCharIndex;
} ProcessValueInvalidInput;

ParameterizedTestParameters(Config, processValue_invalid) {
    size_t count = 4;
    ProcessValueInvalidInput* values = cr_malloc(sizeof(ProcessValueInvalidInput) * count);
    values[0] = (ProcessValueInvalidInput) {cr_strdup("-5"), '-', 0};
    values[1] = (ProcessValueInvalidInput) {cr_strdup("5H"), 'H', 1};
    values[2] = (ProcessValueInvalidInput) {cr_strdup("2.5"), '.', 1};
    values[3] = (ProcessValueInvalidInput) {cr_strdup("125T"), 'T', 3};
    return cr_make_param_array(ProcessValueInvalidInput, values, count);
}

ParameterizedTest(ProcessValueInvalidInput* values, Config, processValue_invalid, .init = cr_redirect_stderr) {
    char* file = "file.yml";
    int line = 23;
    char* error;
    cr_asprintf(&error, "ConfigError: Invalid character '%c' found\nFile: 'file.yml', Line: 23, Index: %d\n", values->badChar, values->badCharIndex);
    long result = processValue(values->inputValue, file, line);
    //logStderr(cr_get_redirected_stderr());
    cr_expect_eq(result, -1);
    cr_expect_stderr_eq_str(error);
}

Test(Config, readConfigFile_valid) {
    VMConfig conf = readConfigFile("tests/valid-config.yml");
    //displayVMConfig(conf);
    cr_expect(conf.dynamicResourceExpansionEnabled);
    cr_expect_not(conf.useHeapStorageBackup);
    cr_expect_eq(conf.framesSoftMax, 512);
    cr_expect_eq(conf.framesHardMax, 1024);
    cr_expect_eq(conf.stackSizeSoftMax, 2048);
    cr_expect_eq(conf.stackSizeHardMax, 1 << 14);
    cr_expect_eq(conf.localsSoftMax, 1 << 18);
    cr_expect_eq(conf.localsHardMax, 1 << 20);
    cr_expect_eq(conf.globalsSoftMax, 1 << 30);
    cr_expect_eq(conf.globalsHardMax, (long) 1 << 31);
}

Test(Config, readConfigFile_notFound, .init = cr_redirect_stderr) {
    VMConfig conf = readConfigFile("some-config.yml");
    cr_expect_stderr_eq_str("Warning: No such file or directory\nCause: 'some-config.yml'\n");
    // returns default config
    VMConfig defaultConf = getDefaultConfig();
    cr_expect_eq(conf.dynamicResourceExpansionEnabled, defaultConf.dynamicResourceExpansionEnabled);
    cr_expect_eq(conf.useHeapStorageBackup, defaultConf.useHeapStorageBackup);
    cr_expect_eq(conf.framesSoftMax, defaultConf.framesSoftMax);
    cr_expect_eq(conf.framesHardMax, defaultConf.framesHardMax);
    cr_expect_eq(conf.stackSizeSoftMax, defaultConf.stackSizeSoftMax);
    cr_expect_eq(conf.stackSizeHardMax, defaultConf.stackSizeHardMax);
    cr_expect_eq(conf.localsSoftMax, defaultConf.localsSoftMax);
    cr_expect_eq(conf.localsHardMax, defaultConf.localsHardMax);
    cr_expect_eq(conf.globalsSoftMax, defaultConf.globalsSoftMax);
    cr_expect_eq(conf.globalsHardMax, defaultConf.globalsHardMax);
}

Test(Config, displayVMConfig_default, .init = cr_redirect_stdout) {
    VMConfig conf = getDefaultConfig();
    displayVMConfig(conf);
    fflush(stdout);
    //logStdout(cr_get_redirected_stdout());
    char* displayValues = "DynamicResourceExpansion: enabled\nHeapStorageBackup: enabled\n";
    cr_asprintf(&displayValues, "%sframes_soft_max: 512 frames\n", displayValues);
    cr_asprintf(&displayValues, "%sframes_hard_max: 1024 frames\n", displayValues);
    cr_asprintf(&displayValues, "%sstack_size_soft_max: 1024 B (32 values)\n", displayValues);
    cr_asprintf(&displayValues, "%sstack_size_hard_max: 8192 B (256 values)\n", displayValues);
    cr_asprintf(&displayValues, "%slocals_soft_max: 65536 B (2048 values)\n", displayValues);
    cr_asprintf(&displayValues, "%slocals_hard_max: 131072 B (4096 values)\n", displayValues);
    cr_asprintf(&displayValues, "%sglobals_soft_max: 1048576 B (32768 values)\n", displayValues);
    cr_asprintf(&displayValues, "%sglobals_hard_max: 536870912 B (16777216 values)\n", displayValues);
    cr_asprintf(&displayValues, "%sEstimated VM memory usage: 33.50 MB (soft limits) - 648.00 MB (hard limits)\n", displayValues);
    cr_expect_stdout_eq_str(displayValues);
}

Test(Config, displayVMConfig_noDRE, .init = cr_redirect_stdout) {
    VMConfig conf = getDefaultConfig();
    conf.dynamicResourceExpansionEnabled = false;
    displayVMConfig(conf);
    fflush(stdout);
    //logStdout(cr_get_redirected_stdout());
    char* displayValues = "DynamicResourceExpansion: disabled\nHeapStorageBackup: enabled\n";
    cr_asprintf(&displayValues, "%sframes_soft_max: 512 frames\n", displayValues);
    cr_asprintf(&displayValues, "%sframes_hard_max: 1024 frames\n", displayValues);
    cr_asprintf(&displayValues, "%sstack_size_soft_max: 1024 B (32 values)\n", displayValues);
    cr_asprintf(&displayValues, "%sstack_size_hard_max: 8192 B (256 values)\n", displayValues);
    cr_asprintf(&displayValues, "%slocals_soft_max: 65536 B (2048 values)\n", displayValues);
    cr_asprintf(&displayValues, "%slocals_hard_max: 131072 B (4096 values)\n", displayValues);
    cr_asprintf(&displayValues, "%sglobals_soft_max: 1048576 B (32768 values)\n", displayValues);
    cr_asprintf(&displayValues, "%sglobals_hard_max: 536870912 B (16777216 values)\n", displayValues);
    cr_asprintf(&displayValues, "%sEstimated VM memory usage: 648.00 MB\n", displayValues);
    cr_expect_stdout_eq_str(displayValues);
}

Test(Config, validateVMConfig_valid) {
    char* filePath = "tests/valid-config.yml";
    VMConfig conf = readConfigFile(filePath);
    cr_expect(validateVMConfig(conf, filePath));
}

Test(Config, validateVMConfig_invalid, .init = cr_redirect_stderr) {
    char* filePath = "tests/invalid-config.yml";
    VMConfig conf = readConfigFile(filePath);
    cr_expect_not(validateVMConfig(conf, filePath));
    char* error = "ConfigError: 'locals_soft_max' must be between 32 and 1048576 in config file: 'tests/invalid-config.yml'\n";
    cr_asprintf(&error, "%sConfigError: 'stack_size_soft_max' must be less than or equal to 'stack_size_hard_max' in config file: 'tests/invalid-config.yml'\n", error);
    cr_asprintf(&error, "%sConfigError: 'globals_soft_max' must be less than or equal to 'globals_hard_max' in config file: 'tests/invalid-config.yml'\n", error);
    cr_asprintf(&error, "%sConfigError: 'frames_hard_max' must be between 1 and 16384 in config file: 'tests/invalid-config.yml'\n", error);
    cr_asprintf(&error, "%sConfigError: 'locals_hard_max' must be between 32 and 1048576 in config file: 'tests/invalid-config.yml'\n", error);
    cr_expect_stderr_eq_str(error);
}