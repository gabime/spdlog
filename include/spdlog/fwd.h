// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/namespace.h"

SPDLOG_NAMESPACE_BEGIN
SPDLOG_EXPORT class logger;
SPDLOG_EXPORT class formatter;

namespace sinks {
SPDLOG_EXPORT class sink;
}

namespace level {
SPDLOG_EXPORT enum level_enum : int;
}

SPDLOG_NAMESPACE_END
