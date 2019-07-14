#!/bin/bash

cd "$(dirname "$0")"

clang-tidy ../example/example.cpp -- -I ../include
