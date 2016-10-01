CXX	?= g++
CXXFLAGS	= -march=native -Wall -std=c++11 -pthread 
CXX_RELEASE_FLAGS = -O2 -DNDEBUG


binaries=spdlog-latency g3log-latency g3log-crush

all: $(binaries)

spdlog-latency: spdlog-latency.cpp
	$(CXX) spdlog-latency.cpp -o spdlog-latency $(CXXFLAGS) $(CXX_RELEASE_FLAGS) -I../../include



g3log-latency: g3log-latency.cpp
	$(CXX) g3log-latency.cpp -o g3log-latency $(CXXFLAGS) $(CXX_RELEASE_FLAGS) -I../../../g3log/src -L. -lg3logger 
		
		
g3log-crush: g3log-crush.cpp
	$(CXX) g3log-crush.cpp -o g3log-crush $(CXXFLAGS) $(CXX_RELEASE_FLAGS) -I../../../g3log/src -L. -lg3logger 
		
	
.PHONY: clean

clean:
	rm -f *.o  *.log $(binaries)


rebuild: clean all



