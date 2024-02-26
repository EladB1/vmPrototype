#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "utils.h"
#include "../src/impl_builtin.h"

TestSuite(impl_builtin);

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