CFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c99
CC = clang $(CFLAGS)
TARGET = encode
TARGET2 = decode
DEPS = endian.h code.h
OBJFILES = encode.o io.o trie.o word.o
OBJFILES2 = decode.o io.o trie.o word.o

all		:$(TARGET) $(TARGET2)

%.o		:%.c $(DEPS)
		$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET)	: $(OBJFILES)
		$(CC) $(CFLAGS) -lm $(OBJFILES) -o $(TARGET)

$(TARGET2)	: $(OBJFILES2)
		$(CC) $(CFLAGS) -lm $(OBJFILES2) -o $(TARGET2)

clean		:
		rm -f $(TARGET) $(TARGET2) $(OBJFILES) $(OBJFILES2)
		rm -rf infer-out a.out
infer		:
		make clean; infer-capture -- make; infer-analyze -- make;
valgrind	:
		valgrind ./$(TARGET)
