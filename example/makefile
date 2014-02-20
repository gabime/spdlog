CXX	= g++
CXXFLAGS	= -march=native -Wall -Wextra -Wshadow -pedantic -std=c++11 -pthread -I../include 
CXX_RELEASE_FLAGS = -O3 -flto 
CXX_DEBUG_FLAGS= -g 

OUTBIN  = testme

all: test.cpp
	$(CXX) test.cpp -o $(OUTBIN) $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	

debug: test.cpp
	$(CXX) test.cpp -o $(OUTBIN)-debug $(CXXFLAGS) $(CXX_DEBUG_FLAFS)



clean:
	rm -f *.text $(OUTBIN) $(OUTBIN)-debug 
	
rebuild: clean all
rebuild-debug: clean debug


