# Compiler options
CC = gcc
CFLAGS = -O3 -Wall -Wextra -Werror -Wno-unused-result -g #-fsanitize=address
#LDFLAGS = -lasan

# Valgrind options
VALGRIND = valgrind
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --undef-value-errors=yes --error-exitcode=1

# Source files and object files
SRCS = main.c commands.c parser.c
OBJS = $(SRCS:.c=.o)
EXEC = project

# Default target
all: $(EXEC)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files into the executable
$(EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Valgrind target
valgrind: $(EXEC)
	$(VALGRIND) $(VALGRIND_FLAGS) ./$(EXEC)

# GDB target
gdb: $(EXEC)
	gdb ./$^

# Lizard target
lizard:
	lizard -L 50 -T nloc=30 -C 12 -m $(SRCS)

# Cppcheck target
cppcheck:
	cppcheck --enable=all --language=c --std=c99 --check-config\
	 --suppress=missingIncludeSystem -v $(SRCS)

# Clean rule
clean:
	rm -f $(OBJS) $(EXEC)


