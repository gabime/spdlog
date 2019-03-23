#!/bin/bash
clang-tidy example/example.cpp -- -I ./include
clang-tidy lite-example/example.cpp -- -I./include -I./lite
