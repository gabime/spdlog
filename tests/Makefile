CXX	?= g++
CXXFLAGS	=  -D_WIN32_WINNT=0x600 -march=native -Wall -Wextra -Wshadow -pedantic -std=c++11 -pthread -Wl,--no-as-needed  -I../include 

all: %.cpp
    $(CXX) $^ -o tests $(CXXFLAGS) $(CXX_RELEASE_FLAGS)    
clean:
    rm -f tests *.o logs/*     
 
rebuild: clean all



