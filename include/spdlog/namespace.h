// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// SPDLOG_NAMESPACE is the outermost namespace token used to qualify spdlog names
// in contexts outside a SPDLOG_NAMESPACE_BEGIN block (e.g. logging macros).
// Override it to rename the outer namespace.
#ifndef SPDLOG_NAMESPACE
    #define SPDLOG_NAMESPACE spdlog
#endif
// SPDLOG_NAMESPACE_BEGIN / SPDLOG_NAMESPACE_END delimit every spdlog
// declaration, defaulting to "namespace SPDLOG_NAMESPACE { ... }".
// Override both to use a more complex namespace expression, e.g. to wrap
// symbols in an inline sub-namespace (keeps "spdlog::" accessible).
// In C++11/17:
//   #define SPDLOG_NAMESPACE_BEGIN  namespace spdlog { inline namespace my_copy {
//   #define SPDLOG_NAMESPACE_END    } }
// In C++20, the nested inline-namespace syntax may be used instead:
//   #define SPDLOG_NAMESPACE_BEGIN  namespace spdlog::inline my_copy {
//   #define SPDLOG_NAMESPACE_END    }
#ifndef SPDLOG_NAMESPACE_BEGIN
    #define SPDLOG_NAMESPACE_BEGIN namespace SPDLOG_NAMESPACE {
#endif
#ifndef SPDLOG_NAMESPACE_END
    #define SPDLOG_NAMESPACE_END }
#endif
