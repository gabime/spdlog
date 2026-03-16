#include "spdlog/business_logger.h"

int main() {
    using namespace spdlog::business;
    
    // 示例1：录制屏幕业务日志
    BusinessLogger::info(BusinessType::SCREEN_RECORD, "开始录制屏幕，分辨率：1920x1080");
    BusinessLogger::debug(BusinessType::SCREEN_RECORD, "录制帧率：30fps");
    BusinessLogger::warn(BusinessType::SCREEN_RECORD, "磁盘空间不足，剩余：10GB");
    
    // 示例2：打开桌面业务日志
    BusinessLogger::info(BusinessType::DESKTOP_OPEN, "用户打开桌面，时间：2024-01-01 10:00:00");
    BusinessLogger::error(BusinessType::DESKTOP_OPEN, "打开桌面失败，错误码：0x80004005");
    
    // 示例3：录制键盘业务日志
    BusinessLogger::info(BusinessType::KEYBOARD_RECORD, "开始录制键盘输入");
    BusinessLogger::debug(BusinessType::KEYBOARD_RECORD, "捕获按键：Ctrl+C");
    
    // 示例4：录制声音业务日志
    BusinessLogger::info(BusinessType::AUDIO_RECORD, "开始录制声音，采样率：44100Hz");
    BusinessLogger::critical(BusinessType::AUDIO_RECORD, "录音设备断开连接");
    
    // 也可以直接获取日志器进行更复杂的操作
    auto screen_logger = BusinessLogger::instance().get_logger(BusinessType::SCREEN_RECORD);
    if (screen_logger) {
        screen_logger->info("录制完成，文件大小：15MB");
    }
    
    return 0;
}