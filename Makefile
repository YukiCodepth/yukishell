CC = gcc
CFLAGS = -Wall -g
TARGET = yukishell
LIBS = -lreadline

SRCS = main.c parser.c builtins.c executor.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c yukishell.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
