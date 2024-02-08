CC=gcc
CFLAGS=-Wall -Wextra -std=c11
SOURCES=vm.c frame.c dataconstant.c stringvector.c builtin.c impl_builtin.c filereader.c
TARGET=bolt

default: clean $(TARGET)

$(TARGET): $(SOURCES-.c=.o)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	@rm -f $(TARGET)