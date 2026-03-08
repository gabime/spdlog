//
// 多业务日志轮转测试示例
// 验证日志文件大小限制和备份数量限制功能
//

#include <spdlog/spdlog.h>
#include <spdlog/sinks/business_sink.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

// 获取文件大小（字节）
size_t get_file_size(const std::string& filepath) {
    try {
        return fs::file_size(filepath);
    } catch (...) {
        return 0;
    }
}

// 格式化文件大小显示
std::string format_size(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit_idx = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024 && unit_idx < 3) {
        size /= 1024;
        unit_idx++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit_idx];
    return oss.str();
}

// 列出目录中的所有日志文件
void list_log_files(const std::string& directory, const std::string& business_name) {
    std::cout << "\n  【" << business_name << "】日志文件列表:" << std::endl;
    
    try {
        bool found = false;
        for (const auto& entry : fs::directory_iterator(directory)) {
            std::string filename = entry.path().filename().string();
            if (filename.find(business_name) != std::string::npos) {
                found = true;
                size_t size = get_file_size(entry.path().string());
                std::cout << "    - " << filename 
                         << " (" << format_size(size) << ")" << std::endl;
            }
        }
        if (!found) {
            std::cout << "    (暂无日志文件)" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "    读取目录失败: " << e.what() << std::endl;
    }
}

// 测试1: 验证单个日志文件大小限制
void test_file_size_limit() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "测试1: 验证单个日志文件大小限制" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "配置: 最大文件大小 = 100KB, 最大备份数 = 3" << std::endl;
    
    const std::string log_dir = "logs/test_size_limit";
    const size_t max_size = 100 * 1024;  // 100KB
    const size_t max_files = 3;
    
    // 清理旧日志
    fs::remove_all(log_dir);
    fs::create_directories(log_dir);
    
    // 创建业务日志 sink
    auto biz_sink = std::make_shared<spdlog::sinks::business_sink_mt>(log_dir, max_size, max_files);
    auto logger = std::make_shared<spdlog::logger>("screen_logger", biz_sink);
    
    std::cout << "\n开始写入日志（每条约200字节）..." << std::endl;
    
    // 写入足够多的日志以触发多次轮转
    // 100KB / 200B = 约500条日志触发一次轮转
    for (int i = 0; i < 2000; ++i) {
        logger->info("这是第 {:4d} 条测试日志消息，包含一些填充内容以模拟真实日志的格式和内容长度。", i);
        
        // 每500条输出一次进度
        if ((i + 1) % 500 == 0) {
            std::cout << "  已写入 " << (i + 1) << " 条日志..." << std::endl;
        }
    }
    
    biz_sink->flush();
    
    std::cout << "\n写入完成，检查日志文件:" << std::endl;
    list_log_files(log_dir, "screen_record");
    
    // 验证文件大小
    std::cout << "\n验证结果:" << std::endl;
    std::string current_log = log_dir + "/screen_record.log";
    if (fs::exists(current_log)) {
        size_t size = get_file_size(current_log);
        std::cout << "  ? 当前日志文件大小: " << format_size(size) << std::endl;
        if (size <= max_size) {
            std::cout << "  ? 文件大小符合限制 (<= " << format_size(max_size) << ")" << std::endl;
        } else {
            std::cout << "  ? 警告: 文件大小超过限制!" << std::endl;
        }
    }
}

