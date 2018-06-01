CXX	?= g++
CXXFLAGS	= -march=native -Wall -Wextra -pedantic -std=c++11 -pthread -I../include
CXX_RELEASE_FLAGS = -O3 -flto -DNDEBUG


binaries=bench spdlog-null-async

all: $(binaries)

bench: bench.cpp
	$(CXX) bench.cpp -o bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	
spdlog-null-async: spdlog-null-async.cpp
	$(CXX) spdlog-null-async.cpp -o spdlog-null-async $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
		


.PHONY: clean

clean:
	rm -f *.o logs/* $(binaries)

rebuild: clean all
