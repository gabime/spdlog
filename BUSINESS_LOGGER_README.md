# 业务日志系统使用说明

## 概述

业务日志系统基于spdlog开发，支持根据不同的业务类型生成独立的日志文件。每个业务类型的日志文件会自动轮转，最多保留3个文件，每个文件最大30MB。

## 功能特性

1. **按业务类型分离日志**：支持录制屏幕、打开桌面、录制键盘、录制音量四种业务类型
2. **自动日志轮转**：每个日志文件达到30MB后自动创建新文件
3. **日志文件数量限制**：每个业务类型最多保留3个日志文件
4. **线程安全**：支持多线程并发写入
5. **日志级别控制**：支持运行时动态调整日志级别
6. **格式化输出**：支持丰富的日志格式化选项

## 使用方法

### 1. 初始化

```cpp
#include "spdlog/business_logger.h"

using namespace business_logger;

// 初始化业务日志系统
// 参数说明:
// - log_dir: 日志文件目录，默认为"logs"
// - max_file_size: 单个日志文件最大大小，默认为30MB
// - max_files: 每个业务类型保留的日志文件数量，默认为3
BusinessLogger::getInstance().initialize("business_logs", 30 * 1024 * 1024, 3);
```

### 2. 写入日志

```cpp
// 使用宏定义写入日志（推荐）
BUSINESS_LOG_INFO(BusinessType::SCREEN_RECORD, "开始录制屏幕");
BUSINESS_LOG_DEBUG(BusinessType::KEYBOARD_RECORD, "按键事件: KeyCode={}", 42);
BUSINESS_LOG_WARN(BusinessType::VOLUME_RECORD, "音量警告: 音量过高");
BUSINESS_LOG_ERROR(BusinessType::DESKTOP_OPEN, "打开桌面失败: {}", error_msg);

// 使用实例方法写入日志
BusinessLogger::getInstance().log(
    BusinessType::SCREEN_RECORD, 
    spdlog::level::info, 
    "录制帧号: {}", frame_number
);
```

### 3. 业务类型

```cpp
enum class BusinessType {
    SCREEN_RECORD,      // 录制屏幕
    DESKTOP_OPEN,       // 打开桌面
    KEYBOARD_RECORD,    // 录制键盘
    VOLUME_RECORD       // 录制音量
};
```

### 4. 日志级别

```cpp
spdlog::level::trace    // 最详细的日志
spdlog::level::debug    // 调试信息
spdlog::level::info     // 一般信息
spdlog::level::warn     // 警告信息
spdlog::level::err      // 错误信息
spdlog::level::critical // 严重错误
spdlog::level::off      // 关闭日志
```

### 5. 控制日志级别

```cpp
// 设置特定业务类型的日志级别
BusinessLogger::getInstance().setLogLevel(
    BusinessType::SCREEN_RECORD, 
    spdlog::level::warn
);

// 设置所有业务类型的日志级别
BusinessLogger::getInstance().setAllLogLevels(spdlog::level::debug);
```

### 6. 刷新日志

```cpp
// 刷新特定业务类型的日志
BusinessLogger::getInstance().flush(BusinessType::SCREEN_RECORD);

// 刷新所有业务类型的日志
BusinessLogger::getInstance().flushAll();
```

### 7. 获取日志器实例

```cpp
// 获取特定业务类型的日志器
auto logger = BusinessLogger::getInstance().getLogger(BusinessType::SCREEN_RECORD);
if (logger) {
    logger->info("直接使用日志器");
}
```

## 日志文件命名规则

日志文件保存在指定的日志目录下，命名规则如下：

```
business_logs/
├── screen_record.log      # 屏幕录制日志
├── screen_record.1.log    # 屏幕录制日志备份1
├── screen_record.2.log    # 屏幕录制日志备份2
├── desktop_open.log       # 桌面打开日志
├── desktop_open.1.log    # 桌面打开日志备份1
├── desktop_open.2.log    # 桌面打开日志备份2
├── keyboard_record.log    # 键盘录制日志
├── keyboard_record.1.log  # 键盘录制日志备份1
├── keyboard_record.2.log  # 键盘录制日志备份2
├── volume_record.log      # 音量录制日志
├── volume_record.1.log    # 音量录制日志备份1
└── volume_record.2.log    # 音量录制日志备份2
```

## 日志格式

默认日志格式为：
```
[2024-01-16 10:30:45.123] [screen_record_logger] [info] [thread_id] 日志内容
```

## 完整示例

```cpp
#include "spdlog/business_logger.h"
#include <thread>
#include <chrono>

int main() {
    using namespace business_logger;
    
    // 初始化
    BusinessLogger::getInstance().initialize("business_logs", 30 * 1024 * 1024, 3);
    
    // 屏幕录制日志
    BUSINESS_LOG_INFO(BusinessType::SCREEN_RECORD, "开始录制屏幕");
    for (int i = 0; i < 100; i++) {
        BUSINESS_LOG_DEBUG(BusinessType::SCREEN_RECORD, "录制帧: {}", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    BUSINESS_LOG_INFO(BusinessType::SCREEN_RECORD, "录制完成");
    
    // 键盘录制日志
    BUSINESS_LOG_INFO(BusinessType::KEYBOARD_RECORD, "开始键盘录制");
    BUSINESS_LOG_DEBUG(BusinessType::KEYBOARD_RECORD, "按键: A");
    BUSINESS_LOG_DEBUG(BusinessType::KEYBOARD_RECORD, "按键: B");
    BUSINESS_LOG_INFO(BusinessType::KEYBOARD_RECORD, "录制完成");
    
    // 刷新所有日志
    BusinessLogger::getInstance().flushAll();
    
    return 0;
}
```

## 编译和运行

### 使用CMake编译

```bash
cd spdlog_a
mkdir build && cd build
cmake ..
cmake --build .
```

### 运行示例程序

```bash
./business_logger_example
```

## 注意事项

1. **线程安全**：BusinessLogger是线程安全的，可以在多线程环境中使用
2. **日志轮转**：当日志文件达到30MB时，会自动轮转，最多保留3个文件
3. **性能考虑**：异步日志模式可以提高性能，但需要确保在程序退出前刷新所有日志
4. **错误处理**：如果日志文件创建失败，会通过spdlog输出错误信息
5. **日志目录**：确保日志目录有写入权限，否则会创建失败

## 扩展业务类型

如果需要添加新的业务类型，可以修改`BusinessType`枚举：

```cpp
enum class BusinessType {
    SCREEN_RECORD,
    DESKTOP_OPEN,
    KEYBOARD_RECORD,
    VOLUME_RECORD,
    // 添加新的业务类型
    NEW_BUSINESS_TYPE
};
```

然后在`BusinessLogger`类的`getLogFileName`和`getLoggerName`方法中添加对应的处理逻辑。
