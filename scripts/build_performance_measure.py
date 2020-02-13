#!/usr/bin/python3 -tt

"""
clausklein$ python scripts/build_performance_measure.py $PWD

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
 4.409 gen
 2.676 build
 0.019 empty build
 0.047 clean
 1.948 rebuild
 9.099 overall
meson
 4.971 gen
 2.137 build
 0.017 empty build
 0.040 clean
 1.872 rebuild
 9.038 overall

clausklein$ find build-cmake-ninja -type f -name '*.o' -o -name '*.a' | wc
     43      43    3052

clausklein$ find build-meson -type f -name '*.o' -o -name '*.a' | wc
     26      26    2656
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
    if len(sys.argv) != 2:
        print(sys.argv[0], '<output dir>')
    os.chdir(sys.argv[1])
    print_times(measure())
