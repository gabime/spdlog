CXX	= g++
CXXFLAGS	= -march=native -Wall -Wextra -pedantic -std=c++11 -pthread -I../include
CXX_RELEASE_FLAGS = -O3 -flto


binaries=bench

all: $(binaries)

bench: bench.cpp
	$(CXX) bench.cpp -o bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
				

.PHONY: clean

clean:
	rm -f *.o logs/* $(binaries)

rebuild: clean all
