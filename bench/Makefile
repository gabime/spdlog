CXX	?= g++
CXXFLAGS	= -march=native -Wall -Wextra -pedantic -std=c++11 -pthread -I../include
CXX_RELEASE_FLAGS = -O3 -flto -DNDEBUG


#         g2log-async
binaries=spdlog-bench spdlog-bench-mt spdlog-async spdlog-null-async \
         boost-bench boost-bench-mt \
         glog-bench glog-bench-mt \
         g3log-async \
         p7-bench p7-bench-mt \
         log4cpp-bench log4cpp-bench-mt \
         log4cplus-bench log4cplus-bench-mt \
         easylogging-bench easylogging-bench-mt easylogging-bench-async \
         plog-bench plog-bench-mt

all: $(binaries)

spdlog-bench: spdlog-bench.cpp
	$(CXX) spdlog-bench.cpp -o spdlog-bench $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	
spdlog-bench-mt: spdlog-bench-mt.cpp
	$(CXX) spdlog-bench-mt.cpp -o spdlog-bench-mt  $(CXXFLAGS) $(CXX_RELEASE_FLAGS)
	
spdlog-async: spdlog-async.cpp
	$(CXX) spdlog-async.cpp -o spdlog-async  $(CXXFLAGS) $(CXX_RELEASE_FLAGS)

spdlog-null-async: spdlog-null-async.cpp
	$(CXX) spdlog-null-async.cpp -o spdlog-null-async $(CXXFLAGS) $(CXX_RELEASE_FLAGS)

BOOST_FLAGS	= -DBOOST_LOG_DYN_LINK -I$(HOME)/include -I/usr/include -L$(HOME)/lib -lboost_log_setup -lboost_log -lboost_filesystem -lboost_system -lboost_thread -lboost_regex -lboost_date_time -lboost_chrono

boost-bench: boost-bench.cpp
	$(CXX) boost-bench.cpp -o boost-bench $(CXXFLAGS) $(BOOST_FLAGS) $(CXX_RELEASE_FLAGS)
	
boost-bench-mt: boost-bench-mt.cpp
	$(CXX) boost-bench-mt.cpp -o boost-bench-mt $(CXXFLAGS) $(BOOST_FLAGS) $(CXX_RELEASE_FLAGS)	

GLOG_FLAGS = -I$(HOME)/include -L$(HOME)/lib -lglog
glog-bench: glog-bench.cpp
	$(CXX) glog-bench.cpp -o glog-bench $(CXXFLAGS) $(GLOG_FLAGS) $(CXX_RELEASE_FLAGS)
	
glog-bench-mt: glog-bench-mt.cpp
	$(CXX) glog-bench-mt.cpp -o glog-bench-mt $(CXXFLAGS) $(GLOG_FLAGS) $(CXX_RELEASE_FLAGS)	

G2LOG_FLAGS = -I$(HOME)/include -L$(HOME)/lib -llib_g2logger 
g2log-async: g2log-async.cpp
	$(CXX) g2log-async.cpp -o g2log-async $(CXXFLAGS) $(G2LOG_FLAGS) $(CXX_RELEASE_FLAGS)
	
G3LOG_FLAGS = -I$(HOME)/include -L$(HOME)/lib -lg3logger
g3log-async: g3log-async.cpp
	$(CXX) g3log-async.cpp -o g3log-async $(CXXFLAGS) $(G3LOG_FLAGS) $(CXX_RELEASE_FLAGS)

P7_FLAGS = -I$(HOME)/P7/Headers -I$(HOME)/include -L$(HOME)/lib -lP7
p7-bench: p7-bench.cpp
	$(CXX) p7-bench.cpp -o p7-bench $(CXXFLAGS) $(P7_FLAGS) $(CXX_RELEASE_FLAGS)

p7-bench-mt: p7-bench-mt.cpp
	$(CXX) p7-bench-mt.cpp -o p7-bench-mt $(CXXFLAGS) $(P7_FLAGS) $(CXX_RELEASE_FLAGS)

LOG4CPP_FLAGS = -I$(HOME)/include -L$(HOME)/lib -llog4cpp
log4cpp-bench: log4cpp-bench.cpp
	$(CXX) log4cpp-bench.cpp -o log4cpp-bench $(CXXFLAGS) $(LOG4CPP_FLAGS) $(CXX_RELEASE_FLAGS)

log4cpp-bench-mt: log4cpp-bench-mt.cpp
	$(CXX) log4cpp-bench-mt.cpp -o log4cpp-bench-mt $(CXXFLAGS) $(LOG4CPP_FLAGS) $(CXX_RELEASE_FLAGS)

LOG4CPLUS_FLAGS = -I$(HOME)/include -L$(HOME)/lib -llog4cplus
log4cplus-bench: log4cplus-bench.cpp
	$(CXX) log4cplus-bench.cpp -o log4cplus-bench $(CXXFLAGS) $(LOG4CPLUS_FLAGS) $(CXX_RELEASE_FLAGS)

log4cplus-bench-mt: log4cplus-bench-mt.cpp
	$(CXX) log4cplus-bench-mt.cpp -o log4cplus-bench-mt $(CXXFLAGS) $(LOG4CPLUS_FLAGS) $(CXX_RELEASE_FLAGS)

EASYL_FLAGS = -I$(HOME)/easyloggingpp/src
easylogging-bench: easylogging-bench.cpp
	$(CXX) easylogging-bench.cpp -o easylogging-bench $(CXXFLAGS) $(EASYL_FLAGS) $(CXX_RELEASE_FLAGS)

easylogging-bench-mt: easylogging-bench-mt.cpp
	$(CXX) easylogging-bench-mt.cpp -o easylogging-bench-mt $(CXXFLAGS) $(EASYL_FLAGS) $(CXX_RELEASE_FLAGS)

easylogging-bench-async: easylogging-bench-async.cpp
	$(CXX) easylogging-bench-async.cpp -o easylogging-bench-async $(CXXFLAGS) $(EASYL_FLAGS) $(CXX_RELEASE_FLAGS)

PLOG_FLAGS = -I$(HOME)/include
plog-bench: plog-bench.cpp
	$(CXX) plog-bench.cpp -o plog-bench $(CXXFLAGS) $(PLOG_FLAGS) $(CXX_RELEASE_FLAGS)

plog-bench-mt: plog-bench-mt.cpp
	$(CXX) plog-bench-mt.cpp -o plog-bench-mt $(CXXFLAGS) $(PLOG_FLAGS) $(CXX_RELEASE_FLAGS)

.PHONY: clean

clean:
	rm -f *.o logs/* $(binaries)

rebuild: clean all
