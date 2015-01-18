CXX	?= g++
CXXFLAGS	= -march=native -Wall -Wshadow -Wextra -pedantic -std=c++11 -pthread -Wl,--no-as-needed  -I../include 
CXX_RELEASE_FLAGS = -O3 -flto
CXX_DEBUG_FLAGS= -g 


all:	example bench
debug:	example-debug bench-debug

example: example.cpp
	$(CXX) example.cpp -o example $(CXXFLAGS) $(CXX_RELEASE_FLAGS)

bench: bench.cpp
	$(CXX) bench.cpp -o bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	

example-debug: example.cpp
	$(CXX) example.cpp -o example-debug $(CXXFLAGS) $(CXX_DEBUG_FLAGS)
	
bench-debug: bench.cpp
	$(CXX) bench.cpp -o bench-debug $(CXXFLAGS) $(CXX_DEBUG_FLAGS)	



clean:
	rm -f *.o logs/*.txt example example-debug bench bench-debug 


rebuild: clean all
rebuild-debug: clean debug


