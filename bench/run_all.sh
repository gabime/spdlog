#~/bin/bash

exec 2>&1


echo "Running benchmakrs (all with 1000,000 writes to the logs folder)"
echo
echo "boost-bench (single thread).."
for i in {1..3}; do time ./boost-bench; done
rm  -f logs/*
echo
echo
sleep 5

echo "glog-bench (single thread).."
for i in {1..3}; do time ./glog-bench; done
rm  -f logs/*
echo
echo
sleep 5

echo "g2log-bench (single thread).."
for i in {1..3}; do time ./g2log-bench; done
rm  -f logs/*
echo
echo
sleep 5

echo "spdlog-bench (single thread)"
for i in {1..3}; do time ./spdlog-bench; done
rm  -f logs/*
echo
echo
sleep 5
echo "------------------------------------"
echo "Multithreaded benchmarks.."
echo "------------------------------------"
echo "boost-bench-mt (10 threads, single logger)"..
for i in {1..3}; do ./boost-bench-mt; done
rm  -f logs/*
echo
echo
sleep 5

echo "glog-bench-mt (10 threads, single logger)"..
for i in {1..3}; do time ./glog-bench-mt; done
rm  -f logs/*
echo
echo
sleep 5

echo "g2log-bench-mt (10 threads, single logger)"..
for i in {1..3}; do time ./g2log-bench-mt; done
rm  -f logs/*
echo
echo
sleep 5

echo "spdlog-bench-mt (10 threads, single logger)"..
for i in {1..3}; do time ./spdlog-bench-mt; done
rm  -f logs/*
echo
echo
sleep 5

echo "------------------------------------"
echo "Async  benchmarks.."
echo "------------------------------------"

echo "spdlog-bench-async (single thread)"..
for i in {1..3}; do time ./spdlog-bench-async; done
rm  -f logs/*
echo
echo
sleep 5

echo "spdlog-bench-mt-async (10 threads, single logger)"..
for i in {1..3}; do time ./spdlog-bench-mt-async; done



