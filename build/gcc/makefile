SRC_DIR=../../src
_SOURCES = factory.cpp formatters.cpp line_logger.cpp os.cpp

SOURCES = $(patsubst %,$(SRC_DIR)/%,$(_SOURCES))
OBJS_RELEASE = $(patsubst %.cpp,release/%.o,$(_SOURCES))
OBJS_DEBUG = $(patsubst %.cpp,debug/%.o,$(_SOURCES))


CXX	= g++
CXXFLAGS	= -Wall -std=c++11 -pthread -I../../include 
CXX_RELEASE_FLAGS = -O3 -flto
CXX_DEBUG_FLAGS= -g

OUTLIB_RELEASE = libc11log.a
OUTLIB_DEBUG = libc11log-debug.a

TEST_RELEASE = testme
TEST_DEBUG = testme-debug

.PHONY: all mkdirs release debug build clean

all: release

release: CXXFLAGS += $(CXX_RELEASE_FLAGS)
release: mkdirs build-release
	

debug: CXXFLAGS += $(CXX_DEBUG_FLAGS)
debug: mkdirs build-debug

mkdirs:
	@mkdir -p release debug


build-release: $(OBJS_RELEASE)	
	ar rs $(OUTLIB_RELEASE) $^
	$(CXX) $(SRC_DIR)/test.cpp $(OUTLIB_RELEASE) -o $(TEST_RELEASE) $(CXXFLAGS)
	
build-debug: $(OBJS_DEBUG)	
	ar rs $(OUTLIB_DEBUG) $^
	$(CXX) $(SRC_DIR)/test.cpp $(OUTLIB_DEBUG) -o $(TEST_DEBUG) $(CXXFLAGS)

release/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)
	
debug/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)		

clean:
	rm -rf release debug daily.* $(TEST_RELEASE) $(TEST_DEBUG) $(OUTLIB_RELEASE) $(OUTLIB_DEBUG)
	

