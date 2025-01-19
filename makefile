CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
SRCS = $(shell find src -name '*.cpp')
OBJS = $(SRCS:src/%.cpp=build/%.o)
EXEC = build/jpl
TEST = test.jpl

all: run

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(EXEC)
	./$(EXEC) $(TEST)

clean:
	rm -rf build
