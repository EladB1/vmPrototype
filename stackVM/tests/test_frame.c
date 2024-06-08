#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/frame.h"

TestSuite(Frame);

StringVector* srcCode;
JumpPoint* jumpPoints;
Frame* test_frame;

void setup() {
    srcCode = createStringVector();
    addString(srcCode, "_entry:");
    addString(srcCode, "HALT");
    jumpPoints = (JumpPoint[]) {
        {"add", 0, 3},
        {"_entry", 5, 9}
    };
    test_frame = loadFrame(srcCode, jumpPoints, 2, 320, 640, 3, 0, NULL);
}

void teardown() {
    deleteFrame(test_frame);
    freeStringVector(srcCode);
}

Test(Frame, loadFrame_noParams, .init = setup, .fini = teardown) {
    cr_expect_eq(test_frame->instructions, srcCode);
    cr_expect_eq(test_frame->returnAddr, 3);
    cr_expect_arr_eq(test_frame->jumps, jumpPoints, 2);
    cr_expect_eq(test_frame->jc, 2);
    cr_expect_eq(test_frame->pc, 0);
    cr_expect_eq(test_frame->lp, -1);
    cr_expect_eq(test_frame->sp, -1);
}

Test(Frame, loadFrame_withParams, .init = setup, .fini = teardown) {
    DataConstant params[2] = {createInt(5), createBoolean(false)};
    test_frame = loadFrame(srcCode, jumpPoints, 2, 640, 640, 3, 2, params);
    cr_expect_eq(test_frame->instructions, srcCode);
    cr_expect_eq(test_frame->returnAddr, 3);
    cr_expect_arr_eq(test_frame->jumps, jumpPoints, 2);
    cr_expect_eq(test_frame->jc, 2);
    cr_expect_eq(test_frame->pc, 0);
    cr_expect_eq(test_frame->lp, 1);
    cr_expect(isEqual(test_frame->locals[0], params[0]));
    cr_expect(isEqual(test_frame->locals[1], params[1]));
    cr_expect_eq(test_frame->sp, -1);
}

Test(Frame, test_frameBasicOperations, .init = setup, .fini = teardown) {
    cr_expect_eq(test_frame->pc, 0);
    setPC(test_frame, 5);
    cr_expect_eq(test_frame->pc, 5);
    incrementPC(test_frame);
    cr_expect_eq(test_frame->pc, 6);

    cr_expect_eq(test_frame->lp, -1);
    cr_expect_eq(test_frame->sp, -1);
    cr_expect(stackIsEmpty(test_frame));
    DataConstant data = createBoolean(true);
    framePush(test_frame, data);
    cr_expect_eq(test_frame->sp, 0);
    cr_expect(!stackIsEmpty(test_frame));

    DataConstant topValue = frameTop(test_frame);
    cr_expect(isEqual(topValue, test_frame->stack[0]));
    DataConstant poppedValue = framePop(test_frame);
    cr_expect_eq(test_frame->sp, -1);
    cr_expect(isEqual(poppedValue, data));

    storeLocal(test_frame, data);
    cr_expect_eq(test_frame->lp, 0);
    DataConstant localVal = loadLocal(test_frame, 0);
    cr_expect(isEqual(test_frame->locals[0], localVal));
    DataConstant newLocalVal = createBoolean(false);
    storeLocalAtAddr(test_frame, newLocalVal, 0);
    cr_expect_eq(test_frame->locals[0].value.boolVal, false);
}

Test(Frame, test_frameWithInstructions, .init = setup, .fini = teardown) {
    cr_expect_eq(test_frame->pc, 0);
    cr_expect_str_eq(peekNextInstruction(test_frame), "_entry:");
    cr_expect_eq(test_frame->pc, 0);
    cr_expect_str_eq(getNextInstruction(test_frame), "_entry:");
    cr_expect_eq(test_frame->pc, 1);
}

Test(Frame, test_frameWithJumps, .init = setup, .fini = teardown) {
    int start = getJumpStart(test_frame, "_entry");
    cr_expect_eq(start, 5);
    int end = getJumpEnd(test_frame, "_entry");
    cr_expect_eq(end, 9);
    start = getJumpStart(test_frame, "notFound");
    cr_expect_eq(start, -1);
    end = getJumpEnd(test_frame, "notFound");
    cr_expect_eq(end, -1);
}

Test(Frame, test_framePrintArray_empty, .init = cr_redirect_stdout) {
    srcCode = createStringVector();
    jumpPoints = NULL;
    test_frame = loadFrame(srcCode, jumpPoints, 0, 32, 32, 0, 0, NULL);

    print_array("stack", test_frame->stack, -1);
    fflush(stdout);
    cr_expect_stdout_eq_str("stack: []\n");

    deleteFrame(test_frame);
    freeStringVector(srcCode);
}

Test(Frame, test_framePrintArray_nonEmpty, .init = cr_redirect_stdout) {
    srcCode = createStringVector();
    jumpPoints = NULL;
    test_frame = loadFrame(srcCode, jumpPoints, 0, 64, 64, 0, 0, NULL);

    framePush(test_frame, createInt(5));
    framePush(test_frame, createInt(21));
    print_array("stack", test_frame->stack, 1);
    fflush(stdout);
    cr_expect_stdout_eq_str("stack: [5, 21]\n");

    deleteFrame(test_frame);
    freeStringVector(srcCode);
}