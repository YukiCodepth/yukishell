CC = gcc
CFLAGS = -Wall -g -Iinclude
TARGET = yukishell
LIBS = -lreadline

SRCS = src/main.c src/parser.c src/builtins.c src/executor.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

src/%.o: src/%.c include/yukishell.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)
