
#include "business_logger.h"

int main() {
    try {
        BusinessLogger& logger = BusinessLogger::instance();
        logger.init("business_logs");

        logger.info(BusinessType::ScreenRecording, "开始录制屏幕，分辨率: {}x{}", 1920, 1080);
        logger.debug(BusinessType::ScreenRecording, "帧率设置为: {} fps", 30);
        
        logger.info(BusinessType::DesktopOpen, "用户打开桌面，当前时间: {}", "2026-03-06 14:30:00");
        logger.warn(BusinessType::DesktopOpen, "桌面加载时间过长，耗时: {} ms", 2500);
        
        logger.info(BusinessType::KeyboardRecording, "开始录制键盘输入");
        logger.debug(BusinessType::KeyboardRecording, "按下键: Ctrl+C");
        
        logger.info(BusinessType::AudioRecording, "开始录制音频，采样率: {} Hz", 44100);
        logger.error(BusinessType::AudioRecording, "音频设备初始化失败: {}", "设备未找到");
        logger.critical(BusinessType::AudioRecording, "系统音频服务异常！");

        logger.shutdown();
        
        return 0;
    } catch (const spdlog::spdlog_ex& ex) {
        printf("日志初始化失败: %s\n", ex.what());
        return 1;
    }
}
