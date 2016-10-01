#!/bin/bash
echo "running spdlog and g3log tests 10 time with ${1:-10} threads each (total 1,000,000 entries).."
rm -f *.log
for i in {1..10}

do
   echo
   sleep 0.5
   ./spdlog-latency ${1:-10} 2>/dev/null || exit
   sleep 0.5
   ./g3log-latency ${1:-10} 2>/dev/null || exit

done
