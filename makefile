CXX = g++
CXXFLAGS = -Wall -Wextra -Wno-unused-parameter -std=c++17
SRCS = $(shell find src -name '*.cpp')
OBJS = $(SRCS:src/%.cpp=build/%.o)
EXEC = build/jpl
TEST = test.jpl
FLAGS = "-s"

all: run

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Generate dependencies with -MMD and -MP
build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include the generated dependency files
-include $(OBJS:.o=.d)

compile: $(EXEC)

run: $(EXEC)
	./$(EXEC) $(TEST) $(FLAGS)

clean:
	rm -rf build
