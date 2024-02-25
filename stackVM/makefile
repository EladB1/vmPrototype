CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -lm
SOURCES=src/*.c
TESTS=tests/*.c
TARGET=bolt
TEST_SOURCES=src/[!app]*.c
TESTBUILD=tests/bolt-tests

default: clean $(TARGET)

$(TARGET): $(SOURCES-.c=.o)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

$(TESTBUILD): $(TESTS- .c=.o)
	$(CC) $(CFLAGS) $< -lcriterion -o $(TESTBUILD) $(TESTS) $(TEST_SOURCES)

test: $(TESTBUILD)
	@echo "Setting up tests..."
	for test in $(TESTBUILD) ; do ./$$test --verbose ; done
	@echo "Cleaning up tests..."
	@rm -f $(TESTBUILD)

clean:
	@rm -f $(TARGET)
	@rm -f $(TESTBUILD)