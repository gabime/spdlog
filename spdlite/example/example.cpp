// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlite/spdlite.h"
#include "spdlite/spdlite_global.h"

#define SPDLITE_ACTIVE_LEVEL SPDLITE_LEVEL_TRACE
#include "spdlite/spdlite_macros.h"
int main()
{
    SPDLITE_TRACE("SOME INFO {}", 123);
}