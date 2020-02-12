#!/usr/bin/python3 -tt

"""
Claus-MBP:spdlog clausklein$ python build_performance_measure.py
Running command: rm -rf build-cmake-ninja && mkdir -p build-cmake-ninja && CXX='ccache clang++' cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DSPDLOG_BUILD_EXAMPLE_HO=ON -G Ninja -B build-cmake-ninja .
Running command: ninja -j 2 -C build-cmake-ninja
Running command: ninja -j 2 -C build-cmake-ninja
Running command: ninja -j 2 -C build-cmake-ninja clean
Running command: ninja -j 2 -C build-cmake-ninja
Running command: rm -rf build-meson && mkdir -p build-meson && CXX='ccache clang++' meson build-meson
Running command: ninja -j 2 -C build-meson
Running command: ninja -j 2 -C build-meson
Running command: ninja -j 2 -C build-meson clean
Running command: ninja -j 2 -C build-meson
cmake-ninja
 2.954 gen
 1.427 build
 0.017 empty build
 0.043 clean
 1.245 rebuild
 5.686 overall
meson
 4.642 gen
 1.246 build
 0.016 empty build
 0.037 clean
 1.159 rebuild
 7.100 overall
Claus-MBP:spdlog clausklein$ cd build-cmake-ninja/
Claus-MBP:build-cmake-ninja clausklein$ !find
find . -type f -name '*.o' -o -name '*.a'
./CMakeFiles/spdlog.dir/src/async.cpp.o
./CMakeFiles/spdlog.dir/src/color_sinks.cpp.o
./CMakeFiles/spdlog.dir/src/file_sinks.cpp.o
./CMakeFiles/spdlog.dir/src/fmt.cpp.o
./CMakeFiles/spdlog.dir/src/spdlog.cpp.o
./CMakeFiles/spdlog.dir/src/stdout_sinks.cpp.o
./example/CMakeFiles/example.dir/example.cpp.o
./example/CMakeFiles/example_header_only.dir/example.cpp.o
./libspdlog.a
./tests/CMakeFiles/spdlog-utests.dir/main.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_async.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_backtrace.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_create_dir.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_daily_logger.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_dup_filter.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_errors.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_file_helper.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_file_logging.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_fmt_helper.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_macros.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_misc.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_mpmc_q.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_pattern_formatter.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_registry.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/test_stdout_api.cpp.o
./tests/CMakeFiles/spdlog-utests.dir/utils.cpp.o
Claus-MBP:build-cmake-ninja clausklein$ find . -type f -name '*.o' -o -name '*.a' | wc
     26      26    1345
Claus-MBP:build-cmake-ninja clausklein$ cd ../build-meson/
Claus-MBP:build-meson clausklein$ find . -type f -name '*.o' -o -name '*.a' | wc
     26      26    1241
Claus-MBP:build-meson clausklein$ find . -type f -name '*.o' -o -name '*.a'
./example/50d858e@@example@exe/example.cpp.o
./example/50d858e@@example_header_only@exe/example.cpp.o
./libspdlog.a
./spdlog@sta/src_async.cpp.o
./spdlog@sta/src_color_sinks.cpp.o
./spdlog@sta/src_file_sinks.cpp.o
./spdlog@sta/src_fmt.cpp.o
./spdlog@sta/src_spdlog.cpp.o
./spdlog@sta/src_stdout_sinks.cpp.o
./tests/59830eb@@spdlog-utests@exe/main.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_async.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_backtrace.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_create_dir.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_daily_logger.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_dup_filter.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_errors.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_file_helper.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_file_logging.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_fmt_helper.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_macros.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_misc.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_mpmc_q.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_pattern_formatter.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_registry.cpp.o
./tests/59830eb@@spdlog-utests@exe/test_stdout_api.cpp.o
./tests/59830eb@@spdlog-utests@exe/utils.cpp.o
Claus-MBP:build-meson clausklein$
"""

import sys
import os
import time
import subprocess


def gettime(command):
    if command is None:
        return 0.0
    print('Running command:', command)
    starttime = time.time()
    subprocess.check_call(command, shell=True, stdout=subprocess.DEVNULL,
                          stderr=subprocess.DEVNULL)
    endtime = time.time()
    return endtime - starttime


def measure():
    measurements = [
        #NO! ['cmake-make', 'rm -rf build-cmake && mkdir -p build-cmake && CXX=g++ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -B build-cmake .',
            # 'make -C build-cmake -j 4', 'make -C build-cmake -j 4 clean'],
        ['cmake-ninja', 'rm -rf build-cmake-ninja && mkdir -p build-cmake-ninja && CXX=\'ccache clang++\' cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DSPDLOG_BUILD_EXAMPLE_HO=ON -G Ninja -B build-cmake-ninja .',
            'ninja -j 2 -C build-cmake-ninja', 'ninja -j 2 -C build-cmake-ninja clean'],
        ['meson', 'rm -rf build-meson && mkdir -p build-meson && CXX=\'ccache clang++\' meson build-meson',
            'ninja -j 2 -C build-meson', 'ninja -j 2 -C build-meson clean'],
    ]
    results = []
    for m in measurements:
        cur = []
        results.append(cur)
        cur.append(m[0])
        conf = m[1]
        make = m[2]
        clean = m[3]
        cur.append(gettime(conf))
        cur.append(gettime(make))
        cur.append(gettime(make))
        cur.append(gettime(clean))
        cur.append(gettime(make))
    return results


def print_times(times):
    for t in times:
        print(t[0])
        print(" %.3f gen" % t[1])
        print(" %.3f build" % t[2])
        print(" %.3f empty build" % t[3])
        print(" %.3f clean" % t[4])
        print(" %.3f rebuild" % t[5])
        overall = t[1] + t[2] + t[3] + t[4] + t[5]
        print(" %.3f overall" % overall)


if __name__ == '__main__':
    # if len(sys.argv) != 2:
    #     print(sys.argv[0], '<output dir>')
    # os.chdir(sys.argv[1])
    print_times(measure())
