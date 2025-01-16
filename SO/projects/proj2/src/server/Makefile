# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread

# Target executable
TARGET = test_kvs

# Source files
SRC = kvs.c subscriptions.c operations.c test_kvs.c io.c

# Object files
OBJ = $(SRC:.c=.o)

# Build target
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJ) $(TARGET)
