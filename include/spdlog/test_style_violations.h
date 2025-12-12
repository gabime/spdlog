// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// This file contains intentional style violations for testing the code style checker

#include <spdlog/common.h>
#include <string>
#include <memory>

namespace spdlog {
namespace details {

// Style violation: class name should be PascalCase (Google style)
// Current: log_processor -> Should be: LogProcessor
class SPDLOG_API log_processor {
public:
    // Style violation: function name should be PascalCase (Google style)
    // Current: processMessage -> Should be: ProcessMessage
    void processMessage(const std::string& msg);
    
    // Style violation: function name should be PascalCase
    // Current: getCount -> Should be: GetCount
    int GetCount() const { return messageCount; }
    
    // Style violation: member variable should have trailing underscore
    // Current: messageCount -> Should be: message_count_
    int messageCount = 0;

private:
    // Style violation: member variable missing trailing underscore
    // Current: bufferSize -> Should be: buffer_size_
    size_t bufferSize;
    
    // Correct style: member with trailing underscore
    std::string name_;
};

// Style violation: class name should be PascalCase
// Current: async_helper -> Should be: AsyncHelper  
class async_helper {
public:
    // Style violation: constructor parameter should be snake_case
    // Current: maxQueueSize -> Should be: max_queue_size
    explicit async_helper(size_t maxQueueSize)
        : max_queue_size_(maxQueueSize),
          isRunning(false) {}
    
    // Style violation: function should be PascalCase
    // Current: startProcessing -> Should be: StartProcessing
    void startProcessing();
    
    // Style violation: function should be PascalCase
    // Current: stopProcessing -> Should be: StopProcessing
    void stopProcessing();
    
    // Style violation: constant should use kConstantName style
    // Current: DEFAULT_QUEUE_SIZE -> Should be: kDefaultQueueSize
    static constexpr size_t DEFAULT_QUEUE_SIZE = 8192;

private:
    size_t max_queue_size_;
    
    // Style violation: member variable missing trailing underscore
    // Current: isRunning -> Should be: is_running_
    bool isRunning;
    
    // Style violation: member variable camelCase instead of snake_case_
    // Current: workerThread -> Should be: worker_thread_
    std::unique_ptr<void*> workerThread;
};

// Style violation: function name should be PascalCase
// Current: formatLogLevel -> Should be: FormatLogLevel
inline std::string formatLogLevel(int level) {
    switch (level) {
        case 0: return "TRACE";
        case 1: return "DEBUG";
        case 2: return "INFO";
        default: return "UNKNOWN";
    }
}

// Style violation: global constant should use kConstantName
// Current: MAX_LOG_SIZE -> Should be: kMaxLogSize
constexpr size_t MAX_LOG_SIZE = 1024 * 1024;

// Style violation: variable name should be snake_case
// Current: defaultLogLevel -> Should be: default_log_level
inline int defaultLogLevel = 2;

}  // namespace details
}  // namespace spdlog
