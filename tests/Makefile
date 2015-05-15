CXX	?= g++
CXXFLAGS	=  -D_WIN32_WINNT=0x600 -march=native -Wall -Wextra -Wshadow -pedantic -std=c++11 -pthread -Wl,--no-as-needed  -I../include 
LDPFALGS = -pthread

CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix ./,$(notdir $(CPP_FILES:.cpp=.o)))


tests: $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(LDPFALGS) -o $@ $^

%.o: %.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f tests *.o logs/*     
 
rebuild: clean all



