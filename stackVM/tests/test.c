#include <criterion/criterion.h>

#include "../src/dataconstant.h"

Test(sample, test) {
    cr_expect(2 * 3 == 6);
}

Test(DataConstant, dc_test1) {
    DataConstant num = createInt(4);
    cr_expect(num.value.intVal == 4);
}