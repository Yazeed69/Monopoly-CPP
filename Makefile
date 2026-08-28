# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Ipublic -Isrc/public  # Add -Isrc/public to tell the compiler where to find headers

# Source files and executable name
SRCS = $(wildcard src/private/*.cpp)
TARGET = monopoly_game.exe

# Build target
all: $(TARGET)

# Rule to build the target
$(TARGET):
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

# Rule to clean the build
clean:
	-rm -f ./monopoly_game.exe ./monopoly_tests.exe

# Rule to clean, build, and run the project
bar: clean all
	./$(TARGET)

# Shortcut rule to run the executable directly
run:
	./$(TARGET)

# Rules regression tests: everything except main.cpp, plus the test driver
TEST_SRCS = $(filter-out src/private/main.cpp,$(SRCS)) tests/rules_tests.cpp
TEST_TARGET = monopoly_tests.exe

test: $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_TARGET)
	./$(TEST_TARGET)
