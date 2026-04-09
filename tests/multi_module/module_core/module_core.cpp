#include "module_core.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/details/os.h"
#include <mutex>

namespace module_core {

namespace {
std::once_flag init_flag;                      // 用于线程安全初始化的标志
const char* const LOGGER_NAME = "core_logger"; // 日志器在注册表中的名称
const int MODULE_ID = 1;                       // 模块唯一标识符

/**
 * @brief 跨平台路径拼接函数
 * @param dir 目录路径
 * @param filename 文件名
 * @return spdlog::filename_t 拼接后的完整路径
 * 
 * 自动处理 Windows 和 Unix 系统的路径分隔符差异：
 * - Windows: 使用反斜杠 '\'
 * - Unix/Linux/macOS: 使用正斜杠 '/'
 * 
 * 同时支持 SPDLOG_WCHAR_FILENAMES 宏定义的宽字符文件名。
 */
spdlog::filename_t path_join(const spdlog::filename_t& dir, const spdlog::filename_t& filename) {
    if (dir.empty()) {
        return filename;
    }
    
    spdlog::filename_t result = dir;
    
    if (!result.empty()) {
        auto last_char = result.back();
        // 检查是否已有路径分隔符
        if (last_char != SPDLOG_FILENAME_T('/') && last_char != SPDLOG_FILENAME_T('\\')) {
#ifdef _WIN32
            // Windows 平台使用反斜杠
            result += SPDLOG_FILENAME_T('\\');
#else
            // Unix/Linux/macOS 平台使用正斜杠
            result += SPDLOG_FILENAME_T('/');
#endif
        }
    }
    
    result += filename;
    return result;
}
}

/**
 * @brief 获取 CoreLogger 单例实例
 * @return CoreLogger& 单例引用
 * 
 * 使用 Meyer's Singleton 模式，C++11 及以上标准保证线程安全。
 * 静态局部变量在首次调用时初始化，且只初始化一次。
 */
CoreLogger& CoreLogger::instance() {
    static CoreLogger instance;
    return instance;
}

/**
 * @brief 初始化核心日志器
 * @param log_dir 日志文件存放目录
 * 
 * 初始化流程：
 * 1. 检查是否已初始化，避免重复初始化
 * 2. 拼接日志文件完整路径
 * 3. 创建文件 sink（basic_file_sink_mt，线程安全）
 * 4. 创建控制台 sink（stdout_color_sink_mt，带颜色输出）
 * 5. 组合多个 sink 创建日志器
 * 6. 设置日志格式、级别和自动刷新策略
 * 7. 注册到 spdlog 全局注册表
 * 
 * 日志格式说明：
 * - [%Y-%m-%d %H:%M:%S.%e] - 时间戳（年-月-日 时:分:秒.毫秒）
 * - [%n] - 日志器名称
 * - [%l] - 日志级别
 * - [%t] - 线程 ID
 * - %v - 实际日志消息
 */
void CoreLogger::init(const spdlog::filename_t& log_dir) {
    if (initialized_) {
        return;
    }
    
    // 拼接日志文件路径
    log_file_ = path_join(log_dir, SPDLOG_FILENAME_T("core.log"));
    
    // 创建文件 sink：追加模式（true），线程安全版本（_mt 后缀）
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_, true);
    
    // 创建控制台 sink：带颜色输出，线程安全版本
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    // 组合多个 sink，日志将同时输出到文件和控制台
    std::vector<spdlog::sink_ptr> sinks = {file_sink, console_sink};
    logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());
    
    // 设置日志输出格式
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
    
    // 设置默认日志级别为 DEBUG（输出所有级别日志）
    logger_->set_level(spdlog::level::debug);
    
    // 设置自动刷新级别：WARN 及以上级别日志立即刷新
    logger_->flush_on(spdlog::level::warn);
    
    // 注册到 spdlog 全局注册表，可通过 spdlog::get("core_logger") 获取
    spdlog::register_logger(logger_);
    initialized_ = true;
}

/**
 * @brief 关闭日志器并释放资源
 * 
 * 清理流程：
 * 1. 刷新所有缓冲的日志消息
 * 2. 从 spdlog 注册表中移除日志器
 * 3. 释放日志器共享指针
 * 4. 重置初始化标志
 */
