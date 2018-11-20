CXX ?= g++
CXXFLAGS =
CXX_FLAGS = -Wall -Wshadow -Wextra -pedantic -std=c++11 -pthread -I../include
CXX_RELEASE_FLAGS = -O3 -march=native
CXX_DEBUG_FLAGS = -g

BIN_PATH = bin/
DEBUG_PATH = $(BIN_PATH)debug/


all:	basic_log.exe daily_log.exe rotating_log.exe console_log.exe bench.exe
debug:	console_log-debug.exe basic_log-debug.exe daily_log-debug.exe rotating_log-debug.exe bench-debug.exe

basic_log.exe: basic_log.cpp
	$(CXX) $^ -o $(BIN_PATH)$@ $(CXX_FLAGS) $(CXX_RELEASE_FLAGS) $(CXXFLAGS)

daily_log.exe: daily_log.cpp
	$(CXX) $^ -o $(BIN_PATH)$@ $(CXX_FLAGS) $(CXX_RELEASE_FLAGS) $(CXXFLAGS)

rotating_log.exe: rotating_log.cpp
	$(CXX) $^ -o $(BIN_PATH)$@ $(CXX_FLAGS) $(CXX_RELEASE_FLAGS) $(CXXFLAGS)

console_log.exe: console_log.cpp
	$(CXX) $^ -o $(BIN_PATH)$@ $(CXX_FLAGS) $(CXX_RELEASE_FLAGS) $(CXXFLAGS)

bench.exe: bench.cpp
	$(CXX) $^ -o $(BIN_PATH)$@ $(CXX_FLAGS) $(CXX_RELEASE_FLAGS) $(CXXFLAGS)


basic_log-debug.exe: basic_log.cpp
	$(CXX) $^ -o $(DEBUG_PATH)$@ $(CXX_FLAGS) $(CXX_DEBUG_FLAGS) $(CXXFLAGS)

daily_log-debug.exe: daily_log.cpp
	$(CXX) $^ -o $(DEBUG_PATH)$@ $(CXX_FLAGS) $(CXX_DEBUG_FLAGS) $(CXXFLAGS)

rotating_log-debug.exe: rotating_log.cpp
	$(CXX) $^ -o $(DEBUG_PATH)$@ $(CXX_FLAGS) $(CXX_DEBUG_FLAGS) $(CXXFLAGS)

console_log-debug.exe: console_log.cpp
	$(CXX) $^ -o $(DEBUG_PATH)$@ $(CXX_FLAGS) $(CXX_DEBUG_FLAGS) $(CXXFLAGS)

bench-debug.exe: bench.cpp
	$(CXX) $^ -o $(DEBUG_PATH)$@ $(CXX_FLAGS) $(CXX_DEBUG_FLAGS) $(CXXFLAGS)

clean:
	rm -f *.o $(BIN_PATH)logs/* $(DEBUG_PATH)logs/* $(BIN_PATH)*.exe $(DEBUG_PATH)*-debug.exe


rebuild: clean all
rebuild-debug: clean debug
