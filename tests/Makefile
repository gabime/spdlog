CXX	?= g++
CXXFLAGS	=  -D_WIN32_WINNT=0x600 -march=native -Wall  -pedantic -std=c++11 -pthread -Wl,--no-as-needed -O
LDPFALGS = -pthread -flto

CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix ./,$(notdir $(CPP_FILES:.cpp=.o)))


tests: $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(LDPFALGS) -o $@ $^

%.o: %.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f tests *.o logs/*     
 
rebuild: clean tests



