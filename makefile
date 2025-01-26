CXX = g++
CXXFLAGS = -Wall -Wextra -Wno-unused-parameter -std=c++17
SRCS = $(shell find src -name '*.cpp')
OBJS = $(SRCS:src/%.cpp=build/%.o)
EXEC = build/jpl
TEST = test.jpl
FLAGS = "-p"

all: run

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

compile: $(EXEC)

run: $(EXEC)
	./$(EXEC) $(TEST) $(FLAGS)

clean:
	rm -rf build
