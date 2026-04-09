#include "module_network.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/details/os.h"
#include <mutex>
#include <random>

namespace module_network {

namespace {
const char* const LOGGER_NAME = "network_logger";   // 日志器在注册表中的唯一名称
const int MODULE_ID = 2;                            // 模块唯一标识符

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
 * 如果目录路径为空，直接返回文件名。
 */
spdlog::filename_t path_join(const spdlog::filename_t& dir, const spdlog::filename_t& filename) {
    if (dir.empty()) {
        return filename;
    }
    
    spdlog::filename_t result = dir;
    
    if (!result.empty()) {
        auto last_char = result.back();
        // 检查是否已有路径分隔符（支持两种格式）
        if (last_char != SPDLOG_FILENAME_T('/') && last_char != SPDLOG_FILENAME_T('\\')) {
#ifdef _WIN32
            // Windows 平台使用反斜杠作为路径分隔符
            result += SPDLOG_FILENAME_T('\\');
#else
            // Unix/Linux/macOS 平台使用正斜杠作为路径分隔符
            result += SPDLOG_FILENAME_T('/');
#endif
        }
    }
    
    result += filename;
    return result;
}
}

/**
 * @brief 获取 NetworkLogger 单例实例
 * @return NetworkLogger& 单例引用
 * 
 * 使用 Meyer's Singleton 模式，C++11 及以上标准保证线程安全。
 * 静态局部变量在首次调用时初始化，且只初始化一次。
 */
NetworkLogger& NetworkLogger::instance() {
    static NetworkLogger instance;
    return instance;
}

/**
 * @brief 初始化网络日志器
 * @param log_dir 日志文件目录路径
 * 
 * 初始化流程：
 * 1. 检查是否已初始化，避免重复初始化
 * 2. 拼接日志文件路径（network.log）
 * 3. 创建文件 sink（basic_file_sink_mt，线程安全，追加模式）
 * 4. 创建控制台 sink（stdout_color_sink_mt，带颜色输出）
 * 5. 组合多个 sink 创建日志器
 * 6. 设置日志格式、级别（默认 INFO）和自动刷新策略（ERROR）
 * 7. 注册到 spdlog 全局注册表
 * 
 * 注意事项：
 * - 网络模块默认日志级别为 INFO，DEBUG 级别的日志（如数据收发详情）默认不会输出
 * - 只有 ERROR 级别的日志会触发自动刷新
 * - 日志文件名为 "network.log"
 */
void NetworkLogger::init(const spdlog::filename_t& log_dir) {
    if (initialized_) {
        return;
    }
    
    // 拼接日志文件完整路径
    log_file_ = path_join(log_dir, SPDLOG_FILENAME_T("network.log"));
    
    // 创建文件 sink：追加模式（true），线程安全版本（_mt 后缀）
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_, true);
    
    // 创建控制台 sink：带颜色输出，线程安全版本
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    // 组合多个 sink，日志将同时输出到文件和控制台
    std::vector<spdlog::sink_ptr> sinks = {file_sink, console_sink};
    logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());
    
    // 设置日志输出格式
    // 格式说明：
    // - [%Y-%m-%d %H:%M:%S.%e] - 时间戳（年-月-日 时:分:秒.毫秒）
    // - [%n] - 日志器名称
    // - [%l] - 日志级别
    // - [%t] - 线程 ID
    // - %v - 实际日志消息
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
    
    // 设置默认日志级别为 INFO
    // 这意味着 DEBUG 和 TRACE 级别的日志默认不会输出
    logger_->set_level(spdlog::level::info);
    
    // 设置自动刷新级别：只有 ERROR 及以上级别日志会立即刷新
    // 这是网络模块与核心模块的区别之一
    logger_->flush_on(spdlog::level::err);
    
    // 注册到 spdlog 全局注册表，可通过 spdlog::get("network_logger") 获取
    spdlog::register_logger(logger_);
    initialized_ = true;
}

