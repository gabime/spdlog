// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlite.h"
#include "spdlite_global.h"

#define SPDLITE_ACTIVE_LEVEL SPDLITE_LEVEL_TRACE
#include "spdlite_macros.h"
int main()
{
    SPDLITE_TRACE("SOME INFO {}", 123);
}