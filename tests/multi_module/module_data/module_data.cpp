#include "module_data.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/details/os.h"
#include <thread>
#include <random>

namespace module_data {

namespace {
const char* const LOGGER_NAME = "data_logger";   // 日志器在注册表中的唯一名称
const int MODULE_ID = 3;                         // 模块唯一标识符

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
 * @brief 获取 DataLogger 单例实例
 * @return DataLogger& 单例引用
 * 
 * 使用 Meyer's Singleton 模式，C++11 及以上标准保证线程安全。
 * 静态局部变量在首次调用时初始化，且只初始化一次。
 */
DataLogger& DataLogger::instance() {
    static DataLogger instance;
    return instance;
}

/**
 * @brief 初始化数据日志器（异步模式）
 * @param log_dir 日志文件目录路径
 * @param queue_size 异步队列大小，默认 8192
 * @param thread_count 后台线程数，默认 1
 * 
 * 初始化流程：
 * 1. 检查是否已初始化，避免重复初始化
 * 2. 拼接日志文件路径（data.log）
 * 3. 创建线程池（thread_pool）用于异步日志处理
 * 4. 创建文件 sink（basic_file_sink_mt，线程安全，追加模式）
 * 5. 创建控制台 sink（stdout_color_sink_mt，带颜色输出）
 * 6. 创建异步日志器（async_logger），使用 block 溢出策略
 * 7. 设置日志格式、级别（默认 DEBUG）和自动刷新策略（ERROR）
 * 8. 注册到 spdlog 全局注册表
 * 
 * 异步日志的工作原理：
 * - 主线程调用日志函数时，消息被放入队列
 * - 后台线程从队列中取出消息并写入 sink
 * - 主线程不会阻塞在 I/O 操作上，提高性能
 * 
 * 队列大小说明：
 * - 队列大小决定了可以缓冲的日志消息数量
 * - 如果队列满了，新的日志消息会根据 overflow_policy 处理
 * - 当前使用 block 策略：阻塞等待队列有空间
 * 
 * 线程数说明：
 * - 单线程：简单场景，顺序处理日志
 * - 多线程：高并发场景，并行处理日志
 * 
 * 注意事项：
 * - 数据模块使用异步日志器，适合高吞吐量的数据处理场景
 * - 默认日志级别为 DEBUG，会输出详细的处理日志
 * - 日志文件名为 "data.log"
 */
void DataLogger::init(const spdlog::filename_t& log_dir, size_t queue_size, size_t thread_count) {
    if (initialized_) {
        return;
    }
    
    // 拼接日志文件完整路径
    log_file_ = path_join(log_dir, SPDLOG_FILENAME_T("data.log"));
    
    // 创建线程池：用于异步日志处理
    // 参数：队列大小、后台线程数
    thread_pool_ = std::make_shared<spdlog::details::thread_pool>(queue_size, thread_count);
    
    // 创建文件 sink：追加模式（true），线程安全版本（_mt 后缀）
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_, true);
    
    // 创建控制台 sink：带颜色输出，线程安全版本
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    // 组合多个 sink，日志将同时输出到文件和控制台
    std::vector<spdlog::sink_ptr> sinks = {file_sink, console_sink};
    
    // 创建异步日志器
    // 参数：日志器名称、sink 列表、线程池、溢出策略
    // async_overflow_policy::block: 队列满时阻塞等待
    logger_ = std::make_shared<spdlog::async_logger>(
        LOGGER_NAME, 
        sinks.begin(), 
        sinks.end(),
        thread_pool_,
        spdlog::async_overflow_policy::block
    );
    
    // 设置日志输出格式
    // 格式说明：
    // - [%Y-%m-%d %H:%M:%S.%e] - 时间戳（年-月-日 时:分:秒.毫秒）
    // - [%n] - 日志器名称
    // - [%l] - 日志级别
    // - [%t] - 线程 ID
    // - %v - 实际日志消息
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
    
    // 设置默认日志级别为 DEBUG
    // 这意味着所有级别的日志（DEBUG、INFO、WARN、ERROR、CRITICAL）都会输出
    logger_->set_level(spdlog::level::debug);
    
    // 设置自动刷新级别：只有 ERROR 及以上级别日志会立即刷新
    logger_->flush_on(spdlog::level::err);
    
