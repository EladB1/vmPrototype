#include <string.h>

#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "../src/impl_builtin.h"

TestSuite(impl_builtin);

typedef struct {
    int index;
    char* result;
} at_input;

ParameterizedTestParameters(impl_builtin, at_valid) {
    static at_input values[] = {{0, "l"}, {1, "a"}, {7, "e"}};
    size_t count = sizeof(values) / sizeof(at_input);

    return cr_make_param_array(at_input, values, count);
}

ParameterizedTest(at_input* input, impl_builtin, at_valid) {
    char* string = "language";
    char* result = at(string, (*input).index);
    cr_expect(strcmp(result, (*input).result) == 0);
}

Test(impl_builtin, startsWith_true) {
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
    cr_expect(strcmp("tahw", reversed) == 0);
}

Test(impl_builtin, contains_true) {
    cr_expect(contains("hello", "ll"));
}

Test(impl_builtin, contains_false) {
    cr_expect(!contains("helLo", "ll"));
}

Test(impl_builtin, slice_str) {
    char* sliced = slice("What time is it?", 5, 9);
    // cr_log_info("%s\n", sliced);
    cr_expect(strcmp(sliced, "time") == 0);
}