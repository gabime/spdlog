CXX	?= g++
CXXFLAGS	=
CXX_FLAGS = -Wall -Wshadow -Wextra -pedantic -std=c++11 -pthread -I../include
CXX_RELEASE_FLAGS = -O3 -march=native
CXX_DEBUG_FLAGS= -g


all:	example bench
debug:	example-debug bench-debug

example: example.cpp
	$(CXX) example.cpp -o example $(CXX_FLAGS) $(CXX_RELEASE_FLAGS) $(CXXFLAGS)

bench: bench.cpp
	$(CXX) bench.cpp -o bench $(CXX_FLAGS) $(CXX_RELEASE_FLAGS) $(CXXFLAGS)


example-debug: example.cpp
	$(CXX) example.cpp -o example-debug $(CXX_FLAGS) $(CXX_DEBUG_FLAGS) $(CXXFLAGS)

bench-debug: bench.cpp
	$(CXX) bench.cpp -o bench-debug $(CXX_FLAGS) $(CXX_DEBUG_FLAGS) $(CXXFLAGS)

clean:
	rm -f *.o logs/*.txt example example-debug bench bench-debug


rebuild: clean all
rebuild-debug: clean debug