/**
 * @brief 关闭日志器并释放资源
 * 
 * 清理流程：
 * 1. 检查日志器是否存在
 * 2. 刷新所有缓冲的日志消息
 * 3. 从 spdlog 全局注册表中移除日志器
 * 4. 释放日志器实例（通过 reset() 减少引用计数）
 * 5. 重置初始化标志
 * 
 * 注意：应该在程序退出前或不再需要日志功能时调用此函数。
 */
void NetworkLogger::shutdown() {
    if (logger_) {
        // 刷新所有缓冲区，确保所有日志都已写入目标 sink
        logger_->flush();
        
        // 从全局注册表中移除，避免内存泄漏和名称冲突
        spdlog::drop(LOGGER_NAME);
        
        // 释放日志器实例（shared_ptr 的 reset() 会减少引用计数）
        logger_.reset();
    }
    initialized_ = false;
}

/**
 * @brief 获取底层 spdlog 日志器指针
 * @return std::shared_ptr<spdlog::logger> 日志器共享指针
 * 
 * 允许直接访问底层日志器以进行高级配置，例如：
 * - 添加自定义 sink
 * - 修改日志格式
 * - 设置错误处理器
 */
std::shared_ptr<spdlog::logger> NetworkLogger::get_logger() const {
    return logger_;
}

/**
 * @brief 获取日志文件路径
 * @return spdlog::filename_t 日志文件完整路径
 * 
 * 返回类型为 spdlog::filename_t，支持宽字符文件名（Windows SPDLOG_WCHAR_FILENAMES）。
 * 使用 test_utils::filename_to_string() 可转换为 std::string。
 */
spdlog::filename_t NetworkLogger::get_log_file() const {
    return log_file_;
}

/**
 * @brief 记录网络连接事件
 * @param host 目标主机地址（IP 或域名）
 * @param port 目标端口号
 * 
 * 日志级别：INFO
 * 消息格式：[Network] Connected to {host}:{port}
 * 
 * 在成功建立网络连接时调用，记录连接的目标地址和端口。
 * 由于默认日志级别为 INFO，此日志默认会输出。
 */
void NetworkLogger::log_connection(const std::string& host, int port) {
    if (logger_) {
        logger_->info("[Network] Connected to {}:{}", host, port);
    }
}

/**
 * @brief 记录网络断开事件
 * @param host 目标主机地址
 * @param port 目标端口号
 * 
 * 日志级别：INFO
 * 消息格式：[Network] Disconnected from {host}:{port}
 * 
 * 在主动断开连接或连接被关闭时调用。
 */
void NetworkLogger::log_disconnection(const std::string& host, int port) {
    if (logger_) {
        logger_->info("[Network] Disconnected from {}:{}", host, port);
    }
}

/**
 * @brief 记录数据发送事件
 * @param host 目标主机地址
 * @param bytes 发送的字节数
 * 
 * 日志级别：DEBUG
 * 消息格式：[Network] Sent {bytes} bytes to {host}
 * 
 * 注意：由于默认日志级别为 INFO，此日志默认不会输出。
 * 需要显式调用 set_level(spdlog::level::debug) 才能看到。
 * 
 * 用于追踪详细的数据传输情况，适合调试网络通信问题。
 */
void NetworkLogger::log_data_sent(const std::string& host, size_t bytes) {
    if (logger_) {
        logger_->debug("[Network] Sent {} bytes to {}", bytes, host);
    }
}

/**
 * @brief 记录数据接收事件
 * @param host 源主机地址
 * @param bytes 接收的字节数
 * 
 * 日志级别：DEBUG
 * 消息格式：[Network] Received {bytes} bytes from {host}
 * 
 * 注意：由于默认日志级别为 INFO，此日志默认不会输出。
 * 
 * 与 log_data_sent() 配对使用，用于双向数据传输追踪。
 */
void NetworkLogger::log_data_received(const std::string& host, size_t bytes) {
    if (logger_) {
        logger_->debug("[Network] Received {} bytes from {}", bytes, host);
    }
}

