#pragma once

#include "spdlog/common.h"
#include "spdlog/details/os.h"
#include <cstddef>
#include <string>
#include <fstream>
#include <vector>

/**
 * @brief 测试工具函数命名空间
 * 
 * 提供多模块日志测试所需的通用工具函数，包括：
 * - 跨平台路径处理
 * - 日志目录准备
 * - 文件内容读取和分析
 * - 文件名类型转换
 */
namespace test_utils {

/**
 * @brief 跨平台路径拼接函数
 * @param dir 目录路径
 * @param filename 文件名
 * @return spdlog::filename_t 拼接后的完整路径
 * 
 * 自动处理不同操作系统的路径分隔符差异：
 * - Windows: 使用反斜杠 '\'
 * - Unix/Linux/macOS: 使用正斜杠 '/'
 * 
 * 同时支持 SPDLOG_WCHAR_FILENAMES 宏定义的宽字符文件名。
 * 如果目录路径为空，直接返回文件名。
 */
inline spdlog::filename_t path_join(const spdlog::filename_t& dir, const spdlog::filename_t& filename) {
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

/**
 * @brief 将 spdlog::filename_t 转换为 std::string
 * @param filename 文件名（可能是宽字符）
 * @return std::string UTF-8 编码的字符串
 * 
 * 这是一个重要的跨平台转换函数：
 * - 在 Windows 上，如果定义了 SPDLOG_WCHAR_FILENAMES，
 *   spdlog::filename_t 是 std::wstring，需要转换为 UTF-8
 * - 在其他平台或未定义该宏时，spdlog::filename_t 就是 std::string
 * 
 * 使用 spdlog 提供的 os::filename_to_str() 函数进行转换，
 * 确保与 spdlog 内部行为一致。
 */
inline std::string filename_to_string(const spdlog::filename_t& filename) {
    return spdlog::details::os::filename_to_str(filename);
}

/**
 * @brief 准备测试日志目录
 * @param dir_name 目录名称，默认为 "test_logs"
 * 
 * 执行以下操作：
 * 1. 清除所有已注册的 spdlog 日志器（避免测试间干扰）
 * 2. 删除已存在的日志目录（跨平台）
 * 3. 创建新的空日志目录
 * 
 * 跨平台实现：
 * - Windows: 使用 rmdir 和 mkdir 命令
 * - Unix/Linux/macOS: 使用 rm -rf 和 mkdir -p 命令
 * 
 * 注意：此函数在测试开始时调用，确保每次测试都从干净状态开始。
 */
void prepare_logdir(const std::string& dir_name = "test_logs");

/**
 * @brief 读取文件全部内容
 * @param filename 文件路径
 * @return std::string 文件内容
 * 
 * 以二进制模式打开文件，读取所有内容到字符串中。
 * 如果文件无法打开，抛出 std::runtime_error 异常。
 * 
 * 用于验证日志文件内容是否符合预期。
 */
std::string file_contents(const std::string& filename);

/**
 * @brief 计算文件行数
 * @param filename 文件路径
 * @return std::size_t 行数
 * 
 * 逐行读取文件并计数。用于验证日志消息数量是否正确。
 * 如果文件无法打开，抛出 std::runtime_error 异常。
 */
std::size_t count_lines(const std::string& filename);

/**
 * @brief 统计文件中指定模式出现的次数
 * @param filename 文件路径
 * @param pattern 要搜索的模式字符串
 * @return std::size_t 出现次数
 * 
 * 读取整个文件内容，使用字符串查找统计模式出现次数。
 * 用于验证特定日志消息或模块标识是否正确写入。
 * 
 * 示例：
 * @code
 * // 验证 core 模块的日志是否写入正确的文件
 * size_t count = count_occurrences("core.log", "[Core]");
 * REQUIRE(count == expected_count);
 * @endcode
 */
std::size_t count_occurrences(const std::string& filename, const std::string& pattern);

/**
 * @brief 检查文件是否包含指定模式
 * @param filename 文件路径
 * @param pattern 要搜索的模式字符串
 * @return true 文件包含模式
 * @return false 文件不包含模式
 * 
 * 基于 count_occurrences() 的便捷函数。
 * 用于验证日志隔离：确认某模块的日志不会出现在其他模块的文件中。
 */
bool file_contains(const std::string& filename, const std::string& pattern);

/**
 * @brief 检查文件是否不包含指定模式
 * @param filename 文件路径
 * @param pattern 要搜索的模式字符串
 * @return true 文件不包含模式
 * @return false 文件包含模式
 * 
 * 基于 count_occurrences() 的便捷函数。
 * 这是日志隔离测试的核心断言函数。
 * 
 * 示例：
 * @code
 * // 验证 network 模块的日志不会出现在 core.log 中
 * REQUIRE(file_does_not_contain("core.log", "[Network]"));
 * @endcode
 */
bool file_does_not_contain(const std::string& filename, const std::string& pattern);

/**
 * @brief 读取文件所有行
 * @param filename 文件路径
 * @return std::vector<std::string> 每行内容的向量
 * 
 * 逐行读取文件，存储到字符串向量中。
 * 用于需要逐行分析日志内容的复杂测试场景。
 * 如果文件无法打开，抛出 std::runtime_error 异常。
 */
std::vector<std::string> read_all_lines(const std::string& filename);

}
