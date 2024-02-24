CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -lm
SOURCES=src/*.c
TESTS=tests/*.c
TARGET=bolt
TESTSRC=src/dataconstant.c src/frame.c src/builtin.c src/impl_builtin.c src/stringvector.c
TESTBUILD=tests/runtests

default: clean $(TARGET)

$(TARGET): $(SOURCES-.c=.o)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

$(TESTBUILD): $(TESTS- .c=.o)
	$(CC) $(CFLAGS) $< -lcriterion -o $(TESTBUILD) $(TESTS) $(TESTSRC)

test: $(TESTBUILD)
	@echo "Setting up tests..."
	for test in $(TESTBUILD) ; do ./$$test --verbose ; done
	@echo "Cleaning up tests..."
	@rm -f $(TESTBUILD)

clean:
	@rm -f $(TARGET)