#~/bin/bash
echo "Running benchmakrs (all with 1000,000 writes to the logs folder)"
echo
echo "boost-bench (single thread).."
time ./boost-bench
rm logs/*
echo
echo
sleep 5

echo "glog-bench (single thread).."
time ./glog-bench
rm logs/*
echo
echo
sleep 5

echo "g2log-bench (single thread).."
time ./g2log-bench
rm logs/*
echo
echo
sleep 5

echo "spdlog-bench (single thread)"
time ./spdlog-bench
rm logs/*
echo
echo
sleep 5
echo "------------------------------------"
echo "Multithreaded benchmarks.."
echo "------------------------------------"
echo "boost-bench-mt (10 threads, single logger)"..
time ./boost-bench-mt
rm logs/*
echo
echo
sleep 5

echo "glog-bench-mt (10 threads, single logger)"..
time ./glog-bench-mt
rm logs/*
echo
echo
sleep 5

echo "g2log-bench-mt (10 threads, single logger)"..
time ./g2log-bench-mt
rm logs/*
echo
echo
sleep 5

echo "spdlog-bench-mt (10 threads, single logger)"..
time ./spdlog-bench-mt
rm logs/*
echo
echo
sleep 5

echo "------------------------------------"
echo "Async  benchmarks.."
echo "------------------------------------"

echo "spdlog-bench-async (single thread)"..
time ./spdlog-bench-async
rm logs/*
echo
echo
sleep 5

echo "spdlog-bench-mt-async (10 threads, single logger)"..
time ./spdlog-bench-mt-async



