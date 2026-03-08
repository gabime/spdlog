//
// 多业务日志简洁示例
// 演示如何使用 BusinessLogger 快速实现按业务分类的日志记录
//

#include <spdlog/business_logger.h>
#include <iostream>

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "   多业务日志简洁使用示例" << std::endl;
        std::cout << "========================================" << std::endl;

        // ========================================
        // 方式1: 使用 BusinessLogger 类
        // ========================================
        std::cout << "\n--- 方式1: 使用 BusinessLogger 类 ---" << std::endl;
        
        // 创建业务日志管理器
        // 参数: 日志目录, 单个文件最大大小(30MB), 保留文件数(3个), 是否输出到控制台
        spdlog::BusinessLogger biz_logger("logs/myapp", 30 * 1024 * 1024, 3, true);
        
        // 直接按业务类型记录日志
        biz_logger.screen_info("开始录制屏幕，分辨率: {}x{}", 1920, 1080);
        biz_logger.screen_debug("帧率设置为: {} fps", 30);
        biz_logger.screen_warn("检测到性能下降");
        
        biz_logger.desktop_info("打开桌面目录: {}", "C:\\Users\\Admin\\Desktop");
        biz_logger.desktop_debug("扫描到 {} 个文件", 25);
        
        biz_logger.keyboard_info("开始记录键盘输入");
        biz_logger.keyboard_debug("捕获按键: {}", "Ctrl+C");
        
        biz_logger.audio_info("开始录制音频");
        biz_logger.audio_info("采样率: {} Hz, 声道: {}", 44100, "立体声");
        biz_logger.audio_error("音频设备初始化失败: {}", "设备未找到");
        
        // 使用通用方法记录
        biz_logger.log(spdlog::BusinessType::ScreenRecord, spdlog::level::info, 
                       "通用方式记录屏幕日志");
        
        // 刷新日志
        biz_logger.flush();
        
        std::cout << "日志已写入 logs/myapp/ 目录" << std::endl;

        // ========================================
        // 方式2: 使用单例管理器 (全局访问)
        // ========================================
        std::cout << "\n--- 方式2: 使用全局单例管理器 ---" << std::endl;
        
        // 初始化全局日志管理器
        spdlog::business_log_manager().initialize("logs/global", 30 * 1024 * 1024, 3, true);
        
        // 在任何地方都可以直接记录日志
        spdlog::business_log_manager().screen_info("全局管理器 - 屏幕录制开始");
        spdlog::business_log_manager().keyboard_info("全局管理器 - 键盘记录开始");
        spdlog::business_log_manager().audio_info("全局管理器 - 音频录制开始");
        spdlog::business_log_manager().desktop_info("全局管理器 - 桌面操作开始");
        
        spdlog::business_log_manager().flush();
        
        std::cout << "日志已写入 logs/global/ 目录" << std::endl;

        // ========================================
        // 方式3: 获取特定业务的 logger 进行详细控制
        // ========================================
        std::cout << "\n--- 方式3: 获取特定业务的 logger ---" << std::endl;
        
        // 获取屏幕录制业务的 logger
        auto screen_lg = biz_logger.get_logger(spdlog::BusinessType::ScreenRecord);
        screen_lg->set_level(spdlog::level::debug);
        screen_lg->debug("使用 logger 对象记录调试信息");
        screen_lg->info("当前录制时长: {} 分钟", 5);
        screen_lg->warn("磁盘空间不足: 剩余 {}%", 10);
        
        // 获取音频录制业务的 logger
        auto audio_lg = biz_logger.get_logger(spdlog::BusinessType::AudioRecord);
        audio_lg->info("音频录制状态: {}", "运行中");
        audio_lg->error("音频缓冲区溢出");
        
        biz_logger.flush();

        // ========================================
        // 演示日志文件结构
        // ========================================
        std::cout << "\n========================================" << std::endl;
        std::cout << "   生成的日志文件结构" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << R"(
logs/
├── myapp/
│   ├── screen_record.log      (当前屏幕录制日志)
│   ├── screen_record.1.log    (轮转备份1)
│   ├── screen_record.2.log    (轮转备份2)
│   ├── screen_record.3.log    (轮转备份3)
│   ├── desktop_open.log       (当前桌面操作日志)
│   ├── desktop_open.1.log     (轮转备份1)
│   ├── desktop_open.2.log     (轮转备份2)
│   ├── keyboard_record.log    (当前键盘记录日志)
│   ├── keyboard_record.1.log  (轮转备份1)
│   ├── audio_record.log       (当前音频录制日志)
│   └── audio_record.1.log     (轮转备份1)
│
└── global/
    ├── screen_record.log
    ├── desktop_open.log
    ├── keyboard_record.log
    └── audio_record.log

说明:
- 每个业务有独立的日志文件
- 每个日志文件最大 30MB
- 最多保留 3 个备份文件
- 超过大小限制时自动轮转
        )" << std::endl;

        std::cout << "\n示例运行完成!" << std::endl;
        
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