void CoreLogger::shutdown() {
    if (logger_) {
        // 刷新所有缓冲区，确保所有日志都已写入
        logger_->flush();
        
        // 从全局注册表中移除
        spdlog::drop(LOGGER_NAME);
        
        // 释放日志器实例
        logger_.reset();
    }
    initialized_ = false;
}

/**
 * @brief 获取底层 spdlog 日志器指针
 * @return std::shared_ptr<spdlog::logger> 日志器共享指针
 * 
 * 允许直接访问底层日志器以进行高级配置。
 */
std::shared_ptr<spdlog::logger> CoreLogger::get_logger() const {
    return logger_;
}

/**
 * @brief 获取日志文件完整路径
 * @return spdlog::filename_t 日志文件路径
 * 
 * 返回类型为 spdlog::filename_t，支持宽字符文件名（Windows SPDLOG_WCHAR_FILENAMES）。
 */
spdlog::filename_t CoreLogger::get_log_file() const {
    return log_file_;
}

/**
 * @brief 记录 INFO 级别日志
 * @param message 日志消息内容
 * 
 * INFO 级别用于记录程序正常运行的重要事件，如：
 * - 服务启动/停止
 * - 配置加载完成
 * - 用户操作记录
 * 
 * 消息自动添加 "[Core]" 前缀，便于在多模块环境中识别来源。
 */
void CoreLogger::log_info(const std::string& message) {
    if (logger_) {
        logger_->info("[Core] {}", message);
    }
}

/**
 * @brief 记录 WARN 级别日志
 * @param message 日志消息内容
 * 
 * WARN 级别用于记录潜在的问题或不寻常的情况，但程序仍可继续运行，如：
 * - 配置使用默认值
 * - 性能警告
 * - 重试操作
 */
void CoreLogger::log_warn(const std::string& message) {
    if (logger_) {
        logger_->warn("[Core] {}", message);
    }
}

/**
 * @brief 记录 ERROR 级别日志
 * @param message 日志消息内容
 * 
 * ERROR 级别用于记录错误事件，可能影响功能但程序仍可运行，如：
 * - 数据库连接失败
 * - 文件读写错误
 * - API 调用失败
 */
void CoreLogger::log_error(const std::string& message) {
    if (logger_) {
        logger_->error("[Core] {}", message);
    }
}

/**
 * @brief 记录 DEBUG 级别日志
 * @param message 日志消息内容
 * 
 * DEBUG 级别用于记录详细的调试信息，通常只在开发环境启用，如：
 * - 变量值输出
 * - 函数调用追踪
 * - 详细执行流程
 */
void CoreLogger::log_debug(const std::string& message) {
    if (logger_) {
        logger_->debug("[Core] {}", message);
    }
}

/**
 * @brief 设置日志输出级别
 * @param level 新的日志级别
 * 
 * 日志级别从低到高：
 * - trace < debug < info < warn < error < critical < off
 * 
 * 只有级别高于或等于设置值的日志才会被输出。
 * 例如设置为 info 时，debug 和 trace 级别的日志将被过滤。
 */
void CoreLogger::set_level(spdlog::level::level_enum level) {
    if (logger_) {
        logger_->set_level(level);
    }
}

/**
 * @brief 获取当前日志级别
 * @return spdlog::level::level_enum 当前日志级别
 * 
 * 如果日志器未初始化，返回 spdlog::level::off。
 */
spdlog::level::level_enum CoreLogger::get_level() const {
    if (logger_) {
        return logger_->level();
    }
    return spdlog::level::off;
}

/**
 * @brief 手动刷新日志缓冲区
 * 
 * 将所有缓冲的日志消息立即写入目标 sink。
 * 通常在程序退出前或需要确保日志已持久化时调用。
 */
void CoreLogger::flush() {
    if (logger_) {
        logger_->flush();
    }
}

/**
 * @brief 测试函数
 * 
 * 用于验证模块基本功能的简单测试函数。
 * 记录一条测试消息到核心日志器。
 */
void test_function() {
    auto& core_logger = CoreLogger::instance();
    core_logger.log_info("Test message from core module");
}

/**
 * @brief 获取模块唯一标识符
 * @return int 模块 ID (1)
 * 
 * 在多模块环境中用于唯一标识日志来源。
 * 各模块的 ID 定义：
 * - Core: 1
 * - Network: 2
 * - Data: 3
 */
int get_module_id() {
    return MODULE_ID;
}

}
