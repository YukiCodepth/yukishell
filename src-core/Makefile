# --- E·MUX Makefile ---

CC = gcc
CFLAGS = -Wall -g -Iinclude

# LDFLAGS tells the compiler to link external libraries (like readline for typing)
LDFLAGS = -lreadline 

# Source files (Added src/parser.c so the compiler can find your parsing logic!)
SRCS = src/main.c src/builtins.c src/executor.c src/parser.c
OBJS = $(SRCS:.c=.o)
TARGET = emux

# Default build target
all: $(TARGET)

# Link the object files to create the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile C files into object files
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f src/*.o $(TARGET)
