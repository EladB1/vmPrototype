#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "utils.h"
#include "../src/impl_builtin.h"

TestSuite(impl_builtin);

typedef struct {
    int index;
    char* result;
} at_input;

void free_at_input(struct criterion_test_params* test_params) {
    at_input* params = (at_input*) test_params->params;
    at_input* param;
    for (size_t i = 0; i < test_params->length; i++) {
        param = params + i;
        cr_free(param->result);
    }
    cr_free(test_params->params);
}

ParameterizedTestParameters(impl_builtin, at_valid) {
    size_t count = 3;
    at_input* values = cr_malloc(sizeof(at_input) * count);
    
    values[0] = (at_input) {0, cr_strdup("l")};
    values[1] = (at_input) {1, cr_strdup("a")};
    values[2] = (at_input) {7, cr_strdup("e")};
    
    return cr_make_param_array(at_input, values, count, free_at_input);
}

ParameterizedTest(at_input* input, impl_builtin, at_valid) {
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

Test(impl_builtin, reverse) {
    char* reversed = reverse("what");
    cr_expect_str_eq("tahw", reversed);
}

Test(impl_builtin, contains_true) {
    cr_expect(contains("hello", "ll"));
}

Test(impl_builtin, contains_false) {
    cr_expect(!contains("helLo", "ll"));
}

Test(impl_builtin, slice_str) {
    char* sliced = slice("What time is it?", 5, 9);
    // cr_log_info("%s", sliced);
    cr_expect_str_eq(sliced, "time");
}