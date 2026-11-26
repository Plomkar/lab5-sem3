CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wold-style-cast
INCLUDES = -Iinclude
LIB_SOURCES = $(filter-out src/main.cpp,$(wildcard src/*.cpp))
LIB_OBJECTS = $(patsubst src/%.cpp,build/%.o,$(LIB_SOURCES))

all: build/worldmap

build/%.o: src/%.cpp $(wildcard include/*.hpp)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

build/worldmap: $(LIB_OBJECTS) build/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

build/tests: $(LIB_OBJECTS) tests/test_main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

test: build/tests
	./build/tests

run: build/worldmap
	./build/worldmap -w 64 -h 22 -s 7

examples: build/worldmap
	./build/worldmap -w 64 -h 22 -s 7 --no-color -o examples/map_64x22_seed7.txt > /dev/null
	./build/worldmap -w 40 -h 14 -s 21 --no-color -o examples/map_40x14_seed21.txt > /dev/null
	./build/worldmap -w 100 -h 30 -s 2026 --octaves 5 --no-color -o examples/map_100x30_seed2026.txt > /dev/null

clean:
	rm -rf build

.PHONY: all test run examples clean