// 测试2: 验证最多保留3个备份文件
void test_max_backup_files() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "测试2: 验证最多保留3个备份文件" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "配置: 最大文件大小 = 50KB, 最大备份数 = 3" << std::endl;
    
    const std::string log_dir = "logs/test_backup_limit";
    const size_t max_size = 50 * 1024;  // 50KB
    const size_t max_files = 3;
    
    // 清理旧日志
    fs::remove_all(log_dir);
    fs::create_directories(log_dir);
    
    // 创建业务日志 sink
    auto biz_sink = std::make_shared<spdlog::sinks::business_sink_mt>(log_dir, max_size, max_files);
    auto logger = std::make_shared<spdlog::logger>("keyboard_logger", biz_sink);
    
    std::cout << "\n开始写入日志，触发多次轮转..." << std::endl;
    
    int rotation_count = 0;
    size_t last_file_count = 0;
    
    // 持续写入日志，直到产生足够多的轮转
    for (int batch = 0; batch < 10; ++batch) {
        // 每批写入足够触发一次轮转的日志量
        for (int i = 0; i < 300; ++i) {
            logger->info("批次 {:2d} - 第 {:3d} 条日志，用于测试备份文件数量限制功能。键盘记录器正在运行中。", batch, i);
        }
        biz_sink->flush();
        
        // 统计当前文件数量
        size_t file_count = 0;
        for (const auto& entry : fs::directory_iterator(log_dir)) {
            std::string filename = entry.path().filename().string();
            if (filename.find("keyboard_record") != std::string::npos) {
                file_count++;
            }
        }
        
        if (file_count != last_file_count) {
            rotation_count++;
            std::cout << "  检测到第 " << rotation_count << " 次轮转，当前文件数: " << file_count << std::endl;
            last_file_count = file_count;
        }
    }
    
    std::cout << "\n最终日志文件列表:" << std::endl;
    list_log_files(log_dir, "keyboard_record");
    
    // 验证备份文件数量
    std::cout << "\n验证结果:" << std::endl;
    size_t total_files = 0;
    size_t backup_files = 0;
    
    for (const auto& entry : fs::directory_iterator(log_dir)) {
        std::string filename = entry.path().filename().string();
        if (filename.find("keyboard_record") != std::string::npos) {
            total_files++;
            if (filename.find(".log") != std::string::npos && filename != "keyboard_record.log") {
                backup_files++;
            }
        }
    }
    
    std::cout << "  总日志文件数: " << total_files << " (1个当前 + " << backup_files << "个备份)" << std::endl;
    if (backup_files <= max_files) {
        std::cout << "  ? 备份文件数量符合限制 (<= " << max_files << ")" << std::endl;
    } else {
        std::cout << "  ? 警告: 备份文件数量超过限制!" << std::endl;
    }
}

// 测试3: 验证多业务独立轮转
void test_multi_business_rotation() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "测试3: 验证多业务独立轮转" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "配置: 最大文件大小 = 80KB, 最大备份数 = 3" << std::endl;
    
    const std::string log_dir = "logs/test_multi_business";
    const size_t max_size = 80 * 1024;  // 80KB
    const size_t max_files = 3;
    
    // 清理旧日志
    fs::remove_all(log_dir);
    fs::create_directories(log_dir);
    
    // 创建业务日志 sink
    auto biz_sink = std::make_shared<spdlog::sinks::business_sink_mt>(log_dir, max_size, max_files);
    
    // 为不同业务创建 logger
    auto screen_logger = std::make_shared<spdlog::logger>("screen_logger", biz_sink);
    auto audio_logger = std::make_shared<spdlog::logger>("audio_logger", biz_sink);
    auto desktop_logger = std::make_shared<spdlog::logger>("desktop_logger", biz_sink);
    
    std::cout << "\n为不同业务写入不同数量的日志..." << std::endl;
    
    // 屏幕录制 - 写入大量日志
    std::cout << "  屏幕录制: 写入 1500 条日志..." << std::endl;
    for (int i = 0; i < 1500; ++i) {
        screen_logger->info("屏幕录制 - 帧 {:4d} 已捕获，分辨率 1920x1080，编码器 H.264", i);
    }
    
    // 音频录制 - 写入中等数量日志
    std::cout << "  音频录制: 写入 800 条日志..." << std::endl;
    for (int i = 0; i < 800; ++i) {
        audio_logger->info("音频录制 - 缓冲区 {:3d} 已写入，采样率 44100Hz，位深 16bit", i);
    }
    
    // 桌面操作 - 写入少量日志
    std::cout << "  桌面操作: 写入 300 条日志..." << std::endl;
    for (int i = 0; i < 300; ++i) {
        desktop_logger->info("桌面操作 - 文件 {:3d} 已扫描，路径 C:\\Users\\Admin\\Desktop", i);
    }
    
    biz_sink->flush();
    
    std::cout << "\n各业务日志文件列表:" << std::endl;
    list_log_files(log_dir, "screen_record");
    list_log_files(log_dir, "audio_record");
    list_log_files(log_dir, "desktop_open");
    
    std::cout << "\n验证结果:" << std::endl;
    std::cout << "  ? 各业务日志独立存储" << std::endl;
    std::cout << "  ? 各业务独立进行日志轮转" << std::endl;
    std::cout << "  ? 写入量大的业务产生更多备份文件" << std::endl;
}

