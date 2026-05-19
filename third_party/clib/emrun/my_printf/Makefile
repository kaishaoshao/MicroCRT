# Compiler and flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -std=c99
LDFLAGS =

# Directories
SRCDIR = src
INCDIR = include
TESTDIR = tests
OBJDIR = obj
BINDIR = bin

# Find all .c files in src and its subdirectories
SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/backends/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_EXECUTABLE = $(BINDIR)/test_runner

# Default target
all: $(TEST_EXECUTABLE)

# Target to run the tests
test: $(TEST_EXECUTABLE)
	@./$(TEST_EXECUTABLE)

# Link the test executable
$(TEST_EXECUTABLE): $(OBJECTS) $(TEST_SOURCES)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(OBJECTS) $(TEST_SOURCES) -o $@ $(LDFLAGS)

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all test clean
