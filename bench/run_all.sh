#~/bin/bash
echo "Running benchmakrs (all with 1000,000 writes to the logs folder"
echo
echo "boost-bench (single thread).."
time ./boost-bench
echo
echo
sleep 3
echo "glog-bench (single thread).."
time ./glog-bench
echo
echo
sleep 3
echo "spdlog-bench (single thread)"
time ./spdlog-bench
echo
echo
sleep 3
echo "boost-bench-mt (10 threads, single logger)"..
time ./boost-bench-mt
echo
echo
sleep 3
echo "glog-bench-mt (10 threads, single logger)"..
time ./glog-bench-mt
echo
echo
sleep 3
echo "spdlog-bench-mt (10 threads, single logger)"..
time ./spdlog-bench-mt
echo
echo
sleep 3
echo "spdlog-bench-mt-async (10 threads, single logger)"..
time ./spdlog-bench-mt-async



