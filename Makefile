# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = bin/ls
SRCDIR = src
OBJDIR = obj
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Default target
all: $(TARGET)

# Create target executable
$(TARGET): $(OBJECTS) | bin
	$(CC) $(OBJECTS) -o $(TARGET)

# Create object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
bin:
	mkdir -p bin

obj:
	mkdir -p obj

# Clean build artifacts
clean:
	rm -rf $(OBJDIR)/*.o $(TARGET)

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all clean install
