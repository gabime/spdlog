CXX	?= g++
CXXFLAGS	= -march=native -Wall -std=c++11 -pthread 
CXX_RELEASE_FLAGS = -Ofast -DNDEBUG


binaries=spdlog-latency g3log-latency

all: $(binaries)

spdlog-latency: spdlog-latency.cpp
	$(CXX) spdlog-latency.cpp -o spdlog-latency $(CXXFLAGS) $(CXX_RELEASE_FLAGS) -I../../include



g3log-latency: g3log-latency.cpp
	$(CXX) g3log-latency.cpp -o g3log-latency $(CXXFLAGS) $(CXX_RELEASE_FLAGS) -I../../../g3log/src -L. -lg3logger 
		
	
.PHONY: clean

clean:
	rm -f *.o  *.log $(binaries)


rebuild: clean all



