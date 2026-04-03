#include "business_logger.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        BusinessLogger& logger = BusinessLogger::instance();
        
        logger.init("business_logs");
        
        std::cout << "业务日志系统已初始化" << std::endl;
        std::cout << "业务类型包括：" << std::endl;
        std::cout << "  - 录制屏幕 (ScreenRecording)" << std::endl;
        std::cout << "  - 打开桌面 (DesktopOpen)" << std::endl;
        std::cout << "  - 录制键盘 (KeyboardRecording)" << std::endl;
        std::cout << "  - 录制声音 (AudioRecording)" << std::endl;
        std::cout << std::endl;
        
        logger.info(BusinessType::ScreenRecording, "开始录制屏幕，分辨率: 1920x1080");
        logger.info(BusinessType::DesktopOpen, "用户打开桌面");
        logger.info(BusinessType::KeyboardRecording, "开始录制键盘输入");
        logger.info(BusinessType::AudioRecording, "开始录制声音，采样率: 44100Hz");
        
        logger.debug(BusinessType::ScreenRecording, "第1帧已捕获");
        logger.debug(BusinessType::ScreenRecording, "第2帧已捕获");
        logger.debug(BusinessType::ScreenRecording, "第3帧已捕获");
        
        logger.info(BusinessType::DesktopOpen, "用户点击了开始菜单");
        
        logger.warn(BusinessType::KeyboardRecording, "检测到快速连续按键");
        
        logger.error(BusinessType::AudioRecording, "音频缓冲区已满");
        
        logger.critical(BusinessType::ScreenRecording, "录制失败，磁盘空间不足");
        
        std::cout << std::endl;
        std::cout << "日志已写入到 business_logs/ 目录" << std::endl;
        std::cout << "每个业务最多保留3个日志文件，单个文件不超过30MB" << std::endl;
        
        spdlog::shutdown();
        
    } catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
    
    return 0;
}