    // 注册到 spdlog 全局注册表，可通过 spdlog::get("data_logger") 获取
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
 * 5. 释放线程池（重要！异步日志器需要显式释放线程池）
 * 6. 重置初始化标志
 * 
 * 注意事项：
 * - 异步日志器需要额外释放线程池
 * - 应该在程序退出前或不再需要日志功能时调用此函数
 * - 确保所有异步日志消息都已处理完成
 */
void DataLogger::shutdown() {
    if (logger_) {
        // 刷新所有缓冲区，确保所有日志都已写入目标 sink
        // 对于异步日志器，flush() 会等待队列中的所有消息处理完成
        logger_->flush();
        
        // 从全局注册表中移除，避免内存泄漏和名称冲突
        spdlog::drop(LOGGER_NAME);
        
        // 释放日志器实例（shared_ptr 的 reset() 会减少引用计数）
        logger_.reset();
    }
    
    // 释放线程池（异步日志器特有）
    // 这是异步日志器与同步日志器的重要区别
    if (thread_pool_) {
        thread_pool_.reset();
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
 * - 配置异步策略
 */
std::shared_ptr<spdlog::logger> DataLogger::get_logger() const {
    return logger_;
}

/**
 * @brief 获取日志文件路径
 * @return spdlog::filename_t 日志文件完整路径
 * 
 * 返回类型为 spdlog::filename_t，支持宽字符文件名（Windows SPDLOG_WCHAR_FILENAMES）。
 * 使用 test_utils::filename_to_string() 可转换为 std::string。
 */
spdlog::filename_t DataLogger::get_log_file() const {
    return log_file_;
}

/**
 * @brief 记录数据处理开始事件
 * @param dataset_name 数据集名称
 * 
 * 日志级别：INFO
 * 消息格式：[Data] Starting processing dataset: {dataset_name}
 * 
 * 在开始处理数据集时调用，标记处理起点。
 * 用于追踪数据处理的开始时间和数据集信息。
 */
void DataLogger::log_processing_start(const std::string& dataset_name) {
    if (logger_) {
        logger_->info("[Data] Starting processing dataset: {}", dataset_name);
    }
}

/**
 * @brief 记录数据处理结束事件
 * @param dataset_name 数据集名称
 * @param duration 处理耗时（毫秒）
 * @param records_processed 处理的记录数
 * 
 * 日志级别：INFO
 * 消息格式：[Data] Finished processing dataset: {name} in {duration} ms, records: {count}
 * 
 * 在数据处理完成时调用，提供处理统计信息：
 * - 数据集名称
 * - 处理耗时（毫秒）
 * - 处理的记录数量
 * 
 * 用于性能分析和监控数据处理进度。
 */
void DataLogger::log_processing_end(const std::string& dataset_name,
                                     const std::chrono::milliseconds& duration,
                                     size_t records_processed) {
    if (logger_) {
        logger_->info("[Data] Finished processing dataset: {} in {} ms, records: {}",
                      dataset_name, duration.count(), records_processed);
    }
}

/**
 * @brief 记录单条记录处理事件
 * @param record_id 记录 ID
 * @param status 处理状态（如 "success", "warning", "failed"）
 * 
 * 日志级别：DEBUG
 * 消息格式：[Data] Record {record_id} processed: {status}
 * 
 * 注意：由于默认日志级别为 DEBUG，此日志默认会输出。
 * 适合追踪每条记录的处理状态，便于排查问题。
 * 
 * 典型状态值：
 * - "success": 处理成功
 * - "warning": 处理成功但有警告
 * - "failed": 处理失败
 */
void DataLogger::log_record_processed(size_t record_id, const std::string& status) {
    if (logger_) {
        logger_->debug("[Data] Record {} processed: {}", record_id, status);
    }
}

/**
 * @brief 记录数据处理错误
 * @param error_msg 错误描述信息
 * @param record_id 相关记录 ID（可选，默认为 0）
 * 
 * 日志级别：ERROR
 * 消息格式：
 * - 有 record_id: [Data] Error processing record {record_id}: {error_msg}
 * - 无 record_id: [Data] Error: {error_msg}
 * 
 * 注意事项：
 * - ERROR 级别的日志会触发自动刷新（flush_on 设置）
 * - 此日志默认会输出（ERROR 级别高于 DEBUG）
 * 
 * 在发生数据处理错误时调用，例如：
 * - 数据格式错误
 * - 数据验证失败
 * - 处理逻辑异常
 * - 外部依赖失败
 */
void DataLogger::log_error(const std::string& error_msg, size_t record_id) {
    if (logger_) {
        if (record_id > 0) {
            // 有记录 ID 的错误
            logger_->error("[Data] Error processing record {}: {}", record_id, error_msg);
        } else {
            // 无记录 ID 的通用错误
            logger_->error("[Data] Error: {}", error_msg);
        }
    }
}

/**
 * @brief 记录数据处理警告
 * @param warning_msg 警告描述信息
 * 
 * 日志级别：WARN
 * 消息格式：[Data] Warning: {warning_msg}
 * 
 * 用于记录非致命但需要注意的情况，例如：
 * - 数据异常但可继续处理
 * - 数据格式不规范但已兼容
 * - 性能警告
 * - 资源使用警告
 * 
 * 与 ERROR 不同，WARN 级别的日志不会中断处理流程。
 */
void DataLogger::log_warning(const std::string& warning_msg) {
    if (logger_) {
        logger_->warn("[Data] Warning: {}", warning_msg);
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
 * 
 * 注意：对于异步日志器，修改日志级别是线程安全的。
 */
void DataLogger::set_level(spdlog::level::level_enum level) {
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
spdlog::level::level_enum DataLogger::get_level() const {
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
 * 注意事项：
 * - 对于异步日志器，flush() 会等待队列中的所有消息处理完成
 * - 这是一个阻塞操作，会等待后台线程完成处理
 * - ERROR 级别的日志会自动触发 flush（通过 flush_on 设置）
 * - 在程序退出前或需要确保日志已持久化时调用
 */
void DataLogger::flush() {
    if (logger_) {
        logger_->flush();
    }
}

/**
 * @brief 获取队列溢出计数器
 * @return size_t 溢出次数
 * 
 * 当使用 overrun_oldest 策略时，队列满时会丢弃最旧的消息。
 * 此计数器记录被丢弃的消息数量。
 * 
 * 注意：当前使用的是 block 策略，此计数器通常为 0。
 * 
 * 溢出策略说明：
 * - block: 队列满时阻塞等待（当前使用）
 * - overrun_oldest: 队列满时丢弃最旧的消息
 * - discard_new: 队列满时丢弃新消息
 */
size_t DataLogger::get_overrun_counter() const {
    if (thread_pool_) {
        return thread_pool_->overrun_counter();
    }
    return 0;
}

/**
 * @brief 获取消息丢弃计数器
 * @return size_t 丢弃次数
 * 
 * 当使用 discard_new 策略时，队列满时会丢弃新消息。
 * 此计数器记录被丢弃的消息数量。
 * 
 * 注意：当前使用的是 block 策略，此计数器通常为 0。
 */
size_t DataLogger::get_discard_counter() const {
    if (thread_pool_) {
        return thread_pool_->discard_counter();
    }
    return 0;
}

/**
 * @brief 构造函数
 * 
 * 默认构造函数，初始化所有成员变量为默认值。
 * 使用 = default 表示使用编译器生成的默认实现。
 */
DataProcessor::DataProcessor() = default;

/**
 * @brief 加载数据集
 * @param name 数据集名称
 * @param record_count 总记录数
 * 
 * 执行操作：
 * 1. 保存数据集名称
 * 2. 保存总记录数
 * 3. 重置已处理记录计数为 0
 * 4. 记录处理开始日志（INFO 级别）
 * 
 * 注意：这是模拟函数，不实际加载数据。
 * 只是为了测试日志功能而设计。
 */
void DataProcessor::load_dataset(const std::string& name, size_t record_count) {
    dataset_name_ = name;
    total_records_ = record_count;
    processed_records_ = 0;
    
    auto& logger = DataLogger::instance();
    logger.log_processing_start(name);
}

/**
 * @brief 处理所有记录
 * 
 * 执行操作：
 * 1. 记录开始时间（使用 high_resolution_clock）
 * 2. 循环调用 process_record() 处理每条记录
 * 3. 记录结束时间
 * 4. 计算处理耗时（转换为毫秒）
 * 5. 记录处理结束日志（包含耗时和处理数量）
 * 
 * 处理流程：
 * - 顺序处理每条记录
 * - 每条记录的处理状态会被记录
 * - 错误记录不会计入已处理计数
 * 
 * 性能监控：
 * - 记录处理开始和结束时间
 * - 计算总耗时
 * - 记录处理的记录数量
 * - 这些信息可用于性能分析
 */
void DataProcessor::process_all_records() {
    // 记录开始时间
    // high_resolution_clock 提供最高精度的时间戳
    auto start = std::chrono::high_resolution_clock::now();
    
    // 顺序处理每条记录
    for (size_t i = 0; i < total_records_; ++i) {
        process_record(i);
    }
    
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    
    // 计算处理耗时（转换为毫秒）
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 记录处理结束日志
    auto& logger = DataLogger::instance();
    logger.log_processing_end(dataset_name_, duration, processed_records_);
}

/**
 * @brief 处理单条记录
 * @param record_id 记录 ID
 * 
 * 处理逻辑：
 * 1. 获取数据日志器单例实例
 * 2. 检查记录是否在错误列表中 → 记录 ERROR 日志，不计入已处理计数
 * 3. 检查记录是否在警告列表中 → 记录 WARN 和 DEBUG 日志，计入已处理计数
 * 4. 正常记录 → 记录 DEBUG 日志，计入已处理计数
 * 
 * 状态判断：
 * - 使用 std::find 在向量中查找记录 ID
 * - error_records_: 需要模拟错误的记录 ID 列表
 * - warning_records_: 需要模拟警告的记录 ID 列表
 * 
 * 注意：
 * - 错误记录不会增加 processed_records_ 计数
 * - 警告记录和正常记录会增加 processed_records_ 计数
 * - 这是模拟函数，不实际处理数据
 */
void DataProcessor::process_record(size_t record_id) {
    auto& logger = DataLogger::instance();
    
    // 检查记录是否在错误列表中
    // std::find 返回指向找到元素的迭代器，或 end() 表示未找到
    bool is_error = std::find(error_records_.begin(), error_records_.end(), record_id) != error_records_.end();
    
    // 检查记录是否在警告列表中
    bool is_warning = std::find(warning_records_.begin(), warning_records_.end(), record_id) != warning_records_.end();
    
    if (is_error) {
        // 错误记录：记录 ERROR 日志，不计入已处理计数
        logger.log_error("Processing failed", record_id);
    } else if (is_warning) {
        // 警告记录：记录 WARN 和 DEBUG 日志，计入已处理计数
        logger.log_warning("Record " + std::to_string(record_id) + " has anomalies");
        logger.log_record_processed(record_id, "warning");
        processed_records_++;
    } else {
        // 正常记录：记录 DEBUG 日志，计入已处理计数
        logger.log_record_processed(record_id, "success");
        processed_records_++;
    }
}

/**
 * @brief 获取总记录数
 * @return size_t 数据集总记录数
 * 
 * 返回 load_dataset() 时设置的总记录数。
 */
size_t DataProcessor::get_total_records() const {
    return total_records_;
}

/**
 * @brief 获取已处理记录数
 * @return size_t 已成功处理的记录数
 * 
 * 注意：错误记录不计入已处理计数。
 * 警告记录和正常记录会计入已处理计数。
 */
size_t DataProcessor::get_processed_records() const {
    return processed_records_;
}

/**
 * @brief 获取数据集名称
 * @return const std::string& 数据集名称引用
 * 
 * 返回 load_dataset() 时设置的数据集名称。
 */
const std::string& DataProcessor::get_dataset_name() const {
    return dataset_name_;
}

/**
 * @brief 模拟指定记录的处理错误
 * @param record_id 记录 ID
 * 
 * 用于测试错误日志功能。
 * 被标记的记录在处理时会记录 ERROR 级别日志。
 * 
 * 实现原理：
 * - 将 record_id 添加到 error_records_ 向量
 * - process_record() 会检查此向量
 * 
 * 典型用法：
 * @code
 * DataProcessor processor;
 * processor.load_dataset("test_data", 100);
 * processor.simulate_error_on_record(42);  // 记录 42 会模拟错误
 * processor.process_all_records();
 * @endcode
 */
void DataProcessor::simulate_error_on_record(size_t record_id) {
    error_records_.push_back(record_id);
}

/**
 * @brief 模拟指定记录的处理警告
 * @param record_id 记录 ID
 * 
 * 用于测试警告日志功能。
 * 被标记的记录在处理时会记录 WARN 级别日志。
 * 
 * 实现原理：
 * - 将 record_id 添加到 warning_records_ 向量
 * - process_record() 会检查此向量
 * 
 * 与错误的区别：
 * - 警告记录仍会计入已处理计数
 * - 错误记录不会计入已处理计数
 */
void DataProcessor::simulate_warning_on_record(size_t record_id) {
    warning_records_.push_back(record_id);
}

/**
 * @brief 测试数据处理功能
 * 
 * 用于验证数据模块基本功能的测试函数。
 * 执行流程：
 * 1. 创建数据处理器实例
 * 2. 加载测试数据集（100 条记录）
 * 3. 标记记录 42 为错误
 * 4. 标记记录 75 为警告
 * 5. 处理所有记录
 * 
 * 此函数用于快速验证模块是否正常工作。
 * 测试场景：
 * - 正常记录：1-41, 43-74, 76-100（共 98 条）
 * - 错误记录：42（1 条，不计入已处理计数）
 * - 警告记录：75（1 条，计入已处理计数）
 * - 预期已处理计数：99 条
 */
void test_data_processing() {
    DataProcessor processor;
    processor.load_dataset("test_dataset", 100);
    processor.simulate_error_on_record(42);
    processor.simulate_warning_on_record(75);
    processor.process_all_records();
}

/**
 * @brief 获取模块 ID
 * @return int 模块唯一标识符 (3)
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
