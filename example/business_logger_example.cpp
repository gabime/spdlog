#include <spdlog/business_logger.h>
#include <iostream>

int main() {
    try {
        auto& logger = spdlog::BusinessLogger::instance();
        logger.init("./business_logs");

        std::cout << "=== 业务日志示例开始 ===" << std::endl;

        logger.log_info(spdlog::BusinessType::RECORD_SCREEN, "开始录制屏幕，分辨率: 1920x1080");
        logger.log_info(spdlog::BusinessType::RECORD_SCREEN, "设置帧率为 30 FPS");

        logger.log_info(spdlog::BusinessType::OPEN_DESKTOP, "正在连接桌面服务");
        logger.log_warn(spdlog::BusinessType::OPEN_DESKTOP, "连接超时，正在重试...");

        logger.log_debug(spdlog::BusinessType::RECORD_KEYBOARD, "检测到按键事件");
        logger.log_info(spdlog::BusinessType::RECORD_KEYBOARD, "已记录 156 个按键");

        logger.log_info(spdlog::BusinessType::RECORD_AUDIO, "开始音频录制，采样率: 44100 Hz");
        logger.log_error(spdlog::BusinessType::RECORD_AUDIO, "音频缓冲区溢出！");

        BUSINESS_LOG_INFO(spdlog::BusinessType::RECORD_SCREEN, "使用宏记录日志 - 录制进度: {}%", 75);
        BUSINESS_LOG_WARN(spdlog::BusinessType::RECORD_KEYBOARD, "使用宏警告 - 按键频率过高: {} 次/秒", 120);

        std::cout << "=== 业务日志示例结束 ===" << std::endl;
        std::cout << "日志文件已保存到 ./business_logs/ 目录下" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
