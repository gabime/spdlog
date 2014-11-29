CXX	= g++
CXXFLAGS	= -g -march=native -Wall -Wextra -pedantic -std=c++11 -pthread -Wl,--no-as-needed  -I../include 
CXX_RELEASE_FLAGS = -O3 -flto


all:	spdlog-bench spdlog-bench-mt spdlog-bench-async spdlog-bench-mt-async boost-bench boost-bench-mt glog-bench glog-bench-mt g2log-bench g2log-bench-mt 

spdlog-bench: spdlog-bench.cpp
	$(CXX) spdlog-bench.cpp -o spdlog-bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	
spdlog-bench-mt: spdlog-bench-mt.cpp
	$(CXX) spdlog-bench-mt.cpp -o spdlog-bench-mt  $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	
spdlog-bench-async: spdlog-bench-async.cpp
	$(CXX) spdlog-bench-async.cpp -o spdlog-bench-async  $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	
spdlog-bench-mt-async: spdlog-bench-mt-async.cpp
	$(CXX) spdlog-bench-mt-async.cpp -o spdlog-bench-mt-async  $(CXXFLAGS) $(CXX_RELEASE_FLAGS)							

BOOST_FLAGS	= -DBOOST_LOG_DYN_LINK  -I/home/gabi/devel/boost_1_56_0/ -L/home/gabi/devel/boost_1_56_0/stage/lib -lboost_log  -lboost_log_setup -lboost_filesystem -lboost_system -lboost_thread -lboost_regex -lboost_date_time -lboost_chrono	

boost-bench: boost-bench.cpp
	$(CXX) boost-bench.cpp -o boost-bench $(CXXFLAGS) $(BOOST_FLAGS) $(CXX_RELEASE_FLAGS)
	
boost-bench-mt: boost-bench-mt.cpp
	$(CXX) boost-bench-mt.cpp -o boost-bench-mt $(CXXFLAGS) $(BOOST_FLAGS) $(CXX_RELEASE_FLAGS)	


GLOG_FLAGS = -lglog
glog-bench: glog-bench.cpp
	$(CXX) glog-bench.cpp -o glog-bench $(CXXFLAGS) $(GLOG_FLAGS) $(CXX_RELEASE_FLAGS)
	
glog-bench-mt: glog-bench-mt.cpp
	$(CXX) glog-bench-mt.cpp -o glog-bench-mt $(CXXFLAGS) $(GLOG_FLAGS) $(CXX_RELEASE_FLAGS)	


G2LOG_FLAGS = -I/home/gabi/devel/g2log/g2log/src -L/home/gabi/devel/g2log/g2log -llib_g2logger 
g2log-bench: g2log-bench.cpp
	$(CXX) g2log-bench.cpp -o g2log-bench $(CXXFLAGS) $(G2LOG_FLAGS) $(CXX_RELEASE_FLAGS)
	
g2log-bench-mt: g2log-bench-mt.cpp
	$(CXX) g2log-bench-mt.cpp -o g2log-bench-mt $(CXXFLAGS) $(G2LOG_FLAGS) $(CXX_RELEASE_FLAGS)	



clean:
	rm -f *.o logs/* spdlog-bench spdlog-bench-mt spdlog-bench-async spdlog-bench-mt-async boost-bench boost-bench-mt glog-bench glog-bench-mt g2log-bench g2log-bench-mt


rebuild: clean all