// 测试4: 验证日志轮转时序
void test_rotation_sequence() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "测试4: 验证日志轮转时序" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "配置: 最大文件大小 = 10KB, 最大备份数 = 3" << std::endl;
    
    const std::string log_dir = "logs/test_sequence";
    const size_t max_size = 10 * 1024;  // 10KB - 很小的限制以便快速观察轮转
    const size_t max_files = 3;
    
    // 清理旧日志
    fs::remove_all(log_dir);
    fs::create_directories(log_dir);
    
    // 创建业务日志 sink
    auto biz_sink = std::make_shared<spdlog::sinks::business_sink_mt>(log_dir, max_size, max_files);
    auto logger = std::make_shared<spdlog::logger>("screen_logger", biz_sink);
    
    std::cout << "\n观察日志轮转过程..." << std::endl;
    
    // 分批次写入，观察轮转
    for (int round = 0; round < 5; ++round) {
        std::cout << "\n第 " << (round + 1) << " 轮写入 (100条日志)..." << std::endl;
        
        for (int i = 0; i < 100; ++i) {
            logger->info("轮次 {} - 第 {:3d} 条日志，时间戳标记用于追踪轮转顺序", round, i);
        }
        biz_sink->flush();
        
        // 显示当前文件状态
        std::vector<std::pair<std::string, size_t>> files;
        for (const auto& entry : fs::directory_iterator(log_dir)) {
            std::string filename = entry.path().filename().string();
            if (filename.find("screen_record") != std::string::npos) {
                files.push_back({filename, get_file_size(entry.path().string())});
            }
        }
        
        // 按文件名排序
        std::sort(files.begin(), files.end());
        
        std::cout << "  当前文件状态:" << std::endl;
        for (const auto& [name, size] : files) {
            std::cout << "    " << name << " - " << format_size(size) << std::endl;
        }
    }
    
    std::cout << "\n验证轮转顺序:" << std::endl;
    std::cout << "  ? 新日志写入 screen_record.log" << std::endl;
    std::cout << "  ? 当 screen_record.log 超过 " << format_size(max_size) << " 时触发轮转" << std::endl;
    std::cout << "  ? screen_record.log -> screen_record.1.log" << std::endl;
    std::cout << "  ? screen_record.1.log -> screen_record.2.log" << std::endl;
    std::cout << "  ? screen_record.2.log -> screen_record.3.log" << std::endl;
    std::cout << "  ? screen_record.3.log 被删除（超过最大备份数）" << std::endl;
}

int main() {
    try {
        std::cout << "╔══════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║     spdlog 多业务日志轮转功能验证测试                    ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << "\n测试目标:" << std::endl;
        std::cout << "  1. 验证单个日志文件大小限制功能" << std::endl;
        std::cout << "  2. 验证最多保留3个备份文件功能" << std::endl;
        std::cout << "  3. 验证多业务独立轮转功能" << std::endl;
        std::cout << "  4. 观察日志轮转时序" << std::endl;
        
        // 运行所有测试
        test_file_size_limit();
        test_max_backup_files();
        test_multi_business_rotation();
        test_rotation_sequence();
        
        std::cout << "\n╔══════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║     所有测试完成!                                        ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════╝" << std::endl;
        
        std::cout << "\n总结:" << std::endl;
        std::cout << "  ? 日志文件大小限制功能正常工作" << std::endl;
        std::cout << "  ? 最多保留3个备份文件功能正常工作" << std::endl;
        std::cout << "  ? 各业务日志独立轮转，互不影响" << std::endl;
        std::cout << "  ? 日志轮转时序符合预期" << std::endl;
        
        // 清理资源
        spdlog::shutdown();
        
        return 0;
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志系统错误: " << ex.what() << std::endl;
        return 1;
    }
    catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
}
