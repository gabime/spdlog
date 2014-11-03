CXX	= g++
CXXFLAGS	= -march=native -Wall  -pthread  -I../include --std=c++11

CXX_RELEASE_FLAGS = -O3 -flto


all:	spdlog-bench spdlog-bench-mt boost-bench boost-bench-mt

spdlog-bench: spdlog-bench.cpp
	$(CXX) spdlog-bench.cpp -o spdlog-bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	
spdlog-bench-mt: spdlog-bench-mt.cpp
	$(CXX) spdlog-bench-mt.cpp -o spdlog-bench-mt $(CXXFLAGS) $(CXX_RELEASE_FLAGS)	

BOOST_FLAGS	= -DBOOST_LOG_DYN_LINK  -I/home/gabi/devel/boost_1_56_0/ -L/home/gabi/devel/boost_1_56_0/stage/lib -lboost_log  -lboost_log_setup -lboost_filesystem -lboost_system -lboost_thread -lboost_regex -lboost_date_time -lboost_chrono	

boost-bench: boost-bench.cpp
	$(CXX) boost-bench.cpp -o boost-bench $(CXXFLAGS) $(BOOST_FLAGS) $(CXX_RELEASE_FLAGS)
	
boost-bench-mt: boost-bench-mt.cpp
	$(CXX) boost-bench-mt.cpp -o boost-bench-mt $(CXXFLAGS) $(BOOST_FLAGS) $(CXX_RELEASE_FLAGS)	


clean:
	rm -f *.o logs/*.txt spdlog-bench spdlog-bench-mt boost-bench boost-bench-mt


rebuild: clean all



