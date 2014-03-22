CXX	= g++
CXXFLAGS	= -march=native -Wall -Wextra -Wshadow -pedantic -std=c++11 -pthread -I../include 
CXX_RELEASE_FLAGS = -O3 -flto -g
CXX_DEBUG_FLAGS= -g 

OUTBIN  = example

all: example.cpp
	$(CXX) example.cpp -o $(OUTBIN) $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	

debug: example.cpp
	$(CXX) example.cpp -o $(OUTBIN)-debug $(CXXFLAGS) $(CXX_DEBUG_FLAGS)



clean:
	rm -f *.txt $(OUTBIN) $(OUTBIN)-debug
	
rebuild: clean all
rebuild-debug: clean debug