/**
 * @brief 记录网络错误事件
 * @param error_msg 错误描述信息
 * 
 * 日志级别：ERROR
 * 消息格式：[Network] Error: {error_msg}
 * 
 * 注意事项：
 * - ERROR 级别的日志会触发自动刷新（flush_on 设置）
 * - 此日志默认会输出（ERROR 级别高于 INFO）
 * 
 * 在发生网络错误时调用，例如：
 * - 连接超时
 * - 连接被拒绝
 * - 数据传输失败
 * - DNS 解析失败
 */
void NetworkLogger::log_error(const std::string& error_msg) {
    if (logger_) {
        logger_->error("[Network] Error: {}", error_msg);
    }
}

/**
 * @brief 设置日志级别
 * @param level 新的日志级别
 * 
 * 可用于动态调整日志详细程度，例如：
 * - 生产环境：INFO 或 WARN（减少日志量，提高性能）
 * - 调试环境：DEBUG 或 TRACE（详细日志，便于排查问题）
 * 
 * 日志级别从低到高：
 * - trace < debug < info < warn < error < critical < off
 * 
 * 只有级别高于或等于设置值的日志才会被输出。
 */
void NetworkLogger::set_level(spdlog::level::level_enum level) {
    if (logger_) {
        logger_->set_level(level);
    }
}

/**
 * @brief 获取当前日志级别
 * @return spdlog::level::level_enum 当前日志级别
 * 
 * 如果日志器未初始化，返回 spdlog::level::off（所有日志都被禁用）。
 */
spdlog::level::level_enum NetworkLogger::get_level() const {
    if (logger_) {
        return logger_->level();
    }
    return spdlog::level::off;
}

/**
 * @brief 手动刷新日志缓冲区
 * 
 * 将所有缓冲的日志消息立即写入目标 sink。
 * 
 * 注意：
 * - 对于同步日志器，flush() 会立即执行 I/O 操作
 * - ERROR 级别的日志会自动触发 flush（通过 flush_on 设置）
 * - 在程序退出前或需要确保日志已持久化时调用
 */
void NetworkLogger::flush() {
    if (logger_) {
        logger_->flush();
    }
}

/**
 * @brief 构造函数
 * 
 * 默认构造函数，初始化连接列表为空。
 * 使用 = default 表示使用编译器生成的默认实现。
 */
NetworkSimulator::NetworkSimulator() = default;

/**
 * @brief 模拟连接到远程主机
 * @param host 目标主机地址（IP 或域名）
 * @param port 目标端口号
 * 
 * 执行操作：
 * 1. 获取网络日志器单例实例
 * 2. 记录连接事件日志（INFO 级别）
 * 3. 将 (host, port) 对添加到连接列表
 * 
 * 注意：这是模拟函数，不进行真实的网络连接。
 * 只是为了测试日志功能而设计。
 */
void NetworkSimulator::connect(const std::string& host, int port) {
    auto& logger = NetworkLogger::instance();
    logger.log_connection(host, port);
    connections_.emplace_back(host, port);
}

/**
 * @brief 模拟断开连接
 * @param host 目标主机地址
 * @param port 目标端口号
 * 
 * 执行操作：
 * 1. 获取网络日志器单例实例
 * 2. 记录断开事件日志（INFO 级别）
 * 3. 从连接列表中移除指定的 (host, port) 对
 * 
 * 使用 std::remove_if + erase 组合从向量中移除匹配元素：
 * - std::remove_if 将匹配元素移到末尾
 * - erase 删除从返回迭代器到末尾的所有元素
 */
void NetworkSimulator::disconnect(const std::string& host, int port) {
    auto& logger = NetworkLogger::instance();
    logger.log_disconnection(host, port);
    
    // 使用 remove_if + erase 组合移除匹配元素
    auto it = std::remove_if(connections_.begin(), connections_.end(),
        [&](const std::pair<std::string, int>& conn) {
            return conn.first == host && conn.second == port;
        });
    connections_.erase(it, connections_.end());
}

