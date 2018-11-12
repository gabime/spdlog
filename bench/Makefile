CXX	?= g++
CXXFLAGS	= -march=native -Wall -Wextra -pedantic -Wconversion -std=c++11 -pthread -I../include -fmax-errors=1
CXX_RELEASE_FLAGS = -O3 -flto -Wl,--no-as-needed


binaries=bench async_bench latency formatter-bench

all: $(binaries)

bench: bench.cpp
	$(CXX) bench.cpp -o bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS)


async_bench: async_bench.cpp
	$(CXX) async_bench.cpp -o async_bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS)


latency: latency.cpp
	$(CXX) latency.cpp -o latency $(CXXFLAGS) $(CXX_RELEASE_FLAGS) -lbenchmark


formatter-bench: formatter-bench.cpp
	$(CXX) formatter-bench.cpp -o formatter-bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS) -lbenchmark


.PHONY: clean

clean:
	rm -f *.o logs/* latecy_logs $(binaries)

rebuild: clean all
