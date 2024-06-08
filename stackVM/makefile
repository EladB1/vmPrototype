CC=gcc
OS := $(shell $(CC) -dumpmachine)

ifneq (, $(findstring linux, $(OS))) 
	STANDARD=gnu11
else
	STANDARD=c11
endif

BUILDDIR=build

CFLAGS=-Wall -Wextra -std=$(STANDARD) -lm
SOURCES=src/*.c
TESTS=tests/*.c
TARGET=$(BUILDDIR)/bolt
TEST_SOURCES=src/[!app]*.c
TESTBUILD=tests/bolt-tests

DEFAULT_CONFIG=default-config.yml
CONFIG=$(BUILDDIR)/.bolt_vm_config.yml

default: clean $(TARGET)

$(TARGET): $(SOURCES-.c=.o)
	if [[ ! -d $(BUILDDIR) ]]; then mkdir ${BUILDDIR}; fi
	if [[ ! -f ${CONFIG} ]]; then cp ${DEFAULT_CONFIG} ${CONFIG}; fi
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)
	

$(TESTBUILD): $(TESTS- .c=.o)
	$(CC) $(CFLAGS) $< -lcriterion -o $(TESTBUILD) $(TESTS) $(TEST_SOURCES)

test: $(TESTBUILD)
	@echo "Setting up tests..."
	-for test in $(TESTBUILD) ; do ./$$test --verbose ; done
	@echo "Cleaning up tests..."
	@rm -f $(TESTBUILD)

clean:
	@rm -f $(TARGET)
	@rm -f $(TESTBUILD)