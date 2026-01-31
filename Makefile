# Disable built-in rules and enable multi-threaded compilation
MAKEFLAGS += --no-builtin-rules -j$(shell nproc)

# Compiler and compilation flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g -std=gnu11 -D_GNU_SOURCE
LDFLAGS = -static

# The source file to build.
SRC ?= exp.c
# Deriving target name (e.g., 'exp.c' -> 'exp')
TARGET = $(SRC:.c=)

# Library source files
SRCS = $(shell find src -name '*.c' ! -path 'src/tests/*')
OBJS = $(SRCS:.c=.o)

# Automatically determine header order using topological sort
# 1. Find all headers and create self-dependencies (to ensure all files are included)
# 2. Grep for #include <axium/...> or "axium/..." to find local dependencies
# 3. Use tsort to get the correct order
HDRS = $(shell { \
    find include -name "*.h" -exec echo {} {} \; ; \
    find include -name "*.h" -exec grep -H '^#include [\"<]axium/' {} \; | \
    sed -E 's|^([^:]+):#include [\"<]axium/(.*)[\">]|include/axium/\2 \1|'; \
} | tsort)

# Test files
TEST_SRCS = $(shell find src/tests -name '*.c')
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_BINS = $(TEST_SRCS:.c=)

.PHONY: all clean test bundle

# Default target
all: $(TARGET) $(TEST_BINS)

# Bundling logic
bundle: $(TARGET)_bundled.c

%_bundled.c: %.c $(SRCS) $(HDRS)
	@echo "Bundling everything into $@..."
	@echo "/**" > $@
	@echo " * Amalgamated axium library and $<" >> $@
	@echo " *" >> $@
	@echo " * Axium: <https://github.com/CuB3y0nd/axium>" >> $@
	@echo " */" >> $@
	@echo "" >> $@
	@echo "#define _GNU_SOURCE" >> $@
	@grep -h "^#include <" $(HDRS) $(SRCS) $< | grep -v "<axium/" | sort | uniq >> $@
	@for f in $(HDRS); do \
		echo "" >> $@; \
		echo "/* --- Start of $$f --- */" >> $@; \
		grep -vE "^#include [\"<]axium/" $$f >> $@; \
	done
	@for f in $(SRCS); do \
		echo "" >> $@; \
		echo "/* --- Start of $$f --- */" >> $@; \
		grep -vE "^#include [\"<]axium/" $$f >> $@; \
	done
	@echo "" >> $@
	@echo "/* --- Start of user script: $< --- */" >> $@
	@grep -vE "^#include [\"<]axium/" $< >> $@

# Rule to link any 'executable' from its 'executable.c' and library objects
%: %.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@if [ -f "$*.c" ]; then $(MAKE) $*_bundled.c; fi

# Rule to link each individual test binary
src/tests/%: src/tests/%.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Run all tests
test: $(TEST_BINS)
	@for test_bin in $(TEST_BINS); do \
		echo "--- Running $$test_bin ---"; \
		./$$test_bin || exit 1; \
	done
	@echo ""
	@echo "All tests passed!"

# Rule for test objects - includes -Isrc/tests
src/tests/%.o: src/tests/%.c
	$(CC) $(CFLAGS) -Isrc/tests -c $< -o $@

# Generic compilation rule (transforms .c into .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -f $(TARGET) $(OBJS) $(TEST_OBJS) $(TEST_BINS) *_bundled.c *.json
	@# Clean up all object files and executables that might have been created
	find . -maxdepth 1 -type f -name "*.o" -delete
	find . -maxdepth 1 -type f -executable -not -name "*.sh" -not -name "Makefile" -delete
	find src/tests -type f -name "*.o" -delete