/**
 * @brief 模拟发送数据
 * @param host 目标主机地址
 * @param data 要发送的数据内容
 * 
 * 执行操作：
 * 1. 获取网络日志器单例实例
 * 2. 记录数据发送日志（DEBUG 级别）
 * 
 * 注意：
 * - 不实际发送数据，只是记录日志
 * - 日志级别为 DEBUG，默认不会输出
 * - 记录的字节数是 data.size()
 */
void NetworkSimulator::send_data(const std::string& host, const std::string& data) {
    auto& logger = NetworkLogger::instance();
    logger.log_data_sent(host, data.size());
}

/**
 * @brief 模拟接收数据
 * @param host 源主机地址
 * @param max_bytes 最大接收字节数
 * @return std::string 接收到的数据（由 'X' 字符组成的测试数据）
 * 
 * 执行操作：
 * 1. 初始化随机数生成器（使用 static 确保只初始化一次）
 * 2. 生成 1 到 max_bytes 之间的随机字节数
 * 3. 记录数据接收日志（DEBUG 级别）
 * 4. 返回由 'X' 字符组成的测试数据
 * 
 * 随机数生成说明：
 * - std::random_device: 真随机数生成器（用于种子）
 * - std::mt19937: Mersenne Twister 伪随机数生成器
 * - std::uniform_int_distribution: 均匀整数分布
 * 
 * 使用 static 关键字确保：
 * - 随机数生成器只初始化一次
 * - 每次调用都使用同一个生成器序列
 */
std::string NetworkSimulator::receive_data(const std::string& host, size_t max_bytes) {
    // 使用 static 确保只初始化一次
    // std::random_device 提供真随机种子
    static std::mt19937 rng(std::random_device{}());
    // 均匀分布：[1, max_bytes]
    static std::uniform_int_distribution<size_t> dist(1, max_bytes);
    
    // 生成随机字节数
    size_t bytes_received = dist(rng);
    
    auto& logger = NetworkLogger::instance();
    logger.log_data_received(host, bytes_received);
    
    // 返回由 'X' 字符组成的测试数据
    return std::string(bytes_received, 'X');
}

/**
 * @brief 获取当前连接数量
 * @return size_t 活跃连接数
 * 
 * 返回连接列表的大小，即当前模拟的活跃连接数量。
 */
size_t NetworkSimulator::get_connections_count() const {
    return connections_.size();
}

/**
 * @brief 模拟网络错误
 * @param error_msg 错误描述信息
 * 
 * 用于测试错误日志功能。
 * 记录 ERROR 级别的日志，会触发自动刷新。
 * 
 * 典型用法：
 * @code
 * NetworkSimulator sim;
 * sim.simulate_error("Connection timeout");
 * @endcode
 */
void NetworkSimulator::simulate_error(const std::string& error_msg) {
    auto& logger = NetworkLogger::instance();
    logger.log_error(error_msg);
}

/**
 * @brief 测试网络操作
 * 
 * 用于验证网络模块基本功能的测试函数。
 * 执行一系列模拟网络操作：
 * 1. 创建网络模拟器实例
 * 2. 连接到 192.168.1.1:8080
 * 3. 发送数据 "Hello, World!"
 * 4. 接收最多 1024 字节数据
 * 5. 断开连接
 * 
 * 此函数用于快速验证模块是否正常工作。
 */
void test_network_operations() {
    NetworkSimulator sim;
    sim.connect("192.168.1.1", 8080);
    sim.send_data("192.168.1.1", "Hello, World!");
    sim.receive_data("192.168.1.1", 1024);
    sim.disconnect("192.168.1.1", 8080);
}

/**
 * @brief 获取模块 ID
 * @return int 模块唯一标识符 (2)
 * 
 * 用于多模块环境中标识日志来源。
 * 各模块的 ID 定义：
 * - Core: 1
 * - Network: 2
 * - Data: 3
 * 
 * 在日志隔离测试中用于验证日志来源。
 */
int get_module_id() {
    return MODULE_ID;
}

}
