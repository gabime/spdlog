@echo off
echo Running benchmarks (all with 1000,000 writes to the logs folder)
echo ==================================
echo boost-bench (single thread)
echo %time%
.\boost-bench
echo %time%
echo ==================================
choice /n /c y /d y /t 1 >NUL
echo spdlog-bench (single thread)
echo %time%
.\spdlog-bench
echo %time%
echo ==================================
choice /n /c y /d y /t 1 >NUL
echo boost-bench-mt (10 threads, single logger)
echo %time%
.\boost-bench-mt
echo %time%
echo ==================================
choice /n /c y /d y /t 1 >NUL
echo spdlog-bench-mt (10 threads, single logger)
echo %time%
.\spdlog-bench-mt
echo %time%
echo ==================================


