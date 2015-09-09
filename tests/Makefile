CXX	?= g++
CXXFLAGS	=  -Wall  -pedantic -std=c++11 -pthread -O2
LDPFALGS = -pthread

CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix ./,$(notdir $(CPP_FILES:.cpp=.o)))

    
tests: $(OBJ_FILES)    
	$(CXX) $(CXXFLAGS) $(LDPFALGS) -o $@ $^
	mkdir -p logs

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f tests *.o logs/*.txt     
 
rebuild: clean tests



