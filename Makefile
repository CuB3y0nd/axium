# Disable built-in rules to prevent interference
MAKEFLAGS += --no-builtin-rules

# Compiler and compilation flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g -std=gnu11 -D_GNU_SOURCE
LDFLAGS = -static

# The source file to build.
SRC ?= exp.c
# Deriving target name (e.g., 'exp.c' -> 'exp')
TARGET = $(SRC:.c=)

# Library source files
SRCS = $(shell find src -name '*.c')
OBJS = $(SRCS:.c=.o)

# Test files
TEST_SRCS = $(shell find tests -name '*.c')
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_BINS = $(TEST_SRCS:.c=)

.PHONY: all clean test

# Default target
all: $(TARGET)

# Rule to link any 'executable' from its 'executable.c' and library objects
%: %.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Rule to link each individual test binary
tests/%: tests/%.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Run all tests
test: $(TEST_BINS)
	@for test_bin in $(TEST_BINS); do \
		echo "--- Running $$test_bin ---"; \
		./$$test_bin || exit 1; \
	done
	@echo "All tests passed!"

# Rule for test objects - includes -Itests
tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -Itests -c $< -o $@

# Generic compilation rule (transforms .c into .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -f $(TARGET) $(OBJS) $(TEST_OBJS) $(TEST_BINS)
	@# Clean up all object files and executables that might have been created
	find . -maxdepth 1 -type f -name "*.o" -delete
	find . -maxdepth 1 -type f -executable -not -name "*.sh" -not -name "Makefile" -delete
	find tests -type f -name "*.o" -delete
