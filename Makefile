CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Isrc
SRC      = $(wildcard src/*.cpp src/*/*.cpp)
HDR      = $(wildcard src/*/*.h)
OBJ      = $(SRC:.cpp=.o)

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

src/%.o: src/%.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: main
	./main

clean:
	rm -f main $(OBJ)

.PHONY: run clean
