//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
//
// spdlog usage example
//
//

#include "spdlog/spdlog.h"

int main(int, char *[])
{
    int i = 123;
    spdlog::info("HELLO STATIC! {}", i);
}