//
// Copyright(c) 2019 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

// core types, forward declarations and defines used by spdlog

#pragma once

namespace spdlog
{
namespace lite
{
    enum class level{
        trace,
        debug,
        info,
        warning,
        error,
        critical,
        off

    };
}}