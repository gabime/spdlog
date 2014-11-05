#~/bin/bash
echo "Running benchmakrs (all with 1000,000 writes to the logs folder"
echo
echo "boost-bench (single thread).."
time ./boost-bench
echo ==================================
sleep 1
echo "spdlog-bench (single thread)"
time ./spdlog-bench
echo ==================================
sleep 1
echo "boost-bench-mt (10 threads, single logger)"..
time ./boost-bench-mt
echo ==================================
sleep 1
echo "spdlog-bench-mt (10 threads, single logger)"..
time ./spdlog-bench-mt
echo ==================================
sleep 1
echo


