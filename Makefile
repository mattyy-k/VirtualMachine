CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
TARGET = vm
COMPILER = compiler
SRC_DIR = src
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(COMPILER): $(SRC_DIR)/compiler.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

test: $(TARGET)
	./test-runner.sh

test-%: $(TARGET)
	./test-runner.sh $*

clean:
	rm -f $(TARGET) $(COMPILER)
	rm -f /tmp/vm-test-* /tmp/vm-source-* /tmp/vm-compiled-*

.PHONY: all test clean
