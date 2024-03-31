#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/frame.h"

TestSuite(Frame);

Test(Frame, loadFrame_noParams) {
    StringVector* srcCode = createStringVector();
    JumpPoint* jumpPoints = NULL;
    Frame* frame = loadFrame(srcCode, jumpPoints, 0, 3, 0, NULL);

    cr_expect_eq(frame->instructions, srcCode);
    cr_expect_eq(frame->returnAddr, 3);
    cr_expect_eq(frame->jumps, NULL);
    cr_expect_eq(frame->jc, 0);
    cr_expect_eq(frame->pc, 0);
    cr_expect_eq(frame->lc, -1);
    cr_expect_eq(frame->sp, -1);

    deleteFrame(frame);
    freeStringVector(srcCode);
}

Test(Frame, frameBasicOperations) {
    StringVector* srcCode = createStringVector();
    JumpPoint* jumpPoints = NULL;
    Frame* frame = loadFrame(srcCode, jumpPoints, 0, 3, 0, NULL);

    cr_expect_eq(frame->pc, 0);
    setPC(frame, 5);
    cr_expect_eq(frame->pc, 5);
    incrementPC(frame);
    cr_expect_eq(frame->pc, 6);

    cr_expect_eq(frame->lc, -1);
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
    cr_expect_eq(frame->lc, 0);
    DataConstant localVal = loadLocal(frame, 0);
    cr_expect(isEqual(frame->locals[0], localVal));
    DataConstant newLocalVal = createBoolean(false);
    storeLocalAtAddr(frame, newLocalVal, 0);
    cr_expect_eq(frame->locals[0].value.boolVal, false);

    deleteFrame(frame);
    freeStringVector(srcCode);
}

Test(Frame, frameWithInstructions) {
    StringVector* srcCode = createStringVector();
    addString(srcCode, "_entry:");
    addString(srcCode, "HALT");
    JumpPoint* jumpPoints = NULL;
    Frame* frame = loadFrame(srcCode, jumpPoints, 0, 0, 0, NULL);

    cr_expect_eq(frame->pc, 0);
    cr_expect_str_eq(peekNextInstruction(frame), "_entry:");
    cr_expect_eq(frame->pc, 0);
    cr_expect_str_eq(getNextInstruction(frame), "_entry:");
    cr_expect_eq(frame->pc, 1);

    deleteFrame(frame);
    freeStringVector(srcCode);
}

Test(Frame, frameWithJumps) {
    StringVector* srcCode = createStringVector();
    JumpPoint* jumpPoints = (JumpPoint[]) {
        {"add", 0},
        {"_entry", 5}
    };
    Frame* frame = loadFrame(srcCode, jumpPoints, 2, 0, 0, NULL);
    int index = getJumpIndex(frame, "_entry");
    cr_expect_eq(index, 5);
    index = getJumpIndex(frame, "notFound");
    cr_expect_eq(index, -1);
    
    deleteFrame(frame);
    freeStringVector(srcCode);
}

Test(Frame, framePrintArray_empty, .init = cr_redirect_stdout) {
    StringVector* srcCode = createStringVector();
    JumpPoint* jumpPoints = NULL;
    Frame* frame = loadFrame(srcCode, jumpPoints, 0, 0, 0, NULL);

    print_array("stack", frame->stack, -1);
    fflush(stdout);
    cr_expect_stdout_eq_str("stack: []\n");

    deleteFrame(frame);
    freeStringVector(srcCode);
}

Test(Frame, framePrintArray_nonEmpty, .init = cr_redirect_stdout) {
    StringVector* srcCode = createStringVector();
    JumpPoint* jumpPoints = NULL;
    Frame* frame = loadFrame(srcCode, jumpPoints, 0, 0, 0, NULL);

    framePush(frame, createInt(5));
    framePush(frame, createInt(21));
    print_array("stack", frame->stack, 1);
    fflush(stdout);
    cr_expect_stdout_eq_str("stack: [5, 21]\n");

    deleteFrame(frame);
    freeStringVector(srcCode);
}