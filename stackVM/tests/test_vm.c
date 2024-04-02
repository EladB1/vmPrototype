#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/vm.h"

TestSuite(VM);

Test(VM, initAndDestroy) {
    SourceCode src;
    src.length = 2;
    StringVector* addBody = split("LOAD 0 LOAD 1 ADD RET", " ");
    StringVector* mainBody = split("LOAD_CONST 1 LOAD_CONST 5 LE JPMT .end LOAD_CONST 3 CALL println 1 .end: HALT", " ");
    src.code[0].label = "add";
    src.code[0].body = addBody;
    src.code[0].jmpCnt = 0;
    src.code[1].label = "_entry";
    src.code[1].body = mainBody;
    src.code[1].jumpPoints[0] = (JumpPoint) {".end", 14};
    src.code[1].jmpCnt = 1;

    VM* vm = init(src);

    cr_expect_eq(vm->fp, 0);
    cr_expect_eq(vm->gc, -1);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->returnAddr, 0);
    cr_expect_eq(frame->pc, 0);
    cr_expect_eq(frame->sp, -1);
    cr_expect_eq(frame->jc, 1);
    cr_expect_eq(frame->lc, -1);

    destroy(vm);
    free(addBody);
    free(mainBody);
}