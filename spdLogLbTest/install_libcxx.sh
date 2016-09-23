#!/bin/bash
#
# Install libc++ under travis

svn --quiet co http://llvm.org/svn/llvm-project/libcxx/trunk libcxx
mkdir libcxx/build
(cd libcxx/build && cmake .. -DLIBCXX_CXX_ABI=libstdc++ -DLIBCXX_CXX_ABI_INCLUDE_PATHS="/usr/include/c++/4.6;/usr/include/c++/4.6/x86_64-linux-gnu")
make -C libcxx/build cxx -j2
sudo cp libcxx/build/lib/libc++.so.1.0 /usr/lib/
sudo cp -r libcxx/build/include/c++/v1 /usr/include/c++/v1/
sudo ln -sf /usr/lib/libc++.so.1.0 /usr/lib/libc++.so
sudo ln -sf /usr/lib/libc++.so.1.0 /usr/lib/libc++.so.1
