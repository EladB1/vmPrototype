#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/frame.h"

TestSuite(Frame);

StringVector* srcCode;
JumpPoint* jumpPoints;
Frame* frame;

void setup() {
    srcCode = createStringVector();
    addString(srcCode, "_entry:");
    addString(srcCode, "HALT");
    jumpPoints = (JumpPoint[]) {
        {"add", 0, 3},
        {"_entry", 5, 9}
    };
    frame = loadFrame(srcCode, jumpPoints, 2, 3, 0, NULL);
}

void teardown() {
    deleteFrame(frame);
    freeStringVector(srcCode);
}

Test(Frame, loadFrame_noParams, .init = setup, .fini = teardown) {
    cr_expect_eq(frame->instructions, srcCode);
    cr_expect_eq(frame->returnAddr, 3);
    cr_expect_arr_eq(frame->jumps, jumpPoints, 2);
    cr_expect_eq(frame->jc, 2);
    cr_expect_eq(frame->pc, 0);
    cr_expect_eq(frame->lp, -1);
    cr_expect_eq(frame->sp, -1);
}

Test(Frame, loadFrame_withParams, .init = setup, .fini = teardown) {
    DataConstant params[2] = {createInt(5), createBoolean(false)};
    frame = loadFrame(srcCode, jumpPoints, 2, 3, 2, params);
    cr_expect_eq(frame->instructions, srcCode);
    cr_expect_eq(frame->returnAddr, 3);
    cr_expect_arr_eq(frame->jumps, jumpPoints, 2);
    cr_expect_eq(frame->jc, 2);
    cr_expect_eq(frame->pc, 0);
    cr_expect_eq(frame->lp, 1);
    cr_expect(isEqual(frame->locals[0], params[0]));
    cr_expect(isEqual(frame->locals[1], params[1]));
    cr_expect_eq(frame->sp, -1);
}

Test(Frame, frameBasicOperations, .init = setup, .fini = teardown) {
    cr_expect_eq(frame->pc, 0);
    setPC(frame, 5);
    cr_expect_eq(frame->pc, 5);
    incrementPC(frame);
    cr_expect_eq(frame->pc, 6);

    cr_expect_eq(frame->lp, -1);
    cr_expect_eq(frame->sp, -1);
    cr_expect(stackIsEmpty(frame));
    DataConstant data = createBoolean(true);
    framePush(frame, data);
    cr_expect_eq(frame->sp, 0);
    cr_expect(!stackIsEmpty(frame));

    DataConstant topValue = frameTop(frame);
    cr_expect(isEqual(topValue, frame->stack[0]));
    DataConstant poppedValue = framePop(frame);
    cr_expect_eq(frame->sp, -1);
    cr_expect(isEqual(poppedValue, data));

    storeLocal(frame, data);
    cr_expect_eq(frame->lp, 0);
    DataConstant localVal = loadLocal(frame, 0);
    cr_expect(isEqual(frame->locals[0], localVal));
    DataConstant newLocalVal = createBoolean(false);
    storeLocalAtAddr(frame, newLocalVal, 0);
    cr_expect_eq(frame->locals[0].value.boolVal, false);
}

Test(Frame, frameWithInstructions, .init = setup, .fini = teardown) {
    cr_expect_eq(frame->pc, 0);
    cr_expect_str_eq(peekNextInstruction(frame), "_entry:");
    cr_expect_eq(frame->pc, 0);
    cr_expect_str_eq(getNextInstruction(frame), "_entry:");
    cr_expect_eq(frame->pc, 1);
}

Test(Frame, frameWithJumps, .init = setup, .fini = teardown) {
    int start = getJumpStart(frame, "_entry");
    cr_expect_eq(start, 5);
    int end = getJumpEnd(frame, "_entry");
    cr_expect_eq(end, 9);
    start = getJumpStart(frame, "notFound");
    cr_expect_eq(start, -1);
    end = getJumpEnd(frame, "notFound");
    cr_expect_eq(end, -1);
}

Test(Frame, framePrintArray_empty, .init = cr_redirect_stdout) {
    srcCode = createStringVector();
    jumpPoints = NULL;
    frame = loadFrame(srcCode, jumpPoints, 0, 0, 0, NULL);

    print_array("stack", frame->stack, -1);
    fflush(stdout);
    cr_expect_stdout_eq_str("stack: []\n");

    deleteFrame(frame);
    freeStringVector(srcCode);
}

Test(Frame, framePrintArray_nonEmpty, .init = cr_redirect_stdout) {
    srcCode = createStringVector();
    jumpPoints = NULL;
    frame = loadFrame(srcCode, jumpPoints, 0, 0, 0, NULL);

    framePush(frame, createInt(5));
    framePush(frame, createInt(21));
    print_array("stack", frame->stack, 1);
    fflush(stdout);
    cr_expect_stdout_eq_str("stack: [5, 21]\n");

    deleteFrame(frame);
    freeStringVector(srcCode);
}