CC = gcc-10

# Compilation flags
CFLAGS = -g -std=c17 -D_POSIX_C_SOURCE=200809L -I. \
         -Wall -Wextra \
         -Wcast-align -Wconversion -Wfloat-equal -Wformat=2 -Wnull-dereference -Wshadow -Wsign-conversion -Wswitch-enum -Wundef -Wunreachable-code -Wunused \
         -pthread \
         -fsanitize=thread


# Add additional error limit flag for non-MacOS systems
ifneq ($(shell uname -s),Darwin) # if not MacOS
    CFLAGS += -fmax-errors=5
endif

# Targets
all: src/server/kvs src/client/client

src/server/kvs: src/common/protocol.h src/common/constants.h src/server/main.c src/server/operations.o src/server/kvs.o src/server/io.o src/server/parser.o src/common/io.o src/server/subscriptions.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/client/client: src/common/protocol.h src/common/constants.h src/client/main.c src/client/api.o src/client/parser.o src/common/io.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Pattern rule for object file generation
%.o: %.c %.h
	$(CC) $(CFLAGS) -c ${@:.o=.c} -o $@

# Clean target to remove compiled artifacts
clean:
	rm -f src/common/*.o src/client/*.o src/server/*.o src/server/core/*.o src/server/kvs src/client/client src/client/client_write
	rm -f src/server/jobs/*.out
	rm -f src/server/jobs/*.bck

# Format target to ensure consistent formatting across source files
format:
	@which clang-format >/dev/null 2>&1 || echo "Please install clang-format to run this command"
	clang-format -i src/common/*.c src/common/*.h src/client/*.c src/client/*.h src/server/*.c src/server/*.h
