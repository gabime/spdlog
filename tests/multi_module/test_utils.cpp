#include "test_utils.h"
#include "spdlog/spdlog.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>

// 跨平台头文件包含
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

namespace test_utils {

/**
 * @brief 准备测试日志目录
 * @param dir_name 目录名称
 * 
 * 这是每个测试用例开始时调用的关键函数，确保测试环境的独立性。
 * 
 * 执行流程：
 * 1. 调用 spdlog::drop_all() 清除所有已注册的日志器
 *    - 防止前一个测试的日志器影响当前测试
 *    - 确保每个测试都从干净的注册表状态开始
 * 
 * 2. 删除已存在的日志目录
 *    - Windows: 使用 rmdir /S /Q 递归删除目录
 *    - Unix/Linux/macOS: 使用 rm -rf 强制删除
 * 
 * 3. 创建新的空目录
 *    - Windows: 使用 mkdir
 *    - Unix/Linux/macOS: 使用 mkdir -p（创建父目录）
 * 
 * 跨平台注意事项：
 * - Windows 的 rmdir 命令在目录不存在时会返回错误，使用 2>nul 抑制错误输出
 * - Unix/Linux/macOS 的 mkdir -p 可以创建多级目录，且目录已存在时不会报错
 */
void prepare_logdir(const std::string& dir_name) {
    // 清除所有已注册的日志器，避免测试间干扰
    spdlog::drop_all();
    
#ifdef _WIN32
    // Windows 平台：使用 cmd 命令
    // rmdir /S /Q: 递归删除目录树，安静模式（不询问确认）
    // 2>nul: 重定向错误输出到 nul，避免目录不存在时的错误提示
    std::string cmd = "rmdir /S /Q " + dir_name + " 2>nul";
    system(cmd.c_str());
    
    // 创建新目录
    cmd = "mkdir " + dir_name;
    system(cmd.c_str());
#else
    // Unix/Linux/macOS 平台：使用 shell 命令
    // rm -rf: 强制递归删除，忽略不存在的文件
    std::string cmd = "rm -rf " + dir_name;
    auto rv = system(cmd.c_str());
    (void)rv;  // 显式忽略返回值，避免编译器警告
    
    // mkdir -p: 创建目录，包括所有不存在的父目录
    // 如果目录已存在，不会报错
    cmd = "mkdir -p " + dir_name;
    rv = system(cmd.c_str());
    if (rv != 0) {
        throw std::runtime_error("Failed to create directory: " + dir_name);
    }
#endif
}

/**
 * @brief 读取文件全部内容
 * @param filename 文件路径
 * @return std::string 文件内容
 * 
 * 以二进制模式打开文件，确保：
 * - 不会进行换行符转换（Windows 的 \r\n 不会被转换为 \n）
 * - 可以正确读取包含特殊字符的文件
 * 
 * 使用 std::istreambuf_iterator 高效读取整个文件内容。
 * 如果文件无法打开（不存在、权限不足等），抛出 std::runtime_error 异常。
 * 
 * 注意：此函数适用于读取日志文件等相对较小的文件。
 * 对于大文件，应该使用逐行读取或内存映射。
 */
std::string file_contents(const std::string& filename) {
    // std::ios_base::binary: 二进制模式，不进行换行符转换
    std::ifstream ifs(filename, std::ios_base::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    // 使用 istreambuf_iterator 一次性读取整个文件
    // 这是读取文件到字符串的高效方式
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
}

/**
 * @brief 计算文件行数
 * @param filename 文件路径
 * @return std::size_t 行数
 * 
 * 逐行读取文件并计数。使用 std::getline 读取每一行。
 * 
 * 注意事项：
 * - 空行也会被计数
 * - 文件最后一行如果没有换行符，仍会被计数
 * - 如果文件无法打开，抛出 std::runtime_error 异常
 * 
 * 用于验证：
 * - 日志消息数量是否正确
 * - 每个模块是否写入了预期数量的日志
 */
std::size_t count_lines(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::string line;
    size_t counter = 0;
    
    // 逐行读取，直到 EOF
    while (std::getline(ifs, line)) {
        counter++;
    }
    
    return counter;
}

/**
 * @brief 统计文件中指定模式出现的次数
 * @param filename 文件路径
 * @param pattern 要搜索的模式字符串
 * @return std::size_t 出现次数
 * 
 * 实现原理：
 * 1. 先调用 file_contents() 读取整个文件内容
 * 2. 使用 std::string::find() 循环查找模式
 * 3. 每次找到后，从下一个位置继续查找
 * 
 * 注意：
 * - 这是简单的字符串匹配，不是正则表达式
 * - 匹配是区分大小写的
 * - 重叠匹配会被计数（例如 "aaa" 中查找 "aa" 会匹配 2 次）
 * 
 * 在日志隔离测试中的典型用法：
 * - 统计 "[Core]" 出现次数，验证 core 模块的日志数量
 * - 统计 "[Network]" 出现次数，验证 network 模块的日志数量
 */
std::size_t count_occurrences(const std::string& filename, const std::string& pattern) {
    // 读取整个文件内容
    std::string content = file_contents(filename);
    size_t count = 0;
    size_t pos = 0;
    
    // 循环查找所有匹配项
    while ((pos = content.find(pattern, pos)) != std::string::npos) {
        count++;
        // 移动到下一个位置，避免无限循环
        // 注意：这样会导致重叠匹配被计数
        pos += pattern.length();
    }
    
    return count;
}

/**
 * @brief 检查文件是否包含指定模式
 * @param filename 文件路径
 * @param pattern 要搜索的模式字符串
 * @return true 文件包含至少一个匹配项
 * @return false 文件不包含匹配项
 * 
 * 基于 count_occurrences() 的便捷函数。
 * 当 count_occurrences() 返回值大于 0 时返回 true。
 * 
 * 使用场景：
 * - 验证某模块的日志是否写入了正确的文件
 * - 验证特定日志消息是否存在
 */
bool file_contains(const std::string& filename, const std::string& pattern) {
    return count_occurrences(filename, pattern) > 0;
}

/**
 * @brief 检查文件是否不包含指定模式
 * @param filename 文件路径
 * @param pattern 要搜索的模式字符串
 * @return true 文件不包含任何匹配项
 * @return false 文件包含至少一个匹配项
 * 
 * 基于 count_occurrences() 的便捷函数。
 * 当 count_occurrences() 返回值等于 0 时返回 true。
 * 
 * 这是日志隔离测试中最重要的断言函数之一。
 * 
 * 典型用法：
 * @code
 * // 验证 network 模块的日志不会出现在 core.log 中
 * REQUIRE(file_does_not_contain("core.log", "[Network]"));
 * 
 * // 验证 data 模块的日志不会出现在 network.log 中
 * REQUIRE(file_does_not_contain("network.log", "[Data]"));
 * @endcode
 */
bool file_does_not_contain(const std::string& filename, const std::string& pattern) {
    return count_occurrences(filename, pattern) == 0;
}

/**
 * @brief 读取文件所有行
 * @param filename 文件路径
 * @return std::vector<std::string> 每行内容的向量
 * 
 * 逐行读取文件，将每行内容存储到字符串向量中。
 * 与 count_lines() 不同，此函数还保存了每行的实际内容。
 * 
 * 使用场景：
 * - 需要逐行分析日志内容的复杂测试
 * - 验证日志格式是否正确
 * - 检查特定行的内容
 * 
 * 注意：
 * - 换行符不会包含在返回的字符串中
 * - 空行会被存储为空字符串
 * - 如果文件无法打开，抛出 std::runtime_error 异常
 */
std::vector<std::string> read_all_lines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream ifs(filename);
    
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::string line;
    while (std::getline(ifs, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

}
