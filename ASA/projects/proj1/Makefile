# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Target executable
TARGET = project

# Source file
SRCS = project.cpp

# Default target: Build the program
all: $(TARGET)

# Rule to build the executable directly from the source
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean up build files
clean:
	rm -f $(TARGET)

# Run the program with input from test.txt
test: $(TARGET)
	./$(TARGET) < test.txt

