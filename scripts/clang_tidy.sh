#!/bin/bash

cd "$(dirname "$0")"
cd ..

clang-tidy example/example.cpp -- -I ./include
