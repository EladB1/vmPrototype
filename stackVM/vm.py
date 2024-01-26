from enum import Enum

class OC(Enum):
    HALT=1
    PUSH=2
    ADD=3
    SUB=4
    

class VM:
    def __init__(self, code, pc: int=0):
        self.stack = []
        self.pc = pc
        self.fp = 0
        self.locals = []
        self.code = code

    def run(self):
        print("Starting program...")
        while True:
            print(self.stack)
            if self.code[self.pc] == OC.HALT:
                print("Halting execution")
                return
            if self.code[self.pc] == OC.PUSH:
                self.stack.append(self.code[self.pc + 1])
                self.pc += 2
                continue
            if self.code[self.pc] == OC.ADD:
                if len(self.stack) >= 2:
                    rhs = self.stack.pop()
                    lhs = self.stack.pop()
                    self.stack.append(lhs + rhs)
            if self.code[self.pc] == OC.SUB:
                if len(self.stack) >= 2:
                    rhs = self.stack.pop()
                    lhs = self.stack.pop()
                    self.stack.append(lhs - rhs)
            self.pc += 1
        print(self.stack)
    

if __name__ == '__main__':
    code = [
        OC.PUSH,
        5,
        OC.PUSH,
        -5,
        OC.ADD,
        OC.PUSH,
        10,
        OC.SUB,
        OC.HALT
    ]
    vm = VM(code)
    vm.run()