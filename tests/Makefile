CXX	?= g++
ifeq ($(STYLE),printf)
    $(info *** PRINTF STYLE ***)
    CXXFLAGS	=  -DSPDLOG_FMT_PRINTF -Wall  -pedantic -std=c++11 -pthread -O2 -I../include
else
    $(info *** FORMAT STYLE ***)
    CXXFLAGS	=  -Wall  -pedantic -std=c++11 -pthread -O2 -I../include
endif
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



