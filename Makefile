CC	= g++
CCFLAGS	= -std=c++11 -pthread -Iinclude -O3 -flto

all: lib testlog

testlog: test.o lib
	$(CC) -o $testlog test.o libc11log.a $(CCFLAGS) 
	
lib: factory.o  formatters.o  line_logger.o os.o
	ar rvs libc11log.a $^;

	
test.o: src/test.cpp
	$(CC) -c -o $@ $^ $(CCFLAGS)	
	
factory.o: src/factory.cpp
	$(CC) -c -o $@ $^ $(CCFLAGS)	

formatters.o: src/formatters.cpp
	$(CC) -c -o $@ $^ $(CCFLAGS)
    
line_logger.o: src/line_logger.cpp
	$(CC) -c -o $@ $^ $(CCFLAGS)
    	
os.o: src/os.cpp
	$(CC) -c -o $@ $^ $(CCFLAGS)
	
.PHONY: clean
clean:
	rm *.o
